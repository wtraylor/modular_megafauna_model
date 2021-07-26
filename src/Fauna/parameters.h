// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Global parameters for the megafauna library.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_PARAMETERS_H
#define FAUNA_PARAMETERS_H

#include <cassert>
#include <stdexcept>
#include "forage_values.h"
#include "text_table_writer_options.h"

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
  Cohort
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

  /// Gross energy content for forage types [MJ/kgDM].
  /**
   * The gross energy is the energy released by complete combustion of dry
   * matter. It is usually measured in a bomb calorimeter.
   * \see \ref get_net_energy_from_gross_energy()
   * \see \ref sec_energy_content
   */
  ForageEnergyContent forage_gross_energy = {19.0};

  /// Days between establishment check for herbivores.
  /** A value of `0` means no re-establishment. */
  int herbivore_establish_interval = 0;

  /// Which kind of herbivore class to use.
  HerbivoreType herbivore_type = HerbivoreType::Cohort;

  /// Whether to allow only herbivores of one HFT in each habitat.
  /**
   * If this is activated, the habitats in each aggregation unit must be an
   * integer multiple of the number of HFTs. For instance, with LPJ-GUESS that
   * means that `npatch`, i.e. the number of patches (=habitats) per grid cell
   * (=aggregation unit) must be a multiple of the number of HFTs. If there are
   * 3 HFTs, `npatch` can take the value of 3, 6, 9, 12, etc.
   *
   * This rule is important because the aggregated output would otherwise not
   * be comparable between aggregation units.
   */
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
