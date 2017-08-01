///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Reproduction of herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_reproduction.h"
#include <cassert> // for assert()
#include <cmath>   // for exp()
#include <stdexcept>

using namespace Fauna;

ReproductionIllius2000::ReproductionIllius2000( 
		const int breeding_season_start,
		const int breeding_season_length,
		const double max_annual_increase):
	max_annual_increase(max_annual_increase),
	breeding_start(breeding_season_start),
	breeding_length(breeding_season_length)
{
	if (breeding_season_start < 0 || breeding_season_start >= 365)
		throw std::invalid_argument("Fauna::ReproductionIllius2000::ReproductionIllius2000() "
				"breeding_season_start out of range.");
	if (breeding_season_length <= 0 || breeding_season_length > 365)
		throw std::invalid_argument("Fauna::ReproductionIllius2000::ReproductionIllius2000() "
				"breeding_season_length out of range.");
	if (max_annual_increase < 0.0)
		throw std::invalid_argument("Fauna::ReproductionIllius2000::ReproductionIllius2000() "
				"max_annual_increase below zero.");
}

double ReproductionIllius2000::get_offspring_density(
		const int day_of_year,
		const double body_condition)const{
	if (day_of_year < 0 || day_of_year >= 365)
		throw std::invalid_argument("Fauna::ReproductionIllius2000::get_offspring_density() "
				"day_of_year is out of range.");
	if (body_condition < 0.0 || body_condition > 1.0)
		throw std::invalid_argument("Fauna::ReproductionIllius2000::get_offspring_density() "
				"body_condition is out of range.");
	assert(breeding_start >= 0 && breeding_start < 365);
	assert(breeding_length > 0 && breeding_length <= 365);

	// Check if we are out of breeding season
	// We are in breeding season if: 
	// START ≤ day ≤ START+LENGTH || 
	// START ≤ day+365 ≤ START+LENGTH  // season extending over year boundary
	const int S=breeding_start;
	const int L=breeding_length;
	const int d=day_of_year;
	if ( !((S<=d && d<=S+L) || (S<=d+365 && d+365<=S+L)) )
		return 0.0;

	// Yes, we are in breeding season and just apply the formula.
	static const double b = 15.0;
	static const double c = 0.3;
	const double k = max_annual_increase;
	
	// see doxygen documentation of the class for formula
	// explanation
	
	// annual rate
	const double annual = ( k / (1.0 + exp(-b*(body_condition-c))));
	assert(annual <= max_annual_increase);
	assert(annual >= 0.0);

	// daily rate for each day in breeding season
	assert(breeding_length > 0);
	const double daily = annual / (double)breeding_length;

	assert(daily >= 0.0);

	return daily;
}
