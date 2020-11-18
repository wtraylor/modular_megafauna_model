// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
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

PopulationList* WorldConstructor::create_populations() const {
  PopulationList* plist = new PopulationList();

  if (get_params().herbivore_type == HerbivoreType::Cohort) {
    // Create one population per HFT.
    for (const auto& hft_ptr : get_hftlist())
      plist->emplace_back(
          new CohortPopulation(CreateHerbivoreCohort(hft_ptr, params)));
  } else
    throw std::logic_error(
        "WorldConstructor::create_population(): unknown herbivore type");

  assert(plist != NULL);
  return plist;
}
