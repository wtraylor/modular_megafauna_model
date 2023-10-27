// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for DummyPopulation.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "dummy_population.h"

#include "catch.hpp"
using namespace Fauna;

TEST_CASE("DummyPopulation") {
  Hft hft1;
  hft1.name = "hft1";
  DummyHerbivore dummy1 = DummyHerbivore(&hft1, 1.0);
  DummyHerbivore dummy2 = DummyHerbivore(&hft1, 0.0);
  DummyPopulation pop = DummyPopulation(&hft1);
  pop.create_offspring(1.0);
  REQUIRE(pop.get_list().size() == 1);
}
