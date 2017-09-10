///////////////////////////////////////////////////////////////////
/// \file 
/// \brief Output classes of the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date August 2017
////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_outputclasses.h"
#include "herbiv_hft.h"

using namespace Fauna;
using namespace FaunaOut;

HabitatData& HabitatData::merge(const HabitatData& other){
	if (&other == this)
		return *this;

	// Build average for each variable:
	eaten_forage.merge(other.eaten_forage,
			this->datapoint_count, other.datapoint_count);
	available_forage.merge(other.available_forage,
			this->datapoint_count, other.datapoint_count);

	// ADD NEW VARIABLES HERE

	datapoint_count += other.datapoint_count;
	return *this;
}

HerbivoreData& HerbivoreData::merge(const HerbivoreData& other){

	// Build average for each variable

	// Average building for double values
	age_years = average(
			age_years, other.age_years,
			datapoint_count, other.datapoint_count);
	bodyfat = average(
			bodyfat, other.bodyfat,
			datapoint_count, other.datapoint_count);
	inddens = average(
			inddens, other.inddens,
			datapoint_count, other.datapoint_count);
	massdens = average(
			massdens, other.massdens,
			datapoint_count, other.datapoint_count);
	expenditure = average(
			expenditure, other.expenditure,
			datapoint_count, other.datapoint_count);

	// Average building for ForageValues
	eaten_forage.merge(other.eaten_forage);
	energy_intake.merge(other.energy_intake);

	// Only use those mortality factors that are included in
	// *both* maps.
	typedef std::map<Fauna::MortalityFactor, double> MortMap;
	MortMap mort_intersect;
	for (MortMap::const_iterator itr = other.mortality.begin();
			itr != other.mortality.end(); itr++)
	{
		const MortalityFactor& mf = itr->first;
		// find the mortality factor of the other object in this object.
		MortMap::iterator found = this->mortality.find(mf);
		if (found != this->mortality.end())
			mort_intersect[mf] = average(
					found->second, itr->second,
					datapoint_count, other.datapoint_count);
	}
	this->mortality = mort_intersect;

	// ADD NEW VARIABLES HERE

	datapoint_count += other.datapoint_count;
	return *this;
}
