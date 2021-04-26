// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::Parameters.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "catch.hpp"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::Parameters", "") {
  // defaults must be valid.
  REQUIRE(Parameters().is_valid());
}
