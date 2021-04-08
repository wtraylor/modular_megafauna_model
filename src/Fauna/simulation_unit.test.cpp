// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Unit test for Fauna::SimulationUnit.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "catch.hpp"
#include "dummy_habitat.h"
#include "population_list.h"
#include "simulation_unit.h"
using namespace Fauna;

TEST_CASE("Fauna::SimulationUnit") {
  Hft HFT;
  CHECK_THROWS(SimulationUnit(NULL, new PopulationList()));
  CHECK_THROWS(SimulationUnit(new DummyHabitat(), NULL));
}

