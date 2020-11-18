// SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Nitrogen uptake and excretion by herbivores
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_NITROGEN_H
#define FAUNA_NITROGEN_H

namespace Fauna {
/// Time of ingesta to pass digestion [hours].
/**
 * Mean retention time (MRT in hours) after
 * Clauss et al. (2007) \cite clauss2007case, Fig. 2, animals
 * heavier than 0.5 kg:
 * \f[
 * 		MRT = 32.8 * M^{0.07}
 * \f]
 * \param bodymass Herbivore live weight [kg/ind].
 * \throw std::invalid_argument If `bodymass <= 0`.
 */
double get_retention_time(const double bodymass);

/// Manages nitrogen uptake & excretion for on herbivore object.
/**
 *
 * The nitrogen (\f$N_{bound}\f$, kgN/ind) inside an animal is the sum
 * of nitrogen in gut content (\f$N_{guts}\f$) and in body tissue
 * (\f$N_{body}\f$).
 *
 * <H3> Nitrogen in Guts </H3>
 * How much ingesta are currently in the guts depends on the daily
 * nitrogen intake (\f$I_N\f$, kgN/ind/day) and the mean retention time
 * (MRT, h).
 * \f[
 * N_{guts} = I_N * MRT
 * \f]
 *
 * <H3> Nitrogen in Body Tissue </H3>
 * \f[
 * N_{body} = M * 0.03
 * \f]
 *
 *
 * Call \ref ingest() any number of times within one day. Then call
 * \ref digest_today() at the end of the day in order to calculate how
 * much ingesta have moved through the body and are available to the
 * soil as excreta.
 *
 * \note If the individual density changes (e.g. some animal die in a
 * cohort), the nitrogen pools stay the same because they are *per area*.
 * With the next call of \ref digest_today(), the nitrogen “surplus”
 * will be counted as excreta. This way, no nitrogen is lost in the
 * system.
 * \see \ref HerbivoreInterface::take_nitrogen_excreta()
 * \see \ref sec_nitrogen_cycling
 * \see \ref get_retention_time()
 */
class NitrogenInHerbivore {
 public:
  /// Constructor.
  NitrogenInHerbivore() : bound(0.0), excreta(0.0), ingested(0.0) {}

  /// The nitrogen that has been excreted [kgN/km²].
  double get_excreta() const { return excreta; }

  /// Get the nitrogen that is inside the animal [kgN/km²].
  double get_unavailable() const { return bound + ingested; }

  /// Move ingested nitrogen into excreta pool.
  /**
   * \param retention_time Time of ingesta to pass digestion [hours].
   * \param massdens Herbivore live weight per area [kg/km²]. Set to
   * zero in order to effectively disable accounting nitrogen in live
   * tissue.
   * \throw std::invalid_argument If `retention_time <= 0` or
   * `massdens < 0`.
   * \see \ref get_retention_time()
   */
  void digest_today(const double retention_time, const double massdens);

  /// Take up nitrogen from forage.
  /**
   * \param eaten_nitrogen Eaten nitrogen [kgN/km²/day].
   * \throw std::invalid_argument If `eaten_nitrogen < 0.0`.
   */
  void ingest(const double eaten_nitrogen);

  /// Merge this object with another one by summing up the nitrogen pools.
  /**
   * \param other The other object.
   */
  void merge(const NitrogenInHerbivore& other);

  /// Reset the excreted nitrogen.
  /**
   * Call this when the excreted nitrogen has been accounted for in
   * the soil.
   * \return Same as \ref get_excreta().
   */
  double reset_excreta();

  /// Reset *all* nitrogen (excreta + gut content + tissue nitrogen).
  /**
   * Call this when the herbivore is dead and the body shall decompose.
   * \return Same as \ref get_unavailable() + \ref get_excreta()
   */
  double reset_total();

  /// Fraction of nitrogen in live weight herbivore mass [kgN/kg].
  /**
   * Hobbs (2006, p. 292)\cite hobbs2006large cites
   * Robbins (1983)\cite robbins1983wildlife, estimating nitrogen content in the
   * body of large mammalian herbivors as 2.5%–3.0%. We choose 3.0% here.
   */
  static const double N_CONTENT_IN_TISSUE;

 private:
  /// Nitrogen in the guts and body of the animal [kgN/km²].
  double bound;

  /// Already excreted nitrogen, ready to go into soil [kgN/km²].
  double excreta;

  /// Nitrogen eaten by the population today [kgN/km²].
  double ingested;
};
}  // namespace Fauna

#endif  //  FAUNA_NITROGEN_H
