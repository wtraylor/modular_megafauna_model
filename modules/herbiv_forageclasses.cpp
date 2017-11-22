///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Large herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_forageclasses.h"
#include <cassert> // for assert()

using namespace Fauna;

namespace{
	std::set<ForageType> get_all_forage_types(){
		std::set<ForageType> result;
		result.insert( FT_GRASS );
		// ADD NEW FORAGE TYPES HERE
		return result;
	}
}
// define global constant
namespace Fauna {
	const std::set<ForageType> FORAGE_TYPES = get_all_forage_types();
}

const std::string& Fauna::get_forage_type_name(const ForageType ft) {
	switch (ft){
		case FT_GRASS   : 
			static const std::string grass("grass");
			return grass;
		case FT_INEDIBLE: 
			static const std::string inedible("inedible");
			return inedible;
		default  : throw std::logic_error(
									 "Fauna::get_forage_type_name() "
									 "Forage type is not implemented.");
	} 
}

ForageValues<POSITIVE_AND_ZERO> Fauna::foragefractions_to_foragevalues(
		const ForageFraction& fractions)
{
	ForageValues<POSITIVE_AND_ZERO> result;
	for (ForageFraction::const_iterator i = fractions.begin();
			i != fractions.end();
			i++) 
		result.set(i->first, i->second);
	return result;
}

ForageFraction Fauna::foragevalues_to_foragefractions(
		const ForageValues<POSITIVE_AND_ZERO> values,
		const double tolerance)
{
	if (tolerance < 0.0)
		throw std::invalid_argument(
				"Fauna::foragevalues_to_foragefractions() "
				"Parameter `tolerance` is negative.");

	ForageFraction result;
	for (ForageFraction::const_iterator i = values.begin();
			i != values.end();
			i++) 
	{
		double v = i->second;
		if (v > 1.0){
			if (v <= 1.0 + tolerance)
				v = 1.0;
			else
				throw std::invalid_argument(
						"Fauna::foragevalues_to_foragefractions() "
						"One forage value exceeds 1.0 and cannot be converted to "
						"a fraction.");
		}

		result.set(i->first, v);
	}
	return result;
}

ForageFraction Fauna::convert_mj_to_kg_proportionally(
		const ForageEnergyContent& mj_per_kg,
		const ForageFraction& mj_proportions)
{
	ForageValues<POSITIVE_AND_ZERO> kg_proportions = 
		foragefractions_to_foragevalues(mj_proportions).
		divide_safely(mj_per_kg, 0.0);

	const double kg_prop_sum = kg_proportions.sum();
	const double mj_prop_sum = mj_proportions.sum();

	if (kg_prop_sum > 0.0){
		kg_proportions = kg_proportions * (mj_prop_sum / kg_prop_sum);

		return foragevalues_to_foragefractions(kg_proportions, 0.01);
	}else
		return ForageFraction(0.0);
}

//------------------------------------------------------------
// FORAGEBASE
//------------------------------------------------------------

ForageBase& ForageBase::merge_base(const ForageBase& other,
		const double this_weight, const double other_weight){
	if (this == &other) return *this;
	set_digestibility(average(
				this->get_digestibility(), other.get_digestibility(),
				this_weight, other_weight));
	set_mass(average(
				this->get_mass(), other.get_mass(),
				this_weight, other_weight));
	return *this;
}

//------------------------------------------------------------
// GRASSFORAGE
//------------------------------------------------------------

GrassForage& GrassForage::merge(const GrassForage& other,
		const double this_weight, const double other_weight){
	if (this == &other) return *this;
	// merge generic properties
	merge_base(other, this_weight, other_weight);

	// merge grass-specific properties
	// (don’t call get_fpc() here, but use private member variable
	//  directly, so that no validity check is done)
	set_fpc(average(
				this->fpc, other.fpc, 
				this_weight, other_weight));
	return *this;
}

//------------------------------------------------------------
// HABITATFORAGE
//------------------------------------------------------------

Digestibility HabitatForage::get_digestibility()const{

	Digestibility result;
	for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end(); ft++) {
		result.set(*ft, operator[](*ft).get_digestibility());
	}
	return result;
}

ForageMass HabitatForage::get_mass()const{
	ForageMass result;
	for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end(); ft++) {
		result.set(*ft, operator[](*ft).get_mass());
	}
	return result;
}

ForageBase HabitatForage::get_total() const{
	// Return object
	ForageBase result;

	const ForageMass mass = get_mass();

	result.set_mass( mass.sum() );

	// build weighted average only if total mass is greater zero
	if (result.get_mass() > 0.0) {
		// Create weighted average

		double dig_sum_weight = 0.0;
		// loop through each forage type
		for (ForageMass::const_iterator itr = mass.begin();
				itr != mass.end(); itr++){
			const ForageType ft = itr->first;
			dig_sum_weight += itr->second * operator[](ft).get_digestibility();
		}
		result.set_digestibility( dig_sum_weight / mass.sum() );
	} else 
		result.set_digestibility(0.0);
	return result;
}

HabitatForage& HabitatForage::merge(const HabitatForage& other,
		const double this_weight, const double other_weight){
	if (this == &other) return *this;

	grass.merge(other.grass, this_weight, other_weight);
	// ADD NEW FORAGE TYPES HERE
	return *this;
}

