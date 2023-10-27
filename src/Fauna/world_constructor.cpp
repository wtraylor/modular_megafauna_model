// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Create objects for the \ref Fauna::World class.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "world_constructor.h"

#include "cohort_population.h"
#include "forage_distribution_algorithms.h"
#include "hft.h"
#include "parameters.h"

using namespace Fauna;

WorldConstructor::WorldConstructor(
    const std::shared_ptr<const Parameters> params, const HftList& hftlist)
    : params(std::move(params)), hftlist(hftlist) {}

DistributeForage* WorldConstructor::create_distribute_forage() const {
  switch (get_params().forage_distribution) {
    case ForageDistributionAlgorithm::Equally:
      return new DistributeForageEqually;
    default:
      throw std::logic_error(
          "WorldConstructor::create_distribute_forage(): "
          "chosen forage distribution algorithm not implemented");
  };
}

PopulationList* WorldConstructor::create_populations(
    const unsigned int habitat_ctr_in_agg_unit) const {
  PopulationList* plist = new PopulationList();

  if (get_hftlist().empty()) return plist;

  if (get_params().herbivore_type == HerbivoreType::Cohort) {
    if (get_params().one_hft_per_habitat) {
      // Create only one HFT, i.e. one population.
      const int hft_idx = habitat_ctr_in_agg_unit % get_hftlist().size();
      assert(hft_idx >= 0);
      assert(hft_idx < get_hftlist().size());
      plist->emplace_back(new CohortPopulation(
          CreateHerbivoreCohort(get_hftlist()[hft_idx], params)));
      assert(plist->size() == 1);
    } else {
      // Create one population for every HFT.
      for (const auto& hft_ptr : get_hftlist())
        plist->emplace_back(
            new CohortPopulation(CreateHerbivoreCohort(hft_ptr, params)));
      assert(plist->size() == get_hftlist().size());
    }
  } else
    throw std::logic_error(
        "WorldConstructor::create_population(): unknown herbivore type");

  assert(plist != NULL);
  assert(!plist->empty());
  return plist;
}
