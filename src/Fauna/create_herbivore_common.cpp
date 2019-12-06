/**
 * \file
 * \brief Base class for creating herbivore cohorts & individuals.
 * \copyright ...
 * \date 2019
 */
#include "create_herbivore_common.h"
#include "hft.h"
#include "parameters.h"

using namespace Fauna;

CreateHerbivoreCommon::CreateHerbivoreCommon(
    const std::shared_ptr<const Hft> hft,
    const std::shared_ptr<const Parameters> parameters)
    : hft(hft), parameters(std::move(parameters)) {
  if (hft.get() == NULL)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCommon::CreateHerbivoreCommon() "
        "hft == NULL");
  if (parameters.get() == NULL)
    throw std::invalid_argument(
        "Fauna::CreateHerbivoreCommon::CreateHerbivoreCommon() "
        "parameters == NULL");
}

double CreateHerbivoreCommon::get_body_condition(const int age_days) const {
  double body_condition;
  if (age_days == 0)  // birth
    body_condition = get_hft().body_fat_birth / get_hft().body_fat_maximum;
  else                     // establishment
    body_condition = 1.0;  // full fat reserves
  assert(body_condition <= 1.0 && body_condition >= 0.0);
  return body_condition;
}

const Hft& CreateHerbivoreCommon::get_hft() const {
  assert(hft != NULL);
  return *hft;
}

/// Global simulation parameters.
const Parameters& CreateHerbivoreCommon::get_params() const {
  assert(parameters != NULL);
  return *parameters;
}
