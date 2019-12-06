/**
 * \file
 * \brief Create objects for the \ref Fauna::World class.
 * \copyright ...
 * \date 2019
 */
#include "world_constructor.h"
#include "cohort_population.h"
#include "forage_distribution_algorithms.h"
#include "hft.h"
#include "hft_list.h"
#include "individual_population.h"
#include "parameters.h"
#include "population_list.h"

using namespace Fauna;

WorldConstructor::WorldConstructor(const std::shared_ptr<const Parameters> params,
                                   const HftList& hftlist)
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

PopulationInterface* WorldConstructor::create_population(
    const Hft* phft) const {
  // Create population instance according to selected herbivore type.
  if (get_params().herbivore_type == HerbivoreType::Cohort) {
    return new CohortPopulation(CreateHerbivoreCohort(phft, params));
  } else if (get_params().herbivore_type == HerbivoreType::Individual) {
    const double AREA = 1.0;  // TODO THis is only a test
    return new IndividualPopulation(
        CreateHerbivoreIndividual(phft, params));
    // TODO Where does the area size come from??
    // -> from Habitat (then merge() doesn’t work anymore)
    // -> from Parameters (then CreateHerbivoreIndividual
    //    can read it directly + new validity checks)
    // -> calculated by framework() ?
  } else
    throw std::logic_error(
        "WorldConstructor::create_population(): unknown herbivore type");
}

PopulationList* WorldConstructor::create_populations() const {
  PopulationList* plist = new PopulationList();

  // Fill the object with one population per HFT
  for (int i = 0; i < get_hftlist().size(); i++) {
    const Hft* phft = &get_hftlist()[i];
    plist->add(create_population(phft));
  }
  assert(plist != NULL);
  return plist;
}

PopulationList* WorldConstructor::create_populations(const Hft* phft) const {
  PopulationList* plist = new PopulationList();

  // Fill the object with one population of one HFT
  plist->add(create_population(phft));
  assert(plist != NULL);
  return plist;
}
