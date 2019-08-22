///////////////////////////////////////////////////////////////////
/// \file
/// \brief Habitat output data.
/// \author Wolfgang Traylor, Senckenberg BiK-F
/// \date June 2019
/// \see \ref sec_output
////////////////////////////////////////////////////////////////////
#ifndef HERBIVORE_DATA
#define HERBIVORE_DATA

#include <map>
#include "forage_values.h"  // for ForageMass
#include "hft.h"            // for MortalityFactor

namespace Fauna {
namespace Output {
/// Herbivore output data for one time unit.
/** \see \ref sec_outputclasses */
struct HerbivoreData {
  /// Constructor, initializing with zero values.
  HerbivoreData()
      : age_years(0.0),
        bodyfat(0.0),
        bound_nitrogen(0.0),
        eaten_forage_per_ind(0.0),
        eaten_forage_per_mass(0.0),
        eaten_nitrogen_per_ind(0.0),
        energy_content(0.0),
        energy_intake_per_ind(0.0),
        energy_intake_per_mass(0.0),
        expenditure(0.0),
        inddens(0.0),
        massdens(0.0),
        offspring(0.0) {}

  //------------------------------------------------------------
  /** @{ \name Per-individual variables */

  /// Age in years.
  double age_years;

  /// Body fat [fraction].
  double bodyfat;

  /// Energy expenditure [MJ/ind/day].
  double expenditure;

  /** @} */  // Per-Individual variables

  //------------------------------------------------------------
  /** @{ \name Per-habitat variables */

  /// Pool of nitrogen bound in the herbivores [kgN/km²]
  double bound_nitrogen;

  /// Individual density [ind/km²].
  double inddens;

  /// Mass density [kg/km²].
  double massdens;

  /// Daily mortality rate [ind/ind/day].
  std::map<Fauna::MortalityFactor, double> mortality;

  /// Newborns (offspring) per day [ind/km²/day].
  double offspring;

  /// Eaten forage per individual [kgDM/ind/day].
  Fauna::ForageMass eaten_forage_per_ind;

  /// Eaten forage per body mass [kgDM/kg/day].
  Fauna::ForageMass eaten_forage_per_mass;

  /// Ingested nitrogen mass per individual and day [kgN/ind/day].
  double eaten_nitrogen_per_ind;

  /// Net energy content of available forage [MJ/kgDM].
  Fauna::ForageEnergyContent energy_content;

  /// Intake of net energy in forage per individual [MJ/ind/day]
  Fauna::ForageEnergy energy_intake_per_ind;

  /// Intake of net energy in forage per herbivore mass [MJ/kg/day]
  Fauna::ForageEnergy energy_intake_per_mass;

  /** @} */  // Per-habitat variables

  //------------------------------------------------------------
  /** @{ \name Aggregation Functionality */
  /// Aggregate data of this object with another one.
  /**
   * This function builds **averages** for all member variables.
   *
   * \ref mortality : Only those factors are included in the
   * result that are present in both objects (intersection).
   * All other map entries are deleted. This is necessary because
   * the statistical weight is the same for *all* variables.
   *
   * This does no calculations if the partners are the same object, or
   * one of the weights is zero.
   *
   * \param other The other object to be merged into this one.
   * \param this_weight Weight of this object in average building.
   * \param other_weight Weight of `other` in average building.
   * \return This object.
   * \see \ref Fauna::average(), \ref Fauna::ForageValues::merge()
   * \throw std::invalid_argument If either weight is not a positive
   * number or if both are zero.
   */
  HerbivoreData& merge(const HerbivoreData& other, const double this_weight,
                       const double other_weight);

  /// Reset to initial values.
  void reset() {
    // Simply call the copy assignment operator
    this->operator=(HerbivoreData());
  }

  /// Aggregate herbivore data *within one habitat*.
  /**
   * As opposed to \ref merge(), this function is intended to combine
   * data of *one habitat* in *one point of time* into a single data
   * point.
   * This can then be merged with other data points across space and
   * time, using \ref merge().
   *
   * For variables *per individual*, this function creates the
   * **average** (just like \ref merge()).
   * For variables *per area* or *per habitat*, this function creates
   * the **sum**, adding up the numbers in the habitat.
   *
   * In contrast to \ref merge(), \ref mortality is summed up, and
   * all mortality factors are included because all merged datapoints
   * have the same weight.
   *
   * \throw std::invalid_argument If length of vector `data` is zero.
   */
  static HerbivoreData create_datapoint(const std::vector<HerbivoreData> data);

  /// Build weighted mean for net energy content, not counting zero values.
  /** Don’t count zero net energy, which results from zero available forage.
   * We need to check every forage type and build average only if energy
   * content in `obj2` is not zero.
   * \see \ref Fauna::average()
   */
  static void merge_energy_content(Fauna::ForageEnergyContent& obj1,
                                   const Fauna::ForageEnergyContent& obj2,
                                   const double weight1 = 1.0,
                                   const double weight2 = 1.0);

  /** @} */  // Aggregation Functionality
};
}  // namespace Output
}  // namespace Fauna

#endif  // HERBIVORE_DATA
