//////////////////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
//////////////////////////////////////////////////////////////////////////

#include "digestibility.h"
#include "config.h"

#include "guess.h"  // for Individual

#include <math.h>  // for pow()

using namespace Fauna;

// ----------------------------------------------------------------------
// PftDigestibility
// ----------------------------------------------------------------------

double PftDigestibility::operator()(const Individual& individual) const {
  const double result = individual.pft.herbiv_params.digestibility;
  assert(result > 0.0 && result <= 1.0);
  return result;
}
// ----------------------------------------------------------------------
// DigestibilityPachzelt2013
// ----------------------------------------------------------------------

const double DigestibilityPachzelt2013::DIG_DEAD_GRASS = 0.4;
const double DigestibilityPachzelt2013::DIG_LIVE_GRASS = 0.7;

double DigestibilityPachzelt2013::operator()(const Individual& indiv) const {
  if (indiv.pft.lifeform != GRASS)
    throw std::invalid_argument(
        "Fauna::DigestibilityPachzelt2013::operator()() "
        "This digestibility model works only for grass, but a plant "
        "individual with non-grass PFT was given as parameter.");

  if (indiv.cmass_leaf <= 0.0) return DIG_DEAD_GRASS;

  // proportions of live and dead grass
  const double frac_live = indiv.get_average_phenology();
  const double frac_dead = 1.0 - frac_live;

  // current live grass density [kgDM/m²]
  const double dens_live = indiv.cmass_leaf * 2.0 * frac_live;

  // Crude protein content [fraction] after van Wijngaarden (1985), which
  // takes grass density in gDM/m².
  const double cpc = .20879 * pow((dens_live * 1000.0), -0.1697);
  assert(cpc >= 0.0);

  // live grass digestibility [fraction] after Bredon & Wilson (1963)
  double dig_live = 0.4605 + 1.4152 * cpc;
  // keep value in boundaries:
  dig_live = min(dig_live, DIG_LIVE_GRASS);
  dig_live = max(dig_live, DIG_DEAD_GRASS);

  return dig_live * frac_live + DIG_DEAD_GRASS * frac_dead;
}

// ----------------------------------------------------------------------
// DigestibilityFromNPP
// ----------------------------------------------------------------------

double DigestibilityFromNPP::operator()(const Individual& indiv) const {
  return get_digestibility_from_dnpp(
      indiv.get_dnpp_record(),
      indiv.pft.herbiv_params.digestibility,        // fresh
      indiv.pft.herbiv_params.digestibility_dead);  // dead
}

double DigestibilityFromNPP::get_digestibility_from_dnpp(
    const std::deque<double>& weights, const double dig_fresh,
    const double dig_dead) {
  if (dig_dead > dig_fresh)
    throw std::invalid_argument(
        "Fauna::DigestibilityFromNPP::get_digestibility_from_dnpp() "
        "Digestibility for dead forage must not be greater than for fresh "
        "forage.");
  if (dig_fresh < 0.0 || dig_fresh > 1.0)
    throw std::invalid_argument(
        "Fauna::DigestibilityFromNPP::get_digestibility_from_dnpp() "
        "Parameter `dig_fresh` out of range.");
  if (dig_dead < 0.0 || dig_dead > 1.0)
    throw std::invalid_argument(
        "Fauna::DigestibilityFromNPP::get_digestibility_from_dnpp() "
        "Parameter `dig_dead` out of range.");

  if (weights.size() == 0) return 0.0;

  double result = 0.0;    // digestibility value being calculated
  int count = 0;          // index of current data point (= age in days)
  double dnpp_sum = 0.0;  // sum of all daily NPP values in the result

  // Iterate through all daily NPP values.
  for (std::deque<double>::const_iterator itr = weights.begin();
       itr != weights.end() && count < ATTRITION_PERIOD; itr++) {
    if (*itr < 0.0)
      throw std::invalid_argument(
          "Fauna::DigestibilityFromNPP::get_digestibility_from_dnpp() "
          "One entry in parameter `weights` is negative.");

    // Digestibility of the forage that has been produced at the day in the
    // record that is `count` days in the past.
    // We assume a linear decreas here from fresh to dead over the
    // “attrition period”.
    const double dig =
        dig_fresh - (dig_fresh - dig_dead) * (double)count / ATTRITION_PERIOD;

    // Build sum of products of digestibility and NPP (weight).
    result += dig * *itr;

    dnpp_sum += *itr;
    count++;
  }

  // Divide the weighted sum by the sum of weights to obtain the weighted
  // mean.
  result = result / dnpp_sum;

  // Make sure that the result doesn’t drop below minimum: in case all
  // weights are zero.
  result = max(dig_dead, result);

  assert(result >= dig_dead);
  assert(result <= dig_fresh);

  return result;
}
