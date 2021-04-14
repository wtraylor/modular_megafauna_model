// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Different models how to distribute available forage among herbivores.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "forage_distribution_algorithms.h"
#include "habitat_forage.h"

using namespace Fauna;

void DistributeForageEqually::operator()(
    const HabitatForage& available,
    ForageDistribution& forage_distribution) const {
  if (forage_distribution.empty()) return;

  // BUILD SUM OF ALL DEMANDED FORAGE
  ForageMass demand_sum;
  for (const auto& pair : forage_distribution) demand_sum += pair.second;

  // Only distribute a little less than `available` in order
  // to mitigate precision errors.
  const ForageMass avail_mass = available.get_mass() * 0.999;

  // If there is not more demanded than is available, nothing
  // needs to be distributed.
  if (demand_sum <= avail_mass) return;

  // MAKE EQUAL PORTIONS
  // iterate through all herbivores
  for (auto& pair : forage_distribution) {
    assert(pair.first != NULL);
    const HerbivoreInterface& herbivore = *(pair.first);
    const ForageMass& demand = pair.second;  // input
    ForageMass& portion = pair.second;       // output

    // calculate the right portion for each forage type
    for (const auto ft : FORAGE_TYPES) {
      if (demand_sum[ft] != 0.0)
        portion.set(ft, avail_mass[ft] * demand[ft] / demand_sum[ft]);
    }
  }
}
