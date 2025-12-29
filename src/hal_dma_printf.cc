#include "hal_dma_printf/hal_dma_printf.h"

#include <cstdio>
#include <cstring>

#include "usart.h"

#ifndef HAL_DMA_PRINTF_BUFFER_SIZE
#define HAL_DMA_PRINTF_BUFFER_SIZE 1024
#endif

namespace hal_dma_printf {

UART_HandleTypeDef* huart_ = nullptr;
uint8_t tx_buffer_[HAL_DMA_PRINTF_BUFFER_SIZE];
uint8_t rx_buffer_[HAL_DMA_PRINTF_BUFFER_SIZE];
size_t tx_read_itr_;
size_t tx_write_itr_;
size_t rx_read_itr_;
bool enable_echo_ = false;

void StartDmaTransmit() {
  if (tx_write_itr_ < tx_read_itr_) {
    size_t first_part_size = HAL_DMA_PRINTF_BUFFER_SIZE - tx_read_itr_;
    HAL_UART_Transmit_DMA(huart_, &tx_buffer_[tx_read_itr_], first_part_size);
    tx_read_itr_ = 0;
  } else {
    HAL_UART_Transmit_DMA(huart_, &tx_buffer_[tx_read_itr_],
                          tx_write_itr_ - tx_read_itr_);
    tx_read_itr_ = tx_write_itr_;
  }
}

void OnDmaTransmitComplete([[maybe_unused]] UART_HandleTypeDef* huart) {
  if (tx_read_itr_ != tx_write_itr_) { StartDmaTransmit(); }
}

void SetUartHandler(UART_HandleTypeDef* huart) {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
  uint8_t message[] =
      "[HalDmaPrintf] Error: USE_HAL_UART_REGISTER_CALLBACKS is 0.\r\n";
  HAL_UART_Transmit(huart_, message, sizeof(message), 100);
  return;
#endif

  if (huart == nullptr) {
    return;
  } else if (huart->hdmatx == nullptr) {
    uint8_t message[] =
        "[HalDmaPrintf] Error: TX DMA not initialized for UART.\r\n";
    HAL_UART_Transmit(huart_, message, sizeof(message), 100);
    return;
  } else if (huart->hdmarx == nullptr) {
    uint8_t message[] =
        "[HalDmaPrintf] Error: RX DMA not initialized for UART.\r\n";
    HAL_UART_Transmit(huart_, message, sizeof(message), 100);
    return;
  }

  huart_ = huart;
  tx_read_itr_ = 0;
  tx_write_itr_ = 0;
  rx_read_itr_ = 0;

  huart_->TxCpltCallback = OnDmaTransmitComplete;
  huart_->AbortTransmitCpltCallback = OnDmaTransmitComplete;
  HAL_UART_Receive_DMA(huart_, (uint8_t*)rx_buffer_,
                       HAL_DMA_PRINTF_BUFFER_SIZE);
}

}  // namespace hal_dma_printf

extern "C" void HalDmaPrintfSetup(UART_HandleTypeDef* huart, bool enable_echo) {
  hal_dma_printf::SetUartHandler(huart);
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  hal_dma_printf::enable_echo_ = enable_echo;
}

extern "C" void HalDmaPrintfEnableEcho() {
  hal_dma_printf::enable_echo_ = true;
}

extern "C" void HalDmaPrintfDisableEcho() {
  hal_dma_printf::enable_echo_ = false;
}

extern "C" int _write([[maybe_unused]] int file, char* ptr, int len) {
  using namespace hal_dma_printf;
  if (huart_ == nullptr) { return len; }

  if (HAL_DMA_PRINTF_BUFFER_SIZE - tx_write_itr_ >= static_cast<size_t>(len)) {
    memcpy(&tx_buffer_[tx_write_itr_], ptr, len);
    tx_write_itr_ += len;
  } else {
    size_t remaining_space = HAL_DMA_PRINTF_BUFFER_SIZE - tx_write_itr_;
    memcpy(&tx_buffer_[tx_write_itr_], ptr, remaining_space);
    size_t overflow_len = len - remaining_space;
    if (overflow_len > HAL_DMA_PRINTF_BUFFER_SIZE) {
      overflow_len = HAL_DMA_PRINTF_BUFFER_SIZE;
    }
    memcpy(tx_buffer_, ptr + remaining_space, overflow_len);
    tx_write_itr_ = overflow_len;
  }
  tx_write_itr_ %= HAL_DMA_PRINTF_BUFFER_SIZE;

  if (huart_->gState == HAL_UART_STATE_READY) { StartDmaTransmit(); }

  return len;
}

extern "C" int _read([[maybe_unused]] int file, char* ptr, int len) {
  using namespace hal_dma_printf;
  if (huart_ == nullptr) { return len; }

  int rx_counter = 0;
  while (rx_counter < len) {
    if (HAL_DMA_PRINTF_BUFFER_SIZE - huart_->hdmarx->Instance->NDTR !=
        rx_read_itr_) {
      ptr[rx_counter] = rx_buffer_[rx_read_itr_];
      ++rx_read_itr_;
      rx_read_itr_ %= HAL_DMA_PRINTF_BUFFER_SIZE;

      if ((ptr[rx_counter] == '\n' || ptr[rx_counter] == '\r')) {
        ptr[rx_counter] = '\n';
        if (enable_echo_) { _write(1, &ptr[rx_counter], 1); }
        return ++rx_counter;
      }

      if (enable_echo_) { _write(1, &ptr[rx_counter], 1); }
      ++rx_counter;
    }
  }
  return rx_counter;
}