cmake_minimum_required(VERSION 3.14)
project(hal_dma_printf)

enable_language(CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} INTERFACE
  src/hal_dma_printf.cc
)

target_include_directories(${PROJECT_NAME} INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)