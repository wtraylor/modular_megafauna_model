cmake_minimum_required (VERSION 3.14)
project (ModularMegafaunaModel VERSION 0.0.0)

set (CMAKE_CXX_STANDARD 11)  # requires CMake 3.1 or higher

add_library (ModularMegafaunaModel STATIC
  src/fauna/output/combined_data.cpp
  src/fauna/output/combined_data.h
  src/fauna/output/habitat_data.cpp
  src/fauna/output/habitat_data.h
  src/fauna/output/herbivore_data.cpp
  src/fauna/output/herbivore_data.h
  src/fauna/createherbivores.cpp
  src/fauna/createherbivores.h
  src/fauna/energetics.cpp
  src/fauna/energetics.h
  src/fauna/environment.h
  src/fauna/feed.cpp
  src/fauna/feed.h
  src/fauna/forageclasses.cpp
  src/fauna/forageclasses.h
  src/fauna/forageenergy.cpp
  src/fauna/forageenergy.h
  src/fauna/foraging.cpp
  src/fauna/foraging.h
  src/fauna/habitat.cpp
  src/fauna/habitat.h
  src/fauna/herbivore.cpp
  src/fauna/herbivore.h
  src/fauna/hft.cpp
  src/fauna/hft.h
  src/fauna/mortality.cpp
  src/fauna/mortality.h
  src/fauna/nitrogen.cpp
  src/fauna/nitrogen.h
  src/fauna/parameters.cpp
  src/fauna/parameters.h
  src/fauna/population.cpp
  src/fauna/population.h
  src/fauna/reproduction.cpp
  src/fauna/reproduction.h
  src/fauna/simulate_day.cpp
  src/fauna/simulate_day.h
  src/fauna/simulation_unit.cpp
  src/fauna/simulation_unit.h
  src/fauna/simulator.cpp
  src/fauna/simulator.h
  src/fauna/utils.cpp
  src/fauna/utils.h
  )

# TODO: Explain namespace alias.
# add_library (ModularMegafaunaModel::fauna ALIAS fauna)

target_include_directories (ModularMegafaunaModel
  INTERFACE
  include
  PRIVATE
  src/fauna/
  src/fauna/output/
  )

add_subdirectory (tests/)
