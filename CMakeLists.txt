cmake_minimum_required (VERSION 3.14)
project ("Modular Megafauna Model"
  VERSION 0.0.0
  DESCRIPTION "A physiological, dynamic herbivore simulator in C++."
  LANGUAGES CXX
  )

add_library (ModularMegafaunaModel STATIC
  src/Fauna/createherbivores.cpp
  src/Fauna/createherbivores.h
  src/Fauna/energetics.cpp
  src/Fauna/energetics.h
  src/Fauna/environment.h
  src/Fauna/feed.cpp
  src/Fauna/feed.h
  src/Fauna/forage_types.cpp
  src/Fauna/forage_types.h
  src/Fauna/forageclasses.cpp
  src/Fauna/forageclasses.h
  src/Fauna/forageenergy.cpp
  src/Fauna/forageenergy.h
  src/Fauna/foraging.cpp
  src/Fauna/foraging.h
  src/Fauna/habitat.cpp
  src/Fauna/habitat.h
  src/Fauna/herbivore.cpp
  src/Fauna/herbivore.h
  src/Fauna/hft.cpp
  src/Fauna/hft.h
  src/Fauna/mortality.cpp
  src/Fauna/mortality.h
  src/Fauna/nitrogen.cpp
  src/Fauna/nitrogen.h
  src/Fauna/Output/combined_data.cpp
  src/Fauna/Output/combined_data.h
  src/Fauna/Output/habitat_data.cpp
  src/Fauna/Output/habitat_data.h
  src/Fauna/Output/herbivore_data.cpp
  src/Fauna/Output/herbivore_data.h
  src/Fauna/parameters.cpp
  src/Fauna/parameters.h
  src/Fauna/population.cpp
  src/Fauna/population.h
  src/Fauna/reproduction.cpp
  src/Fauna/reproduction.h
  src/Fauna/simulate_day.cpp
  src/Fauna/simulate_day.h
  src/Fauna/simulation_unit.cpp
  src/Fauna/simulation_unit.h
  src/Fauna/simulator.cpp
  src/Fauna/simulator.h
  src/Fauna/utils.cpp
  src/Fauna/utils.h
  )

# TODO: Explain namespace alias.
# add_library (ModularMegafaunaModel::Fauna ALIAS Fauna)

# This library uses C++11 features, but does not require it from programs that
# use this library.
target_compile_features (ModularMegafaunaModel PRIVATE cxx_std_11)

target_include_directories (ModularMegafaunaModel
  INTERFACE
  include
  PRIVATE
  src/Fauna/
  src/Fauna/Output/
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
