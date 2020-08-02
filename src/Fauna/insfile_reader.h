/**
 * \file
 * \brief Read the instruction file with HFTs and global parameters.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_INSFILE_READER_H
#define FAUNA_INSFILE_READER_H

#include <list>
#include "cpptoml.h"
#include "parameters.h"

namespace Fauna {
// Forward declarations
class Hft;

// Repeat typedef from hft.h
typedef std::vector<std::shared_ptr<const Hft> > HftList;

/// Exception that an array parameter does not have the correct length.
class bad_array_size : public std::runtime_error {
 public:
  /// Constructor
  /**
   * \param key The fully qualified TOML key.
   * \param given_size The number of elements found in the TOML file.
   * \param expected_size The required size of the array.
   */
  bad_array_size(const std::string& key, const unsigned int given_size,
                 const std::string& expected_size)
      : std::runtime_error("Array parameter '" + key + "' has " +
                           std::to_string(given_size) + "'elements," +
                           " but required are: " + expected_size){};
};

/// Exception that a string parameter does not match possible options.
class invalid_option : public std::runtime_error {
 public:
  /// Constructor for global parameter.
  /**
   * \param key The fully qualified TOML key.
   * \param value The given (invalid) value.
   * \param valid_options Possible valid values.
   * \throw std::logic_error If `valid_options` is empty.
   */
  invalid_option(const std::string& key, const std::string& value,
                 const std::set<std::string>& valid_options)
      : runtime_error(construct_message(key, value, valid_options)){};

  /// Constructor for HFT parameter.
  /**
   * \param hft Reference to the HFT.
   * \param key The fully qualified TOML key.
   * \param value The given (invalid) value.
   * \param valid_options Possible valid values.
   * \throw std::logic_error If `valid_options` is empty.
   */
  invalid_option(const Hft& hft, const std::string& key,
                 const std::string& value,
                 const std::set<std::string>& valid_options)
      : runtime_error(construct_message(key, value, valid_options)){};

 private:
  std::string construct_message(const std::string& key,
                                const std::string& value,
                                const std::set<std::string>& valid_options);
};

/// Exception that a parameter is missing in the instruction file.
struct missing_parameter : public std::runtime_error {
  /// Constructor for missing global parameter.
  /**
   * \param key The fully qualified TOML key.
   */
  missing_parameter(const std::string& key)
      : runtime_error("Missing mandatory parameter: \"" + key + '"'){};
};

/// Exception that an HFT parameter is missing in the instruction file.
struct missing_hft_parameter : public std::runtime_error {
  /// Constructor for missing HFT parameter.
  /**
   * \param hft_name Identifier of the HFT.
   * \param key The fully qualified TOML key.
   */
  missing_hft_parameter(const std::string& hft_name, const std::string& key)
      : runtime_error("Missing mandatory parameter \"" + key + "\" in HFT \"" +
                      hft_name + "\"."){};
};

/// Exception if a group listed in `hft.groups` cannot be found.
struct missing_group : public std::runtime_error {
  /// Constructor.
  /**
   * \param hft_name Name of the HFT where the group was defined.
   * \param group_name The name of the missing group.
   */
  missing_group(const std::string& hft_name, const std::string& group_name)
      : std::runtime_error("Cannot find group with name \"" + group_name +
                           "\". " + "Required by HFT \"" + hft_name + "\"."){};
};

/// Exception if an integer or double parameter is out of range.
/**
 * Usually the functions \ref Hft::is_valid() and \ref
 * Parameters::is_valid() are responsible for checking the bounds of a
 * parameter. Use this exception only if the data type of the \ref Hft
 * or \ref Parameters member variable does not allow being assigned the
 * user-specified value.
 */
struct param_out_of_range : public std::runtime_error {
  /// Constructor for a double value.
  /**
   * \param key The fully qualified TOML key.
   * \param value The given (invalid) value. Use `std::to_string()` to convert
   * from double or integer to string. \param allowed_interval The permitted
   * range of the parameter in mathematical notation. For example [0,1) allows
   * values between zero and one, including zero, but excluding one. Use the
   * infinity symbol, ∞, for intervals not bound on one side.
   */
  param_out_of_range(const std::string& key, const std::string& value,
                     const std::string& allowed_interval)
      : std::runtime_error("The parameter \"" + key + "\" " +
                           "is out of range. The specified value is " + value +
                           ", which lies outside of the interval " +
                           allowed_interval + "."){};
};

/// Exception that parameters couldn’t be parsed in the TOML file.
struct unknown_parameters : public std::runtime_error {
  /// Constructor
  /**
   * \param elements A list of unknown elements. Each should contain the fully
   * qualified TOML key, and may give more helpful details to the user. Each
   * element is printed as one line.
   */
  unknown_parameters(const std::list<std::string> elements)
      : runtime_error("Unknown parameters encountered:\n" +
                      concatenate_elements(elements)){};

  /// Concatenate list of strings with line breaks.
  static std::string concatenate_elements(
      const std::list<std::string> elements) {
    std::ostringstream str;
    for (const auto& i : elements) str << i << "\n";
    return str.str();
  }
};

/// Exception that a parameter is not of an expected data type.
struct wrong_param_type : public std::runtime_error {
  /// Constructor
  /**
   * \param key The fully qualified TOML key.
   * \param type_expected What `key` is supposed to be.
   * \param type_found The type of `key` in the given instruction file.
   */
  wrong_param_type(const std::string& key, const std::string& type_expected,
                   const std::string& type_found)
      : std::runtime_error("The parameter \"" + key + "\" " +
                           "is of a wrong type. I found type " + type_found +
                           ", but expected " + type_expected + "."){};
};

/// Class to read parameters and HFTs from given instruction file.
class InsfileReader {
 public:
  /// Constructor
  /**
   * \param[in] filename Relative or absolute file path to the instruction
   * file.
   * \return The HFTs and parameters from the instruction file.
   * \throw std::runtime_error If the instruction file cannot be parsed
   * completely, contains logical errors, or has missing parameters.
   */
  InsfileReader(const std::string filename);

  /// Get the HFT list that was read from the instruction file.
  const HftList& get_hfts() const { return hfts; };

  /// Get the global parameters that were read from the instruction file.
  const Parameters& get_params() const { return params; };

 private:
  /// Find table with group.
  /**
   * \param group_name The name of the group.
   * \return Pointer to the group. NULL if group was not found.
   */
  std::shared_ptr<cpptoml::table> get_group_table(
      const std::string& group_name) const;

  /// Retrieve a value from the TOML file.
  /**
   * \param key The fully qualified TOML key.
   * \tparam T Expected type of the parameter `key`.
   */
  template <class T>
  std::shared_ptr<T> get_value(const std::string& key) const;

  /// Throw an exception if parameter is present, but with wrong type.
  /**
   * \param key The fully qualified TOML key.
   * \tparam Expected Expected type of the parameter `key`.
   * \throw wrong_param_type If `key` found, but of wrong type.
   */
  template <class Expected>
  void check_wrong_type(const std::string& key) const;

  /// Retrieve HFT parameter from HFT table itself or one of its groups.
  /**
   * If the key is not defined in the HFT itself, the groups are checked in the
   * order they are defined until the value is found.
   * \param hft_table The TOML table of the HFT itself.
   * \param key The string identifier of the parameter, e.g. "digestion.limit".
   * \param mandatory Whether to throw \ref missing_parameter if the value
   * couldn’t be found.
   * \return A pointer to the value. If the value wasn’t found and is not
   * mandatory, the result is `NULL`.
   */
  template <class T>
  cpptoml::option<T> find_hft_parameter(
      const std::shared_ptr<cpptoml::table>& hft_table, const std::string& key,
      const bool mandatory);

  /// Like \ref find_hft_parameter(), but for an array of values.
  /**
   * \copydoc find_hft_parameter()
   */
  template <class T>
  typename cpptoml::array_of_trait<T>::return_type find_hft_array_parameter(
      const std::shared_ptr<cpptoml::table>& hft_table, const std::string& key,
      const bool mandatory);

  /// Iterate through TOML tree and list all "leaves".
  /**
   * \param table A TOML tree or subtree.
   * \return A list of fully qualified TOML keys.
   */
  std::list<std::string> get_all_keys(std::shared_ptr<cpptoml::table> table);

  /// Read the TOML table `forage`.
  void read_table_forage();

  /// Read the TOML table `output`.
  void read_table_output();

  /// Read the TOML table `output.text_tables`.
  void read_table_output_text_tables();

  /// Read the TOML table `simulation`.
  void read_table_simulation();

  /// Remove TOML element (in order to indicate it’s been parsed).
  /**
   * \param table A TOML (sub)tree.
   * \param key A fully qualified TOML key that should exist in `table`.
   * \throw std::invalid_argument If `table` is NULL.
   * \throw std::out_of_range If `key` couldn’t be found.
   */
  void remove_qualified_key(std::shared_ptr<cpptoml::table> table,
                            const std::string key) const;

  /// Construct HFT object from an entry in the array of tables.
  /**
   * \throw std::invalid_argument If `table` is NULL.
   */
  Hft read_hft(const std::shared_ptr<cpptoml::table>& table);

  /// The root table of the instruction file from `cpptoml::parse_file()`.
  std::shared_ptr<cpptoml::table> ins;

  /// All TOML keys in HFTs and HFT groups that have been parsed.
  /**
   * In general, TOML elements are erased from \ref ins as soon as they have
   * been parsed. Any remaining keys/elements are unknown and issue an error.
   * However, parameters in HFT groups may be read several times. Therefore
   * they need to be erased after all HFTs have been read. This variable keeps
   * track of all valid keys so that they can be removed from the HFT groups in
   * the end.
   */
  std::set<std::string> hft_keys_parsed;

  Parameters params;
  HftList hfts;
};
}  // namespace Fauna

#endif  // FAUNA_INSFILE_READER_H
