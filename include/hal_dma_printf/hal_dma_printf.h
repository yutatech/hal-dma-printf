/**
 * @file hal_dma_printf.h
 * @brief DMA-based printf/scanf for STM32 HAL UART
 * @version 1.0.0
 * @date 2025-12-30
 *
 * @details
 * This library enables printf(), scanf(), std::cout, and std::cin functionality
 * over UART using DMA (Direct Memory Access) with ring buffers for efficient,
 * non-blocking I/O operations on STM32 microcontrollers.
 *
 * Key Features:
 * - Non-blocking I/O using DMA
 * - Ring buffer implementation for transmit and receive
 * - Support for C standard I/O (printf/scanf)
 * - Support for C++ streams (std::cout/std::cin)
 * - Configurable echo mode for input
 *
 * @note Requires USE_HAL_UART_REGISTER_CALLBACKS=1 in STM32 HAL configuration
 */

#ifndef HAL_DMA_PRINTF_H
#define HAL_DMA_PRINTF_H

#include <stdbool.h>
#include <stdint.h>

#include "usart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup HAL_DMA_PRINTF_Error_Codes Error Codes
 * @{
 */
#define HAL_DMA_PRINTF_OK 0                 /**< Operation successful */
#define HAL_DMA_PRINTF_ERROR_NULL_PTR -1    /**< Null pointer error */
#define HAL_DMA_PRINTF_ERROR_NO_DMA_TX -2   /**< TX DMA not configured */
#define HAL_DMA_PRINTF_ERROR_NO_DMA_RX -3   /**< RX DMA not configured */
#define HAL_DMA_PRINTF_ERROR_NO_CALLBACK -4 /**< Register callbacks disabled \
                                             */
/** @} */

/**
 * @brief Initialize the HAL DMA printf library
 *
 * @details
 * Sets up the UART peripheral for DMA-based I/O operations. This function must
 * be called before using any printf/scanf or C++ stream operations.
 *
 * Requirements:
 * - UART peripheral must be initialized via CubeMX/HAL
 * - Both TX and RX DMA must be enabled for the UART
 * - USE_HAL_UART_REGISTER_CALLBACKS must be set to 1
 *
 * @param[in] huart Pointer to UART handle structure
 * @param[in] enable_echo Enable character echo for input (true/false)
 *
 * @return int Error code (HAL_DMA_PRINTF_OK on success)
 *
 * @note This function configures stdin/stdout buffering to unbuffered mode
 *
 * @code
 * // Example usage in main.c:
 * HalDmaPrintfSetup(&huart1, false);
 * printf("Hello, World!\r\n");
 * @endcode
 */
int HalDmaPrintfSetup(UART_HandleTypeDef* huart, bool enable_echo);

/**
 * @brief Enable echo mode for input characters
 *
 * @details
 * When echo is enabled, characters received via scanf or std::cin
 * will be automatically transmitted back (echoed) to the terminal.
 */
void HalDmaPrintfEnableEcho(void);

/**
 * @brief Disable echo mode for input characters
 *
 * @details
 * When echo is disabled, characters received via scanf or std::cin
 * will not be transmitted back to the terminal.
 */
void HalDmaPrintfDisableEcho(void);

/**
 * @brief Get the current buffer size configuration
 *
 * @return size_t Buffer size in bytes
 *
 * @note Buffer size can be configured at compile time using
 *       HAL_DMA_PRINTF_BUFFER_SIZE macro
 */
size_t HalDmaPrintfGetBufferSize(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // HAL_DMA_PRINTF_H