///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_digestibility.h"

#include "guess.h" // for Individual

#include <math.h> // for pow()

using namespace Fauna;

// ----------------------------------------------------------------------
// PftDigestibility
// ----------------------------------------------------------------------

double PftDigestibility::operator()(const Individual& individual) const{
	const double result = individual.pft.herbiv_params.digestibility;
	assert( result > 0.0 && result <= 1.0 );
	return result;
}
// ----------------------------------------------------------------------
// DigestibilityPachzelt2013
// ----------------------------------------------------------------------

const double DigestibilityPachzelt2013::DIG_DEAD_GRASS = 0.4;
const double DigestibilityPachzelt2013::DIG_LIVE_GRASS = 0.7;

double DigestibilityPachzelt2013::operator()(
		const Individual& indiv) const
{
	if (indiv.pft.lifeform != GRASS)
		throw std::invalid_argument(
				"Fauna::DigestibilityPachzelt2013::operator()() "
				"This digestibility model works only for grass, but a plant "
				"individual with non-grass PFT was given as parameter.");

	if (indiv.cmass_leaf <= 0.0)
		return DIG_DEAD_GRASS;

	// proportions of live and dead grass
	const double frac_live = indiv.get_average_phenology();
	const double frac_dead = 1.0 - frac_live;

	// current live grass density [kgDM/m²]
	const double dens_live = indiv.cmass_leaf * 2.0 * frac_live;

	// Crude protein content [fraction] after van Wijngaarden (1985), which
	// takes grass density in gDM/m².
	const double cpc = .20879 * pow((dens_live*1000.0), -0.1697);
	assert( cpc >= 0.0 );

	// live grass digestibility [fraction] after Bredon & Wilson (1963)
	double dig_live = 0.4605 + 1.4152*cpc;
	// keep value in boundaries:
	dig_live = min(dig_live, DIG_LIVE_GRASS);
	dig_live = max(dig_live, DIG_DEAD_GRASS);

	return dig_live * frac_live + DIG_DEAD_GRASS * frac_dead;
}
