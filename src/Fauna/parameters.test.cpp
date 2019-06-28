#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::Parameters", "") {
  // defaults must be valid.
  REQUIRE(Parameters().is_valid());
}

