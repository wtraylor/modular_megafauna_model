//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simulation unit: a habitat + herbivore populations.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////

#include "simulation_unit.h"
#include "habitat.h"
#include "herbivore.h"
#include "population.h"
#include "population_list.h"

using namespace Fauna;

SimulationUnit::SimulationUnit(Habitat* habitat, PopulationList* populations)
    :  // move ownership to private unique_ptr objects
      habitat(habitat),
      populations(populations),
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

Output::CombinedData SimulationUnit::get_output() const{
  Output::CombinedData result;

  // HERBIVORES
  std::map<const Hft*, std::vector<Output::HerbivoreData> > hft_output;

  for (const auto& herbivore : get_populations().get_all_herbivores()) {
    const Hft* hft = &herbivore->get_hft();
    hft_output[hft].push_back(herbivore->get_todays_output());
  }

  for (auto& itr : hft_output) {
    const Hft* hft = itr.first;
    const std::vector<Output::HerbivoreData>& vector = itr.second;
    result.hft_data[hft] = Output::HerbivoreData::create_datapoint(vector);
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
