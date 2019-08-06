#include "world.h"
#include <stdexcept>
#include "hft.h"
#include "population.h"
#include "read_insfile.h"
#include "simulate_day.h"
#include "simulation_unit.h"

using namespace Fauna;

World::World(const std::string instruction_filename)
    : insfile_content(read_instruction_file(instruction_filename)),
      days_since_last_establishment(params.herbivore_establish_interval) {}

void World::create_simulation_unit(std::auto_ptr<Habitat> habitat) {
  std::auto_ptr<HftPopulationsMap> pmap(new HftPopulationsMap());

  // Fill the object with one population per HFT.
  for (int i = 0; i < hftlist.size(); i++) {
    const Hft* phft = &hftlist[i];
    pmap->add(create_population(phft));
  }
  assert(pmap.get() != NULL);
  assert(pmap->size() == hftlist.size());

  sim_units.push_back(SimulationUnit(habitat, pmap));
}

const HftList& World::get_hfts() {
  if (insfile_content.hftlist.get() == NULL)
    throw std::logic_error(
        "World::get_params(): The member insfile_content::hftlist is not set.");
  return *insfile_content.hftlist.get();
}

const Parameters& World::get_params() {
  if (insfile_content.params.get() == NULL)
    throw std::logic_error(
        "World::get_params(): The member insfile_content::params is not set.");
  return *insfile_content.params.get();
}

void World::simulate_day(const int day_of_year, const bool do_herbivores) {
  if (day_of_year < 0 || day_of_year >= 365)
    throw std::invalid_argument(
        "Fauna::World::simulate_day(): Argument 'day_of_year' out of range");
  for (auto sim_unit : sim_units) {
    // If there was no initial establishment yet, we may do this now.
    bool establish_if_needed = !simulation_unit.is_initial_establishment_done();

    // If one check interval has passed, we will check if HFTs have died out
    // and need to be re-established.
    // Note that re-establishment is only activated if the interval length is
    // a positive number.
    if (days_since_last_establishment ==
            get_params().herbivore_establish_interval &&
        get_params().herbivore_establish_interval > 0) {
      establish_if_needed = true;
      days_since_last_establishment = 0;
    }

    // Keep track of the establishment cycle.
    days_since_last_establishment++;

    // Create function object to delegate all simulations for this day to.
    // TODO: Create function object only once per day and for all simulation
    //       units.
    SimulateDay simulate_day(day_of_year, simulation_unit,
                             FeedHerbivores(create_distribute_forage()));

    // Call the function object.
    simulate_day(do_herbivores, establish_if_needed);
  }
}
