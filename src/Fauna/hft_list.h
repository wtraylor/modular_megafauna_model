/**
 * \file
 * \brief A list of \ref Hft objects.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_HFT_LIST_H
#define FAUNA_HFT_LIST_H

#include <stdexcept>
#include <vector>
#include "hft.h"

namespace Fauna {
// Forward Declarations
class Parameters;

/// A set of herbivore functional types, unique by name
class HftList {
 public:
  /// Get \ref Hft object by its name identifier.
  const Hft& operator[](const std::string& name) const {
    const int pos = find(name);
    if (pos < 0)
      throw std::logic_error(
          "HftList::operator[](): "
          "No Hft object with name \"" +
          name + "\" in list.");
    else
      return operator[](pos);
  }

  /// Get \ref Hft object by its number.
  /** \param pos Position in the vector
   * \throw std::out_of_range If not 0≤pos≤size()
   */
  const Hft& operator[](const int pos) const {
    if (pos >= size() || pos < 0)
      throw std::out_of_range(
          "Fauna::HftList::operator[]() "
          "Parameter \"pos\" out of range.");
    return vec.at(pos);
  }

  /// Check whether an \ref Hft of given name exists in the list
  /** \return true if object in list, false if not */
  bool contains(const std::string& name) const { return find(name) >= 0; }

  /// Add or replace an \ref Hft object.
  /**
   * If an object of the same name already exists, it will be replaced
   * with the new one.
   * \throw std::invalid_argument `if (hft.name=="")`
   */
  void insert(const Hft hft);

  /// Check all HFTs if they are valid.
  /**
   * \param[in] params The global simulation parameters.
   * \param[out] msg Warning or error messages for output.
   * \return `true` if all HFTs are valid. `false` if one Hft is not valid
   * or if the list is empty.
   */
  bool is_valid(const Fauna::Parameters& params, std::string& msg) const;

  //------------------------------------------------------------
  /** @{ \name Wrapper around std::vector */
  typedef std::vector<Hft>::iterator iterator;
  typedef std::vector<Hft>::const_iterator const_iterator;
  iterator begin() { return vec.begin(); }
  const_iterator begin() const { return vec.begin(); }
  iterator end() { return vec.end(); }
  const_iterator end() const { return vec.end(); }
  const int size() const { return vec.size(); }
  /** @} */  // Container Functionality
 private:
  /// Vector holding the Hft instances.
  std::vector<Hft> vec;

  /// Find the position of the \ref Hft with given name.
  /** \param name \ref Hft::name identifier
   * \return list position if found, -1 if not in list */
  int find(const std::string& name) const;
};

}  // namespace Fauna
#endif  // FAUNA_HFT_LIST_H
