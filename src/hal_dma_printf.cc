/**
 * @file hal_dma_printf.cc
 * @brief Implementation of DMA-based printf/scanf for STM32 HAL UART
 * @version 1.0.0
 * @date 2025-12-30
 */

#include "hal_dma_printf/hal_dma_printf.h"

#include <cstdio>
#include <cstring>

#include "usart.h"

// Default buffer size (can be overridden by compiler flag)
#ifndef HAL_DMA_PRINTF_BUFFER_SIZE
#define HAL_DMA_PRINTF_BUFFER_SIZE 1024
#endif

namespace {

// Internal state (anonymous namespace for encapsulation)
UART_HandleTypeDef* g_huart = nullptr;
uint8_t g_tx_buffer[HAL_DMA_PRINTF_BUFFER_SIZE];
uint8_t g_rx_buffer[HAL_DMA_PRINTF_BUFFER_SIZE];
volatile int g_tx_read_idx = 0;
volatile int g_tx_write_idx = 0;
volatile int g_rx_read_idx = 0;
bool g_enable_echo = false;


/**
 * @brief Calculate available data in TX buffer
 * @return Number of bytes available to transmit
 */
inline int GetTxAvailableBytes() {
  if (g_tx_write_idx >= g_tx_read_idx) {
    return g_tx_write_idx - g_tx_read_idx;
  }
  return HAL_DMA_PRINTF_BUFFER_SIZE - g_tx_read_idx + g_tx_write_idx;
}

/**
 * @brief Start DMA transmission for pending data
 * @details Handles ring buffer wraparound by transmitting in two parts if needed
 */
void StartDmaTransmit() {
  if (g_tx_write_idx < g_tx_read_idx) {
    // Wraparound case: transmit from read position to end of buffer
    const int first_part_size = HAL_DMA_PRINTF_BUFFER_SIZE - g_tx_read_idx;
    HAL_UART_Transmit_DMA(g_huart, &g_tx_buffer[g_tx_read_idx], first_part_size);
    g_tx_read_idx = 0;
  } else {
    // Normal case: transmit from read to write position
    const int transmit_size = g_tx_write_idx - g_tx_read_idx;
    HAL_UART_Transmit_DMA(g_huart, &g_tx_buffer[g_tx_read_idx], transmit_size);
    g_tx_read_idx = g_tx_write_idx;
  }
}

/**
 * @brief DMA transmit complete callback
 * @param huart UART handle (unused in this implementation)
 */
void OnDmaTransmitComplete([[maybe_unused]] UART_HandleTypeDef* huart) {
  // If there's more data to send, start next transmission
  if (g_tx_read_idx != g_tx_write_idx) {
    StartDmaTransmit();
  }
}

/**
 * @brief Initialize UART handler and DMA for printf/scanf
 * @param huart Pointer to UART handle
 * @return Error code
 */
int SetupUartHandler(UART_HandleTypeDef* huart) {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
  const uint8_t error_msg[] =
      "[HalDmaPrintf] Error: USE_HAL_UART_REGISTER_CALLBACKS is disabled.\r\n"
      "Enable it in stm32f4xx_hal_conf.h\r\n";
  HAL_UART_Transmit(huart, error_msg, sizeof(error_msg) - 1, 100);
  return HAL_DMA_PRINTF_ERROR_NO_CALLBACK;
#endif

  if (huart == nullptr) {
    return HAL_DMA_PRINTF_ERROR_NULL_PTR;
  }
  
  if (huart->hdmatx == nullptr) {
    const uint8_t error_msg[] =
        "[HalDmaPrintf] Error: TX DMA not initialized for UART.\r\n"
        "Configure TX DMA in CubeMX.\r\n";
    HAL_UART_Transmit(huart, error_msg, sizeof(error_msg) - 1, 100);
    return HAL_DMA_PRINTF_ERROR_NO_DMA_TX;
  }
  
  if (huart->hdmarx == nullptr) {
    const uint8_t error_msg[] =
        "[HalDmaPrintf] Error: RX DMA not initialized for UART.\r\n"
        "Configure RX DMA in CubeMX.\r\n";
    HAL_UART_Transmit(huart, error_msg, sizeof(error_msg) - 1, 100);
    return HAL_DMA_PRINTF_ERROR_NO_DMA_RX;
  }

  // Initialize global state
  g_huart = huart;
  g_tx_read_idx = 0;
  g_tx_write_idx = 0;
  g_rx_read_idx = 0;

  // Register callbacks
  g_huart->TxCpltCallback = OnDmaTransmitComplete;
  g_huart->AbortTransmitCpltCallback = OnDmaTransmitComplete;
  
  // Start continuous DMA reception
  HAL_UART_Receive_DMA(g_huart, g_rx_buffer, HAL_DMA_PRINTF_BUFFER_SIZE);

  return HAL_DMA_PRINTF_OK;
}

}  // anonymous namespace

// ============================================================================
// Public C API Implementation
// ============================================================================

extern "C" int HalDmaPrintfSetup(UART_HandleTypeDef* huart, bool enable_echo) {
  const int result = SetupUartHandler(huart);
  if (result == HAL_DMA_PRINTF_OK) {
    // Disable buffering for immediate output
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    g_enable_echo = enable_echo;
  }
  return result;
}

extern "C" void HalDmaPrintfEnableEcho(void) {
  g_enable_echo = true;
}

extern "C" void HalDmaPrintfDisableEcho(void) {
  g_enable_echo = false;
}

extern "C" size_t HalDmaPrintfGetBufferSize(void) {
  return HAL_DMA_PRINTF_BUFFER_SIZE;
}


// ============================================================================
// Syscall hooks for printf/scanf and C++ streams
// ============================================================================

/**
 * @brief Write syscall hook for printf() and std::cout
 * @param file File descriptor (unused)
 * @param ptr Pointer to data to write
 * @param len Length of data
 * @return Number of bytes written
 */
extern "C" int _write([[maybe_unused]] int file, char* ptr, int len) {
  if (g_huart == nullptr || ptr == nullptr || len <= 0) {
    return 0;
  }

  const int space_at_end = HAL_DMA_PRINTF_BUFFER_SIZE - g_tx_write_idx;

  if (space_at_end >= len) {
    // Enough space without wraparound
    memcpy(&g_tx_buffer[g_tx_write_idx], ptr, len);
    g_tx_write_idx = (g_tx_write_idx + len) % HAL_DMA_PRINTF_BUFFER_SIZE;
  } else {
    // Need to wrap around
    memcpy(&g_tx_buffer[g_tx_write_idx], ptr, space_at_end);
    const int remaining = len - space_at_end;
    const int to_copy = (remaining > HAL_DMA_PRINTF_BUFFER_SIZE) 
                           ? HAL_DMA_PRINTF_BUFFER_SIZE 
                           : remaining;
    memcpy(g_tx_buffer, ptr + space_at_end, to_copy);
    g_tx_write_idx = to_copy;
  }

  // Trigger DMA transmission if UART is ready
  if (g_huart->gState == HAL_UART_STATE_READY) {
    StartDmaTransmit();
  }

  return len;
}

/**
 * @brief Read syscall hook for scanf() and std::cin
 * @param file File descriptor (unused)
 * @param ptr Pointer to buffer for received data
 * @param len Maximum number of bytes to read
 * @return Number of bytes read
 */
extern "C" int _read([[maybe_unused]] int file, char* ptr, int len) {
  if (g_huart == nullptr || ptr == nullptr || len <= 0) {
    return 0;
  }

  int rx_count = 0;
  
  // Get current DMA position
  const volatile int dma_write_idx = 
      HAL_DMA_PRINTF_BUFFER_SIZE - g_huart->hdmarx->Instance->NDTR;

  while (rx_count < len) {
    // Check if new data is available
    if (dma_write_idx != g_rx_read_idx) {
      char ch = static_cast<char>(g_rx_buffer[g_rx_read_idx]);
      g_rx_read_idx = (g_rx_read_idx + 1) % HAL_DMA_PRINTF_BUFFER_SIZE;

      // Handle line endings
      if (ch == '\n' || ch == '\r') {
        ptr[rx_count] = '\n';
        if (g_enable_echo) {
          _write(1, &ptr[rx_count], 1);
        }
        return rx_count + 1;
      }

      ptr[rx_count] = ch;
      if (g_enable_echo) {
        _write(1, &ptr[rx_count], 1);
      }
      ++rx_count;
    }
  }

  return rx_count;
}