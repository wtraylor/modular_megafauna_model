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
  /// Use class \ref TextTableWriter.
  TextTables
};

/// Parameters for the herbivory module.
struct Parameters {
  // alphabetical order

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

  /// Whether to allow only herbivores of one HFT in each patch (default false).
  bool one_hft_per_patch = false;

  /// General output options.
  struct {
    /// The module that writes megafauna output to disk.
    OutputFormat format = OutputFormat::TextTables;

    /// Time interval for aggregating output.
    OutputInterval interval = OutputInterval::Annual;
  } output;

  /// Preferences for the \ref TextTableWriter output class.
  /**
   * Which tables to write in text files is specified by boolean variables: If
   * the corresponding member variable is `true`, a file with the same name
   * plus extension (\ref TextTableWriter::FILE_EXTENSION) will be created in
   * \ref directory.
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

    /** @{ \name Output Files */

    /// Herbivore mass density per HFT in kg/km².
    bool mass_density_per_hft = false;

    /** @} */  // Output Files
  } text_table_output;

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
