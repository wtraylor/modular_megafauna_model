// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Hft.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
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
