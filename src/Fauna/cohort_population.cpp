/**
 * \file
 * \brief Population class for herbivore cohorts.
 * \copyright ...
 * \date 2019
 */
#include "cohort_population.h"
#include "herbivore_cohort.h"
#include "hft.h"

using namespace Fauna;

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
    create_offspring_by_sex(Sex::Male, ind_per_km2 / 2.0);
    create_offspring_by_sex(Sex::Female, ind_per_km2 / 2.0);
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
        age, Sex::Male));

    // add females
    list.push_back(create_cohort(
        get_hft().establishment_density / cohort_count,  // [ind/km²]
        age, Sex::Female));
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

ConstHerbivoreVector CohortPopulation::get_list() const {
  // We just copy the pointers from the cohort list to the HerbivoreInterface
  // list.
  std::vector<const HerbivoreInterface*> result;
  result.reserve(list.size());
  for (List::const_iterator itr = list.begin(); itr != list.end(); itr++) {
    result.push_back(&*itr);
  }
  return result;
}

HerbivoreVector CohortPopulation::get_list() {
  // We just copy the pointers from the cohort list to the HerbivoreInterface
  // list.
  std::vector<HerbivoreInterface*> result;
  result.reserve(list.size());
  for (List::iterator itr = list.begin(); itr != list.end(); itr++) {
    result.push_back(&*itr);
  }
  return result;
}

void CohortPopulation::kill_nonviable() {
  // If the population’s density is below minimum, mark all
  // herbivores as dead.
  const double min_ind_per_km2 = get_hft().mortality_minimum_density_threshold *
                                 get_hft().establishment_density;
  if (get_ind_per_km2() < min_ind_per_km2) kill_all();
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
