/**
 * \file
 * \brief Options for \ref Fauna::Output::TextTableWriter.
 * \copyright ...
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
  /// Digestibility of available forage in the habitat.
  /** \see \ref Fauna::HabitatForage::get_digestibility() */
  bool digestibility = false;
  /** @} */

  /** @{ \name Per-HFT tables: one column per HFT. */
  /// Herbivore mass density per HFT in kg/km².
  /** \see \ref Fauna::Output::HerbivoreData::massdens */
  bool mass_density_per_hft = false;
  /** @} */
};
}  // namespace Output
}  // namespace Fauna

#endif
