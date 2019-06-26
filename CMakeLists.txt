cmake_minimum_required (VERSION 3.14)
project ("Modular Megafauna Model"
  VERSION 0.0.0
  DESCRIPTION "A physiological, dynamic herbivore simulator in C++."
  LANGUAGES CXX
  )

add_library (ModularMegafaunaModel STATIC
  src/fauna/createherbivores.cpp
  src/fauna/createherbivores.h
  src/fauna/energetics.cpp
  src/fauna/energetics.h
  src/fauna/environment.h
  src/fauna/feed.cpp
  src/fauna/feed.h
  src/fauna/forage_types.cpp
  src/fauna/forage_types.h
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
  src/fauna/output/combined_data.cpp
  src/fauna/output/combined_data.h
  src/fauna/output/habitat_data.cpp
  src/fauna/output/habitat_data.h
  src/fauna/output/herbivore_data.cpp
  src/fauna/output/herbivore_data.h
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

# This library uses C++11 features, but does not require it from programs that
# use this library.
target_compile_features (ModularMegafaunaModel PRIVATE cxx_std_11)

target_include_directories (ModularMegafaunaModel
  INTERFACE
  include
  PRIVATE
  src/fauna/
  src/fauna/output/
  )

target_compile_options (ModularMegafaunaModel
  PRIVATE
  -Wno-deprecated  # Suppress warnings about deprecated std::auto_ptr.
  )

option (BUILD_DOC "Build doxygen documentaton" ON)
if (BUILD_DOC)
  add_subdirectory (docs/)
endif()

add_subdirectory (tests/)
