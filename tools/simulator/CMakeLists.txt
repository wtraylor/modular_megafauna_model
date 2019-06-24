cmake_minimum_required (VERSION 3.14)
project (MegafaunaTestSimulator VERSION 0.0.0)

set (CMAKE_CXX_STANDARD 11)  # requires CMake 3.1 or higher

# Load the megafauna library’s CMakeLists.txt file.
add_subdirectory ("${PROJECT_SOURCE_DIR}/../")

add_executable (megafauna_simulator
  testhabitat.cpp
  testhabitat.h
  testsimulation.cpp
  testsimulation.h
  )
target_link_libraries (megafauna_simulator ModularMegafaunaModel)

