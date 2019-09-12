/**
 * \file
 * \brief Population class for herbivore individuals.
 * \copyright ...
 * \date 2019
 */
#include "individual_population.h"
#include "herbivore_individual.h"
#include "hft.h"

using namespace Fauna;

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
    list.push_back(create_individual(AGE_DAYS, Sex::Male));
}

void IndividualPopulation::create_offspring(const double ind_per_km2) {
  const int old_listsize = list.size();
  if (ind_per_km2 < 0.0)
    throw std::invalid_argument(
        "Fauna::IndividualPopulation::create() "
        "Parameter `ind_per_km2` is negative.");
  if (ind_per_km2 > 0.0) {
    create_offspring_by_sex(Sex::Male, ind_per_km2 / 2.0);
    create_offspring_by_sex(Sex::Female, ind_per_km2 / 2.0);
  }
}

void IndividualPopulation::establish() {
  if (!get_list().empty())
    throw std::logic_error(
        "Fauna::IndividualPopulation::establish() "
        "Trying to establish into a non-empty population.");
  if (get_hft().establishment.density == 0.0) return;

  // determine total number of individuals, assuming even sex ratio.
  int ind_count =
      ceil(get_hft().establishment.density * create_individual.get_area_km2());

  // Now distribute the number of individuals as evenly as possible over
  // the age range that is defined in the HFT.

  const int age_class_count = get_hft().establishment.age_range.second -
                              get_hft().establishment.age_range.first + 1;
  assert(age_class_count > 0);

  const int ind_count_per_age = ind_count / age_class_count;
  int ind_count_remainder = ind_count % age_class_count;
  assert(ind_count_per_age * age_class_count + ind_count_remainder ==
         ind_count);

  for (int age_years = get_hft().establishment.age_range.first;
       age_years <= get_hft().establishment.age_range.second; age_years++) {
    int count = ind_count_per_age;
    if (ind_count_remainder > 0) {
      count++;
      ind_count_remainder--;
    }

    // add new objects to the list, alternate male and female with odd and
    // even numbers
    for (int i = 1; i <= count; i++) {
      list.push_back(create_individual(age_years * 365,
                                       i % 2 == 0 ? Sex::Female : Sex::Male));
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

