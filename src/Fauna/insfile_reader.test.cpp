/**
 * \file
 * \brief Unit test for Fauna::InsfileReader.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "fileystem.h"
#include "insfile_reader.h"
using namespace Fauna;

TEST_CASE("Fauna::InsfileReader") {
  SECTION("Read example instruction file") {
    // The example instruction file has been copied to the build directory by
    // CMake.
    static const std::string INSFILE = "megafauna.toml";

    REQUIRE(file_exists(INSFILE));

    CHECK_THROWS(InsfileReader(""));
    CHECK_THROWS(InsfileReader("this_file_does_not_exist"));

    CHECK_NOTHROW(InsfileReader(INSFILE));
  }
}
