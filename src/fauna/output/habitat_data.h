///////////////////////////////////////////////////////////////////
/// \file
/// \brief Habitat output data.
/// \author Wolfgang Traylor, Senckenberg BiK-F
/// \date June 2019
/// \see \ref sec_output
////////////////////////////////////////////////////////////////////
#ifndef HABITAT_DATA_H
#define HABITAT_DATA_H

#include "forageclasses.h" // for HabitatForage
#include "environment.h" // for HabitatEnvironment

namespace Fauna::Output {
/// Habitat output data for one time unit.
/**
 * \see \ref sec_outputclasses
 */
struct HabitatData {
  /// Constructor, initializing with zero values.
  HabitatData()
      : available_forage(),  // already zeros by default.
        eaten_forage()       // already zeros by default.
  {}

  /// Available forage in the habitat.
  Fauna::HabitatForage available_forage;

  /// Forage mass [kgDM/km²/day] eaten by herbivores.
  /** This equals the sum of \ref HerbivoreData::eaten_forage_per_ind
   * over all HFTs */
  Fauna::ForageMass eaten_forage;

  /// Abiotic conditions in the habitat.
  Fauna::HabitatEnvironment environment;

  //------------------------------------------------------------
  /** @{ \name Aggregation Functionality */
  /// Aggregate data of this object with another one.
  /**
   * This does no calculations if the partners are the same object, or
   * one of the weights is zero.
   * \param other The other object to be merged into this one.
   * \param this_weight Weight of this object in average building.
   * \param other_weight Weight of `other` in average building.
   * \return This object.
   * \see \ref Fauna::average(),
   *      \ref Fauna::HabitatForage::merge(),
   *      \ref Fauna::ForageValues::merge()
   * \throw std::invalid_argument If either weight is not a positive
   * number or if both are zero.
   */
  HabitatData& merge(const HabitatData& other, const double this_weight,
                     const double other_weight);

  /// Reset to initial values.
  void reset() {
    // Simply call the copy assignment operator
    this->operator=(HabitatData());
  }
  /** @} */  // Aggregation Functionality
};

} // namespace fauna::output

#endif // HABITAT_DATA_H
