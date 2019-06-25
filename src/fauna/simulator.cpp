//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
//////////////////////////////////////////////////////////////////////////

#include "simulator.h"
#include <stdexcept>  // for std::logic_error, std::invalid_argument
#include "herbivore.h"        // for HerbivoreInterface
#include "hft.h"              // for Hft and HftList
#include "parameters.h"       // for Fauna::Parameters
#include "population.h"       // for HftPopulationsMap and PopulationInterface
#include "simulate_day.h"     // for SimulateDay
#include "simulation_unit.h"  // for SimulationUnit
#include "snowdepth.h"        // for GetSnowDepth implementations

using namespace Fauna;

//============================================================
// Simulator
//============================================================

Simulator::Simulator(const Parameters& params)
    : params(params),
      days_since_last_establishment(params.herbivore_establish_interval),
      feed_herbivores(create_distribute_forage())
// Non-static data member initialization happens in the order
// of declaration in the header file: feed_herbivores must be
// *after* params so that create_distribute_forage() does not
// cause segmentation fault.
{}

std::auto_ptr<DistributeForage> Simulator::create_distribute_forage() {
  switch (params.forage_distribution) {
    case FD_EQUALLY:
      return std::auto_ptr<DistributeForage>(new DistributeForageEqually);
    default:
      throw std::logic_error(
          "Fauna::Simulator::distribute_forage(): "
          "chosen forage distribution algorithm not implemented");
  };
}

std::auto_ptr<GetSnowDepth> Simulator::create_snow_depth_model() const {
  switch (params.snow_depth_model) {
    case SD_TEN_TO_ONE:
      return std::auto_ptr<GetSnowDepth>(new SnowDepthTenToOne());
    default:
      throw std::logic_error(
          "Simulator::create_snow_depth_model(): "
          "Snow depth model not implemented.");
  };
}

std::auto_ptr<PopulationInterface> Simulator::create_population(
    const Hft* phft) const {
  // Create population instance according to selected herbivore
  // type
  if (params.herbivore_type == HT_COHORT) {
    return (std::auto_ptr<PopulationInterface>(
        new CohortPopulation(CreateHerbivoreCohort(phft, &params))));
  } else if (params.herbivore_type == HT_INDIVIDUAL) {
    const double AREA = 1.0;  // TODO THis is only a test
    return (std::auto_ptr<PopulationInterface>(
        new IndividualPopulation(CreateHerbivoreIndividual(phft, &params))));
    // TODO Where does the area size come from??
    // -> from Habitat (then merge() doesn’t work anymore)
    // -> from Parameters (then CreateHerbivoreIndividual
    //    can read it directly + new validity checks)
    // -> calculated by framework() ?
  } else
    throw std::logic_error(
        "Simulator::create_populations(): "
        "unknown herbivore type");
}

std::auto_ptr<HftPopulationsMap> Simulator::create_populations(
    const HftList& hftlist) const {
  // instantiate the HftPopulationsMap object
  std::auto_ptr<HftPopulationsMap> pmap(new HftPopulationsMap());

  // Fill the object with one population per HFT
  for (int i = 0; i < hftlist.size(); i++) {
    const Hft* phft = &hftlist[i];
    pmap->add(create_population(phft));
  }
  assert(pmap.get() != NULL);
  assert(pmap->size() == hftlist.size());
  return pmap;
}

std::auto_ptr<HftPopulationsMap> Simulator::create_populations(
    const Hft* phft) const {
  // instantiate the HftPopulationsMap object
  std::auto_ptr<HftPopulationsMap> pmap(new HftPopulationsMap());
  // Fill the object with one population of one HFT
  pmap->add(create_population(phft));
  assert(pmap.get() != NULL);
  assert(pmap->size() == 1);
  return pmap;
}

void Simulator::simulate_day(const int day_of_year,
                             SimulationUnit& simulation_unit,
                             const bool do_herbivores) {
  if (day_of_year < 0 || day_of_year >= 365)
    throw std::invalid_argument(
        "Simulator::simulate_day(): "
        "Argument 'day_of_year' out of range");

  // If there was no initial establishment yet, we may do this now.
  bool establish_if_needed = !simulation_unit.is_initial_establishment_done();

  // If one check interval has passed, we will check if HFTs have died out
  // and need to be re-established.
  // Note that re-establishment is only activated if the interval length is
  // a positive number.
  if (days_since_last_establishment == params.herbivore_establish_interval &&
      params.herbivore_establish_interval > 0) {
    establish_if_needed = true;
    days_since_last_establishment = 0;
  }

  // Keep track of the establishment cycle.
  days_since_last_establishment++;

  // Create function object to delegate all simulations for this day to.
  SimulateDay simulate_day(day_of_year, simulation_unit, feed_herbivores);

  // Call the function object.
  simulate_day(do_herbivores, establish_if_needed);
}
