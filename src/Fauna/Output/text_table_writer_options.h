// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Options for \ref Fauna::Output::TextTableWriter.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_OUTPUT_TEXT_TABLE_WRITER_OPTIONS_H
#define FAUNA_OUTPUT_TEXT_TABLE_WRITER_OPTIONS_H
#include <string>

namespace Fauna {
namespace Output {
/// Options for \ref Fauna::Output::TextTableWriter.
/**
 * Which tables to write in text files is specified by boolean variables: If
 * the corresponding member variable is `true`, a file with the same name
 * plus extension (\ref Output::TextTableWriter::FILE_EXTENSION) will be
 * created in \ref directory.
 * \see \ref Output::TextTableWriter
 */
struct TextTableWriterOptions {
  /// Relative or absolute path to directory where output files are placed.
  /**
   * The names of the output text files within the directory are hard-coded.
   * If the directory doesn’t exist, it will be created.
   */
  std::string directory = "./";

  /// Number of figures after the decimal point.
  unsigned int precision = 3;

  /** @{ \name Per-ForageType tables: one column per forage type. */

  /// Dry matter weight of available forage in the habitat [kgDM/km²].
  /** \see \ref Fauna::HabitatForage::get_mass() */
  bool available_forage = false;

  /// Digestibility of available forage in the habitat.
  /** \see \ref Fauna::HabitatForage::get_digestibility() */
  bool digestibility = false;

  /** @} */

  /** @{ \name By-HFT tables: one column for each HFT. */

  /// Proportional body fat, i.e. fat mass per total body mass [kg/kg].
  /** \see \ref Fauna::HerbivoreBase::get_bodyfat() */
  bool body_fat = false;

  /// Daily consumption of nitrogen by herbivore individuals [mgDM/day/ind].
  /**
   * This is measured in milligram because the amount might be very tiny. For
   * them to appear not as zeros in the output table, the unit must be made
   * very small.
   * \see \ref Fauna::Output::HerbivoreData::eaten_nitrogen_per_ind
   */
  bool eaten_nitrogen_per_ind = false;

  /// Herbivore individual density by HFT in ind/km².
  /** \see \ref Fauna::Output::HerbivoreData::inddens */
  bool individual_density = false;

  /// Herbivore mass density by HFT in kg/km².
  /** \see \ref Fauna::Output::HerbivoreData::massdens */
  bool mass_density = false;

  /**
   * \copydoc mass_density
   * \deprecated The name `per_hft` is confusing because in `per_ind` it has a
   * different meaning, e.g. in \ref eaten_forage_per_ind. Use
   * \ref mass_density instead.
   */
  bool mass_density_per_hft = false;  // DEPRECATED

  /** @} */

  /** @{ \name Per-HFT/per-forage tables: one column per HFT. */
  /// Daily consumption of dry matter by herbivore individuals [kgDM/day/ind].
  /** \see \ref Fauna::Output::HerbivoreData::eaten_forage_per_ind */
  bool eaten_forage_per_ind = false;
  /** @} */
};
}  // namespace Output
}  // namespace Fauna

#endif
