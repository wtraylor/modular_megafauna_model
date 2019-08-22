//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Classes for the spatial units where herbivores live.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
//////////////////////////////////////////////////////////////////////////
#include "habitat.h"

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
