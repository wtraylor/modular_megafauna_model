///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Energy-related classes and functions for herbivores
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_energetics.h"
#include <cassert> // for assert()
#include <stdexcept>

using namespace Fauna;

//============================================================
// FatmassEnergyBudget
//============================================================

// Definition of static constants.
const double FatmassEnergyBudget::FACTOR_ANABOLISM  = 54.6; // [MJ/kg]
const double FatmassEnergyBudget::FACTOR_CATABOLISM = 39.3; // [MJ/kg]

FatmassEnergyBudget::FatmassEnergyBudget(
		const double initial_fatmass,
		const double maximum_fatmass):
	energy_needs(0.0),
	fatmass(initial_fatmass), 
	max_fatmass(maximum_fatmass)
{
	if (initial_fatmass < 0.0)
		throw std::invalid_argument(
				"Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
				"initial_fatmass < 0.0");
	if (maximum_fatmass <= 0.0)
		throw std::invalid_argument(
				"Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
				"maximum_fatmass <= 0.0");
	if (initial_fatmass > maximum_fatmass)
		throw std::invalid_argument(
				"Fauna::FatmassEnergyBudget::FatmassEnergyBudget() "
				"initial_fatmass > maximum_fatmass");
}

void FatmassEnergyBudget::add_energy_needs(const double energy){
	if (energy<0.0)
		throw std::invalid_argument("Fauna::FatmassEnergyBudget::add_energy_needs() "
				"energy < 0.0");
	energy_needs += energy;
}

void FatmassEnergyBudget::catabolize_fat(){
	assert(energy_needs >= 0.0);
	assert(fatmass >= 0.0);
	if (energy_needs == 0.0)
		return;

	// fat mass [kg] to burn in order to meet energy needs
	const double burned_fatmass = energy_needs / FACTOR_CATABOLISM;

	/// Fat mass never drops below zero.
	fatmass = max(0.0, fatmass - burned_fatmass);
	assert(fatmass >= 0.0);

	energy_needs = 0.0;
}

double FatmassEnergyBudget::get_max_anabolism_per_day()const{
	return (max_fatmass-fatmass) * FACTOR_ANABOLISM;
}

void FatmassEnergyBudget::merge(const FatmassEnergyBudget& other,
		const double this_weight, const double other_weight){
	energy_needs = 
		(energy_needs*this_weight + other.energy_needs*other_weight) /
		(this_weight + other_weight);
	fatmass = 
		(fatmass*this_weight + other.fatmass*other_weight) /
		(this_weight + other_weight);
	max_fatmass = 
		(max_fatmass*this_weight + other.max_fatmass*other_weight) /
		(this_weight + other_weight);
}

void FatmassEnergyBudget::metabolize_energy(double energy){
	if (energy < 0.0)
		throw std::invalid_argument("Fauna::FatmassEnergyBudget::metabolize_energy() "
				"energy < 0.0");
	assert(energy_needs >= 0.0);
	assert(fatmass >= 0.0);

	if (energy <= energy_needs){
		energy_needs -= energy; // just meet immediate energy needs
	} else {

		// meet immediate energy needs
		energy -= energy_needs;
		energy_needs = 0.0;

		// store surplus as fat (anabolism)
		const double fatmass_gain = energy / FACTOR_ANABOLISM;
		if (fatmass + fatmass_gain > max_fatmass)
			throw std::logic_error("Fauna::FatmassEnergyBudget::metabolize_energy() "
					"Received energy exceeds maximum allowed fat anabolism.");

		// increase fat reserves
		fatmass += fatmass_gain;
	}
}

void FatmassEnergyBudget::set_max_fatmass(const double maximum){
	if (maximum < fatmass)
		throw std::logic_error("Fauna::FatmassEnergyBudget::set_max_fatmass() "
				"Maximum fat mass is lower than current fat mass.");
	if (maximum <= 0.0)
		throw std::invalid_argument("Fauna::FatmassEnergyBudget::set_max_fatmass() "
				"Received maximum fat mass smaller than zero");
	max_fatmass = maximum;
}
