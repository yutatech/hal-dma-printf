#ifndef HAL_DMA_PRINTF_H
#define HAL_DMA_PRINTF_H

#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

void HalDmaPrintfSetUartHandler(UART_HandleTypeDef* huart);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // HAL_DMA_PRINTF_H