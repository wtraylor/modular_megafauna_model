//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Classes to construct herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
//////////////////////////////////////////////////////////////////////////

#include "createherbivores.h"
#include <stdexcept>
#include "config.h"
#include "herbivore.h"
#include "hft.h"
#include "parameters.h"

using namespace Fauna;

//------------------------------------------------------------
//------ CreateHerbivoreCommon -------------------------------
//------------------------------------------------------------

CreateHerbivoreCommon::CreateHerbivoreCommon(const Hft* hft,
                                             const Parameters* parameters)
    : hft(hft), parameters(parameters) {
  if (hft == NULL)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCommon::CreateHerbivoreCommon() "
        "hft == NULL");
  if (parameters == NULL)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCommon::CreateHerbivoreCommon() "
        "parameters == NULL");
}

double CreateHerbivoreCommon::get_body_condition(const int age_days) const {
  double body_condition;
  if (age_days == 0)  // birth
    body_condition = get_hft().bodyfat_birth / get_hft().bodyfat_max;
  else                     // establishment
    body_condition = 1.0;  // full fat reserves
  assert(body_condition <= 1.0 && body_condition >= 0.0);
  return body_condition;
}

//------------------------------------------------------------
//------ CreateHerbivoreIndividual ---------------------------
//------------------------------------------------------------

double CreateHerbivoreIndividual::get_area_km2() const {
  return get_params().habitat_area_km2;
}

HerbivoreIndividual CreateHerbivoreIndividual::operator()(const int age_days,
                                                          Sex sex) const {
  if (age_days < 0)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreIndividual::operator()() "
        "age_days < 0");

  assert(get_area_km2() > 0.0);
  if (age_days == 0)
    // Call birth constructor
    return HerbivoreIndividual(&get_hft(), sex, get_area_km2());
  else
    // Call establishment constructor
    return HerbivoreIndividual(age_days, get_body_condition(age_days),
                               &get_hft(), sex, get_area_km2());
}

//------------------------------------------------------------
//------ CreateHerbivoreCohort -------------------------------
//------------------------------------------------------------

HerbivoreCohort CreateHerbivoreCohort::operator()(const double ind_per_km2,
                                                  const int age_years,
                                                  Sex sex) const {
  if (ind_per_km2 <= 0.0)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCohort::operator()() "
        "ind_per_km2 <= 0.0");
  if (age_years < 0)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCohort::operator()() "
        "age_years < 0");

  const int age_days = age_years * 365;
  if (age_days == 0)
    // Call birth constructor
    return HerbivoreCohort(&get_hft(), sex, ind_per_km2);
  else
    // Call establishment constructor
    return HerbivoreCohort(age_days, get_body_condition(age_days), &get_hft(),
                           sex, ind_per_km2);
}
