cmake_minimum_required (VERSION 3.14)
project (ModularMegafaunaModel VERSION 0.0.0)

set (CMAKE_CXX_STANDARD 11)  # requires CMake 3.1 or higher

add_library (ModularMegafaunaModel STATIC
  )
# TODO: All all source files here

# TODO: Explain namespace alias.
add_library (ModularMegafaunaModel::fauna ALIAS fauna)

target_include_directories (ModularMegafaunaModel
  INTERFACE include
  PRIVATE src)

add_subdirectory (tests/)
