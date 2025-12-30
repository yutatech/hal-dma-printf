# hal-dma-printf

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C/C++](https://img.shields.io/badge/language-C%2FC%2B%2B-blue.svg)](https://isocpp.org/)

Library for buffered UART printf with DMA for STM32 HAL

STM32 の HAL 向け、DMA を使ったバッファーありの UART printf を実現するライブラリ



---

## 📖 Table of Contents / 目次

- [English](#english)
  - [Overview](#overview)
  - [Features](#features)
  - [Design Philosophy](#design-philosophy)
  - [Requirements](#requirements)
  - [Installation](#installation)
  - [Usage Examples](#usage-examples)
  - [API Reference](#api-reference)
  - [Performance Notes](#performance-notes)
- [日本語](#日本語)
  - [概要](#概要)
  - [特徴](#特徴)
  - [設計思想](#設計思想)
  - [必要要件](#必要要件)
  - [導入手順](#導入手順)
  - [使用例](#使用例)
  - [APIリファレンス](#apiリファレンス)
  - [パフォーマンスノート](#パフォーマンスノート)

---

## English

### Overview

**hal-dma-printf** is a lightweight C/C++ library that enables `printf()`, `scanf()`, `std::cout`, and `std::cin` functionality over UART on STM32 microcontrollers using DMA (Direct Memory Access) for efficient, non-blocking I/O operations.

### Features

- ✅ **Non-blocking I/O**: DMA-based transmission/reception eliminates CPU blocking
- ✅ **Ring Buffer Implementation**: Efficient circular buffer for smooth data flow
- ✅ **Full Standard I/O Support**:
  - C standard library: `printf()`, `scanf()`, `getchar()`, `puts()`
  - C++ streams: `std::cout`, `std::cin`, `std::cerr`
- ✅ **Configurable Buffer Size**: Compile-time customization via CMake
- ✅ **Echo Mode**: Optional character echo for interactive terminals
- ✅ **Error Handling**: Comprehensive error detection and reporting
- ✅ **Zero External Dependencies**: Only requires STM32 HAL

### Design Philosophy

Traditional UART `printf()` implementations on embedded systems use blocking HAL functions like `HAL_UART_Transmit()`, which stalls the CPU until transmission completes. This library solves this problem by:

1. **Hooking Syscalls**: Overrides `_write()` and `_read()` syscalls to intercept standard I/O
2. **Ring Buffers**: Uses circular buffers to decouple application logic from hardware timing
3. **DMA Automation**: Hardware handles data transfer, freeing the CPU for other tasks

This approach maintains compatibility with standard C/C++ I/O while achieving high performance.

### Requirements

#### Hardware
- STM32 microcontroller with HAL library
- UART peripheral with TX and RX DMA channels

#### Software
- STM32CubeMX (for initial configuration)
- CMake 3.14 or later
- C++17 compatible compiler (e.g., arm-none-eabi-g++)

### Installation

#### 1. CubeMX Configuration

**Step 1**: Enable UART with DMA

In STM32CubeMX, configure your UART peripheral:

1. Enable UART (e.g., USART1)
2. Navigate to **DMA Settings** tab
3. Add **USART_TX** with:
   - Mode: Normal
   - Priority: Medium
4. Add **USART_RX** with:
   - Mode: Circular (important!)
   - Priority: Medium

**Step 2**: Enable Register Callbacks

In **Project Manager** → **Advanced Settings**:
- Set **UART** to **Register Callback** → **ENABLE**

![](./doc/enable_uart_register_callback.drawio.svg)


#### 2. CMake Integration

Add the library as a subdirectory in your project's `CMakeLists.txt`:

```cmake
# Add hal-dma-printf library
add_subdirectory(lpath/to/hal-dma-printf)

# Link to your executable
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # ... other libraries ...
    hal_dma_printf
)
```

**Optional**: Customize buffer size:

```cmake
# Set buffer size before add_subdirectory
set(HAL_DMA_PRINTF_BUFFER_SIZE 2048 CACHE STRING "" FORCE)
add_subdirectory(lpath/to/hal-dma-printf)
```

#### 3. Source Code Setup

In your `main.c` or `main.cpp`:

```c
/* USER CODE BEGIN Includes */
#include "hal_dma_printf/hal_dma_printf.h"
/* USER CODE END Includes */

int main(void)
{
  /* ... HAL initialization ... */
  MX_USART1_UART_Init();
  
  /* USER CODE BEGIN 2 */
  // Initialize hal-dma-printf (disable echo mode)
  HalDmaPrintfSetup(&huart1, false);
  
  printf("Hello from STM32!\r\n");
  printf("System clock: %lu Hz\r\n", SystemCoreClock);
  /* USER CODE END 2 */
  
  /* ... main loop ... */
}
```

### Usage Examples

#### C Standard I/O

```c
#include <stdio.h>
#include "hal_dma_printf/hal_dma_printf.h"

void example_c_stdio(void) {
  // Printf family
  printf("Integer: %d\r\n", 42);
  printf("Float: %.2f\r\n", 3.14159);
  printf("String: %s\r\n", "STM32");
  
  // Scanf (requires echo enabled)
  HalDmaPrintfEnableEcho();
  int value;
  printf("Enter a number: ");
  scanf("%d", &value);
  printf("You entered: %d\r\n", value);
}
```

#### C++ Streams

```cpp
#include <iostream>
#include <iomanip>
#include "hal_dma_printf/hal_dma_printf.h"

void example_cpp_streams(void) {
  // Output
  std::cout << "Hello from C++!" << std::endl;
  std::cout << "Hex: 0x" << std::hex << 255 << std::endl;
  std::cout << "Float: " << std::fixed 
            << std::setprecision(2) << 3.14159 << std::endl;
  
  // Input
  HalDmaPrintfEnableEcho();
  int number;
  std::cout << "Enter value: ";
  std::cin >> number;
  std::cout << "Received: " << number << std::endl;
}
```

### API Reference

#### Initialization

```c
int HalDmaPrintfSetup(UART_HandleTypeDef* huart, bool enable_echo);
```
- **Parameters**:
  - `huart`: Pointer to UART handle (e.g., `&huart1`)
  - `enable_echo`: Enable character echo for input
- **Returns**: `HAL_DMA_PRINTF_OK` on success, error code otherwise
- **Note**: Call this after UART initialization

#### Echo Control

```c
void HalDmaPrintfEnableEcho(void);
void HalDmaPrintfDisableEcho(void);
```
- Enable/disable character echo for interactive terminals

#### Buffer Size Query

```c
size_t HalDmaPrintfGetBufferSize(void);
```
- **Returns**: Configured buffer size in bytes

#### Error Codes

| Code | Value | Description |
|------|-------|-------------|
| `HAL_DMA_PRINTF_OK` | 0 | Success |
| `HAL_DMA_PRINTF_ERROR_NULL_PTR` | -1 | Null pointer error |
| `HAL_DMA_PRINTF_ERROR_NO_DMA_TX` | -2 | TX DMA not configured |
| `HAL_DMA_PRINTF_ERROR_NO_DMA_RX` | -3 | RX DMA not configured |
| `HAL_DMA_PRINTF_ERROR_NO_CALLBACK` | -4 | Register callbacks disabled |

### Performance Notes

- **Buffer Size**: Default 1024 bytes. Adjust based on application needs.
- **Baud Rate**: Higher baud rates reduce latency but require larger buffers for burst data.
- **Overhead**: Minimal CPU usage (~1-2% at 115200 baud on STM32F4).

---

## 日本語

### 概要

**hal-dma-printf** は、STM32マイコン上でUART経由の `printf()`、`scanf()`、`std::cout`、`std::cin` を実現する軽量なC/C++ライブラリです。DMA（ダイレクトメモリアクセス）を活用した効率的なノンブロッキングI/Oを提供します。

### 特徴

- ✅ **ノンブロッキングI/O**: DMAベースの送受信でCPUブロッキングを排除
- ✅ **リングバッファ実装**: 効率的な循環バッファでスムーズなデータフロー
- ✅ **標準I/Oの完全サポート**:
  - C標準ライブラリ: `printf()`, `scanf()`, `getchar()`, `puts()`
  - C++ストリーム: `std::cout`, `std::cin`, `std::cerr`
- ✅ **バッファサイズ設定可能**: CMakeでコンパイル時にカスタマイズ
- ✅ **エコーモード**: 対話型ターミナル用の文字エコー機能
- ✅ **エラーハンドリング**: 包括的なエラー検出と報告
- ✅ **外部依存ゼロ**: STM32 HALのみで動作

### 設計思想

組み込みシステムにおける従来のUART `printf()` 実装は、`HAL_UART_Transmit()` のようなブロッキング関数を使用するため、送信完了までCPUが停止します。本ライブラリはこの問題を以下の方法で解決します:

1. **システムコールのフック**: `_write()` と `_read()` をオーバーライドして標準I/Oをインターセプト
2. **リングバッファ**: 循環バッファでアプリケーションロジックとハードウェアタイミングを分離
3. **DMA自動化**: ハードウェアがデータ転送を処理し、CPUを他のタスクに解放

この手法により、標準C/C++ I/Oとの互換性を保ちながら高性能を実現します。

### 必要要件

#### ハードウェア
- STM32マイコン（HALライブラリ）
- TXおよびRX DMAチャネルを持つUARTペリフェラル

#### ソフトウェア
- STM32CubeMX（初期設定用）
- CMake 3.14以降
- C++17対応コンパイラ（例: arm-none-eabi-g++）

### 導入手順

#### 1. CubeMXでの設定

**手順1**: DMA付きUARTの有効化

STM32CubeMXでUARTペリフェラルを設定:

1. UART（例: USART1）を有効化
2. **DMA Settings** タブに移動
3. **USART_TX** を追加:
   - Mode: Normal
   - Priority: Medium
4. **USART_RX** を追加:
   - Mode: Circular（重要！）
   - Priority: Medium

**手順2**: レジスタコールバックの有効化

**Project Manager** → **Advanced Settings**:
- **UART** を **Register Callback** → **ENABLE** に設定

![](./doc/enable_uart_register_callback.drawio.svg)

#### 2. CMake統合

プロジェクトの `CMakeLists.txt` でライブラリをサブディレクトリとして追加:

```cmake
# hal-dma-printfライブラリを追加
add_subdirectory(lpath/to/hal-dma-printf)

# 実行ファイルにリンク
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # ... 他のライブラリ ...
    hal_dma_printf
)
```

**オプション**: バッファサイズのカスタマイズ:

```cmake
# add_subdirectory前にバッファサイズを設定
set(HAL_DMA_PRINTF_BUFFER_SIZE 2048 CACHE STRING "" FORCE)
add_subdirectory(lpath/to/hal-dma-printf)
```

#### 3. ソースコードのセットアップ

`main.c` または `main.cpp` で:

```c
/* USER CODE BEGIN Includes */
#include "hal_dma_printf/hal_dma_printf.h"
/* USER CODE END Includes */

int main(void)
{
  /* ... HAL初期化 ... */
  MX_USART1_UART_Init();
  
  /* USER CODE BEGIN 2 */
  // hal-dma-printfを初期化（エコーモード無効）
  HalDmaPrintfSetup(&huart1, false);
  
  printf("Hello from STM32!\r\n");
  printf("System clock: %lu Hz\r\n", SystemCoreClock);
  /* USER CODE END 2 */
  
  /* ... メインループ ... */
}
```

### 使用例

#### C標準I/O

```c
#include <stdio.h>
#include "hal_dma_printf/hal_dma_printf.h"

void example_c_stdio(void) {
  // printf系
  printf("整数: %d\r\n", 42);
  printf("浮動小数点: %.2f\r\n", 3.14159);
  printf("文字列: %s\r\n", "STM32");
  
  // scanf（エコー有効化が必要）
  HalDmaPrintfEnableEcho();
  int value;
  printf("数値を入力: ");
  scanf("%d", &value);
  printf("入力値: %d\r\n", value);
}
```

#### C++ストリーム

```cpp
#include <iostream>
#include <iomanip>
#include "hal_dma_printf/hal_dma_printf.h"

void example_cpp_streams(void) {
  // 出力
  std::cout << "C++から こんにちは!" << std::endl;
  std::cout << "16進数: 0x" << std::hex << 255 << std::endl;
  std::cout << "浮動小数点: " << std::fixed 
            << std::setprecision(2) << 3.14159 << std::endl;
  
  // 入力
  HalDmaPrintfEnableEcho();
  int number;
  std::cout << "値を入力: ";
  std::cin >> number;
  std::cout << "受信: " << number << std::endl;
}
```

### APIリファレンス

#### 初期化

```c
int HalDmaPrintfSetup(UART_HandleTypeDef* huart, bool enable_echo);
```
- **パラメータ**:
  - `huart`: UARTハンドルへのポインタ（例: `&huart1`）
  - `enable_echo`: 入力文字のエコーを有効化
- **戻り値**: 成功時 `HAL_DMA_PRINTF_OK`、失敗時エラーコード
- **注意**: UART初期化後に呼び出すこと

#### エコー制御

```c
void HalDmaPrintfEnableEcho(void);
void HalDmaPrintfDisableEcho(void);
```
- 対話型ターミナル用の文字エコーを有効化/無効化

#### バッファサイズ取得

```c
size_t HalDmaPrintfGetBufferSize(void);
```
- **戻り値**: 設定されたバッファサイズ（バイト単位）

#### エラーコード

| コード | 値 | 説明 |
|------|-------|-------------|
| `HAL_DMA_PRINTF_OK` | 0 | 成功 |
| `HAL_DMA_PRINTF_ERROR_NULL_PTR` | -1 | NULLポインタエラー |
| `HAL_DMA_PRINTF_ERROR_NO_DMA_TX` | -2 | TX DMA未設定 |
| `HAL_DMA_PRINTF_ERROR_NO_DMA_RX` | -3 | RX DMA未設定 |
| `HAL_DMA_PRINTF_ERROR_NO_CALLBACK` | -4 | レジスタコールバック無効 |

### パフォーマンスノート

- **バッファサイズ**: デフォルト1024バイト。用途に応じて調整可能。
- **ボーレート**: 高速なボーレートは遅延を減らしますが、バースト データには大きなバッファが必要。
- **オーバーヘッド**: 最小限のCPU使用率（STM32F4で115200ボー時約1-2%）。

---

## License

MIT License - See [LICENSE](LICENSE) file for details

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Author

[yutatech](https://github.com/yutatech)