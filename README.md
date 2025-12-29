# hal-dma-printf

STM32 の HAL 向け、DMA を使ったバッファーありの UART printf を実現するライブラリ


## CubeMX での設定

UART の RegisterCallback を Enable

![](./doc/enable_uart_register_callback.drawio.svg)


## CMake 設定

``` cmake
add_subdirectory(path/to/hal-dma-printf)
target_link_libraries(${CMAKE_PROJECT_NAME}
    # ...
    hal_dma_printf
)
```


## main.c の設定

```c
  /* USER CODE BEGIN Includes */
  #include "hal_dma_printf/hal_dma_printf.h"
  /* USER CODE END Includes */

  // ...

  /* USER CODE BEGIN 2 */
  HalDmaPrintfSetUartHandler(&huartx);
  /* USER CODE END 2 */
```