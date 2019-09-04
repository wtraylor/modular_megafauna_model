/**
 * \file
 * \brief Unit test for Fauna::Parameters.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::Parameters", "") {
  // defaults must be valid.
  REQUIRE(Parameters().is_valid());
}

