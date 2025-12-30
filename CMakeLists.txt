# ============================================================================
# hal-dma-printf - DMA-based printf/scanf for STM32 HAL UART
# ============================================================================

cmake_minimum_required(VERSION 3.14)

# Project definition with version
project(hal_dma_printf 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "DMA-based printf/scanf for STM32 HAL UART"
)

enable_language(CXX)

# ============================================================================
# Configuration Options
# ============================================================================

option(HAL_DMA_PRINTF_BUILD_EXAMPLES "Build example code" OFF)

# Configurable buffer size (default: 1024 bytes)
set(HAL_DMA_PRINTF_BUFFER_SIZE "1024" CACHE STRING 
    "Size of TX/RX ring buffers in bytes")

# ============================================================================
# Library Definition
# ============================================================================

# Create OBJECT library
add_library(${PROJECT_NAME} INTERFACE)

# Source files
target_sources(${PROJECT_NAME} INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/hal_dma_printf.cc
)

# Include directories
target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} INTERFACE
    HAL_DMA_PRINTF_BUFFER_SIZE=${HAL_DMA_PRINTF_BUFFER_SIZE}
)

# ============================================================================
# Status Messages
# ============================================================================

message(STATUS "hal-dma-printf configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Buffer size: ${HAL_DMA_PRINTF_BUFFER_SIZE} bytes")
message(STATUS "  Build examples: ${HAL_DMA_PRINTF_BUILD_EXAMPLES}")