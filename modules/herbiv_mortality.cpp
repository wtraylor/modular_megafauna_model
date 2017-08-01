//////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivore mortality factor implementations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
/////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_mortality.h"
#include <cassert> // for assert()
#include <cmath>   // for pow()
#include <stdexcept>

using namespace Fauna;

namespace {
	/// Convert a yearly proportional rate to a daily one.
	double annual_to_daily_rate(const double annual_rate){
		return 1.0 - pow(1.0 - annual_rate, 1.0/365.0);
	}
}

GetBackgroundMortality::GetBackgroundMortality(
		const double annual_mortality_1st_year,
		const double annual_mortality):
annual_mortality_1st_year(annual_mortality_1st_year),
annual_mortality(annual_mortality){

	if (annual_mortality_1st_year >= 1.0 ||
			annual_mortality_1st_year < 0.0)
		throw std::invalid_argument("Fauna::GetBackgroundMortality::GetBackgroundMortality() "
				"annual_mortality_1st_year out of range [0,1]");

	if (annual_mortality >= 1.0 ||
			annual_mortality< 0.0)
		throw std::invalid_argument("Fauna::GetBackgroundMortality::GetBackgroundMortality() "
				"annual_mortality out of range [0,1]");

}

double GetBackgroundMortality::operator()(const int age_days)const{
	if (age_days < 0)
		throw std::invalid_argument("Fauna::GetBackgroundMortality::GetBackgroundMortality() "
				"age_days < 0");

	if (age_days < 365) // first year
		return annual_to_daily_rate(annual_mortality_1st_year);
	else
		return annual_to_daily_rate(annual_mortality);
}

//------------------------------------------------------------

GetSimpleLifespanMortality::GetSimpleLifespanMortality(
		const int lifespan_years):
	lifespan_years(lifespan_years)
{
	if (lifespan_years <= 0)
		throw std::invalid_argument("Fauna::GetSimpleLifespanMortality::GetSimpleLifespanMortality() "
				"lifespan_years <= 0");
}

//------------------------------------------------------------

GetStarvationMortalityIllius2000::GetStarvationMortalityIllius2000(
	const double fat_standard_deviation	):
	fat_standard_deviation(fat_standard_deviation)
{
	if (fat_standard_deviation < 0.0 || fat_standard_deviation > 1.0)
		throw std::invalid_argument(
				"Fauna::GetStarvationMortalityIllius2000::GetStarvationMortalityIllius2000() "
				"fat_standard_deviation not in interval [0,1]");
}

double GetStarvationMortalityIllius2000::cumulative_normal_distribution(double x){
    // constants
    static const double a1 =  0.254829592;
    static const double a2 = -0.284496736;
    static const double a3 =  1.421413741;
    static const double a4 = -1.453152027;
    static const double a5 =  1.061405429;
    static const double p  =  0.3275911;

    // Save the sign of x
    int sign = 1;
    if (x < 0)
        sign = -1;
    x = fabs(x)/sqrt(2.0);

    // A&S formula 7.1.26 (Handbook of Mathematical Functions by 
		// Abramowitz and Stegun)
    const double t = 1.0/(1.0 + p*x);
    const double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

    return 0.5*(1.0 + sign*y);}

double GetStarvationMortalityIllius2000::operator()(
		const double body_condition)const{
	if (body_condition < 0.0 || body_condition > 1.0)
		throw std::invalid_argument(
				"Fauna::GetStarvationMortalityIllius2000::operator()() "
				"body_condition is not in interval [0,1].");
	const double result = cumulative_normal_distribution(
			-body_condition / fat_standard_deviation);
	assert(result >= 0.0);
	return result;
}

//------------------------------------------------------------

GetStarvationMortalityThreshold::GetStarvationMortalityThreshold(
		const double min_bodyfat):
	min_bodyfat(min_bodyfat)
{
	if (min_bodyfat < 0.0 || min_bodyfat >= 1.0)
		throw std::invalid_argument(
				"Fauna::GetStarvationMortalityThreshold::GetStarvationMortalityThreshold() "
				"min_bodyfat not in interval [0,1)");
}

double GetStarvationMortalityThreshold::operator()(
		const double bodyfat)const{
	if (bodyfat < 0.0 || bodyfat > 1.0)
		throw std::invalid_argument(
				"Fauna::GetStarvationMortalityThreshold::operator()() "
				"bodyfat not in interval [0,1)");
	assert (min_bodyfat >= 0.0 && min_bodyfat < 1.0);
	if (bodyfat < min_bodyfat)
		return 1.0;
	else 
		return 0.0;
}


