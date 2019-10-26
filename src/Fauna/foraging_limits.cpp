/**
 * \file
 * \brief Different models how to restrict daily forage intake of herbivores.
 * \copyright ...
 * \date 2019
 */
#include "foraging_limits.h"

using namespace Fauna;

//============================================================
// GetDigestiveLimitIlliusGordon1992
//============================================================

GetDigestiveLimitIlliusGordon1992::GetDigestiveLimitIlliusGordon1992(
    const double bodymass_adult, const DigestionType digestion_type)
    : bodymass_adult(bodymass_adult), digestion_type(digestion_type) {
  if (bodymass_adult <= 0.0)
    throw std::invalid_argument(
        "Fauna::GetDigestiveLimitIlliusGordon1992::"
        "GetDigestiveLimitIlliusGordon1992() "
        "Parameter `bodymass_adult` <= zero.");
}

const ForageEnergy GetDigestiveLimitIlliusGordon1992::operator()(
    const double bodymass, const Digestibility& digestibility) const {
  if (bodymass <= 0.0)
    throw std::invalid_argument(
        "Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
        "Parameter `bodymass` <= zero.");
  if (bodymass > bodymass_adult)
    throw std::invalid_argument(
        "Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
        "bodymass > bodymass_adult");

  typedef ForageValues<ForageValueTag::PositiveAndZero> ParameterConstant;

  ParameterConstant i, j, k;

  // Initialize constants
  bool initialized = false;
  if (!initialized) {
    if (FORAGE_TYPES.size() > 1)  // change this if adding new forage types
      throw std::logic_error(
          "Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
          "Not all forage types are implemented.");

    if (digestion_type == DigestionType::Ruminant) {
      i.set(FT_GRASS, 0.034);
      j.set(FT_GRASS, 3.565);
      k.set(FT_GRASS, 0.077);
      // ADD NEW FORAGE TYPES HERE
    } else if (digestion_type == DigestionType::Hindgut) {
      i.set(FT_GRASS, 0.108);
      j.set(FT_GRASS, 3.284);
      k.set(FT_GRASS, 0.080);
      // ADD NEW FORAGE TYPES HERE
    } else
      throw std::logic_error(
          "Fauna::GetDigestiveLimitIlliusGordon1992::operator()() "
          "Digestion type not implemented.");
    initialized = true;
  }

  ForageEnergy result;

  // rename variables to match formula
  const Digestibility& d = digestibility;
  const double& M_ad = bodymass_adult;  // [kg]
  const double& M = bodymass;           // [kg]
  const double u_g = pow(M / M_ad, .75);

  // Because of power calculations we cannot use the
  // arithmetic operators of ForageValues<>, but need to
  // iterate over all forage types.
  for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
       ft != FORAGE_TYPES.end(); ft++) {
    const ForageType f = *ft;

    // Only for the supported forage types, the result is calculated.
    // ADD NEW FORAGE TYPES HERE IN IF QUERY
    if ((f == FT_GRASS) && digestibility[f] > 0.0)
      result.set(
          f, i[f] * exp(j[f] * d[f]) * pow(M_ad, k[f] * exp(d[f]) + .73) * u_g);
    else
      result.set(f, 0.0);  // zero digestibility -> zero energy
  }

  return result;
}

//============================================================
// HalfMaxIntake
//============================================================

HalfMaxIntake::HalfMaxIntake(const double half_max_density,
                             const double max_intake)
    : half_max_density(half_max_density), max_intake(max_intake) {
  if (!(half_max_density > 0.0))
    throw std::invalid_argument(
        "Fauna::HalfMaxIntake::HalfMaxIntake() "
        "Parameter `half_max_density` is not a positive number.");
  if (!(max_intake > 0.0))
    throw std::invalid_argument(
        "Fauna::HalfMaxIntake::HalfMaxIntake() "
        "Parameter `max_intake` is not a positive number.");
}

double HalfMaxIntake::get_intake_rate(const double density) const {
  if (!(density >= 0.0))
    throw std::invalid_argument(
        "Fauna::HalfMaxIntake::get_intake_rate() "
        "Parameter `density` must be zero or a positive number.");
  return max_intake * density / (half_max_density + density);
}
