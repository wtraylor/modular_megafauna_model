/**
 * \file
 * \brief Unit test for Fauna::InsfileReader.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "insfile_reader.h"
#include "fileystem.h"
using namespace Fauna;

TEST_CASE("Fauna::InsfileReader") {
  // The example instruction file has been copied to the build directory by
  // CMake.
  static const std::string INSFILE = "megafauna.toml";

  REQUIRE(file_exists(INSFILE));
}
