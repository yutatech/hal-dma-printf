#ifndef HAL_DMA_PRINTF_H
#define HAL_DMA_PRINTF_H

#include <stdbool.h>

#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

void HalDmaPrintfSetup(UART_HandleTypeDef* huart,
                                bool enable_echo);
void HalDmaPrintfEnableEcho();
void HalDmaPrintfDisableEcho();

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // HAL_DMA_PRINTF_H