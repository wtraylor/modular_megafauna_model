/**
 * \file
 * \brief Read the instruction file with HFTs and global parameters.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_READ_INSFILE_H
#define FAUNA_READ_INSFILE_H

#include "cpptoml.h"
#include "hft_list.h"
#include "parameters.h"

namespace Fauna {

/// All global and herbivore parameters in an instruction file.
struct InsfileContent {
  /// Herbivore functional types from the instruction file.
  HftList hftlist;
  /// Global parameters from the instruction file.
  Parameters params;
};

/// Exception that a string parameter does not match possible options.
class invalid_option : public std::runtime_error {
 public:
  /// Constructor.
  /**
   * \param key The fully qualified TOML key.
   * \param value The given (invalid) value.
   * \param valid_options Possible valid values.
   * \throw std::logic_error If `valid_options` is empty.
   */
  invalid_option(const std::string& key, const std::string& value,
                 const std::set<std::string>& valid_options)
      : runtime_error(construct_message(key, value, valid_options)){};

 private:
  std::string construct_message(const std::string& key,
                                const std::string& value,
                                const std::set<std::string>& valid_options);
};

/// Exception that a parameter is missing in the instruction file.
class missing_parameter : public std::runtime_error {
 public:
  /// Constructor.
  /**
   * \param key The fully qualified TOML key.
   */
  missing_parameter(const std::string& key)
      : runtime_error("Missing mandatory parameter: \"" + key + '"'){};
};

/// Class to read parameters and HFTs from given instruction file.
class InsfileReader {
 public:
  /// Constructor
  /**
   * \param[in] filename Relative or absolute file path to the instruction
   * file.
   * \return The HFTs and parameters from the instruction file.
   * \throw TODO
   */
  InsfileReader(const std::string filename);

  /// Get the HFT list that was read from the instruction file.
  const HftList& get_hfts() const { return hfts; };

  /// Get the global parameters that were read from the instruction file.
  const Parameters& get_params() const { return params; };

 private:
  void read_table_output();
  void read_table_output_text_tables();
  void read_table_simulation();

  /// The root table of the instruction file from `cpptoml::parse_file()`.
  std::shared_ptr<cpptoml::table> ins;

  Parameters params;
  HftList hfts;
};
}  // namespace Fauna

#endif  // FAUNA_READ_INSFILE_H
