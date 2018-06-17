///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_framework.h"
#include "herbiv_digestibility.h"   // for GetDigestibility
#include "herbiv_herbivore.h"       // for HerbivoreInterface
#include "herbiv_hft.h"             // for Hft and HftList
#include "herbiv_parameters.h"      // for Fauna::Parameters
#include "herbiv_population.h"      // for HftPopulationsMap and PopulationInterface
#include "herbiv_simulate_day.h"    // for SimulateDay
#include "herbiv_simulation_unit.h" // for SimulationUnit
#include "herbiv_snowdepth.h"       // for GetSnowDepth implementations
#include <stdexcept>                // for std::logic_error, std::invalid_argument

using namespace Fauna;

//============================================================
// Simulator
//============================================================

Simulator::Simulator(const Parameters& params, const HftList& hftlist):
	hftlist(hftlist), params(params),
	days_since_last_establishment(params.herbivore_establish_interval),
	feed_herbivores(create_distribute_forage())
	// Non-static data member initialization happens in the order
	// of declaration in the header file: feed_herbivores must be
	// *after* params so that create_distribute_forage() does not
	// cause segmentation fault.
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
		case DM_NPP: 
			return std::auto_ptr<GetDigestibility>(new DigestibilityFromNPP()); 
		case DM_PFT_PACHZELT2013: 
			return std::auto_ptr<GetDigestibility>(new DigestibilityPachzelt2013()); 
		default: throw std::logic_error("Simulator::create_digestibility_model(): "
								 "Digestibility model not implemented.");
	};
}

std::auto_ptr<DistributeForage> Simulator::create_distribute_forage(){
	switch (params.forage_distribution) {
		case FD_EQUALLY:
			return std::auto_ptr<DistributeForage>(
					new DistributeForageEqually);
		default:
			throw std::logic_error("Fauna::Simulator::distribute_forage(): "
					"chosen forage distribution algorithm not implemented");
	};
}

std::auto_ptr<GetSnowDepth> Simulator::create_snow_depth_model()const{
	switch (params.snow_depth_model){
		case SD_TEN_TO_ONE: 
			return std::auto_ptr<GetSnowDepth>(new SnowDepthTenToOne()); 
		default: throw std::logic_error("Simulator::create_snow_depth_model(): "
								 "Snow depth model not implemented.");
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
						CreateHerbivoreCohort(phft, &params)));
			pmap->add(pcohort_pop);
		} else if (params.herbivore_type == HT_INDIVIDUAL) {
			const double AREA=1.0; // TODO THis is only a test
			std::auto_ptr<PopulationInterface> pind_pop(
					new IndividualPopulation(
						CreateHerbivoreIndividual(phft, &params)));
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

void Simulator::simulate_day(const int day_of_year, 
					SimulationUnit& simulation_unit,
		const bool do_herbivores){
	if (day_of_year < 0 || day_of_year >= 365)
		throw std::invalid_argument("Simulator::simulate_day(): "
				"Argument 'day_of_year' out of range");

	// If there was no initial establishment yet, we may do this now.
	bool establish_if_needed = !simulation_unit.is_initial_establishment_done();

	// If one check interval has passed, we will check if HFTs have died out
	// and need to be re-established.
	// Note that re-establishment is only activated if the interval length is 
	// a positive number.
	if (days_since_last_establishment == params.herbivore_establish_interval &&
		 params.herbivore_establish_interval > 0)
	{
		establish_if_needed = true;
		days_since_last_establishment = 0;
	}

	// Keep track of the establishment cycle.
	days_since_last_establishment++;

	// Create function object to delegate all simulations for this day to.
	SimulateDay simulate_day(day_of_year, simulation_unit, feed_herbivores);

	// Call the function object.
	simulate_day(do_herbivores && hftlist.size()>0, establish_if_needed);
}

