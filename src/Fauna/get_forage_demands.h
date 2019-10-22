/**
 * \file
 * \brief Helper class to determine how much an herbivore wants to eat.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_GET_FORAGE_DEMANDS_H
#define FAUNA_GET_FORAGE_DEMANDS_H

#include "forage_values.h"
#include "habitat_forage.h"

namespace Fauna {
// Forward Declarations
class Hft;
enum class Sex;

/// Function object to calculate forage demands for a herbivore.
/**
 * \see HerbivoreInterface::get_forage_demands()
 */
class GetForageDemands {
 public:
  /// Constructor.
  /**
   * \param hft Herbivore functional type.
   * \param sex The sex of the herbivore individual/cohort.
   * \throw std::invalid_argument If `hft==NULL`.
   */
  GetForageDemands(const Hft* hft, const Sex sex);

  /// Register ingested forage so that less forage will be demanded.
  /**
   * \param eaten_forage Ingested plant material [kgDM/ind].
   * \throw std::logic_error If `eaten_forage` exceeds the maximum
   * intake of today.
   */
  void add_eaten(ForageMass eaten_forage);

  /// Get maximum intake for each forage type limited by total mass.
  /**
   * \param mj_proportions Diet composition in energy proportions
   * [MJ/MJ].
   * \param mj_per_kg Energy content of forage [MJ/kgDM].
   * \param kg_total The maximum total intake [kgDM/day].
   * \return The maximum intake for each forage type [kgDM/day] while
   * retaining the given relative energy proportions.
   * \throw std::invalid_argument If `kg_total < 0.0`.
   * \throw std::invalid_argument If `mj_proportions.sum() != 1.0`.
   * \see \ref DL_ALLOMETRIC
   */
  static ForageMass get_max_intake_as_total_mass(const ForageFraction& mj_proportions,
      const ForageEnergyContent& mj_per_kg,
      const double kg_total);

  /// Initialize foraging for another day.
  /**
   * - Calculate the diet composition with different forage types.
   * - Set \ref max_intake to the minimum of digestive and foraging
   *   constraints.
   * \param available_forage The forage in the habitat.
   * \param day Current day of year, 0=Jan. 1st.
   * \param bodymass Current live weight body mass [kg/ind].
   * \param energy_content Net energy content of the available forage
   * [MJ/kgDM].
   * \throw std::invalid_argument If `day` not in [0,364] or if
   * `bodymass<=0` or if `bodymass` greater than adult body mass.
   */
  void init_today(const int day, const HabitatForage& available_forage,
                  const ForageEnergyContent& energy_content,
                  const double bodymass);

  /// Whether the given day has been initialized with \ref init_today().
  /** \throw std::invalid_argument If `day<0 || day>364`. */
  bool is_day_initialized(const int day) const;

  /// Calculate current forage demands.
  /**
   * Call this only after \ref init_today().
   * \return Forage [kgDM/ind/day] demanded by the herbivore today.
   * This will not exceed the available forage in the patch.
   * \throw std::logic_error If \ref init_today() hasn’t been called
   * yet.
   * \throw std::invalid_argument If `energy_needs < 0.0`.
   */
  ForageMass operator()(const double energy_needs);

 private:
  /// Adult herbivore body mass [kg/ind].
  double get_bodymass_adult() const;

  /// The herbivore functional type.
  const Hft& get_hft() const {
    assert(hft != NULL);
    return *hft;
  }

  /// Get energy-wise preferences for forage types.
  /**
   * To what fractions the different forage types are eaten (in
   * sum the fractions must be 1.0).
   *
   * \ref Hft::foraging_diet_composer defines the algorithm used to put
   * together the fractions of different forage types in the preferred
   * diet for each day.
   * Note that this function may be called several times a day in
   * cases of food scarcity, when the available forage needs to be
   * split among herbivores according to their needs
   * (see \ref DistributeForage).
   * This allows for switching to another, less preferred, forage
   * type if the first choice is not available anymore.
   *
   * This is the ad-libidum diet according to the preferences of the
   * HFT.
   * The fractions refer to energy, not mass.
   * The composition is *set*, i.e. that the demanded forage will
   * be put together accordingly.
   * In case of forage shortage in the habitat,
   * there is the chance to switch to other forage types when the
   * demands are queried again in the same day.
   * (⇒ see Fauna::DistributeForage).
   * \return Energy fractions of forage types composing current diet;
   * the sum is 1.0.
   * \throw std::logic_error If the \ref Hft::foraging_diet_composer is
   * not implemented.
   * \throw std::logic_error If the selected algorithm does not
   * produce a sum of fractions that equals 1.0 (100%).
   */
  ForageFraction get_diet_composition() const;

  /// Maximum forage [kgDM/ind/day] that could be potentially digested.
  /**
   * The algorithm selected by \ref Hft::digestion_limit is employed.
   * Note that this is only the digestion-limited maximum intake.
   * It does not consider metabolic needs (“hunger”, compare
   * \ref FatmassEnergyBudget::get_energy_needs()) nor foraging
   * capabilities (\ref get_max_foraging()) nor actual available
   * forage.
   *
   * \return Maximum digestible dry matter today with given forage
   * composition [kgDM/ind/day].
   * \throw std::logic_error If the \ref Hft::digestion_limit is not
   * implemented.
   */
  ForageMass get_max_digestion() const;

  /// Get the amount of forage the herbivore would be able to
  /// harvest [kgDM/day/ind].
  /**
   * The relative amount of each forage type is prescribed, and
   * the absolute mass that the herbivore could potentially ingest
   * is returned. This does not consider digestive limits or actual
   * metabolic needs (“hunger”), but only considers the harvesting
   * efficiency of the herbivore this day.
   *
   * Each forage type is  calculated separately and independently.
   *
   * \return Maximum potentially harvested dry matter mass of
   * each forage type [kgDM/day/ind].
   * \throw std::logic_error If one of \ref Hft::foraging_limits is
   * not implemented.
   */
  ForageMass get_max_foraging() const;

  /// Current day of the year, as set in \ref init_today().
  /** \throw std::logic_error If current day not yet set by an
   * initial call to \ref init_today(). */
  int get_today() const;

  /// @{ \name Constants
  Hft const* hft;
  Sex sex;
  /** @} */  // constants

  /// @{ \name State Variables
  HabitatForage available_forage;
  double bodymass;                     // [kg/ind]
  ForageFraction diet_composition;     // [frac.] sum = 1.0
  Digestibility digestibility;         // [frac.]
  ForageEnergyContent energy_content;  // [MJ/kgDM]
  double energy_needs;                 // [MJ/ind]
  ForageMass max_intake;               // [kgDM/ind/day]
  int today;                           // 0 = January 1st
  /** @} */                            // State Variables
};

}  // namespace Fauna

#endif  // FAUNA_GET_FORAGE_DEMANDS_H
