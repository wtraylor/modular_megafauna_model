/**
 * \file
 * \brief Global parameters for the megafauna library.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_PARAMETERS_H
#define FAUNA_PARAMETERS_H

#include <cassert>
#include <stdexcept>
#include "Fauna/Output/text_table_writer_options.h"
#include "forage_values.h"

namespace Fauna {

/// Parameter for selecting algorithm for forage distribution among herbivores
enum class ForageDistributionAlgorithm {
  /// Equal forage distribution: \ref Fauna::DistributeForageEqually
  Equally
};

/// Parameter for selecting the class implementing \ref
/// Fauna::HerbivoreInterface.
enum class HerbivoreType {
  /// Use class \ref HerbivoreCohort
  Cohort,
  /// Use class \ref HerbivoreIndividual
  Individual
};

/// Time interval for aggregating output.
// Note that we define it as a strictly typed C++11 enum *class* in order to be
// able to forward-declare it in other header files.
enum class OutputInterval {
  /// Don’t aggregate output over time, but write every day.
  Daily,
  /// Aggregate output for each month.
  Monthly,
  /// Aggregate output for each year.
  Annual,
  /// Aggregate output for 10 years intervals.
  Decadal
};

/// Parameter for selecting the output writer implementation.
enum class OutputFormat {
  /// Use class \ref Output::TextTableWriter.
  TextTables
};

/// Parameters for the herbivory module.
/**
 * Like in the \ref Hft class, each member variable corresponds to a key in the
 * TOML instruction file. Both are spelled the same, and the member variable is
 * prefixed with the category (i.e. the TOML table). Note that general
 * simulation parameters are in the TOML table “simulation”, but their
 * corresponding member variables don’t have a prefix.
 *
 * The initialization values are the same as in the example file under
 * `examples/megafauna.toml`.
 */
struct Parameters {
  /** @{ \name "simulation": General simulation parameters. */
  /// Algorithm for how to distribute available forage among herbivores.
  ForageDistributionAlgorithm forage_distribution =
      ForageDistributionAlgorithm::Equally;

  /// Habitat area [km²].
  /** Only relevant if \ref herbivore_type == \ref HerbivoreType::Individual. */
  double habitat_area_km2 = 1.0;

  /// Days between establishment check for herbivores.
  /** A value of `0` means no re-establishment. */
  int herbivore_establish_interval = 0;

  /// Which kind of herbivore class to use.
  HerbivoreType herbivore_type = HerbivoreType::Cohort;

  /// Metabolizable energy content for forage types [MJ/kgDM].
  /**
   * \see \ref get_net_energy_content_default()
   *
   * \section Grass
   * Givens et al. (1989, p. 39)\cite givens1989digestibility :
   *
   * > “In the absence of energetic data, it has been common to calculate ME
   * > from DOMD content. MAFF et al. (1984) \cite maff1984energy stated that
   * > for a wide range of feedstuffs ME may be calculated as 0.015*DOMD. This
   * > is based on the assumption that the GE of digested OM is 19.0 MJ/kg
   * > together with a ME/DE ratio of 0.81.”
   *
   * - ME = Metabolizable Energy [MJ/kg]
   * - DE = Digestible Energy [MJ/kg]
   * - GE = Gross Energy [MJ/kg]
   * - OM = Organic Matter [kg]
   * - DOMD = Digestible Organic Matter Content [percent]
   *        = digestibility for dry matter forage
   * \note ME is in MJ/kg, but appears in the above quotation as divided
   * by 100 to compensate for DOMD being in percent [0--100].
   */
  ForageEnergyContent metabolizable_energy = {15.0};

  /// Whether to allow only herbivores of one HFT in each habitat.
  bool one_hft_per_habitat = false;
  /** @} */

  /** @{ \name "output": General output options. */
  /// The module that writes megafauna output to disk.
  OutputFormat output_format = OutputFormat::TextTables;

  /// Time interval for aggregating output.
  OutputInterval output_interval = OutputInterval::Annual;

  /// Options for \ref Output::TextTableWriter, in TOML table
  /// "output.text_tables".
  Output::TextTableWriterOptions output_text_tables;
  /** @} */

  /// Check if the parameters are valid
  /**
   * \param[out] messages Warning and error messages.
   * \return true if everything is valid, false if not
   */
  bool is_valid(std::string& messages) const;

  /// Check if the parameters are valid
  /** \return true if everything is valid, false if not */
  bool is_valid() const {
    std::string dump;
    return is_valid(dump);
  }
};

}  // namespace Fauna

#endif  // FAUNA_PARAMETERS_H
