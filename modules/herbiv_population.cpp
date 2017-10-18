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

void IndividualPopulation::create_offspring_by_sex(
		const Sex sex,
		const double ind_per_km2)
{
	// Convert density to continuous individual count and add the 
	// remainder of previous offspring creation.
	const double ind_count_dbl = 
		ind_per_km2 * create_individual.get_area_km2() 
		+ incomplete_offspring[sex];
	
	// This is the discrete individual count:
	const int ind_count = (int) ind_count_dbl;
	
	// Save the new remainder (decimal part) for next time.
	incomplete_offspring[sex] = ind_count_dbl - ind_count;
	
	// Now create herbivore objects.
	static const double AGE_DAYS = 0; // age in days
	for (int i=1; i<=ind_count; i++)
		list.push_back( create_individual(AGE_DAYS, SEX_MALE) );
}

void IndividualPopulation::create_offspring(const double ind_per_km2){
	const int old_listsize = list.size();
	if (ind_per_km2 < 0.0)
		throw std::invalid_argument( "Fauna::IndividualPopulation::create() "
				"Parameter `ind_per_km2` is negative.");
	if (ind_per_km2 > 0.0){
		create_offspring_by_sex(SEX_MALE,   ind_per_km2/2.0);
		create_offspring_by_sex(SEX_FEMALE, ind_per_km2/2.0);
	}
}

void IndividualPopulation::establish(){
	if (!get_list().empty())
		throw std::logic_error("Fauna::IndividualPopulation::establish() "
				"Trying to establish into a non-empty population.");
	if (get_hft().establishment_density == 0.0)
		return;

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
	cumulated_offspring[SEX_MALE]   = 0.0;
	cumulated_offspring[SEX_FEMALE] = 0.0;
}

void CohortPopulation::create_offspring_by_sex(const Sex sex, 
		double ind_per_km2)
{
	assert(ind_per_km2 >= 0.0);

	// Add offspring from previous calls that was then too low to be
	// established.
	ind_per_km2             += cumulated_offspring[sex];
	cumulated_offspring[sex] = 0.0;

	List::iterator found = find_cohort(0, sex);
	if (found == list.end())
	{ // no existing cohort

		// Only create a new cohort if it is above the threshold.
		if (ind_per_km2 > dead_herbivore_threshold)
			list.push_back(create_cohort(ind_per_km2, 0, sex));
		else // otherwise remember it for next time.
			cumulated_offspring[sex] += ind_per_km2;

	}
	else{ // cohort exists already

		// Only merge new offspring into existing cohort if the resulting
		// density is viable (this new offspring would be “lost” otherwise.

		if (found->get_ind_per_km2() + ind_per_km2
				> dead_herbivore_threshold)
		{
			// create new temporary cohort object to merge into existing cohort
			HerbivoreCohort new_cohort = create_cohort(ind_per_km2, 0, sex);
			found->merge(new_cohort);
		} else // otherwise remember it for next time.
			cumulated_offspring[sex] += ind_per_km2;
	}

}

void CohortPopulation::create_offspring(const double ind_per_km2){
	if (ind_per_km2 < 0.0)
		throw std::invalid_argument(
				"Fauna::CohortPopulation::create_offspring() "
				"ind_per_km2 < 0.0");

	if (ind_per_km2 != 0.0){
		create_offspring_by_sex(SEX_MALE, ind_per_km2/2.0);
		create_offspring_by_sex(SEX_FEMALE, ind_per_km2/2.0);
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
		const int age_years, const Sex sex)
{
	for (List::iterator itr = list.begin();
			itr!=list.end();
			itr++)
	{
		if ((int) itr->get_age_years() == age_years &&
				itr->get_sex() == sex)
			return itr;
	}
	return list.end(); // not found
}

std::vector<const HerbivoreInterface*> CohortPopulation::get_list()const{
	// Here we cannot change this object, but we need to create a new list
	// without the dead cohorts.
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
	// Here we can change this object and directly erase dead cohorts from
	// the list.
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


