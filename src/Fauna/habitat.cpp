/**
 * \file
 * \brief The spatial unit where herbivores live in.
 * \copyright ...
 * \date 2019
 */
#include "habitat.h"
#include "habitat_forage.h"

using namespace Fauna;

void Habitat::init_day(const int today) {
  if (is_dead())
    throw std::logic_error("Fauna::Habitat::init_day() The object is dead.");
  if (today < 0 || today >= 365)
    throw std::invalid_argument(
        "Fauna::Habitat::init_day() "
        "Parameter \"today\" is out of range.");
  day_of_year = today;

  // Initialize new output.
  get_todays_output().reset();
  get_todays_output().available_forage = get_available_forage();
  get_todays_output().environment = get_environment();
}
