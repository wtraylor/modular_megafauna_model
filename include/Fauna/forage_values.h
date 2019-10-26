/**
 * \file
 * \brief Basic classes encapsulating forage amounts & fractions.
 * \copyright ...
 * \date 2019
 */
#ifndef FAUNA_FORAGE_VALUES_H
#define FAUNA_FORAGE_VALUES_H

#include <cmath>
#include <map>
#include "Fauna/average.h"
#include "Fauna/forage_types.h"

namespace Fauna {
class HerbivoreInterface;  // for ForageDistribution

/// Describes which values are allowed in \ref Fauna::ForageValues.
enum class ForageValueTag {
  /// Only values in interval [0,∞) are allowed.
  PositiveAndZero,
  /// Only values in interval [0,1] are allowed.
  ZeroToOne
};

/// Multi-purpose template class for double values mapped by edible(!) forage
/// type.
/**
 * The forage type \ref FT_INEDIBLE is excluded from all operations.
 * \tparam tag Defines the allowed data range.
 *
 * \note Operators that take a number as argument will interpret that as
 * a ForageValues object where all forage type values are that number.
 *
 * \warning It is important to understand and use the binary comparison
 * operators correctly. Be `F1` and `F2` ForageValues objects.
 * `F1>F2` then means that *each* value in `F1` (one for each forage type)
 * is greater than the corresponding value in `F2`.
 * In the same way, `F1==F2` means that all corresponding values are
 * identical.
 * `F1!=F2` means then that *each* pair of values is not equal. If one
 * value pair, *is* identical, the result is `false`!
 * In the same logic, `F1==d` is true if *all* values in `F1` are equal
 * to the double value `d`.
 * Now, `F1!=d` is true only if *all* values in `F1` are not equal to `d`.
 * If only one value of `F1` matches `d`, the result will be `false`!
 * To check if not all values in `F1` are equal to `d`, you would use
 * `!(F1==d)`.
 */
template <ForageValueTag tag>
class ForageValues {
 private:
  typedef std::map<ForageType, double> MapType;

  /** @{ \name Write access iteration */
  typedef MapType::iterator iterator;  // write access
  iterator begin() { return map.begin(); }
  iterator end() { return map.end(); }
  /** @} */

  /// Initialize: create map entries for every forage type.
  /** \see \ref FORAGE_TYPES */
  void init() {
    assert(map.empty());
    for (std::set<ForageType>::const_iterator ft = FORAGE_TYPES.begin();
         ft != FORAGE_TYPES.end(); ft++)
      map[*ft] = 0.0;
    assert(map.size() == FORAGE_TYPES.size());
  }

 public:
  /// Constructor with initializing value.
  /**
   * \throw std::invalid_argument If `init_value` is not allowed
   * by given `tag`.
   * \throw std::logic_error If `tag` is not implemented.
   *
   */
  ForageValues(const double init_value = 0.0) {
    init();
    for (iterator i = begin(); i != end(); i++) set(i->first, init_value);
  }

  /// Divide safely also by zero values.
  /**
   * \param divisor Numbers to divide by; can contain zeros.
   * \param na_value Value that shall be set as a result for
   * division by zero.
   * \return New object with the division result.
   * \see operator/()
   */
  ForageValues<tag> divide_safely(const ForageValues<tag>& divisor,
                                  const double na_value) const {
    ForageValues<tag> result(*this);
    for (const_iterator i = begin(); i != end(); i++) {
      const double d = divisor.get(i->first);
      if (d != 0.0)
        result.set(i->first, i->second / d);  // normal
      else
        result.set(i->first, na_value);  // division by zero
    }
    return result;
  }

  /// Get a value (read-only).
  /**
   * \throw std::invalid_argument If \ref FT_INEDIBLE is passed.
   * \throw std::logic_error If forage type not accessible.
   * This error should never occur, as all forage types are
   * initialized in the constructors.
   */
  double get(const ForageType ft) const {
    if (ft == FT_INEDIBLE)
      throw std::invalid_argument(
          "Fauna::ForageValues<>::get() "
          "The forage type `FT_INEDIBLE` is not allowed.");
    const_iterator element = map.find(ft);
    if (element != map.end())
      return element->second;
    else
      throw std::logic_error(
          "Fauna::ForageValues<>::get() "
          "Forage type \"" +
          get_forage_type_name(ft) + "\" not implemented or invalid.");
  }

  /// Merge this object with another one by building (weighted) means.
  /**
   * \param other Other object to merge into this one.
   * \param this_weight Weight of this object’s values.
   * \param other_weight Weight of the other object’s values.
   * \return This object.
   * \throw std::invalid_argument The same as \ref average().
   * \see \ref Fauna::average().
   */
  ForageValues<tag>& merge(const ForageValues<tag>& other,
                           const double this_weight = 1.0,
                           const double other_weight = 1.0) {
    for (const_iterator i = other.begin(); i != other.end(); i++)
      set(i->first,
          average((*this)[i->first], i->second, this_weight, other_weight));
    return *this;
  }

  /// For each forage type, take the maximum value.
  /** \param other The object to compare this object with.
   * \return This object. */
  ForageValues<tag>& max(const ForageValues<tag>& other) {
    if (&other == this) return *this;
    for (const_iterator i = other.begin(); i != other.end(); i++)
      set(i->first, std::max(get(i->first), i->second));
    return *this;
  }

  /// For each forage type, take the minimum value.
  /** \param other The object to compare this object with.
   * \return This object. */
  ForageValues<tag>& min(const ForageValues<tag>& other) {
    if (&other == this) return *this;
    for (const_iterator i = other.begin(); i != other.end(); i++)
      set(i->first, std::min(get(i->first), i->second));
    return *this;
  }

  /// Read-only value access.
  /** \throw std::logic_error If forage type not accessible.
   * This error should never occur, as all forage types are
   * initialized in the constructors. */
  double operator[](const ForageType ft) const { return get(ft); }

  /// Set a value, only finite values are allowed..
  /**
   * \throw std::invalid_argument If `value` is not allowed
   * by given `tag`, is NAN or is INFINITY.
   * \throw std::invalid_argument If `forage_type==FT_INEDIBLE`.
   * \throw std::logic_error If `tag` is not implemented.
   */
  void set(const ForageType forage_type, const double value) {
    switch (tag) {
      case ForageValueTag::PositiveAndZero:
        if (value < 0.0)
          throw std::invalid_argument((std::string)
									"ForageValues<PositiveAndZero> "
									"Value < 0 not allowed."+
									" ("+get_forage_type_name(forage_type)+")");
        break;
      case ForageValueTag::ZeroToOne:
        if (value < 0.0 || value > 1.0)
          throw std::invalid_argument((std::string)
									"ForageValues<ZeroToOne> "
									"Value is not in interval [0,1]."+
									" ("+get_forage_type_name(forage_type)+")");
        break;
      default:
        throw std::logic_error(
            "ForageValues<> "
            "ForageValueTag not implemented.");
    }
    if (std::isnan(value))
      throw std::invalid_argument((std::string)
							"ForageValues<> "
							"NAN is not allowed as a value."+
							" ("+get_forage_type_name(forage_type)+")");
    if (std::isinf(value))
      throw std::invalid_argument((std::string)"ForageValues<> "
							"INFINITY is not allowed as a value."+
							" ("+get_forage_type_name(forage_type)+")");
    if (forage_type == FT_INEDIBLE)
      throw std::invalid_argument((std::string)"ForageValues<> "
							"Forage type `FT_INEDIBLE` is not allowed."+
							" ("+get_forage_type_name(forage_type)+")");

    // The map entry for any forage type should have been
    // created on initialization.
    assert(map.size() == FORAGE_TYPES.size());
    assert(map.count(forage_type));

    // Change the value.
    map[forage_type] = value;

    assert(map.size() == FORAGE_TYPES.size());
  }

  /// Sum of all values.
  double sum() const {
    double sum = 0.0;
    for (const_iterator i = begin(); i != end(); i++) sum += i->second;
    return sum;
  }

  /** @{ \name Read-only wrapper around std::map. */
  typedef MapType::const_iterator const_iterator;
  const_iterator begin() const { return map.begin(); }
  const_iterator end() const { return map.end(); }
  /** @} */  // Wrapper around std::map

 public:
  /** @{ \name Operator overload. */
  ForageValues<tag>& operator+=(const double rhs) {
    for (iterator i = begin(); i != end(); i++) set(i->first, i->second + rhs);
    return *this;
  }
  ForageValues<tag>& operator-=(const double rhs) {
    for (iterator i = begin(); i != end(); i++) set(i->first, i->second - rhs);
    return *this;
  }
  ForageValues<tag>& operator*=(const double rhs) {
    for (iterator i = begin(); i != end(); i++) set(i->first, i->second * rhs);
    return *this;
  }
  /** \throw std::domain_error If `rhs==0.0`. */
  ForageValues<tag>& operator/=(const double rhs) {
    if (rhs == 0)
      throw std::domain_error("Fauna::ForageValues<> Division by zero.");
    for (iterator i = begin(); i != end(); i++) set(i->first, i->second / rhs);
    return *this;
  }

  ForageValues<tag> operator+(const double rhs) const {
    ForageValues<tag> result(*this);
    return result.operator+=(rhs);
  }
  ForageValues<tag> operator-(const double rhs) const {
    ForageValues<tag> result(*this);
    return result.operator-=(rhs);
  }
  ForageValues<tag> operator*(const double rhs) const {
    ForageValues<tag> result(*this);
    return result.operator*=(rhs);
  }
  /** \throw std::domain_error If `rhs==0.0`. */
  ForageValues<tag> operator/(const double rhs) const {
    ForageValues<tag> result(*this);
    return result.operator/=(rhs);
  }

  ForageValues<tag> operator+(const ForageValues<tag>& rhs) const {
    ForageValues<tag> result(*this);
    return result.operator+=(rhs);
  }
  ForageValues<tag> operator-(const ForageValues<tag>& rhs) const {
    ForageValues<tag> result(*this);
    return result.operator-=(rhs);
  }
  ForageValues<tag> operator*(const ForageValues<tag>& rhs) const {
    ForageValues<tag> result(*this);
    return result.operator*=(rhs);
  }
  /** \throw std::domain_error On division by zero. */
  ForageValues<tag> operator/(const ForageValues<tag>& rhs) const {
    ForageValues<tag> result(*this);
    return result.operator/=(rhs);
  }

  ForageValues<tag>& operator+=(const ForageValues<tag>& rhs) {
    for (iterator i = begin(); i != end(); i++)
      set(i->first, i->second + rhs.get(i->first));
    return *this;
  }
  ForageValues<tag>& operator-=(const ForageValues<tag>& rhs) {
    for (iterator i = begin(); i != end(); i++)
      set(i->first, i->second - rhs.get(i->first));
    return *this;
  }
  ForageValues<tag>& operator*=(const ForageValues<tag>& rhs) {
    for (iterator i = begin(); i != end(); i++)
      set(i->first, i->second * rhs.get(i->first));
    return *this;
  }
  /** \throw std::domain_error On division by zero. */
  ForageValues<tag>& operator/=(const ForageValues<tag>& rhs) {
    for (iterator i = begin(); i != end(); i++) {
      const double divisor = rhs.get(i->first);
      if (divisor == 0)
        throw std::domain_error(
            (std::string) "Fauna::ForageValues<> Division by zero." + " (" +
            get_forage_type_name(i->first) + ")");
      set(i->first, i->second / divisor);
    }
    return *this;
  }

  ForageValues<tag>& operator=(const ForageValues<tag>& rhs) {
    for (const_iterator i = rhs.begin(); i != rhs.end(); i++)
      set(i->first, i->second);
    return *this;
  }

  bool operator==(const ForageValues<tag>& rhs) const {
    for (const_iterator i = rhs.begin(); i != rhs.end(); i++)
      if (get(i->first) != i->second) return false;
    return true;
  }
  bool operator!=(const ForageValues<tag>& rhs) const {
    for (const_iterator i = rhs.begin(); i != rhs.end(); i++)
      if (get(i->first) == i->second) return false;
    return true;
  }

  bool operator<(const ForageValues<tag>& rhs) const {
    bool result = true;
    for (const_iterator i = rhs.begin(); i != rhs.end(); i++)
      result &= (get(i->first) < i->second);
    return result;
  }
  bool operator<=(const ForageValues<tag>& rhs) const {
    bool result = true;
    for (const_iterator i = rhs.begin(); i != rhs.end(); i++)
      result &= (get(i->first) <= i->second);
    return result;
  }
  bool operator>(const ForageValues<tag>& rhs) const {
    bool result = true;
    for (const_iterator i = rhs.begin(); i != rhs.end(); i++)
      result &= (get(i->first) > i->second);
    return result;
  }
  bool operator>=(const ForageValues<tag>& rhs) const {
    bool result = true;
    for (const_iterator i = rhs.begin(); i != rhs.end(); i++)
      result &= (get(i->first) >= i->second);
    return result;
  }
  /** @} */  // Operator overload
 private:
  MapType map;
};

/// Digestibility [fraction] for different forage types.
typedef ForageValues<ForageValueTag::ZeroToOne> Digestibility;

/// Energy values [MJ] for different forage types.
typedef ForageValues<ForageValueTag::PositiveAndZero> ForageEnergy;

/// Net energy content [MJ/kgDM] for different forage types.
typedef ForageValues<ForageValueTag::PositiveAndZero> ForageEnergyContent;

/// A fraction for each forage type.
typedef ForageValues<ForageValueTag::ZeroToOne> ForageFraction;

/// Dry matter mass values [kgDM or kgDM/km²] for different forage types.
typedef ForageValues<ForageValueTag::PositiveAndZero> ForageMass;

/// Map defining which herbivore gets what to eat [kgDM/km²].
typedef std::map<HerbivoreInterface*, ForageMass> ForageDistribution;

/** @{ \name Overload operator * as non-member. */

/// Multiply forage fractions with coefficient, allowing numbers >0.
/**
 * Note that this function takes the double value on the left side
 * whereas the member function ForageValues<>::operator*() takes the
 * double value as the right operand and returns a ForageValues<> object
 * of the same template, which doesn’t allow numbers exceeding 1.0 in
 * case of the ForageFraction (=ForageValues<ForageValueTag::ZeroToOne>) class.
 */
inline ForageValues<ForageValueTag::PositiveAndZero> operator*(
    const double lhs, const ForageFraction& rhs) {
  ForageValues<ForageValueTag::PositiveAndZero> result;
  for (ForageFraction::const_iterator i = rhs.begin(); i != rhs.end(); i++)
    result.set(i->first, i->second * lhs);
  return result;
}

inline ForageValues<ForageValueTag::PositiveAndZero> operator*(
    const ForageFraction& lhs,
    const ForageValues<ForageValueTag::PositiveAndZero>& rhs) {
  ForageValues<ForageValueTag::PositiveAndZero> result;
  for (ForageFraction::const_iterator i = rhs.begin(); i != rhs.end(); i++)
    result.set(i->first, i->second * lhs[i->first]);
  return result;
}

inline ForageValues<ForageValueTag::PositiveAndZero> operator*(
    const ForageValues<ForageValueTag::PositiveAndZero>& lhs,
    const ForageFraction& rhs) {
  return operator*(rhs, lhs);
}
/** @} */

/// Convert forage fractions (in [0,1]) into values in [0,∞].
ForageValues<ForageValueTag::PositiveAndZero> foragefractions_to_foragevalues(
    const ForageFraction& fractions);

/// Convert forage values to fractional values.
/**
 * \param values The object to convert.
 * \param tolerance By how much a value can exceed 1.0 and still be
 * set to 1.0. E.g. `tolerance == 0.1` means that any values from
 * 1.0 to 1.1 will be set to 1.0.
 * \return Forage fractional values within [0,1].
 * \throw std::invalid_argument If one number in `values` exceeds
 * `1.0 + tolerance`.
 * \throw std::invalid_argument If `tolerance < 0.0`.
 */
ForageFraction foragevalues_to_foragefractions(
    const ForageValues<ForageValueTag::PositiveAndZero> values,
    const double tolerance);

/// Convert forage energy to mass keeping the energy-wise proportions.
/**
 * \param mj_per_kg Energy content of the forage [MJ/kgDM].
 * \param mj_proportions Energy-wise proportions [MJ/MJ]. This doesn’t
 * need to add up to 1.0.
 * \return Mass-wise proportions [kgDM/kgDM] whose sum equals the sum of
 * `mj_proportions`. When converting back from mass to energy, the
 * proportion relative to each other will be like in `mj_proportions`.
 */
ForageFraction convert_mj_to_kg_proportionally(
    const ForageEnergyContent& mj_per_kg, const ForageFraction& mj_proportions);

}  // namespace Fauna
#endif  // FAUNA_FORAGE_VALUES_H
