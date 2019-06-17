//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Function objects to perform simulations in the herbivore model.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "simulate_day.h"
#include "environment.h"     // for HabitatEnvironment
#include "feed.h"            // for FeedHerbivores
#include "habitat.h"         // for Habitat and Population
#include "herbivore.h"       // for HerbivoreInterface
#include "simulation_unit.h" // for SimulationUnit
#include "population.h"      // for HftPopulationsMap and PopulationInterface

using namespace Fauna;

//============================================================
// SimulateDay
//============================================================

SimulateDay::SimulateDay(const int day_of_year,
					SimulationUnit& simulation_unit,
					const FeedHerbivores& feed_herbivores):
	day_of_year(day_of_year),
	excreted_nitrogen(0.0),
	environment(simulation_unit.get_habitat().get_environment()),
	feed_herbivores(feed_herbivores),
	forage_before_feeding(get_corrected_forage(simulation_unit.get_habitat())),
	herbivores(simulation_unit.get_populations().get_all_herbivores()),
	simulation_unit(simulation_unit)
{
}

void SimulateDay::aggregate_output(){

	// Output data of all herbivores for today in this habitat.
	std::map<const Hft*, std::vector<FaunaOut::HerbivoreData> > hft_output;

	// Note: If herbivores were not simulated, the HFT output will simply
	// be empty.

	// GATHER HERBIVORE OUTPUT
	// Loop through all herbivores: gather output.
	for (HerbivoreVector::iterator itr_h=herbivores.begin();
			itr_h != herbivores.end(); itr_h++)
	{
		HerbivoreInterface& herbivore = **itr_h;

		// Add the output of this herbivore to the vector of output
		// data for this HFT.
		hft_output[&herbivore.get_hft()].push_back(
				herbivore.get_todays_output());
	}

	// MERGE HFT OUTPUT

	// Iterate over HFT output.
	for (std::map<const Hft*, std::vector<FaunaOut::HerbivoreData> >::const_iterator
			itr = hft_output.begin();
			itr != hft_output.end();
			itr++)
	{
		const Hft& hft = *itr->first;
		// Create a datapoint for each HFT that can then be merged
		// across habitats and time.
		todays_datapoint.hft_data[&hft] =
			FaunaOut::HerbivoreData::create_datapoint( itr->second );
	}

	// HABITAT OUTPUT
	// Add the habitat data to the output even if no herbivores are simulated.
	const Habitat& const_habitat  = simulation_unit.get_habitat();
	todays_datapoint.habitat_data = const_habitat.get_todays_output();
	// The output data container is now one complete datapoint.
	todays_datapoint.datapoint_count = 1;
	// Merge today’s output into temporal aggregation of the simulation
	// unit.
	simulation_unit.get_output().merge(todays_datapoint);
}

void SimulateDay::create_offspring(){
	for (std::map<const Hft*, double>::iterator itr = total_offspring.begin();
			itr != total_offspring.end();
			itr++)
	{
		const Hft* hft = itr->first;
		const double offspring = itr->second;
		if (offspring > 0.0)
			simulation_unit.get_populations()[*hft].create_offspring(offspring);
	}
}

void SimulateDay::do_establishment(){
	// iterate through HFT populations
	HftPopulationsMap& pops = simulation_unit.get_populations();
	for (HftPopulationsMap::iterator itr_p = pops.begin();
			itr_p != pops.end(); itr_p++)
	{
		PopulationInterface& pop = **itr_p; // one population
		const Hft& hft = pop.get_hft();

		// Let the population handle the establishment
		if (pop.get_list().empty())
			pop.establish();
	}
	simulation_unit.set_initial_establishment_done();
}

HabitatForage SimulateDay::get_corrected_forage(const Habitat& habitat){
	// available forage in the habitat [kgDM/km²]
	HabitatForage available_forage = habitat.get_available_forage();
	static const double NEGLIGIBLE_FORAGE_MASS = 10000; // [kgDM/km²] ٍ= 10 g/m²
	for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
			ft != FORAGE_TYPES.end();
			ft++)
		if (available_forage[*ft].get_mass() <= NEGLIGIBLE_FORAGE_MASS) {
			available_forage[*ft].set_nitrogen_mass(0.0);
			available_forage[*ft].set_mass(0.0);
		}
	return available_forage;
}

void SimulateDay::operator()(const bool do_herbivores,
		const bool establish_if_needed)
{
	if (day_of_year < 0 || day_of_year >= 365)
		throw std::invalid_argument("SimulateDay::operator()() "
				"Argument 'day_of_year' out of range");

	// pass the current date into the herbivore module
	simulation_unit.get_habitat().init_day(day_of_year);

	if (do_herbivores && !simulation_unit.get_populations().empty()) {
		// Kill herbivore populations below the minimum density threshold here
		// so that simulate_herbivores() can take the nitrogen back before
		// the herbivore objects are removed from memory in purge_of_dead()
		// below.
		simulation_unit.get_populations().kill_nonviable();

		if (establish_if_needed)
			do_establishment();

		simulate_herbivores();

		// FEEDING
		HabitatForage available_forage = forage_before_feeding;
		feed_herbivores(available_forage, herbivores);
		// remove the eaten forage
		simulation_unit.get_habitat().remove_eaten_forage(
				forage_before_feeding.get_mass() - available_forage.get_mass());
	}

	aggregate_output();

	simulation_unit.get_habitat().add_excreted_nitrogen(excreted_nitrogen);

	// Now we will change the populations, and the herbivore pointers could
	// become invalid. So we need to delete the pointers to be sure that
	// nobody uses those pointers.
	herbivores.clear();

	create_offspring();

	simulation_unit.get_populations().purge_of_dead();
}

void SimulateDay::simulate_herbivores(){
	// loop through all herbivores: simulate
	for (HerbivoreVector::iterator itr_h=herbivores.begin();
			itr_h != herbivores.end(); itr_h++)
	{
		HerbivoreInterface& herbivore = **itr_h;

		// If this herbivore is dead, just take all of its nitrogen and
		// skip it. The Population object will take care of releasing its
		// memory.
		if (herbivore.is_dead()){
			excreted_nitrogen += herbivore.take_nitrogen_excreta();
			continue;
		}

		// ---------------------------------------------------------
		// HERBIVORE SIMULATION

		// Offspring by this one herbivore today
		// [ind/km²]
		double offspring = 0.0;

		// Let the herbivores do their simulation.
		herbivore.simulate_day(day_of_year, environment, offspring);

		// Gather the offspring.
		total_offspring[&herbivore.get_hft()] += offspring;

		// Gather nitrogen excreta.
		excreted_nitrogen += herbivore.take_nitrogen_excreta();
	}
}

