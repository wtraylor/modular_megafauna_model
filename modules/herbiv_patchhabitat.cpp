///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Implementation of a herbivore habitat for a \ref Patch.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_patchhabitat.h"
#include "herbiv_digestibility.h"
#include "guess.h"
#include "assert.h"

using namespace Fauna;

//============================================================
// PatchHabitat
//============================================================

PatchHabitat::PatchHabitat(
				std::auto_ptr<HftPopulationsMap> populations,
				Patch& patch, 
				std::auto_ptr<GetDigestibility> digestibility_model):
	Habitat(populations), // parent constructor
	patch(patch), 
	get_digestibility(digestibility_model)
{
	if (this->get_digestibility.get() == NULL)
		throw std::invalid_argument("Fauna::PatchHabitat::PatchHabitat() "
				"Parameter \"digestibility_model\" is NULL.");
}

HabitatForage PatchHabitat::get_available_forage() const {
	// Result object (initialized with zero values)
	HabitatForage result; 

	// sum of grass FPC (foliar percentage cover)
	double grass_fpc = 0.0;

	// Loop through all vegetation individuals in this patch
	for (patch.vegetation.firstobj(); 
			patch.vegetation.isobj;
			patch.vegetation.nextobj()) 
	{
		const Individual& indiv = patch.vegetation.getobj();
		if (!indiv.alive)
			continue;


		const ForageType ft = indiv.get_forage_type();

		if (ft != FT_INEDIBLE){
			//------------------------------------------------------------
			// GENERIC BEHAVIOUR FOR ALL FORAGE TYPES

			// get digestibility
			assert( get_digestibility.get() != NULL );
			const double indiv_dig  = (*get_digestibility)(indiv); // [frac]
			assert( indiv_dig >= 0.0 );
			assert( indiv_dig <= 1.0 );

			// get forage mass
			double indiv_mass = indiv.get_forage_mass(); // [kg/km²]
			// avoid precision errors in extremely  low values.
			if (negligible(indiv_mass))
				indiv_mass = 0.0; 
			assert( indiv_mass >= 0.0 );
			// Digestibility: Build average, weighted by mass.
			if (indiv_mass + result[ft].get_mass() > 0.0)
				result[ft].set_digestibility(Fauna::average(
							result[ft].get_digestibility(), indiv_dig,
							result[ft].get_mass(), indiv_mass));

			// Simply sum up the mass for the whole habitat [kg/km²]
			result[ft].set_mass( result[ft].get_mass() + indiv_mass );

			//------------------------------------------------------------
			// GRASS SPECIFIC
			if (ft == FT_GRASS){
				if (indiv_mass > 0.0) {
					// Build sum of FPCs
					// We assume there is never any overlap of grass PFTs.
					grass_fpc += indiv.fpc;
					result.grass.set_fpc( grass_fpc );
				}
			}

			//------------------------------------------------------------
			// ADD OTHER FORAGE-SPECIFIC PROPERTIES HERE

		}
	}

	return result;
}

void PatchHabitat::remove_eaten_forage(const ForageMass& eaten_forage) {
	// Call the base class function
	Habitat::remove_eaten_forage(eaten_forage);

	// sum of the current forage in the patch before eating [kg/km²]
	ForageMass old_forage;

	// Sum up the old forage
	for (patch.vegetation.firstobj(); 
			patch.vegetation.isobj;
			patch.vegetation.nextobj()) 
	{
		const Individual& indiv = patch.vegetation.getobj();
		if (!indiv.alive)
			continue;
		const ForageType ft = indiv.get_forage_type();

		if (ft != FT_INEDIBLE){
			old_forage.set(ft, 
					old_forage.get(ft) + indiv.get_forage_mass());
		}
	}

	// The fraction of forage that is left after eating.
	ForageFraction fraction_left;

	// iterate over all forage types
	for (std::set<ForageType>::const_iterator ft=FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end(); ft++)
	{
		// Check if eaten forage exceeds what is available.
		if (old_forage[*ft] - eaten_forage[*ft] < 0.0)
			throw std::logic_error((std::string)
					"Fauna::PatchHabitat::remove_eaten_forage() "
					"Eaten forage exceeds available forage ("+
					get_forage_type_name(*ft)+").");

		if (old_forage[*ft] == 0.0)
			continue;

		// build fraction
		assert( old_forage[*ft] != 0.0 );
		fraction_left.set(*ft, eaten_forage[*ft] / old_forage[*ft]);
	}

	// Reduce the forage of each plant individual
	for (patch.vegetation.firstobj(); 
			patch.vegetation.isobj;
			patch.vegetation.nextobj()) 
	{
		Individual& indiv = patch.vegetation.getobj();
		if (!indiv.alive)
			continue;
		const ForageType ft = indiv.get_forage_type();

		// Reduce the forage of each individual proportionally
		if (ft != FT_INEDIBLE)
			indiv.reduce_forage_mass(fraction_left[ft]);
	} 
}
