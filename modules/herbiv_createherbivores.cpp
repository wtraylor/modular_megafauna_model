///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Classes to construct herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_createherbivores.h"
#include "herbiv_hft.h"
#include "herbiv_parameters.h"
#include <stdexcept>

using namespace Fauna;

//------------------------------------------------------------
//------ CreateHerbivoreCommon -------------------------------
//------------------------------------------------------------

CreateHerbivoreCommon::CreateHerbivoreCommon(
					const Hft* hft,
					const Parameters* parameters):
	hft(hft), parameters(parameters)
{
	if (hft==NULL)
		throw std::invalid_argument("Fauna::CreateHerbivoreCommon::CreateHerbivoreCommon() "
				"hft == NULL");
	if (parameters==NULL)
		throw std::invalid_argument("Fauna::CreateHerbivoreCommon::CreateHerbivoreCommon() "
				"parameters == NULL");
}

double CreateHerbivoreCommon::get_body_condition(
		const int age_days)const
{
	double body_condition;
	if (age_days == 0) // birth
		body_condition = get_hft().bodyfat_birth / get_hft().bodyfat_max;
	else // establishment
		body_condition = 1.0; // full fat reserves
	assert(body_condition <= 1.0 && body_condition >= 0.0);
	return body_condition;
}

//------------------------------------------------------------
//------ CreateHerbivoreIndividual ---------------------------
//------------------------------------------------------------

CreateHerbivoreIndividual::CreateHerbivoreIndividual(
					const Hft* hft,
					const Parameters* parameters,
					const double area_km2):
	CreateHerbivoreCommon(hft, parameters),
	area_km2(area_km2)
{
	if (area_km2 <= 0.0)
		throw std::invalid_argument("Fauna::CreateHerbivoreIndividual::CreateHerbivoreIndividual() "
				"area_km2 <= 0.0");
}

HerbivoreIndividual CreateHerbivoreIndividual::operator()(
		const int age_days, Sex sex)const{
	if (age_days < 0)
		throw std::invalid_argument("Fauna::CreateHerbivoreIndividual::operator()() "
				"age_days < 0");

	assert(area_km2 > 0.0);
	if (age_days == 0) 
		// Call birth constructor
		return HerbivoreIndividual(&get_hft(), sex, area_km2);
	else
		// Call establishment constructor
		return HerbivoreIndividual(
				age_days,
				get_body_condition(age_days),
				&get_hft(),
				sex,
				area_km2);
}

//------------------------------------------------------------
//------ CreateHerbivoreCohort -------------------------------
//------------------------------------------------------------

HerbivoreCohort CreateHerbivoreCohort::operator()(
		const double ind_per_km2, const int age_years, Sex sex)const{
	if (ind_per_km2 <= 0.0)
		throw std::invalid_argument("Fauna::CreateHerbivoreCohort::operator()() "
				"ind_per_km2 <= 0.0");
	if (age_years < 0)
		throw std::invalid_argument("Fauna::CreateHerbivoreCohort::operator()() "
				"age_years < 0");

	const int age_days = age_years*365;
	if (age_days == 0) 
		// Call birth constructor
		return HerbivoreCohort(&get_hft(), sex, ind_per_km2);
	else
		// Call establishment constructor
		return HerbivoreCohort(
				age_days,
				get_body_condition(age_days),
				&get_hft(),
				sex,
				ind_per_km2);
}
