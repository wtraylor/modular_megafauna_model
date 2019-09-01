cmake_minimum_required (VERSION 3.10)
project ("Modular Megafauna Model"
  VERSION 0.0.0
  DESCRIPTION "A physiological, dynamic herbivore simulator in C++."
  LANGUAGES CXX
  )

set (SOURCE_FILES
  include/Fauna/Output/habitat_data.h
  include/Fauna/date.h
  include/Fauna/environment.h
  include/Fauna/forage_types.h
  include/Fauna/forage_values.h
  include/Fauna/grass_forage.h
  include/Fauna/habitat.h
  include/Fauna/habitat_forage.h
  include/Fauna/parameters.h
  include/Fauna/world.h
  src/Fauna/Output/aggregator.cpp
  src/Fauna/Output/aggregator.h
  src/Fauna/Output/combined_data.cpp
  src/Fauna/Output/combined_data.h
  src/Fauna/Output/habitat_data.cpp
  src/Fauna/Output/herbivore_data.cpp
  src/Fauna/Output/herbivore_data.h
  src/Fauna/Output/writer_interface.h
  src/Fauna/Output/text_table_writer.cpp
  src/Fauna/Output/text_table_writer.h
  src/Fauna/createherbivores.cpp
  src/Fauna/createherbivores.h
  src/Fauna/date.cpp
  src/Fauna/date_interval.cpp
  src/Fauna/date_interval.h
  src/Fauna/energetics.cpp
  src/Fauna/energetics.h
  src/Fauna/feed_herbivores.cpp
  src/Fauna/feed_herbivores.h
  src/Fauna/fileystem.cpp
  src/Fauna/fileystem.h
  src/Fauna/forage_base.cpp
  src/Fauna/forage_distribution_algorithms.cpp
  src/Fauna/forage_distribution_algorithms.h
  src/Fauna/forage_types.cpp
  src/Fauna/forage_values.cpp
  src/Fauna/forageenergy.cpp
  src/Fauna/forageenergy.h
  src/Fauna/foraging.cpp
  src/Fauna/foraging.h
  src/Fauna/grass_forage.cpp
  src/Fauna/habitat.cpp
  src/Fauna/habitat_forage.cpp
  src/Fauna/herbivore_base.cpp
  src/Fauna/herbivore_base.h
  src/Fauna/herbivore_cohort.cpp
  src/Fauna/herbivore_cohort.h
  src/Fauna/herbivore_individual.cpp
  src/Fauna/herbivore_individual.h
  src/Fauna/herbivore_interface.h
  src/Fauna/herbivore_vector.h
  src/Fauna/hft.cpp
  src/Fauna/hft.h
  src/Fauna/mortality.cpp
  src/Fauna/mortality.h
  src/Fauna/nitrogen.cpp
  src/Fauna/nitrogen.h
  src/Fauna/parameters.cpp
  src/Fauna/population.cpp
  src/Fauna/population.h
  src/Fauna/population_list.cpp
  src/Fauna/population_list.h
  src/Fauna/read_insfile.cpp
  src/Fauna/read_insfile.h
  src/Fauna/reproduction.cpp
  src/Fauna/reproduction.h
  src/Fauna/simulate_day.cpp
  src/Fauna/simulate_day.h
  src/Fauna/simulation_unit.cpp
  src/Fauna/simulation_unit.h
  src/Fauna/utils.cpp
  src/Fauna/world.cpp
  src/Fauna/world_constructor.cpp
  src/Fauna/world_constructor.h
  include/Fauna/utils.h # TODO: do not include this

  src/Fauna/makeshift.h
  src/Fauna/makeshift.cpp
  )

add_library (ModularMegafaunaModel STATIC ${SOURCE_FILES})

# This library uses C++11 features, but does not require it from programs that
# use this library.
target_compile_features (ModularMegafaunaModel PRIVATE cxx_std_11)

target_include_directories (ModularMegafaunaModel
  PUBLIC
  include/
  PRIVATE
  include/Fauna/
  include/Fauna/Output/
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

include (CTest)  # Automatically sets BUILD_TESTING=ON by default.
if (BUILD_TESTING)
  add_executable(megafauna_unit_tests
    ${SOURCE_FILES}
    src/Fauna/Output/aggregator.test.cpp
    src/Fauna/Output/combined_data.test.cpp
    src/Fauna/Output/datapoint.h
    src/Fauna/Output/habitat_data.test.cpp
    src/Fauna/Output/herbivore_data.test.cpp
    src/Fauna/Output/text_table_writer.test.cpp
    src/Fauna/date.test.cpp
    src/Fauna/date_interval.test.cpp
    src/Fauna/energetics.test.cpp
    src/Fauna/feed_herbivores.test.cpp
    src/Fauna/fileystem.test.cpp
    src/Fauna/forage_distribution_algorithms.test.cpp
    src/Fauna/forage_values.test.cpp
    src/Fauna/forageenergy.test.cpp
    src/Fauna/foraging.test.cpp
    src/Fauna/grass_forage.test.cpp
    src/Fauna/habitat.test.cpp
    src/Fauna/habitat_forage.test.cpp
    src/Fauna/herbivore_base.test.cpp
    src/Fauna/herbivore_cohort.test.cpp
    src/Fauna/herbivore_individual.test.cpp
    src/Fauna/hft.test.cpp
    src/Fauna/mortality.test.cpp
    src/Fauna/nitrogen.test.cpp
    src/Fauna/parameters.test.cpp
    src/Fauna/population.test.cpp
    src/Fauna/population_list.test.cpp
    src/Fauna/reproduction.test.cpp
    src/Fauna/utils.test.cpp
    src/Fauna/world_constructor.test.cpp
    tests/catch.hpp
    tests/catch_main.cpp
    tests/dummy_habitat.h
    tests/dummy_herbivore.h
    tests/dummy_herbivore.test.cpp
    tests/dummy_herbivore_base.h
    tests/dummy_hft.h
    tests/dummy_population.h
    tests/dummy_population.test.cpp
    )
  target_include_directories (megafauna_unit_tests
    PRIVATE
    include/
    include/Fauna/
    include/Fauna/Output/
    src/Fauna/
    src/Fauna/Output/
    tests/
    )
  target_compile_options (megafauna_unit_tests
    PRIVATE
    -Wno-deprecated  # Suppress warnings about deprecated std::auto_ptr.
    )
endif(BUILD_TESTING)

###########################################################################
########################  TEST SIMULATOR  #################################
###########################################################################

option (BUILD_TEST_SIMULATOR
  "Whether to build the simple megafauna test simulator."
  ON
  )

if(BUILD_TEST_SIMULATOR)
  add_executable (megafauna_test_simulator
    tools/simulator/testhabitat.cpp
    tools/simulator/testhabitat.h
    tools/simulator/testsimulation.cpp
    tools/simulator/testsimulation.h
    )
  target_link_libraries (megafauna_test_simulator
    ModularMegafaunaModel
    )
endif(BUILD_TEST_SIMULATOR)

###########################################################################
####################  DOXYGEN DOCUMENTATION  ##############################
###########################################################################

option (BUILD_DOC "Build doxygen documentaton" OFF)
if (BUILD_DOC)
  add_subdirectory (docs/)
endif()
