#include "catch.hpp"
#include "hft.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::Hft", "") {
  Hft hft = Hft();
  std::string msg;

  SECTION("not valid without name") {
    hft.name = "";
    CHECK_FALSE(hft.is_valid(Fauna::Parameters(), msg));
  }
}
