/**
 * \file
 * \brief Population class for herbivore cohorts.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_COHORT_POPULATION_H
#define FAUNA_COHORT_POPULATION_H

#include "population_interface.h"
#include <list>
#include "create_herbivores.h"

namespace Fauna {

// Forward Declarations
enum class Sex;

/// A population of \ref HerbivoreCohort objects.
class CohortPopulation : public PopulationInterface {
 public:  // ------ PopulationInterface -------
  /** \copydoc PopulationInterface::create_offspring() */
  virtual void create_offspring(const double ind_per_km2);
  /** \copydoc PopulationInterface::establish()
   * Establish with even sex ratio and *at least* as many
   * individuals as given by \ref Hft::establishment_density.
   */
  virtual void establish();
  virtual const Hft& get_hft() const { return create_cohort.get_hft(); }
  virtual ConstHerbivoreVector get_list() const;
  virtual HerbivoreVector get_list();
  virtual void purge_of_dead();

 public:
  /// Constructor
  /**
   * \param create_cohort Functor for creating new
   * \ref HerbivoreCohort instances.
   * \throw std::invalid_argument if any parameter is wrong.
   */
  CohortPopulation(const CreateHerbivoreCohort create_cohort);

 private:
  typedef std::list<HerbivoreCohort> List;

  /// Add newborn animals to the population either males or females.
  /** \see \ref create_offspring() */
  void create_offspring_by_sex(const Sex sex, double ind_per_km2);

  /// Find a cohort in the list.
  /**
   * \param age_years Age-class number (0=first year of life).
   * \param sex Male or female cohort?
   * \return If found: iterator pointing to the \ref
   * HerbivoreCohort object. If not found: end() iterator of
   * the cohort list.
   */
  List::iterator find_cohort(const int age_years, const Sex sex);

  const CreateHerbivoreCohort create_cohort;
  /// Offspring accumulated until above minimum threshold [ind/km²].
  List list;
};
}
#endif // FAUNA_COHORT_POPULATION_H
