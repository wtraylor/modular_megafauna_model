#include "world_constructor.h"
#include "feed.h"
#include "hft.h"
#include "parameters.h"
#include "population.h"

using namespace Fauna;

WorldConstructor::WorldConstructor(const Parameters& params,
                                   const HftList& hftlist)
    : params(params), hftlist(hftlist) {}

std::auto_ptr<DistributeForage> WorldConstructor::create_distribute_forage()
    const {
  switch (get_params().forage_distribution) {
    case FD_EQUALLY:
      return std::auto_ptr<DistributeForage>(new DistributeForageEqually);
    default:
      throw std::logic_error(
          "WorldConstructor::create_distribute_forage(): "
          "chosen forage distribution algorithm not implemented");
  };
}

std::auto_ptr<PopulationInterface> WorldConstructor::create_population(
    const Hft* phft) const {
  // Create population instance according to selected herbivore type.
  if (get_params().herbivore_type == HT_COHORT) {
    return (std::auto_ptr<PopulationInterface>(new CohortPopulation(
        CreateHerbivoreCohort(phft, insfile_content.params.get()))));
  } else if (get_params().herbivore_type == HT_INDIVIDUAL) {
    const double AREA = 1.0;  // TODO THis is only a test
    return (std::auto_ptr<PopulationInterface>(new IndividualPopulation(
        CreateHerbivoreIndividual(phft, insfile_content.params.get()))));
    // TODO Where does the area size come from??
    // -> from Habitat (then merge() doesn’t work anymore)
    // -> from Parameters (then CreateHerbivoreIndividual
    //    can read it directly + new validity checks)
    // -> calculated by framework() ?
  } else
    throw std::logic_error(
        "WorldConstructor::create_population(): unknown herbivore type");
}

std::auto_ptr<HftPopulationsMap> WorldConstructor::create_populations() const {
  // instantiate the HftPopulationsMap object
  std::auto_ptr<HftPopulationsMap> pmap(new HftPopulationsMap());

  // Fill the object with one population per HFT
  for (int i = 0; i < get_hftlist().size(); i++) {
    const Hft* phft = &get_hftlist()[i];
    pmap->add(create_population(phft));
  }
  assert(pmap.get() != NULL);
  assert(pmap->size() == get_hftlist().size());
  return pmap;
}

std::auto_ptr<HftPopulationsMap> WorldConstructor::create_populations(
    const Hft* phft) const {
  // instantiate the HftPopulationsMap object
  std::auto_ptr<HftPopulationsMap> pmap(new HftPopulationsMap());
  // Fill the object with one population of one HFT
  pmap->add(create_population(phft));
  assert(pmap.get() != NULL);
  assert(pmap->size() == 1);
  return pmap;
}

