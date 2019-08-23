#include "world.h"
#include <algorithm>
#include <stdexcept>
#include "date.h"
#include "feed.h"
#include "habitat.h"
#include "hft.h"
#include "parameters.h"
#include "population.h"
#include "read_insfile.h"
#include "simulate_day.h"
#include "simulation_unit.h"
#include "world_constructor.h"

using namespace Fauna;

World::World(const std::string instruction_filename)
    : insfile_content(
          new InsfileContent(read_instruction_file(instruction_filename))),
      days_since_last_establishment(get_params().herbivore_establish_interval),
      world_constructor(new WorldConstructor(get_params(), get_hfts())) {}

// The destructor must be implemented here in the source file, where the
// forward-declared types are complete.
World::~World() = default;

void World::create_simulation_unit(Habitat* habitat) {
  if (habitat == NULL)
    throw std::invalid_argument(
        "World::create_simulation_unit(): Pointer to habitat is NULL.");

  HftPopulationsMap* pmap(new HftPopulationsMap());

  // Fill the object with one population per HFT.
  for (int i = 0; i < get_hfts().size(); i++) {
    const Hft* phft = &get_hfts()[i];
    pmap->add(world_constructor->create_population(phft));
  }
  assert(pmap != NULL);
  assert(pmap->size() == get_hfts().size());

  // Use emplace_back() instead of push_back() to directly construct the new
  // SimulationUnit object without copy.
  sim_units.emplace_back(habitat, pmap);
}

const HftList& World::get_hfts() {
  if (insfile_content.get() == NULL)
    throw std::logic_error(
        "World::get_hfts(): The member insfile_content is not set.");
  return insfile_content->hftlist;
}

const Parameters& World::get_params() const {
  if (insfile_content.get() == NULL)
    throw std::logic_error(
        "World::get_params(): The member insfile_content is not set.");
  return insfile_content->params;
}

void World::simulate_day(const Date& date, const bool do_herbivores) {
  // We use an iterator in order to be able to call std::list::erase().
  for (auto iter = sim_units.begin(); iter != sim_units.end();) {
    SimulationUnit& sim_unit = *iter;

    // Remove invalid simulation units immediately.
    if (sim_unit.get_habitat().is_dead()) {
      iter = sim_units.erase(iter);
      continue;
    }

    // If there was no initial establishment yet, we may do this now.
    bool establish_if_needed = !sim_unit.is_initial_establishment_done();

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
    SimulateDay simulate_day(
        date.get_julian_day(), sim_unit,
        FeedHerbivores(world_constructor->create_distribute_forage()));

    // Call the function object.
    simulate_day(do_herbivores, establish_if_needed);

    iter++;
  }
}
