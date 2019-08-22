////////////////////////////////////////////////////////////
/// \file
/// \brief Instruction file parameters of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
////////////////////////////////////////////////////////////
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <cassert>  // for assert()
#include <stdexcept>

namespace Fauna {

/// Parameter for selecting algorithm for forage distribution among herbivores
enum ForageDistributionAlgorithm {
  /// Equal forage distribution: \ref Fauna::DistributeForageEqually
  FD_EQUALLY
};

/// Parameter for selecting the class implementing \ref
/// Fauna::HerbivoreInterface.
enum HerbivoreType {
  /// Use class \ref HerbivoreCohort
  HT_COHORT,
  /// Use class \ref HerbivoreIndividual
  HT_INDIVIDUAL
};

/// Time interval for aggregating output.
enum OutputInterval{
  /// Don’t aggregate output over time, but write every day.
  OI_DAILY,
  /// Aggregate output for each month.
  OI_MONTHLY,
  /// Aggregate output for each year.
  OI_ANNUAL,
  /// Aggregate output for 10 years intervals.
  OI_DECADAL
};

/// Parameter for selecting the output writer implementation.
enum OutputWriter {
  /// Use class \ref TextTableWriter
  OW_TEXT_TABLES
};

/// Parameters for the herbivory module.
struct Parameters {
  // alphabetical order

  /// Algorithm for how to distribute available forage among herbivores.
  /** Default: \ref FD_EQUALLY */
  ForageDistributionAlgorithm forage_distribution;

  /// Habitat area [km²].
  /** Only relevant if \ref herbivore_type == \ref HT_INDIVIDUAL. */
  double habitat_area_km2;

  /// Days between establishment check for herbivores.
  /** A value of `0` means no re-establishment. This is the default. */
  int herbivore_establish_interval;

  /// Which kind of herbivore class to use
  HerbivoreType herbivore_type;

  /// Whether to allow only herbivores of one HFT in each patch (default false).
  bool one_hft_per_patch;

  /// Time interval for aggregating output.
  OutputInterval output_interval = OI_ANNUAL;

  /// The module that writes megafauna output to disk.
  OutputWriter output_writer = OW_TEXT_TABLES;

  /// Constructor with default (valid!) settings
  Parameters()
      :  // alphabetical order
        forage_distribution(FD_EQUALLY),
        habitat_area_km2(100.0),
        herbivore_establish_interval(0),
        herbivore_type(HT_COHORT),
        one_hft_per_patch(false) {
    // Make sure that the default values are implemented
    // correctly
    assert(is_valid());
  }

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
  /// @}
};

}  // namespace Fauna

#endif  // PARAMETERS_H
