//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Management classes of herbivore populations.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
//////////////////////////////////////////////////////////////////////////

#include "population.h"
#include <cmath>      // for round() and ceil()
#include <sstream>    // for std::ostringstream
#include <stdexcept>  // for std::out_of_range
#include "config.h"
#include "herbivore.h"
#include "hft.h"

using namespace Fauna;

//============================================================
// PopulationInterface
//============================================================

const double PopulationInterface::get_kg_per_km2() const {
  double sum = 0.0;
  const ConstHerbivoreVector vec = get_list();
  for (ConstHerbivoreVector::const_iterator itr = vec.begin(); itr != vec.end();
       itr++)
    sum += (*itr)->get_kg_per_km2();
  return sum;
}

const double PopulationInterface::get_ind_per_km2() const {
  double sum = 0.0;
  const ConstHerbivoreVector vec = get_list();
  for (ConstHerbivoreVector::const_iterator itr = vec.begin(); itr != vec.end();
       itr++)
    sum += (*itr)->get_ind_per_km2();
  return sum;
}

void PopulationInterface::kill_all() {
  HerbivoreVector vec = get_list();
  for (HerbivoreVector::const_iterator itr = vec.begin(); itr != vec.end();
       itr++)
    (*itr)->kill();
}

//============================================================
// IndividualPopulation
//============================================================

IndividualPopulation::IndividualPopulation(
    const CreateHerbivoreIndividual create_individual)
    : create_individual(create_individual) {}

void IndividualPopulation::create_offspring_by_sex(const Sex sex,
                                                   const double ind_per_km2) {
  // Convert density to continuous individual count and add the
  // remainder of previous offspring creation.
  const double ind_count_dbl = ind_per_km2 * create_individual.get_area_km2() +
                               incomplete_offspring[sex];

  // This is the discrete individual count:
  const int ind_count = (int)ind_count_dbl;

  // Save the new remainder (decimal part) for next time.
  incomplete_offspring[sex] = ind_count_dbl - ind_count;

  // Now create herbivore objects.
  static const double AGE_DAYS = 0;  // age in days
  for (int i = 1; i <= ind_count; i++)
    list.push_back(create_individual(AGE_DAYS, SEX_MALE));
}

void IndividualPopulation::create_offspring(const double ind_per_km2) {
  const int old_listsize = list.size();
  if (ind_per_km2 < 0.0)
    throw std::invalid_argument(
        "Fauna::IndividualPopulation::create() "
        "Parameter `ind_per_km2` is negative.");
  if (ind_per_km2 > 0.0) {
    create_offspring_by_sex(SEX_MALE, ind_per_km2 / 2.0);
    create_offspring_by_sex(SEX_FEMALE, ind_per_km2 / 2.0);
  }
}

void IndividualPopulation::establish() {
  if (!get_list().empty())
    throw std::logic_error(
        "Fauna::IndividualPopulation::establish() "
        "Trying to establish into a non-empty population.");
  if (get_hft().establishment_density == 0.0) return;

  // determine total number of individuals, assuming even sex ratio.
  int ind_count =
      ceil(get_hft().establishment_density * create_individual.get_area_km2());

  // Now distribute the number of individuals as evenly as possible over
  // the age range that is defined in the HFT.

  const int age_class_count = get_hft().establishment_age_range.second -
                              get_hft().establishment_age_range.first + 1;
  assert(age_class_count > 0);

  const int ind_count_per_age = ind_count / age_class_count;
  int ind_count_remainder = ind_count % age_class_count;
  assert(ind_count_per_age * age_class_count + ind_count_remainder ==
         ind_count);

  for (int age_years = get_hft().establishment_age_range.first;
       age_years <= get_hft().establishment_age_range.second; age_years++) {
    int count = ind_count_per_age;
    if (ind_count_remainder > 0) {
      count++;
      ind_count_remainder--;
    }

    // add new objects to the list, alternate male and female with odd and
    // even numbers
    for (int i = 1; i <= count; i++) {
      list.push_back(create_individual(age_years * 365,
                                       i % 2 == 0 ? SEX_FEMALE : SEX_MALE));
    }
  }
  assert(ind_count_remainder == 0);
}

std::vector<const HerbivoreInterface*> IndividualPopulation::get_list() const {
  // We just copy the pointers from the individual list to the
  // HerbivoreInterface list.
  std::vector<const HerbivoreInterface*> result;
  result.reserve(list.size());
  for (List::const_iterator itr = list.begin(); itr != list.end(); itr++) {
    result.push_back(&*itr);
  }
  return result;
}

std::vector<HerbivoreInterface*> IndividualPopulation::get_list() {
  // We just copy the pointers from the individual list to the
  // HerbivoreInterface list.
  std::vector<HerbivoreInterface*> result;
  result.reserve(list.size());
  for (List::iterator itr = list.begin(); itr != list.end(); itr++) {
    result.push_back(&*itr);
  }
  return result;
}

void IndividualPopulation::purge_of_dead() {
  List::iterator itr = list.begin();
  while (itr != list.end()) {
    if (itr->is_dead())
      itr = list.erase(itr);  // Remove entry and increment iterator.
    else
      itr++;
  }
}

//============================================================
// CohortPopulation
//============================================================

CohortPopulation::CohortPopulation(const CreateHerbivoreCohort create_cohort)
    : create_cohort(create_cohort) {}

void CohortPopulation::create_offspring_by_sex(const Sex sex,
                                               double ind_per_km2) {
  assert(ind_per_km2 >= 0.0);

  List::iterator found = find_cohort(0, sex);
  if (found == list.end()) {  // no existing cohort
    list.push_back(create_cohort(ind_per_km2, 0, sex));
  } else {  // cohort exists already

    // create new temporary cohort object to merge into existing cohort
    HerbivoreCohort new_cohort = create_cohort(ind_per_km2, 0, sex);
    found->merge(new_cohort);
  }
}

void CohortPopulation::create_offspring(const double ind_per_km2) {
  if (ind_per_km2 < 0.0)
    throw std::invalid_argument(
        "Fauna::CohortPopulation::create_offspring() "
        "ind_per_km2 < 0.0");

  if (ind_per_km2 != 0.0) {
    create_offspring_by_sex(SEX_MALE, ind_per_km2 / 2.0);
    create_offspring_by_sex(SEX_FEMALE, ind_per_km2 / 2.0);
  }
}

void CohortPopulation::establish() {
  if (!get_list().empty())
    throw std::logic_error(
        "Fauna::CohortPopulation::establish() "
        "Trying to establish into a non-empty population.");
  if (get_hft().establishment_density == 0.0) return;

  assert(list.empty());

  // We create one male and one female for each age specified in the HFT.

  const double cohort_count = 2 * (get_hft().establishment_age_range.second -
                                   get_hft().establishment_age_range.first + 1);

  // Density of one cohort (ind/km²).
  const double cohort_density = get_hft().establishment_density / cohort_count;

  for (int age = get_hft().establishment_age_range.first;
       age <= get_hft().establishment_age_range.second; age++) {
    // Since the list is empty, we can simply create new cohorts
    // without needing to check if the age-class already exists.

    // add males
    list.push_back(create_cohort(
        get_hft().establishment_density / cohort_count,  // [ind/km²]
        age, SEX_MALE));

    // add females
    list.push_back(create_cohort(
        get_hft().establishment_density / cohort_count,  // [ind/km²]
        age, SEX_FEMALE));
  }
}

CohortPopulation::List::iterator CohortPopulation::find_cohort(
    const int age_years, const Sex sex) {
  for (List::iterator itr = list.begin(); itr != list.end(); itr++) {
    if ((int)itr->get_age_years() == age_years && itr->get_sex() == sex)
      return itr;
  }
  return list.end();  // not found
}

std::vector<const HerbivoreInterface*> CohortPopulation::get_list() const {
  // We just copy the pointers from the cohort list to the HerbivoreInterface
  // list.
  std::vector<const HerbivoreInterface*> result;
  result.reserve(list.size());
  for (List::const_iterator itr = list.begin(); itr != list.end(); itr++) {
    result.push_back(&*itr);
  }
  return result;
}

std::vector<HerbivoreInterface*> CohortPopulation::get_list() {
  // We just copy the pointers from the cohort list to the HerbivoreInterface
  // list.
  std::vector<HerbivoreInterface*> result;
  result.reserve(list.size());
  for (List::iterator itr = list.begin(); itr != list.end(); itr++) {
    result.push_back(&*itr);
  }
  return result;
}

void CohortPopulation::purge_of_dead() {
  List::iterator itr = list.begin();
  while (itr != list.end()) {
    if (itr->is_dead())
      itr = list.erase(itr);  // Remove entry and increment iterator.
    else
      itr++;
  }
}

//============================================================
// HftPopulationsMap
//============================================================

void HftPopulationsMap::add(std::auto_ptr<PopulationInterface> new_pop) {
  if (new_pop.get() == NULL)
    throw std::invalid_argument(
        "HftPopulationsMap::add(): "
        "Parameter new_pop is NULL.");

  const Hft& hft = new_pop->get_hft();

  for (iterator iter = begin(); iter != end(); iter++) {
    PopulationInterface& pop = **iter;
    if (pop.get_hft() == hft)
      throw std::logic_error(
          "HftPopulationsMap::add(): "
          "A population of HFT \"" +
          hft.name + "\" already exists.");
  }

  // release the pointer from the owner std::auto_ptr and add it
  // to the vector -> transfer of ownership
  vec.push_back(new_pop.release());
}

HerbivoreVector HftPopulationsMap::get_all_herbivores() {
  // This function is called many times.
  // In order to reserve adequate space for the vector and thus
  // avoid memory reallocation, we remember the vector size of the
  // last call and use it as an estimate for this call.

  HerbivoreVector result;
  result.reserve(last_all_herbivores_count *
                 1.1);  // reserve last size + some etra

  for (iterator itr_pop = begin(); itr_pop != end(); itr_pop++) {
    // get herbivore vector from one population
    HerbivoreVector vec = (*itr_pop)->get_list();
    // append the vector
    result.reserve(vec.size() + result.size());
    result.insert(result.end(), vec.begin(), vec.end());
  }
  last_all_herbivores_count = result.size();
  return result;
}

void HftPopulationsMap::kill_nonviable() {
  for (iterator itr = begin(); itr != end(); itr++) {
    PopulationInterface& pop = **itr;
    // If the population’s density is below minimum, mark all
    // herbivores as dead.
    const double min_ind_per_km2 = pop.get_hft().minimum_density_threshold *
                                   pop.get_hft().establishment_density;
    if (pop.get_ind_per_km2() < min_ind_per_km2) pop.kill_all();
  }
}

PopulationInterface& HftPopulationsMap::operator[](const Hft& hft) {
  for (iterator iter = begin(); iter != end(); iter++) {
    PopulationInterface& pop = **iter;
    if (pop.get_hft() == hft) return pop;
  }
  // loop ended without finding the HFT
  throw std::invalid_argument(
      "HftPopulationsMap::operator[](): "
      "No population of HFT \"" +
      hft.name + "\"");
}

HftPopulationsMap::~HftPopulationsMap() {
  for (iterator itr = begin(); itr != end(); itr++) delete *itr;
  vec.clear();
}
