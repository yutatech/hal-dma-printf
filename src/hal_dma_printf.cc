#include "hal_dma_printf/hal_dma_printf.h"

#ifndef HAL_DMA_PRINTF_BUFFER_SIZE
#define HAL_DMA_PRINTF_BUFFER_SIZE 1024
#endif

namespace hal_dma_printf {

UART_HandleTypeDef* huart_ = nullptr;
char tx_buffer_[HAL_DMA_PRINTF_BUFFER_SIZE];
size_t read_itr_;
size_t write_itr_;

void StartDmaTransmit() {
  if (write_itr_ < read_itr_) {
    size_t first_part_size = HAL_DMA_PRINTF_BUFFER_SIZE - read_itr_;
    HAL_UART_Transmit_DMA(huart_, (uint8_t*)&tx_buffer_[read_itr_],
                          first_part_size);
    read_itr_ = 0;
  } else {
    HAL_UART_Transmit_DMA(huart_, (uint8_t*)&tx_buffer_[read_itr_],
                          write_itr_ - read_itr_);
    read_itr_ = write_itr_;
  }
}

void OnDmaTransmitComplete([[maybe_unused]] UART_HandleTypeDef* huart) {
  if (read_itr_ != write_itr_) { StartDmaTransmit(); }
}

void SetUartHandler(UART_HandleTypeDef* huart) {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 0)
  uint8_t message[] =
      "[HalDmaPrintf] Error: USE_HAL_UART_REGISTER_CALLBACKS is 0.\r\n";
  HAL_UART_Transmit(&huart1, message, sizeof(message), 100);
  return;
#endif

  if (huart == nullptr) {
    return;
  } else if (huart->hdmatx == nullptr) {
    uint8_t message[] =
        "[HalDmaPrintf] Error: TX DMA not initialized for UART.\r\n";
    HAL_UART_Transmit(&huart1, message, sizeof(message), 100);
    return;
  }

  huart_ = huart;
  read_itr_ = 0;
  write_itr_ = 0;

  huart_->TxCpltCallback = OnDmaTransmitComplete;
  huart_->AbortTransmitCpltCallback = OnDmaTransmitComplete;
}


}  // namespace hal_dma_printf

extern "C" void HalDmaPrintfSetUartHandler(UART_HandleTypeDef* huart) {
  hal_dma_printf::SetUartHandler(huart);
}

extern "C" int _write([[maybe_unused]] int file, char* ptr, int len) {
  using namespace hal_dma_printf;
  if (huart_ == nullptr) { return len; }

  if (HAL_DMA_PRINTF_BUFFER_SIZE - write_itr_ >= static_cast<size_t>(len)) {
    memcpy(&tx_buffer_[write_itr_], ptr, len);
    write_itr_ += len;
  } else {
    size_t remaining_space = HAL_DMA_PRINTF_BUFFER_SIZE - write_itr_;
    memcpy(&tx_buffer_[write_itr_], ptr, remaining_space);
    size_t overflow_len = len - remaining_space;
    if (overflow_len > HAL_DMA_PRINTF_BUFFER_SIZE) {
      overflow_len = HAL_DMA_PRINTF_BUFFER_SIZE;
    }
    memcpy(tx_buffer_, ptr + remaining_space, overflow_len);
    write_itr_ = overflow_len;
  }
  write_itr_ %= HAL_DMA_PRINTF_BUFFER_SIZE;

  if (huart_->gState == HAL_UART_STATE_READY) { StartDmaTransmit(); }

  return len;
}