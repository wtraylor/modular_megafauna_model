cmake_minimum_required (VERSION 3.10)
project ("Modular Megafauna Model"
  VERSION 0.3.0
  DESCRIPTION "A physiological, dynamic herbivore simulator in C++."
  LANGUAGES CXX
  )

set (SOURCE_FILES
  external/cpptoml/include/cpptoml.h
  include/Fauna/Output/habitat_data.h
  include/Fauna/Output/text_table_writer_options.h
  include/Fauna/average.h
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
  src/Fauna/Output/text_table_writer.cpp
  src/Fauna/Output/text_table_writer.h
  src/Fauna/Output/writer_interface.h
  src/Fauna/average.cpp
  src/Fauna/breeding_season.cpp
  src/Fauna/breeding_season.h
  src/Fauna/cohort_population.cpp
  src/Fauna/cohort_population.h
  src/Fauna/create_herbivore_cohort.cpp
  src/Fauna/create_herbivore_cohort.h
  src/Fauna/create_herbivore_common.cpp
  src/Fauna/create_herbivore_common.h
  src/Fauna/create_herbivore_individual.cpp
  src/Fauna/create_herbivore_individual.h
  src/Fauna/date.cpp
  src/Fauna/date_interval.cpp
  src/Fauna/date_interval.h
  src/Fauna/expenditure_components.cpp
  src/Fauna/expenditure_components.h
  src/Fauna/fatmass_energy_budget.cpp
  src/Fauna/fatmass_energy_budget.h
  src/Fauna/feed_herbivores.cpp
  src/Fauna/feed_herbivores.h
  src/Fauna/fileystem.cpp
  src/Fauna/fileystem.h
  src/Fauna/forage_base.cpp
  src/Fauna/forage_distribution_algorithms.cpp
  src/Fauna/forage_distribution_algorithms.h
  src/Fauna/forage_types.cpp
  src/Fauna/forage_values.cpp
  src/Fauna/foraging_limits.cpp
  src/Fauna/foraging_limits.h
  src/Fauna/get_forage_demands.cpp
  src/Fauna/get_forage_demands.h
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
  src/Fauna/individual_population.cpp
  src/Fauna/individual_population.h
  src/Fauna/insfile_reader.cpp
  src/Fauna/insfile_reader.h
  src/Fauna/mortality_factors.cpp
  src/Fauna/mortality_factors.h
  src/Fauna/net_energy_models.cpp
  src/Fauna/net_energy_models.h
  src/Fauna/nitrogen.cpp
  src/Fauna/nitrogen.h
  src/Fauna/parameters.cpp
  src/Fauna/population_interface.cpp
  src/Fauna/population_interface.h
  src/Fauna/population_list.h
  src/Fauna/reproduction_models.cpp
  src/Fauna/reproduction_models.h
  src/Fauna/simulate_day.cpp
  src/Fauna/simulate_day.h
  src/Fauna/simulation_unit.cpp
  src/Fauna/simulation_unit.h
  src/Fauna/world.cpp
  src/Fauna/world_constructor.cpp
  src/Fauna/world_constructor.h
  )

add_library (ModularMegafaunaModel STATIC ${SOURCE_FILES})

# This library uses C++11 features, but does not require it from programs that
# use this library.
target_compile_features (ModularMegafaunaModel PRIVATE cxx_std_11)

target_include_directories (ModularMegafaunaModel
  PUBLIC
  include/
  PRIVATE
  external/cpptoml/include/
  include/Fauna/
  include/Fauna/Output/
  src/Fauna/
  src/Fauna/Output/
  )

###########################################################################
##########################  UNIT TESTS  ###################################
###########################################################################

include (CTest)  # Automatically sets BUILD_TESTING=ON by default.
if (BUILD_TESTING)
  add_executable (megafauna_unit_tests
    ${SOURCE_FILES}
    src/Fauna/Output/aggregator.test.cpp
    src/Fauna/Output/combined_data.test.cpp
    src/Fauna/Output/datapoint.h
    src/Fauna/Output/habitat_data.test.cpp
    src/Fauna/Output/herbivore_data.test.cpp
    src/Fauna/Output/text_table_writer.test.cpp
    src/Fauna/average.test.cpp
    src/Fauna/breeding_season.test.cpp
    src/Fauna/cohort_population.test.cpp
    src/Fauna/date.test.cpp
    src/Fauna/date_interval.test.cpp
    src/Fauna/expenditure_components.test.cpp
    src/Fauna/fatmass_energy_budget.test.cpp
    src/Fauna/feed_herbivores.test.cpp
    src/Fauna/fileystem.test.cpp
    src/Fauna/forage_distribution_algorithms.test.cpp
    src/Fauna/forage_values.test.cpp
    src/Fauna/foraging_limits.test.cpp
    src/Fauna/get_forage_demands.test.cpp
    src/Fauna/grass_forage.test.cpp
    src/Fauna/habitat.test.cpp
    src/Fauna/habitat_forage.test.cpp
    src/Fauna/herbivore_base.test.cpp
    src/Fauna/herbivore_cohort.test.cpp
    src/Fauna/herbivore_individual.test.cpp
    src/Fauna/hft.test.cpp
    src/Fauna/individual_population.test.cpp
    src/Fauna/insfile_reader.test.cpp
    src/Fauna/mortality_factors.test.cpp
    src/Fauna/net_energy_models.test.cpp
    src/Fauna/nitrogen.test.cpp
    src/Fauna/parameters.test.cpp
    src/Fauna/reproduction_models.test.cpp
    src/Fauna/world.test.cpp
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
    tools/demo_simulator/logistic_grass.cpp
    tools/demo_simulator/logistic_grass.h
    tools/demo_simulator/logistic_grass.test.cpp
    tools/demo_simulator/simple_habitat.cpp
    tools/demo_simulator/simple_habitat.h
    tools/demo_simulator/simple_habitat.test.cpp
    )
  target_compile_features (megafauna_unit_tests PRIVATE cxx_std_11)
  target_include_directories (megafauna_unit_tests
    PRIVATE
    external/cpptoml/include/
    include/
    include/Fauna/
    include/Fauna/Output/
    src/Fauna/
    src/Fauna/Output/
    tests/
    )
  # We need to check that the example TOML file is read correctly.
  configure_file (
    "examples/megafauna.toml"
    "${CMAKE_CURRENT_BINARY_DIR}/megafauna.toml"
    COPYONLY
    )
endif(BUILD_TESTING)

###########################################################################
########################  DEMO SIMULATOR  #################################
###########################################################################

option (BUILD_DEMO_SIMULATOR
  "Whether to build the simple megafauna demo simulator."
  ON
  )

if(BUILD_DEMO_SIMULATOR)
  add_executable (megafauna_demo_simulator
    tools/demo_simulator/demo_simulator.cpp
    tools/demo_simulator/demo_simulator.h
    tools/demo_simulator/logistic_grass.cpp
    tools/demo_simulator/logistic_grass.h
    tools/demo_simulator/simple_habitat.cpp
    tools/demo_simulator/simple_habitat.h
    )
  target_include_directories (megafauna_demo_simulator
    PRIVATE
    external/cpptoml/include/
    )
  target_compile_features (megafauna_demo_simulator PRIVATE cxx_std_11)
  target_link_libraries (megafauna_demo_simulator
    ModularMegafaunaModel
    )
  configure_file (
    "tools/demo_simulator/demo_results.Rmd"
    "${CMAKE_CURRENT_BINARY_DIR}/demo_results.Rmd"
    COPYONLY
    )
  configure_file (
    "examples/demo_simulation.toml"
    "${CMAKE_CURRENT_BINARY_DIR}/demo_simulation.toml"
    COPYONLY
    )
  file(
    COPY "tools/demo_simulator/run_demo"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
    FILE_PERMISSIONS
    WORLD_READ GROUP_READ OWNER_READ
    WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE
    OWNER_WRITE
    )
endif()

###########################################################################
####################  DOXYGEN DOCUMENTATION  ##############################
###########################################################################

option (BUILD_DOC "Build doxygen documentaton" OFF)
option (STOP_DOXYGEN_ON_WARNING "Let building of documentation fail on warning (for CI)" OFF)
if (BUILD_DOC)
  add_subdirectory (docs/)
endif()
