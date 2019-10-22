/**
 * \file
 * \brief Central management of all herbivores and habitats.
 * \copyright ...
 * \date 2019
 */
#include "world.h"
#include <algorithm>
#include <stdexcept>
#include "aggregator.h"
#include "date.h"
#include "feed_herbivores.h"
#include "habitat.h"
#include "hft.h"
#include "insfile_reader.h"
#include "parameters.h"
#include "population_interface.h"
#include "population_list.h"
#include "simulate_day.h"
#include "simulation_unit.h"
#include "text_table_writer.h"
#include "world_constructor.h"

using namespace Fauna;

namespace {
/// Helper function to initialize Fauna::InsfileContent object.
InsfileContent* read_instruction_file(const std::string& filename) {
  try {
    InsfileReader reader(filename);
    return new InsfileContent({reader.get_hfts(), reader.get_params()});
  } catch (std::runtime_error& err) {
    throw std::runtime_error("Error reading instruction file \"" + filename +
                             "\":\n" + err.what());
  }
}
}  // namespace

World::World(const std::string instruction_filename)
    : insfile_content(read_instruction_file(instruction_filename)),
      days_since_last_establishment(get_params().herbivore_establish_interval),
      world_constructor(new WorldConstructor(get_params(), get_hfts())),
      output_aggregator(new Output::Aggregator()) {
  // Create Output::WriterInterface implementation according to selected
  // setting.
  switch (get_params().output_format) {
    case OutputFormat::TextTables:
      output_writer.reset(new Output::TextTableWriter(
          get_params().output_interval, get_params().text_table_output));
      break;
    default:
      std::logic_error(
          "Fauna::World::World() "
          "Selected output format parameter is not implemented.");
  }
}

// The destructor must be implemented here in the source file, where the
// forward-declared types are complete.
World::~World() = default;

void World::create_simulation_unit(Habitat* habitat) {
  if (habitat == NULL)
    throw std::invalid_argument(
        "World::create_simulation_unit(): Pointer to habitat is NULL.");

  PopulationList* populations = new PopulationList();

  // Fill the object with one population per HFT.
  for (int i = 0; i < get_hfts().size(); i++) {
    const Hft* phft = &get_hfts()[i];
    populations->add(world_constructor->create_population(phft));
  }
  assert(populations != NULL);

  // Use emplace_back() instead of push_back() to directly construct the new
  // SimulationUnit object without copy.
  sim_units.emplace_back(habitat, populations);
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
  // Create one function object to feed all herbivores.
  const FeedHerbivores feed_herbivores(
      world_constructor->create_distribute_forage());

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
    if (days_since_last_establishment >=
            get_params().herbivore_establish_interval &&
        get_params().herbivore_establish_interval > 0) {
      establish_if_needed = true;
      days_since_last_establishment = 0;
    }

    // Keep track of the establishment cycle.
    days_since_last_establishment++;

    // Establish each HFT if it got extinct.
    if (establish_if_needed)
      for (const auto& hft : get_hfts()) {
        PopulationList& pops = sim_unit.get_populations();
        if (!pops.exists(hft))
          pops.add(world_constructor->create_population(&hft));

        PopulationInterface& p = pops.get(hft);
        if (p.get_list().empty()) pops.get(hft).establish();
      }

    // Create function object to delegate all simulations for this day to.
    // TODO: Create function object only once per day and for all simulation
    //       units.
    SimulateDay simulate_day(date.get_julian_day(), sim_unit, feed_herbivores);

    // Call the function object.
    simulate_day(do_herbivores);

    // Aggregate output.
    assert(output_aggregator.get() != NULL);
    output_aggregator->add(date, sim_unit.get_habitat().get_aggregation_unit(),
                           sim_unit.get_output());

    iter++;
  }

  // Write output when it’s ready.
  assert(output_writer.get() != NULL);
  if (output_aggregator->get_interval().matches_output_interval(
          get_params().output_interval))
    for (const auto& datapoint : output_aggregator->retrieve())
      output_writer->write_datapoint(datapoint);
}
