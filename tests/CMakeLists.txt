if (UNIT_TESTS)
  add_executable(megafauna_unit_tests
    main.cpp
    unittests.cpp
    catch.hpp)
endif()
