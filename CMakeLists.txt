cmake_minimum_required (VERSION 3.14)
project ("Modular Megafauna Model"
  VERSION 0.0.0
  DESCRIPTION "A physiological, dynamic herbivore simulator in C++."
  LANGUAGES CXX
  )

set (SOURCE_FILES
  include/Fauna/forage_types.h
  src/Fauna/Output/combined_data.cpp
  src/Fauna/Output/combined_data.h
  src/Fauna/Output/habitat_data.cpp
  src/Fauna/Output/habitat_data.h
  src/Fauna/Output/herbivore_data.cpp
  src/Fauna/Output/herbivore_data.h
  src/Fauna/createherbivores.cpp
  src/Fauna/createherbivores.h
  src/Fauna/energetics.cpp
  src/Fauna/energetics.h
  src/Fauna/environment.h
  src/Fauna/feed.cpp
  src/Fauna/feed.h
  src/Fauna/forage_types.cpp
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

add_library (ModularMegafaunaModel STATIC ${SOURCE_FILES})

# TODO: Explain namespace alias.
# add_library (ModularMegafaunaModel::Fauna ALIAS Fauna)

# This library uses C++11 features, but does not require it from programs that
# use this library.
target_compile_features (ModularMegafaunaModel PRIVATE cxx_std_11)

target_include_directories (ModularMegafaunaModel
  PUBLIC
  include/
  include/Fauna/
  PRIVATE
  src/Fauna/
  src/Fauna/Output/
  )

target_compile_options (ModularMegafaunaModel
  PRIVATE
  -Wno-deprecated  # Suppress warnings about deprecated std::auto_ptr.
  )

###########################################################################
##########################  UNIT TESTS  ###################################
###########################################################################

find_package(Catch2 REQUIRED)
add_executable(megafauna_unit_tests
  ${SOURCE_FILES}
  # src/Fauna/Output/combined_data.test.cpp
  src/Fauna/Output/habitat_data.test.cpp
  # src/Fauna/Output/herbivore_data.test.cpp
  src/Fauna/energetics.test.cpp
  # src/Fauna/feed.test.cpp
  src/Fauna/forageclasses.test.cpp
  src/Fauna/forageenergy.test.cpp
  # src/Fauna/foraging.test.cpp
  src/Fauna/habitat.test.cpp
  # src/Fauna/herbivore.test.cpp
  src/Fauna/hft.test.cpp
  # src/Fauna/mortality.test.cpp
  src/Fauna/nitrogen.test.cpp
  src/Fauna/parameters.test.cpp
  # src/Fauna/population.test.cpp
  # src/Fauna/reproduction.test.cpp
  src/Fauna/simulation_unit.test.cpp
  # src/Fauna/simulator.test.cpp
  src/Fauna/utils.test.cpp
  tests/catch_main.cpp
  tests/dummy_habitat.h
  )
target_include_directories (megafauna_unit_tests
  PRIVATE
  include/Fauna/
  src/Fauna/
  src/Fauna/Output/
  tests/
  )
target_link_libraries (megafauna_unit_tests Catch2::Catch2)
include (CTest)
include (Catch)
catch_discover_tests (megafauna_unit_tests)
target_compile_options (megafauna_unit_tests
  PRIVATE
  -Wno-deprecated  # Suppress warnings about deprecated std::auto_ptr.
  )

###########################################################################
####################  DOXYGEN DOCUMENTATION  ##############################
###########################################################################

option (BUILD_DOC "Build doxygen documentaton" ON)
if (BUILD_DOC)
  add_subdirectory (docs/)
endif()
