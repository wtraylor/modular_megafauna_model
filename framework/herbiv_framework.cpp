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
#include "herbiv_population.h"   // for HftPopulationsMap and PopulationInterface
#include <stdexcept>              // for std::logic_error, std::invalid_argument

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
		default: throw std::logic_error("Simulator::create_digestibility_model(): "
								 "unknown digestibility model");
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
				"argument day_of_year out of range");

	// The habitat to simulate
	Habitat& habitat = simulation_unit.get_habitat();
	// all populations in the habitat (one for each HFT)
	HftPopulationsMap& populations = simulation_unit.get_populations();

	// pass the current date into the herbivore module
	habitat.init_day(day_of_year);

	// Keep track of the establishment cycle.
	days_since_last_establishment++;

	if (do_herbivores && hftlist.size()>0) {

		// ---------------------------------------------------------
		// ESTABLISHMENT
		// If we have reached the end of one establishment cycle.
		if (days_since_last_establishment > params.herbivore_establish_interval)
		{
			// iterate through HFT populations
			for (HftPopulationsMap::iterator itr_p = populations.begin();
					itr_p != populations.end(); itr_p++)
			{
				PopulationInterface& pop = **itr_p; // one population
				const Hft& hft = pop.get_hft();

				// Let the population handle the establishment
				if (pop.get_list().empty())
					pop.establish();
			}
			days_since_last_establishment = 0;
		}

		// ---------------------------------------------------------
		// PREPARE VARIABLES FOR SIMULATION

		// All offspring for each HFT today [ind/km²]
		std::map<const Hft*, double> total_offspring;

		// Output data of all herbivores for today in this habitat.
		// We define a local data type to save some typing.
		typedef std::map<const Hft*, std::vector<FaunaOut::HerbivoreData> >
			TodaysHftOutput;
		TodaysHftOutput hft_output;

		{ // Custom variable scope: to make clear that the
			// pointers to the herbivores in the variable `herbivores`
			// are only valid within this scope.
			// As soon as the populations change (new offspring, 
			// delete dead ones,...), the herbivore pointers may become
			// invalid.

			// All herbivores in the habitat
			HerbivoreVector herbivores = populations.get_all_herbivores();

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

				// Let the herbivores do their simulation.
				herbivore.simulate_day(day_of_year, offspring);

				// Gather the offspring.
				total_offspring[&herbivore.get_hft()] += offspring;
			}

			// ---------------------------------------------------------
			// FORAGING

			// available forage in the habitat [kgDM/km²]
			HabitatForage available_forage = habitat.get_available_forage();
			const ForageMass old_forage = available_forage.get_mass();

			// call function object
			feed_herbivores(
					available_forage,
					herbivores);

			// remove the eaten forage
			habitat.remove_eaten_forage(
					old_forage - available_forage.get_mass()); 

			// ---------------------------------------------------------
			// GATHER OUTPUT

			// loop through all herbivores: gather output
			for (HerbivoreVector::iterator itr_h=herbivores.begin();
					itr_h != herbivores.end(); itr_h++) 
			{
				HerbivoreInterface& herbivore = **itr_h;

				// Add the output of this herbivore to the vector of output
				// data for this HFT.
				hft_output[&herbivore.get_hft()].push_back(
						herbivore.get_todays_output());
			} 

		} // end of custom scope

		// ---------------------------------------------------------
		// MERGE OUTPUT
		// Aggregate output of herbivores for one habitat.
		FaunaOut::CombinedData todays_datapoint;
		for (TodaysHftOutput::const_iterator itr = hft_output.begin();
				itr != hft_output.end();
				itr++)
		{
			const Hft& hft = *itr->first;

			// Create a datapoint for each HFT that can then be merged
			// across habitats and time.
			todays_datapoint.hft_data[&hft] = 
				FaunaOut::HerbivoreData::create_datapoint( itr->second );
		}
		// Add the habitat data to the output
		todays_datapoint.habitat_data =
			((const Habitat&) habitat).get_todays_output();
		// The output data container is now one complete datapoint.
		todays_datapoint.datapoint_count = 1;
		// Merge today’s output into temporal aggregation of the simulation
		// unit.
		simulation_unit.get_output().merge(todays_datapoint);

		// ---------------------------------------------------------
		// REPRODUCTION
		// For each HFT, let the PopulationInterface object create herbivores.
		// These new herbivores will be counted in the output next simulation 
		// cycle.
		for (std::map<const Hft*, double>::iterator itr = total_offspring.begin();
				itr != total_offspring.end(); 
				itr++)
		{
			const Hft* hft = itr->first;
			const double offspring = itr->second;
			if (offspring > 0.0)
				populations[*hft].create_offspring(offspring);
		}
	}

}

//============================================================
// SimulationUnit
//============================================================

SimulationUnit::SimulationUnit( std::auto_ptr<Habitat> _habitat,
		std::auto_ptr<HftPopulationsMap> _populations):
	// move ownership to private auto_ptr objects
	habitat(_habitat),
	populations(_populations)
{
	if (habitat.get() == NULL)
		throw std::invalid_argument("Fauna::SimulationUnit::SimulationUnit() "
				"Pointer to habitat is NULL.");
	if (populations.get() == NULL)
		throw std::invalid_argument("Fauna::SimulationUnit::SimulationUnit() "
				"Pointer to populations is NULL.");
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
		for (ForageMass::const_iterator itr_ft = demand.begin();
				itr_ft != demand.end(); itr_ft++){
			const ForageType ft = itr_ft->first;

			if (demand_sum[ft] != 0.0)
				portion.set(ft,  avail_mass[ft] * 
						demand[ft] / demand_sum[ft]);
		}
	}
}

//============================================================
// FeedHerbivores
//============================================================

FeedHerbivores::FeedHerbivores(std::auto_ptr<DistributeForage> _distribute_forage):
	distribute_forage(_distribute_forage)
{
	if (distribute_forage.get() == NULL)
		throw std::invalid_argument("Fauna::FeedHerbivores::FeedHerbivores() "
				"Parameter `distribute_forage` is NULL.");
}

void FeedHerbivores::operator()(
		HabitatForage& available,
		const HerbivoreVector& herbivores) const{

	// loop as many times as there are forage types
	// to allow prey switching: 
	// If one forage type gets “empty” in the first loop, the
	// herbivores can then demand from another forage type, and so
	// on until it’s all empty or they are all satisfied or cannot
	// switch to another forage type.
	for (int i=0; i<FORAGE_TYPES.size(); i++){

		// If there is no forage available (anymore), abort!
		if (available.get_mass() <= 0.00001)
			break;

		//------------------------------------------------------------
		// GET FORAGE DEMANDS
		ForageDistribution forage_demand;
		for (HerbivoreVector::const_iterator itr=herbivores.begin();
				itr != herbivores.end(); itr++){
			HerbivoreInterface& herbivore = **itr;

			// calculate forage demand for this herbivore
			const ForageMass ind_demand = herbivore.get_forage_demands(available);

			// only add those herbivores that do want to eat
			if (ind_demand != 0.0) 
				forage_demand[&herbivore] = ind_demand;
		}

		// abort if all herbivores are satisfied
		if (forage_demand.empty())
			break;

		// get the forage distribution
		assert(distribute_forage.get() != NULL);
		(*distribute_forage)( available, forage_demand);

		// rename variable to make clear it’s not the demands anymore
		// but the portions to feed the herbivores
		ForageDistribution& forage_portions = forage_demand;

		//------------------------------------------------------------
		// LET THE HERBIVORES EAT

		const Digestibility digestibility = available.get_digestibility();

		// Loop through all portions and feed it to the respective
		// herbivore
		for (ForageDistribution::iterator iter=forage_portions.begin();
				iter != forage_portions.end(); iter++)
		{
			const ForageMass& portion = iter->second; // [kgDM/km²]
			HerbivoreInterface& herbivore = *(iter->first);

			if (herbivore.get_ind_per_km2() > 0.0) {
				// feed this herbivore
				herbivore.eat(portion, digestibility);

				// reduce the available forage
				for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
						ft != FORAGE_TYPES.end(); ft++)
					available[*ft].set_mass( 
							available[*ft].get_mass() - portion[*ft] );
			}
		}
	}
}
