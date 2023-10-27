// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Simulation unit: a habitat + herbivore populations.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#include "simulation_unit.h"

#include "combined_data.h"
#include "habitat.h"
#include "herbivore_interface.h"
#include "population_interface.h"

using namespace Fauna;

SimulationUnit::SimulationUnit(std::shared_ptr<Habitat> habitat,
                               PopulationList* populations)
    : habitat(habitat),
      populations(populations),  // move ownership to unique_ptr object
      initial_establishment_done(false) {
  if (habitat == NULL)
    throw std::invalid_argument(
        "Fauna::SimulationUnit::SimulationUnit() Pointer to habitat is NULL.");
  if (populations == NULL)
    throw std::invalid_argument(
        "Fauna::SimulationUnit::SimulationUnit() "
        "Pointer to populations is NULL.");
}

// The destructor needs to be implemented here in the source file and not
// inline in the header file. The reason is that std::unique_ptr needs to call
// the destructor of Fauna::Habitat when it is itself released. But the
// destructor of Fauna::Habitat is incomplete at compile time.
SimulationUnit::~SimulationUnit() = default;

Habitat& SimulationUnit::get_habitat() {
  if (habitat.get() == NULL)
    throw std::logic_error(
        "Fauna::SimulationUnit::get_habitat() "
        "The unique pointer to habitat is NULL. "
        "The SimulationUnit object lost ownership "
        "of the Habitat object.");
  return *habitat;
};

const Habitat& SimulationUnit::get_habitat() const {
  if (habitat.get() == NULL)
    throw std::logic_error(
        "Fauna::SimulationUnit::get_habitat() "
        "The unique pointer to habitat is NULL. "
        "The SimulationUnit object lost ownership "
        "of the Habitat object.");
  return *habitat;
};

Output::CombinedData SimulationUnit::get_output() const {
  Output::CombinedData result;

  // HERBIVORES
  std::map<const std::string, std::vector<Output::HerbivoreData> > hft_output;

  for (auto& pop : get_populations())
    for (auto& herbivore : pop->get_list()) {
      hft_output[herbivore->get_output_group()].push_back(
          herbivore->get_todays_output());
    }

  for (auto& itr : hft_output) {
    const std::vector<Output::HerbivoreData>& vector = itr.second;
    result.hft_data[itr.first] =
        Output::HerbivoreData::create_datapoint(vector);
  }

  // HABITAT
  const Habitat& habitat = get_habitat();
  result.habitat_data = habitat.get_todays_output();

  // The output data container is now complete for today.
  result.datapoint_count = 1;

  return result;
}

PopulationList& SimulationUnit::get_populations() {
  if (populations.get() == NULL)
    throw std::logic_error(
        "Fauna::SimulationUnit::get_populations() "
        "The unique pointer to populations is NULL. "
        "The SimulationUnit object lost ownership "
        "of the PopulationList object.");
  return *populations;
}

const PopulationList& SimulationUnit::get_populations() const {
  if (populations.get() == NULL)
    throw std::logic_error(
        "Fauna::SimulationUnit::get_populations() "
        "The unique pointer to populations is NULL. "
        "The SimulationUnit object lost ownership "
        "of the PopulationList object.");
  return *populations;
}
