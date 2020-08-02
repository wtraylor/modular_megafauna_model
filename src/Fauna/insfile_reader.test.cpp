/**
 * \file
 * \brief Unit test for Fauna::InsfileReader.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "fileystem.h"
#include "hft.h"
#include "insfile_reader.h"
using namespace Fauna;

TEST_CASE("Fauna::InsfileReader") {
  SECTION("Error on non-existant files") {
    CHECK_THROWS(InsfileReader(""));
    CHECK_THROWS(InsfileReader("this_file_does_not_exist"));
  }

  SECTION("Read good example instruction file") {
    // The example instruction file has been copied to the build directory by
    // CMake.
    static const std::string INSFILE = "megafauna.toml";

    REQUIRE(file_exists(INSFILE));

    REQUIRE_NOTHROW(InsfileReader(INSFILE));

    const InsfileReader reader = InsfileReader(INSFILE);
    SECTION("Check Parameters.is_valid()") {
      std::string msg;
      REQUIRE(reader.get_params().is_valid(msg));
      INFO("Message from is_valid():\n" + msg);
      CHECK(msg.empty());  // There should also be no warnings.
    }
    SECTION("Check Hft.is_valid()") {
      std::string msg;
      REQUIRE(reader.get_hfts().size() == 1);  // one HFT
      for (const auto& hft : reader.get_hfts()) {
        INFO("HFT name: " + hft->name);
        REQUIRE(hft->is_valid(reader.get_params(), msg));
        INFO("Message from is_valid():\n" + msg);
        CHECK(msg.empty());  // There should also be no warnings.
      }
    }
  }
}
