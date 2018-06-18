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
#include "herbiv_snowdepth.h"
#include "guess.h"
#include "assert.h"

using namespace Fauna;

//============================================================
// PatchHabitat
//============================================================

PatchHabitat::PatchHabitat(
				Patch& patch, 
				std::auto_ptr<GetDigestibility> digestibility_model,
				std::auto_ptr<GetSnowDepth> snow_depth_model):
	patch(patch), 
	get_digestibility(digestibility_model),
	get_snow_depth(snow_depth_model)
{
	if (this->get_digestibility.get() == NULL)
		throw std::invalid_argument("Fauna::PatchHabitat::PatchHabitat() "
				"Parameter \"digestibility_model\" is NULL.");
	if (this->get_snow_depth.get() == NULL)
		throw std::invalid_argument("Fauna::PatchHabitat::PatchHabitat() "
				"Parameter \"snow_depth_model\" is NULL.");
}

void PatchHabitat::add_excreted_nitrogen(const double kgN_per_km2){
	if (kgN_per_km2 < 0.0)
		throw std::invalid_argument("Fauna::PatchHabitat::add_excreted_nitrogen() "
				"Value for excreted nitrogen is negative.");
	const double kgN_per_m2 = kgN_per_km2 * 10e-6; // kgN/km²⇒kgN/m²
	patch.soil.nmass_avail += kgN_per_m2;
	patch.fluxes.report_flux(Fluxes::EXCRETEDN, -kgN_per_m2);
}

HabitatForage PatchHabitat::get_available_forage() const {
	// Result object (initialized with zero values)
	HabitatForage result; 

	// Fractional nitrogen content in each forage type.
	ForageFraction nitrogen_content;

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

			// Avoid precision errors in extremely low values.
			if (indiv_mass < 10)
				indiv_mass = 0.0; 
			assert( indiv_mass >= 0.0 );

			// Digestibility: Build average, weighted by mass.
			if (indiv_mass + result[ft].get_mass() > 0.0)
				result[ft].set_digestibility(Fauna::average(
							result[ft].get_digestibility(), indiv_dig,
							result[ft].get_mass(), indiv_mass));

			// Nitrogen Content: Build weighted average.
			if (indiv_mass != 0)
				nitrogen_content.set(ft,
						average(nitrogen_content[ft], // old total N fraction
							indiv.nmass_leaf / indiv_mass, // new individual N fraction
							result[ft].get_mass(), // total weight
							indiv_mass)); // individual weight

			// Simply sum up the mass for the whole habitat [kg/km²]
			result[ft].set_mass( result[ft].get_mass() + indiv_mass );

			//------------------------------------------------------------
			// GRASS SPECIFIC
			if (ft == FT_GRASS){

				// if (indiv_mass > 0.0) {
        //
				// 	// Maximum reasonable grass sward density [kgDM/km²].
				// 	static const double MAX_SWARD_DENS = 20 * 10000; // = 20 kg/m²
        //
				// 	if (indiv.fpc <= 0.0)
				// 		throw std::logic_error(
				// 				"Fauna::PatchHabitat::get_available_forage() "
				// 				"The grass individual has positive leaf biomass "
				// 				"but its FPC is zero or negative.");
        //
				// 	if (indiv_mass/indiv.fpc > MAX_SWARD_DENS)
				// 		throw std::logic_error(
				// 				"Fauna::PatchHabitat::get_available_forage() "
				// 				"The grass individual has positive leaf biomass "
				// 				"but its FPC is near zero. "
				// 				"This results in a preposterously high grass sward "
				// 				"density.");
        //
				// 	// Build sum of FPCs
				// 	// We assume there is never any overlap of grass PFTs.
				// 	grass_fpc += indiv.fpc;
				// }
			}

			//------------------------------------------------------------
			// ADD OTHER FORAGE-SPECIFIC PROPERTIES HERE

		}
	}
	// TODO: Grass FPC is made constant because 
	// LPJ-GUESS produced near-zero FPC values.
	if (result.grass.get_mass() > 0.0) {
		grass_fpc = 0.5; // DIRTY FIX CONSTANT
	} else
		grass_fpc = 0.0;

	// Keep FPC in valid range. If LPJ-GUESS has unreasonable values, we just
	// correct them. It’s not our issue to complain about errors in the
	// vegetation model here.
	grass_fpc = max(grass_fpc, 0.0);
	grass_fpc = min(grass_fpc, 1.0);
	result.grass.set_fpc( grass_fpc );

	result.set_nitrogen_content(nitrogen_content);

	return result;
}

HabitatEnvironment PatchHabitat::get_environment() const{
	HabitatEnvironment env;
	env.snow_depth = (*get_snow_depth)(patch);
	env.air_temperature = patch.get_climate().temp;
	return env;
}

void PatchHabitat::init_day(const int today){
	// Call parent function.
	Habitat::init_day(today);

	for (patch.vegetation.firstobj(); 
			patch.vegetation.isobj;
			patch.vegetation.nextobj()) 
	{
		Individual& indiv = patch.vegetation.getobj();

		indiv.update_average_phenology();
	}
}

void PatchHabitat::remove_eaten_forage(const ForageMass& eaten_forage) {
	// Call the base class function to register output.
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
	ForageFraction fraction_left(1.0);

	// Iterate over all forage types and see which proportion has been eaten.
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
		fraction_left.set(*ft, 1.0 - eaten_forage[*ft] / old_forage[*ft]);
	}

	// Reduce the forage of each plant individual.
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

