/**
 * \file
 * \brief Different models how to distribute available forage among herbivores.
 * \copyright ...
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
  // iterate through all herbivores
  for (ForageDistribution::iterator itr = forage_distribution.begin();
       itr != forage_distribution.end(); itr++) {
    demand_sum += itr->second;
  }

  // Only distribute a little less than `available` in order
  // to mitigate precision errors.
  const ForageMass avail_mass = available.get_mass() * 0.999;

  // If there is not more demanded than is available, nothing
  // needs to be distributed.
  if (demand_sum <= avail_mass) return;

  // MAKE EQUAL PORTIONS
  // iterate through all herbivores
  for (ForageDistribution::iterator itr = forage_distribution.begin();
       itr != forage_distribution.end(); itr++) {
    assert(itr->first != NULL);
    const HerbivoreInterface& herbivore = *(itr->first);
    const ForageMass& demand = itr->second;  // input
    ForageMass& portion = itr->second;       // output

    // calculate the right portion for each forage type
    for (const auto ft : FORAGE_TYPES) {
      if (demand_sum[ft] != 0.0)
        portion.set(ft, avail_mass[ft] * demand[ft] / demand_sum[ft]);
    }
  }
}
