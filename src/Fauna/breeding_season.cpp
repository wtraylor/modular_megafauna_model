/**
 * \file
 * \brief Determine erbivore breeding season.
 * \copyright ...
 * \date 2019
 */
#include "breeding_season.h"
#include <cassert>
#include <stdexcept>

using namespace Fauna;

BreedingSeason::BreedingSeason(const int breeding_season_start,
                               const int breeding_season_length)
    : start(breeding_season_start), length(breeding_season_length) {
  if (breeding_season_start < 0 || breeding_season_start >= 365)
    throw std::invalid_argument(
        "Fauna::BreedingSeason::BreedingSeason() "
        "Parameter breeding_season_start out of range.");
  if (breeding_season_length <= 0 || breeding_season_length > 365)
    throw std::invalid_argument(
        "Fauna::BreedingSeason::BreedingSeason() "
        "Parameter breeding_season_length out of range.");
}

bool BreedingSeason::is_in_season(const int day) const {
  if (day < 0 || day >= 365)
    throw std::invalid_argument(
        "Fauna::BreedingSeason::is_in_season() "
        "Parameter `day` is out of range.");

  // We are in breeding season if:
  // START ≤ day ≤ START+LENGTH ||
  // START ≤ day+365 ≤ START+LENGTH  // season extending over year boundary
  const int S = start;
  const int L = length;
  const int d = day;
  return (S <= d && d <= S + L) || (S <= d + 365 && d + 365 <= S + L);
}

double BreedingSeason::annual_to_daily_rate(const double annual) const {
  assert(length > 0);
  return annual / (double)length;
}
