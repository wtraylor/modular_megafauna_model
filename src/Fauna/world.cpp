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

Output::WriterInterface* World::construct_output_writer() const {
  switch (get_params().output_format) {
    case OutputFormat::TextTables: {
      std::set<std::string> hft_names;
      for (const auto& h : get_hfts()) hft_names.insert(h->name);
      return new Output::TextTableWriter(get_params().output_interval,
                                         get_params().output_text_tables,
                                         hft_names);
    }
      // Construct your new output writer here.
    default:
      throw std::logic_error(
          "Fauna::World::World() "
          "Selected output format parameter is not implemented.");
  }
}

World::World(const std::string instruction_filename, const SimMode mode)
    : activated(true),
      insfile(read_instruction_file(instruction_filename)),
      days_since_last_establishment(get_params().herbivore_establish_interval),
      output_aggregator(new Output::Aggregator()),
      output_writer(mode == SimMode::Lint ? NULL : construct_output_writer()),
      world_constructor(new WorldConstructor(insfile.params, get_hfts())) {}

World::World() : activated(false) {}

World::World(const std::shared_ptr<const Parameters> params,
             const std::shared_ptr<const HftList> hftlist)
    : activated(true),
      insfile({hftlist, params}),
      days_since_last_establishment(get_params().herbivore_establish_interval),
      output_aggregator(new Output::Aggregator()),
      output_writer(construct_output_writer()),
      world_constructor(new WorldConstructor(insfile.params, get_hfts())) {}

// The destructor must be implemented here in the source file, where the
// forward-declared types are complete.
World::~World() = default;

void World::create_simulation_unit(std::shared_ptr<Habitat> habitat) {
  if (habitat == NULL)
    throw std::invalid_argument(
        "World::create_simulation_unit(): Pointer to habitat is NULL.");
  if (!activated) return;

  int habitat_ctr = 0;
  // Find the number of habitats already created in this aggregation unit.
  const std::string this_agg_unit(habitat->get_aggregation_unit());
  for (const auto& sim_unit : sim_units) {
    const std::string existing_agg_unit(
        sim_unit.get_habitat().get_aggregation_unit());
    if (this_agg_unit == existing_agg_unit) habitat_ctr++;
  }
  PopulationList* populations =
      world_constructor->create_populations(habitat_ctr);
  assert(populations);

  // Use emplace_back() instead of push_back() to directly construct the new
  // SimulationUnit object without copy.
  sim_units.emplace_back(habitat, populations);

  simulation_units_checked = false;
}

const HftList& World::get_hfts() const {
  if (!insfile.hftlist)
    throw std::logic_error(
        "Fauna::World::get_hfts() "
        "The member variable insfile.hftlist is not set.");
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

int World::get_habitat_count_per_agg_unit() const {
  // Habitat count for each aggregation unit.
  std::unordered_map<std::string, int> hab_counts;
  for (const auto& sim_unit : get_sim_units()) {
    auto& map_entry = hab_counts[sim_unit.get_habitat().get_aggregation_unit()];
    if (map_entry == 0)
      map_entry = 1;
    else
      map_entry++;
  }

  if (hab_counts.empty()) return 0;

  // Use the first habitat count as (preliminary) result.
  const int result = hab_counts.begin()->second;

  // Check that they all have the same habitat count.
  std::string msg;
  bool counts_differ = false;
  for (const auto& itr : hab_counts) {
    const std::string& agg_unit = itr.first;
    const int count = itr.second;
    if (count != result) {
      counts_differ = true;
      msg += "\t\"" + agg_unit + "\": " + std::to_string(count) + " habitats\n";
    }
  }
  if (counts_differ)
    throw std::logic_error(
        "Fauna::World::get_habitat_count_per_agg_unit() "
        "The number of habitats is not the same in all aggregation units."
        "These are the aggregation units that differ from the expected number "
        "of habitats (" +
        std::to_string(result) + "):\n" + msg);
  return result;
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

  // Sanity checks for simulation units
  if (!simulation_units_checked) {
    // Count the number of habitats in any case to throw an exception if they
    // differ. Compare the habitat count against HFT count only if necessary.
    const int habitat_count = get_habitat_count_per_agg_unit();
    if (get_params().one_hft_per_habitat &&
        (get_params().herbivore_type == HerbivoreType::Cohort) &&
        (habitat_count % get_hfts().size() != 0))
      throw std::logic_error(
          "Fauna::World::simulate_day() "
          "If simulation.one_hft_per_habitat == true, the number of habitats "
          "in each aggregation unit must be a multiple of HFT count. I found " +
          std::to_string(habitat_count) +
          " habitats per aggregation unit, and there are " +
          std::to_string(get_hfts().size()) + " HFTs.");
  }
  simulation_units_checked = true;

  // Check if `date` follows `last_date`, but only if `last_date` has already
  // been initialized (which happens on the first call.
  if (last_date && !last_date->is_successive(date))
    throw std::invalid_argument(
        "Fauna::World::simulate_day() "
        "Simulation dates did not come in consecutively.\n"
        "In the last call I received Julian day " +
        std::to_string(last_date->get_julian_day()) + " in year " +
        std::to_string(last_date->get_year()) + ".\n" +
        "Now I received Julian day " + std::to_string(date.get_julian_day()) +
        " in year " + std::to_string(date.get_year()));

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
  if (!sim_units.empty() &&
      output_aggregator->get_interval().matches_output_interval(
          get_params().output_interval))
    for (const auto& datapoint : output_aggregator->retrieve())
      output_writer->write_datapoint(datapoint);

  last_date.reset(new Date(date));
}
