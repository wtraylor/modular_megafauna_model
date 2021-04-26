// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Central management of all herbivores and habitats.
 * \copyright LGPL-3.0-or-later
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

World::World(const std::string instruction_filename, const SimMode mode)
    : activated(true),
      insfile(read_instruction_file(instruction_filename)),
      days_since_last_establishment(get_params().herbivore_establish_interval),
      world_constructor(new WorldConstructor(insfile.params, get_hfts())),
      output_aggregator(new Output::Aggregator()) {
  if (mode == SimMode::Simulate) {
    // Create Output::WriterInterface implementation according to selected
    // setting.
    switch (get_params().output_format) {
      case OutputFormat::TextTables: {
        std::set<std::string> hft_names;
        for (const auto& h : get_hfts()) hft_names.insert(h->name);
        output_writer.reset(new Output::TextTableWriter(
            get_params().output_interval, get_params().output_text_tables,
            hft_names));
        break;
      }
      default:
        std::logic_error(
            "Fauna::World::World() "
            "Selected output format parameter is not implemented.");
    }
  }
}

World::World() : activated(false) {}

// The destructor must be implemented here in the source file, where the
// forward-declared types are complete.
World::~World() = default;

void World::create_simulation_unit(std::shared_ptr<Habitat> habitat) {
  if (habitat == NULL)
    throw std::invalid_argument(
        "World::create_simulation_unit(): Pointer to habitat is NULL.");
  if (!activated) return;

  PopulationList* populations = world_constructor->create_populations();
  assert(populations);

  // Use emplace_back() instead of push_back() to directly construct the new
  // SimulationUnit object without copy.
  sim_units.emplace_back(habitat, populations);
}

const HftList& World::get_hfts() {
  assert(insfile.hftlist.get());
  return *(insfile.hftlist);
}

const Parameters& World::get_params() const {
  if (!activated)
    throw std::logic_error(
        "Fauna::World::get_params() "
        "The megafauna model was created without an instruction file. "
        "Parameters are not available.");
  if (!insfile.params)
    throw std::logic_error(
        "Fauna::World::get_params() "
        "The member variable insfile.params is not set.");
  assert(insfile.params.get());
  return *(insfile.params);
}

World::InsfileContent World::read_instruction_file(
    const std::string& filename) {
  try {
    InsfileReader reader(filename);
    return World::InsfileContent(
        {std::make_shared<const HftList>(reader.get_hfts()),
         std::make_shared<const Parameters>(reader.get_params())});
  } catch (std::runtime_error& err) {
    throw std::runtime_error("Error reading instruction file \"" + filename +
                             "\":\n" + err.what());
  }
}

void World::simulate_day(const Date& date, const bool do_herbivores) {
  if (!activated) return;
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

    // Whether herbivores shall be (re-)established today.
    bool establish_as_needed = false;

    // If there was no initial establishment yet, we may do this now.
    if (!sim_unit.is_initial_establishment_done()) establish_as_needed = true;

    // If one check interval has passed, we will check if HFTs have died out
    // and need to be re-established.
    // Note that re-establishment is only activated if the interval length is
    // a positive number.
    if (days_since_last_establishment >=
            get_params().herbivore_establish_interval &&
        get_params().herbivore_establish_interval > 0) {
      establish_as_needed = true;
      days_since_last_establishment = 0;
    }

    // Keep track of the establishment cycle.
    if (do_herbivores) days_since_last_establishment++;

    // Create function object to delegate all simulations for this day to.
    // TODO: Create function object only once per day and for all simulation
    //       units.
    SimulateDay simulate_day(date.get_julian_day(), sim_unit, feed_herbivores);

    // Call the function object.
    simulate_day(do_herbivores, establish_as_needed);

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
