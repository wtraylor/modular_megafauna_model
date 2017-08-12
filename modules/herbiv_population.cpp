///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Management classes of herbivore populations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_population.h"
#include "herbiv_herbivore.h"
#include "herbiv_hft.h"
#include <cmath>     // for round() and ceil()
#include <stdexcept> // for std::out_of_range
#include <sstream>   // for std::ostringstream

using namespace Fauna;

//============================================================
// IndividualPopulation
//============================================================

IndividualPopulation::IndividualPopulation(
		const CreateHerbivoreIndividual create_individual	):
	create_individual(create_individual)
{ }

void IndividualPopulation::create_offspring(const double ind_per_km2){
	const int old_listsize = list.size();
	if (ind_per_km2 < 0.0)
		throw std::invalid_argument( "Fauna::IndividualPopulation::create() "
				"ind_per_km2 < 0.0");

	// convert double to discrete integer
	const int ind_count = round(
			ind_per_km2 * create_individual.get_area_km2());

	int male_count = ind_count / 2; // floored integer division
	int female_count = male_count;
	assert( ind_count - (male_count+female_count) <= 1 );

	// If the number is odd, we just create a male and female
	// alternating so that on average the sex ratio remains even.
	if (ind_count%2 == 1){
		static Sex sex_of_odd_remainder = SEX_FEMALE;
		if (sex_of_odd_remainder == SEX_FEMALE){
			female_count++;
			sex_of_odd_remainder = SEX_MALE; // switch for next time
		} else {
			male_count++;
			sex_of_odd_remainder = SEX_FEMALE; // switch for next time
		}
	}

	const int age_days = 0;

	// create males and females
	for (int i=1; i<=male_count; i++)
		list.push_back( create_individual(age_days, SEX_MALE) );
	for (int i=1; i<=female_count; i++)
		list.push_back( create_individual(age_days, SEX_FEMALE) );
	assert( list.size() == old_listsize + round(ind_count) );
}

void IndividualPopulation::establish(){
	if (!get_list().empty())
		throw std::logic_error("Fauna::IndividualPopulation::establish() "
				"Trying to establish into a non-empty population.");
	if (get_hft().establishment_density == 0.0)
		return;

	/**
	 * Establish with even sex ratio and *at least* as many 
	 * individuals as given by \ref Hft::establishment_density.
	 */
	// determine number of individuals, assuming even sex ratio.
	int ind_count = ceil(
			get_hft().establishment_density * create_individual.get_area_km2());
	// make the number even
	ind_count += ind_count%2;
	// produce at least two individuals
	ind_count = max(ind_count, 2);

	// establishment as mature adults
	const int age_days_male   = get_hft().maturity_age_phys_male * 365;
	const int age_days_female = get_hft().maturity_age_phys_female * 365;

	// add new objects to the list
	for (int i=1; i<=ind_count/2; i++){
		list.push_back( create_individual(age_days_male, SEX_MALE) );
		list.push_back( create_individual(age_days_female, SEX_FEMALE) );
	}
}

std::vector<const HerbivoreInterface*> IndividualPopulation::get_list()const{
	std::vector<const HerbivoreInterface*> result;
	result.reserve(list.size());
	for (List::const_iterator itr=list.begin(); 
			itr != list.end(); itr++) {
		// only add alive individuals
		if (!itr->is_dead())
			result.push_back(&*itr);
	}
	return result;
}

std::vector<HerbivoreInterface*> IndividualPopulation::get_list(){
	std::vector<HerbivoreInterface*> result;
	result.reserve(list.size());
	List::iterator itr = list.begin();
	while (itr!=list.end()){
		if (!itr->is_dead()){
			// herbivore is alive, add it to the list and proceed
			result.push_back(&*itr);
			itr++;
		}
		else
			itr = list.erase(itr); // remove dead herbivore
	}
	return result;
}

//============================================================
// CohortPopulation
//============================================================

CohortPopulation::CohortPopulation(
		const CreateHerbivoreCohort create_cohort,
		const double dead_herbivore_threshold
		):
	create_cohort(create_cohort), 
	dead_herbivore_threshold(dead_herbivore_threshold)
{
	if (dead_herbivore_threshold < 0.0)
		throw std::invalid_argument("Fauna::CohortPopulation::CohortPopulation() "
				"dead_herbivore_threshold must be >=0.");
}

void CohortPopulation::create_offspring(const double ind_per_km2){
	if (ind_per_km2 < 0.0)
		throw std::invalid_argument(
				"Fauna::CohortPopulation::create_offspring() "
				"ind_per_km2 < 0.0");

	List::iterator itr_male = find_cohort(0, SEX_MALE);
	if (itr_male == list.end()) // no existing cohort
		list.push_back(create_cohort(ind_per_km2/2.0, 0, SEX_MALE));
	else{ // cohort exists already
		// create new temporary cohort object to merge into
		// existing cohort
		HerbivoreCohort new_cohort = create_cohort(
				ind_per_km2/2.0, 0, SEX_MALE);
		itr_male->merge(new_cohort);
	}

	List::iterator itr_female = find_cohort(0, SEX_FEMALE);
	if (itr_female == list.end()) // no existing cohort
		list.push_back(create_cohort(ind_per_km2/2.0, 0, SEX_FEMALE));
	else{ // cohort exists already
		// create new temporary cohort object to merge into
		// existing cohort
		HerbivoreCohort new_cohort = create_cohort(
				ind_per_km2/2.0, 0, SEX_FEMALE);
		itr_female->merge(new_cohort);
	}
}

void CohortPopulation::establish(){
	if (!get_list().empty())
		throw std::logic_error("Fauna::CohortPopulation::establish() "
				"Trying to establish into a non-empty population.");
	if (get_hft().establishment_density == 0.0)
		return;

	assert(list.empty());

	// Since the list is empty, we can simply create new cohorts
	// without needing to check if the age-class already exists.
	// add males
	list.push_back(create_cohort(
				get_hft().establishment_density/2.0, // [ind/km²]
				get_hft().maturity_age_phys_male,
				SEX_MALE));
	// add females
	list.push_back(create_cohort(
				get_hft().establishment_density/2.0, // [ind/km²]
				get_hft().maturity_age_phys_female,
				SEX_FEMALE));

}

CohortPopulation::List::iterator CohortPopulation::find_cohort(
		const int age_years, const Sex sex){
	for (List::iterator itr = list.begin(); itr!=list.end(); itr++)
		if (itr->get_age_years() == age_years &&
				itr->get_sex() == sex)
			return itr;
	return list.end(); // not found
}

std::vector<const HerbivoreInterface*> CohortPopulation::get_list()const{
	std::vector<const HerbivoreInterface*> result;
	result.reserve(list.size());
	for (List::const_iterator itr=list.begin(); 
			itr != list.end(); itr++){
		// only add alive cohorts
		if (itr->get_ind_per_km2() >= dead_herbivore_threshold)
			result.push_back(&*itr);
	}
	return result;
}

std::vector<HerbivoreInterface*> CohortPopulation::get_list(){
	std::vector<HerbivoreInterface*> result;
	result.reserve(list.size());
	List::iterator itr = list.begin();
	while (itr != list.end()){
		if (itr->get_ind_per_km2() >= dead_herbivore_threshold){
			// cohort is alive, add it to the list
			result.push_back(&*itr);
			itr++;
		}
		else
			itr = list.erase(itr); // remove dead cohort
	}
	return result;
}

//============================================================
// HftPopulationsMap
//============================================================

void HftPopulationsMap::add(std::auto_ptr<PopulationInterface> new_pop) {
	if (new_pop.get() == NULL)
		throw std::invalid_argument("HftPopulationsMap::add(): "
				"Parameter new_pop is NULL.");

	const Hft& hft = new_pop->get_hft();

	for (iterator iter=begin(); iter!=end(); iter++){
		PopulationInterface& pop = **iter;
		if (pop.get_hft() == hft)
			throw std::logic_error("HftPopulationsMap::add(): "
					"A population of HFT \""+hft.name+"\" already exists.");
	}

	// release the pointer from the owner std::auto_ptr and add it
	// to the vector -> transfer of ownership
	vec.push_back(new_pop.release());
}

HerbivoreVector HftPopulationsMap::get_all_herbivores(){
	// This function is called many times.
	// In order to reserve adequate space for the vector and thus
	// avoid memory reallocation, we remember the vector size of the
	// last call and use it as an estimate for this call.

	HerbivoreVector result;
	result.reserve( last_all_herbivores_count*1.1 ); // reserve last size + some etra

	for (iterator itr_pop = begin(); itr_pop != end(); itr_pop++){
		// get herbivore vector from one population
		HerbivoreVector vec = (*itr_pop)->get_list();
		// append the vector
		result.reserve( vec.size() + result.size() );
		result.insert( result.end(), vec.begin(), vec.end() );
	}
	last_all_herbivores_count = result.size();
	return result;
}

PopulationInterface& HftPopulationsMap::operator[](const Hft& hft){
	for (iterator iter=begin(); iter!=end(); iter++){
		PopulationInterface& pop = **iter;
		if (pop.get_hft() == hft)
			return pop;
	}
	// loop ended without finding the HFT
	throw std::invalid_argument("HftPopulationsMap::operator[](): "
			"No population of HFT \""+hft.name+"\"");
}

HftPopulationsMap::~HftPopulationsMap(){
	for (iterator itr=begin(); itr!=end(); itr++)
		delete *itr;
	vec.clear();
}


