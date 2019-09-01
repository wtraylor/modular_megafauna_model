#ifndef FAUNA_INDIVIDUAL_POPULATION_H
#define FAUNA_INDIVIDUAL_POPULATION_H

#include <list>
#include <map>
#include "create_herbivores.h"
#include "population_interface.h"

namespace Fauna {

/// A population of \ref HerbivoreIndividual objects.
class IndividualPopulation : public PopulationInterface {
 public:  // ------ PopulationInterface -------
  /** \copydoc PopulationInterface::create_offspring()
   * Since we can only creat ‘complete’ (discrete) individuals, but
   * the given density `ind_per_km2` is continuous, the remainder
   * (‘incomplete individual’) for each sex will be remembered until
   * next call of `create_offspring()`.
   */
  virtual void create_offspring(double ind_per_km2);
  virtual void establish();
  virtual const Hft& get_hft() const { return create_individual.get_hft(); }
  virtual ConstHerbivoreVector get_list() const;
  virtual HerbivoreVector get_list();
  virtual void purge_of_dead();

 public:
  /// Constructor
  /**
   * \param create_individual Functor for creating new
   * herbivore individuals.
   */
  IndividualPopulation(const CreateHerbivoreIndividual create_individual);

 private:
  /// Create either male or female newborn individuals.
  /** \see \ref create_offspring() */
  void create_offspring_by_sex(const Sex sex, const double ind_per_km2);

  const CreateHerbivoreIndividual create_individual;
  typedef std::list<HerbivoreIndividual> List;
  List list;
  /// ‘Incomplete` newborn herbivore (<1.0).
  /** \see \ref create_offspring() */
  std::map<Sex, double> incomplete_offspring;
};
}  // namespace Fauna
#endif  // FAUNA_INDIVIDUAL_POPULATION_H
