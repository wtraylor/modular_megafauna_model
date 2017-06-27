///////////////////////////////////////////////////////////////////////////////////////
/// \file herbiv_patchhabitat.cpp
/// \brief Implementation of a herbivore habitat for a \ref Patch.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_patchhabitat.h"
#include "herbiv_digestibility.h"
#include "guess.h"

using namespace Fauna;

HabitatForage PatchHabitat::get_available_forage() const {
	/// Result object
	HabitatForage forage; 

	/// Reference to the digestibility model
	const DigestibilityModel& dig_model = DigestibilityModel::get_model();

	/// Sum of grass digestibility to build average
	double gr_dig_sum_weight, gr_dig_sum = 0.0;

	/// Number of grass individuals
	double gr_count; // double for floating point division later

	// Loop through all vegetation individuals in this patch
	patch.vegetation.firstobj();
	while (patch.vegetation.isobj) {
		const Individual& indiv = patch.vegetation.getobj();

		// Get common forage properties
		// TODO: Digestibility Model
		const double indiv_dig  = dig_model.get_digestibility(indiv); // [frac]
		const double indiv_mass = indiv.get_forage_mass(); // [kg/m²]
		assert(indiv_mass >= 0.0);


		// Sum up the grass forage
		if (indiv.get_forage_type() == FT_GRASS){
			gr_count += 1.0;

			// Sum up digestibility to build (weighted) average later
			gr_dig_sum_weight += indiv_dig * indiv_mass;
			gr_dig_sum        += indiv_dig;

			// Simply sum up the mass for the whole habitat [kg/m²]
			forage.grass.set_mass(
					forage.grass.get_mass()
					+ indiv_mass);

			// Build sum of FPCs
			forage.grass.set_fpc(
					forage.grass.get_fpc()
					+ indiv.fpc);
		}

		// ADD OTHER FORAGE (BROWSE) HERE ...

		patch.vegetation.nextobj();
	}

	// calculate digestibility average: weighted if there is mass,
	// otherwise simple average
	if (forage.grass.get_mass() > 0.0) 
		forage.grass.set_digestibility( gr_dig_sum_weight / forage.grass.get_mass());
	else
		forage.grass.set_digestibility( gr_dig_sum / gr_count );

	return forage;
}

void PatchHabitat::remove_eaten_forage(const ForageMass& eaten_forage) {
	// Call the base class function
	Habitat::remove_eaten_forage(eaten_forage);

	/// sum of the current grass forage in the patch before eating [kg/m²]
	double old_grass = 0.0;

	// Sum up the forage
	patch.vegetation.firstobj();
	while (patch.vegetation.isobj) {
		const Individual& indiv = patch.vegetation.getobj();

		if (indiv.get_forage_type() == FT_GRASS){
			old_grass += indiv.get_forage_mass();
		}
		// ADD OTHER FORAGE (BROWSE) HERE ... 
		patch.vegetation.nextobj();
	}

	/// sum of the new grass forage after eating [kg/m²]
	const double new_grass = old_grass - eaten_forage.grass;
	assert(new_grass >= 0.0);
	assert(old_grass > 0.0);
	assert(new_grass <= old_grass);

	// Reduce the forage of each plant individual
	patch.vegetation.firstobj();
	while (patch.vegetation.isobj) {
		Individual& indiv = patch.vegetation.getobj();

		// Reduce the forage of each individual proportionally
		if (indiv.get_forage_type() == FT_GRASS){
			indiv.reduce_forage_mass(new_grass/old_grass);
		} 
		// ADD OTHER FORAGE (BROWSE) HERE ... 

		patch.vegetation.nextobj();
	} 
}

