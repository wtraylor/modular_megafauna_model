///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_framework.h"
#include "herbiv_digestibility.h" // for GetDigestibility
#include "herbiv_habitat.h"       // for Habitat and Population
#include "herbiv_herbivore.h"     // for HerbivoreInterface
#include "herbiv_hft.h"           // for Hft and HftList
#include "herbiv_parameters.h"    // for Fauna::Parameters
#include <stdexcept>              // for std::logic_error, std::invalid_argument

using namespace Fauna;

Simulator::Simulator(const Parameters& params, const HftList& hftlist):
	hftlist(hftlist), params(params)
{
	// CHECK PARAMETERS OF HFTS AND GLOBALLY

	std::string all_msg; 
	bool all_valid = true;

	HftList::const_iterator itr = hftlist.begin();
	while (itr != hftlist.end()){
		std::string msg;
		all_valid &= itr->is_valid(params, msg);
		all_msg   += msg;
		itr++;
	}

	std::string global_msg;
	all_valid &= params.is_valid(global_msg);
	all_msg   += global_msg;

	if (!all_valid)
		throw std::invalid_argument("Fauna::Simulator::Simulator() "
				"Invalid parameters:\n"+all_msg);
}

std::auto_ptr<GetDigestibility> Simulator::create_digestibility_model()const{
	switch (params.digestibility_model){
		case DM_PFT_FIXED: 
			return std::auto_ptr<GetDigestibility>(new PftDigestibility()); 
		default: throw std::logic_error("Simulator::create_digestibility_model(): "
								 "unknown digestibility model");
	};
}

std::auto_ptr<HftPopulationsMap> Simulator::create_populations()const{
	// instantiate the HftPopulationsMap object
	std::auto_ptr<HftPopulationsMap> pmap(new HftPopulationsMap());

	// Fill the object with one population per HFT
	for (int i=0; i<hftlist.size(); i++){
		const Hft* phft = &hftlist[i];

		// Create population instance according to selected herbivore
		// type
		if (params.herbivore_type == HT_COHORT) {
			std::auto_ptr<PopulationInterface> pcohort_pop(
					new CohortPopulation(
						CreateHerbivoreCohort(phft, &params),
						params.dead_herbivore_threshold));
			pmap->add(pcohort_pop);
		} else if (params.herbivore_type == HT_INDIVIDUAL) {
			const double AREA=1.0; // TODO THis is only a test
			std::auto_ptr<PopulationInterface> pind_pop(
					new IndividualPopulation(
						CreateHerbivoreIndividual(phft, &params, 
							AREA)));
			// TODO Where does the area size come from??
			// -> from Habitat (then merge() doesn’t work anymore)
			// -> from Parameters (then CreateHerbivoreIndividual
			//    can read it directly + new validity checks)
			// -> calculated by framework() ?
			pmap->add(pind_pop);
		} else 
			throw std::logic_error("Simulator::create_populations(): "
					"unknown herbivore type");
	}
	assert( pmap.get()  != NULL );
	assert( pmap->size() == hftlist.size() );
	return pmap;
}

void Simulator::simulate_day(const int day_of_year, Habitat& habitat,
		const bool do_herbivores){
	if (day_of_year < 0 || day_of_year >= 365)
		throw std::invalid_argument("Simulator::simulate_day(): "
				"argument day_of_year out of range");

	// pass the current date into the herbivore module
	habitat.init_todays_output(day_of_year);

	if (do_herbivores && hftlist.size()>0) {

		// all populations in the habitat (one for each HFT)
		HftPopulationsMap& populations = habitat.get_populations();

		// ---------------------------------------------------------
		// ESTABLISHMENT
		// iterate through HFT populations
		for (HftPopulationsMap::iterator itr_p = populations.begin();
				itr_p != populations.end(); itr_p++){
			PopulationInterface& pop = **itr_p; // one population
			const Hft& hft = pop.get_hft();

			// ESTABLISHMENT
			if (pop.get_list().empty())
				pop.establish();
		}

		// ---------------------------------------------------------
		// PREPARE VARIABLES FOR SIMULATION

		// All offspring for each HFT today [ind/km²]
		std::map<const Hft*, double> total_offspring;

		{ // Custom variable scope: to make clear that the
			// pointers to the herbivores in the variabes `herbivores`
			// and `forage_demand` are only valid within this scope.
			// As soon as the populations change (new offspring, 
			// delete dead ones,...), the herbivore pointers may become
			// invalid.

			// All herbivores in the habitat
			HerbivoreVector herbivores = populations.get_all_herbivores();

			// demanded forage per herbivore [kgDM/m²]
			ForageDistribution forage_demand; 

			// available forage in the habitat [kgDM/m²]
			const HabitatForage available_forage = habitat.get_available_forage();

			// loop through all herbivores: simulate and get forage demands
			for (HerbivoreVector::iterator itr_h=herbivores.begin();
					itr_h != herbivores.end(); itr_h++) 
			{
				HerbivoreInterface& herbivore = **itr_h;

				// ---------------------------------------------------------
				// HERBIVORE SIMULATION

				// Offspring by this one herbivore today 
				// [ind/km²]
				double offspring = 0.0;

				herbivore.simulate_day(day_of_year, offspring);

				total_offspring[&herbivore.get_hft()] += offspring;

				// collect forage demands
				const ForageMass this_demand 
					= herbivore.get_forage_demands(available_forage);
				forage_demand[&herbivore] = this_demand;
			}

			// ---------------------------------------------------------
			// FORAGING
			// TODO loop as many times as there are forage types
			// to allow prey switching

			// get the forage distribution
			distribute_forage()(
					habitat.get_available_forage(),
					forage_demand);
			// rename variable to make clear it’s not the demands anymore
			ForageDistribution& forage_portions = forage_demand;

			// let the herbivores eat
			ForageMass eaten_forage; // [kgDM/m²]
			for (ForageDistribution::iterator iter=forage_portions.begin();
					iter != forage_portions.end(); iter++)
			{
				const ForageMass& portion = iter->second; // [kgDM/m²]

				// TODO make this generic for all forage types
				iter->first->eat(FT_GRASS, 
						portion.get_grass(),
						available_forage.grass.get_digestibility());

				eaten_forage += portion;
			}

			// Finally: remove the eaten forage
			habitat.remove_eaten_forage(eaten_forage); 

		} // end of custom scope

		// ---------------------------------------------------------
		// REPRODUCTION
		// create new herbivores
		for (std::map<const Hft*, double>::iterator itr = total_offspring.begin();
				itr != total_offspring.end(); itr++){
			const Hft* hft = itr->first;
			const double offspring = itr->second;
			if (offspring > 0.0)
				populations[*hft].create_offspring(offspring);
		}
	}

}

DistributeForage& Simulator::distribute_forage(){
	// returns a function object
	static DistributeForageEqually equal;
	switch (params.forage_distribution) {
		case FD_EQUALLY:
			return equal;
		default:
			throw std::logic_error("Fauna::Simulator::distribute_forage(): "
					"chosen forage distribution algorithm not implemented");
	};
}

//============================================================
// DistributeForageEqually
//============================================================

void DistributeForageEqually::operator()(
		const HabitatForage& available,
		ForageDistribution& forage_distribution) const {
	if (forage_distribution.empty()) return;

	// BUILD SUM OF ALL DEMANDED FORAGE
	ForageMass demand_sum;
	// iterate through all herbivores
	for (ForageDistribution::iterator itr = forage_distribution.begin();
			itr != forage_distribution.end(); itr++) {
		demand_sum += itr->second;
	}

	// Only distribute a little less than `available` in order
	// to mitigate precision errors.
	const ForageMass avail_mass = available.get_mass() * 0.999;

	// If there is not more demanded than is available, nothing
	// needs to be distributed.
	if (demand_sum <= avail_mass)
		return;

	// MAKE EQUAL PORTIONS
	// iterate through all herbivores
	for (ForageDistribution::iterator itr = forage_distribution.begin();
			itr != forage_distribution.end(); itr++) {
		assert(itr->first != NULL);
		const HerbivoreInterface& herbivore = *(itr->first);
		const ForageMass& demand = itr->second; // input
		ForageMass&      portion = itr->second; // output

		// calculate the right portion for each forage type
		if (demand_sum.get_grass() != 0.0)
			portion.set_grass( avail_mass.get_grass() * 
					demand.get_grass() / demand_sum.get_grass());
	}
}
