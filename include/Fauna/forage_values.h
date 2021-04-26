// SPDX-FileCopyrightText: 2020 W. Traylor <wolfgang.traylor@senckenberg.de>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * \file
 * \brief Basic classes encapsulating forage amounts & fractions.
 * \copyright LGPL-3.0-or-later
 * \date 2019
 */
#ifndef FAUNA_FORAGE_VALUES_H
#define FAUNA_FORAGE_VALUES_H

#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>
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
 * The forage type \ref ForageType::Inedible is excluded from all operations.
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
 public:
  /// Constructor with initializing value.
  /**
   * \throw std::invalid_argument If `init_value` is not allowed
   * by given `tag`.
   * \throw std::logic_error If `tag` is not implemented.
   *
   */
  ForageValues(const double init_value = 0.0) { set(init_value); }

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
    for (int ft = 0; ft < array.size(); ft++) {
      const double d = divisor.array[ft];
      if (d != 0.0)
        result.set((ForageType)ft, array[ft] / d);  // normal
      else
        result.set((ForageType)ft, na_value);  // division by zero
    }
    return result;
  }

  /// Get a value (read-only).
  /**
   * \throw std::invalid_argument If \ref ForageType::Inedible is passed.
   */
  double get(const ForageType ft) const {
    if (ft == ForageType::Inedible)
      throw std::invalid_argument(
          "Fauna::ForageValues<>::get() "
          "The forage type `ForageType::Inedible` is not allowed.");
    assert((int)ft < array.size());
    assert((int)ft >= 0);
    return array[(int)ft];
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
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft,
          average(array[ft], other.array[ft], this_weight, other_weight));
    return *this;
  }

  /// For each forage type, take the maximum value.
  /** \param other The object to compare this object with.
   * \return This object. */
  ForageValues<tag>& max(const ForageValues<tag>& other) {
    if (&other == this) return *this;
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, std::max(array[ft], other.array[ft]));
    return *this;
  }

  /// For each forage type, take the minimum value.
  /** \param other The object to compare this object with.
   * \return This object. */
  ForageValues<tag>& min(const ForageValues<tag>& other) {
    if (&other == this) return *this;
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, std::min(array[ft], other.array[ft]));
    return *this;
  }

  /// Read-only value access.
  double operator[](const ForageType ft) const { return get(ft); }

  /// Write access to values.
  double& operator[](const ForageType ft) {
    if (ft == ForageType::Inedible)
      throw std::invalid_argument(
          "Fauna::ForageValues<>::get() "
          "The forage type `ForageType::Inedible` is not allowed.");
    assert((int)ft < array.size());
    assert((int)ft >= 0);
    return array[(int)ft];
  }

  /// Set a value, only finite values are allowed.
  /**
   * \throw std::invalid_argument If `value` is not allowed
   * by given `tag`, is NAN or is INFINITY.
   * \throw std::invalid_argument If `forage_type==ForageType::Inedible`.
   * \throw std::logic_error If `tag` is not implemented.
   */
  void set(const ForageType forage_type, double value) {
    check_value(value);
    if (forage_type == ForageType::Inedible)
      throw std::invalid_argument((std::string)"ForageValues<> "
							"Forage type `ForageType::Inedible` is not allowed."+
							" ("+get_forage_type_name(forage_type)+")");

    // Change the value.
    assert((int)forage_type < array.size());
    assert((int)forage_type >= 0);
    array[(int)forage_type] = value;
  }

  /// Set all forage types to one value.
  /**
   * \throw std::invalid_argument If `value` is not allowed
   * by given `tag`, is NAN or is INFINITY.
   * \throw std::invalid_argument If `forage_type==ForageType::Inedible`.
   * \throw std::logic_error If `tag` is not implemented.
   */
  void set(double value) {
    check_value(value);
    array.fill(value);
  }

  /// Sum of all values.
  double sum() const {
    return std::accumulate(array.begin(), array.end(), 0.0);
  }

  /** @{ \name Operator overload. */
  ForageValues<tag>& operator+=(const double rhs) {
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, array[ft] + rhs);
    return *this;
  }
  ForageValues<tag>& operator-=(const double rhs) {
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, array[ft] - rhs);
    return *this;
  }
  ForageValues<tag>& operator*=(const double rhs) {
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, array[ft] * rhs);
    return *this;
  }
  /** \throw std::domain_error If `rhs==0.0`. */
  ForageValues<tag>& operator/=(const double rhs) {
    if (rhs == 0)
      throw std::domain_error("Fauna::ForageValues<> Division by zero.");
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, array[ft] / rhs);
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
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, array[ft] + rhs.array[ft]);
    return *this;
  }
  ForageValues<tag>& operator-=(const ForageValues<tag>& rhs) {
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, array[ft] - rhs.array[ft]);
    return *this;
  }
  ForageValues<tag>& operator*=(const ForageValues<tag>& rhs) {
    for (int ft = 0; ft < array.size(); ft++)
      set((ForageType)ft, array[ft] * rhs.array[ft]);
    return *this;
  }
  /** \throw std::domain_error On division by zero. */
  ForageValues<tag>& operator/=(const ForageValues<tag>& rhs) {
    for (int ft = 0; ft < array.size(); ft++) {
      if (rhs.array[ft] == 0)
        throw std::domain_error(
            (std::string) "Fauna::ForageValues<> Division by zero." + " (" +
            get_forage_type_name((ForageType)ft) + ")");
      set((ForageType)ft, array[ft] / rhs.array[ft]);
    }
    return *this;
  }

  bool operator==(const ForageValues<tag>& rhs) const {
    for (int ft = 0; ft < array.size(); ft++)
      if (array[ft] != rhs.array[ft]) return false;
    return true;
  }

  // TODO: Is this function necessary and logical?
  bool operator!=(const ForageValues<tag>& rhs) const {
    for (int ft = 0; ft < array.size(); ft++)
      if (array[ft] == rhs.array[ft]) return false;
    return true;
  }

  bool operator<(const ForageValues<tag>& rhs) const {
    for (int ft = 0; ft < array.size(); ft++)
      if (array[ft] >= rhs.array[ft]) return false;
    return true;
  }
  bool operator<=(const ForageValues<tag>& rhs) const {
    for (int ft = 0; ft < array.size(); ft++)
      if (array[ft] > rhs.array[ft]) return false;
    return true;
  }
  bool operator>(const ForageValues<tag>& rhs) const {
    for (int ft = 0; ft < array.size(); ft++)
      if (array[ft] <= rhs.array[ft]) return false;
    return true;
  }
  bool operator>=(const ForageValues<tag>& rhs) const {
    for (int ft = 0; ft < array.size(); ft++)
      if (array[ft] < rhs.array[ft]) return false;
    return true;
  }
  /** @} */  // Operator overload

  /// Tolerance range for imprecise floating point results.
  /**
   * For example, if no negative values are allowed, a value only *slightly*
   * below zero could result from an imprecise floating point calculation. This
   * must be corrected. So a value barely below zero will be corrected to
   * actual zero.
   *
   * Setting the tolerance is an arbitrary decision. In general, ecologically
   * significant numbers throughout the program should be above 1.0. Therefore
   * it is important to choose your units carefully. If you encounter errors
   * that are certainly coming from rounding imprecision, you may try to
   * increase this tolerance value.
   * \see \ref check_value()
   */
  constexpr static const double IMPRECISION_TOLERANCE = 1e-3;

 private:
  /// Forage values for all but \ref ForageType::Inedible.
  std::array<double, 1> array;

  /// Helper function to throw exceptions in the `set()` functions.
  /**
   * \param value The value to check. It is passed as reference so that it can
   * be corrected with \ref IMPRECISION_TOLERANCE.
   */
  void check_value(double& value) const {
    switch (tag) {
      case ForageValueTag::PositiveAndZero:
        if (value < 0.0) {
          // Correct floating point rounding errors.
          if (value >= -IMPRECISION_TOLERANCE)
            value = 0.0;
          else
            throw std::invalid_argument(
                "ForageValues<PositiveAndZero> Value < 0 not allowed. "
                "(value == " +
                std::to_string(value) + ")");
        }
        break;
      case ForageValueTag::ZeroToOne:
        if (value < 0.0 || value > 1.0) {
          // Correct floating point rounding errors.
          if (value < 0.0 && value >= 0.0 - IMPRECISION_TOLERANCE)
            value = 0.0;
          else if (value > 1.0 && value <= 1.0 + IMPRECISION_TOLERANCE)
            value = 1.0;
          else
            throw std::invalid_argument(
                "ForageValues<ZeroToOne> Value is not in interval [0,1]. "
                "(value == " +
                std::to_string(value) + ")");
        }
        break;
      default:
        throw std::logic_error(
            "ForageValues<> "
            "ForageValueTag not implemented.");
    }
    if (std::isnan(value))
      throw std::invalid_argument(
          "ForageValues<> NAN is not allowed as a value.");
    if (std::isinf(value))
      throw std::invalid_argument(
          "ForageValues<> INFINITY is not allowed as a value.");
  }
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

/// Data structure defining which herbivore gets what to eat [kgDM/km²].
typedef std::vector<std::pair<HerbivoreInterface*, ForageMass>>
    ForageDistribution;

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
  for (const auto ft : FORAGE_TYPES) result.set(ft, rhs[ft] * lhs);
  return result;
}

inline ForageValues<ForageValueTag::PositiveAndZero> operator*(
    const ForageFraction& lhs,
    const ForageValues<ForageValueTag::PositiveAndZero>& rhs) {
  ForageValues<ForageValueTag::PositiveAndZero> result;
  for (const auto ft : FORAGE_TYPES) result.set(ft, rhs[ft] * lhs[ft]);
  return result;
}

inline ForageValues<ForageValueTag::PositiveAndZero> operator*(
    const ForageValues<ForageValueTag::PositiveAndZero>& lhs,
    const ForageFraction& rhs) {
  return operator*(rhs, lhs);
}
/** @} */

/// Convert forage fractions (in [0,1]) into values in [0,∞].
/**
 * \param fractions A \ref ForageValues object limited to values between zero
 * and one.
 * \return A \ref ForageValues object that has no upper limit for values.
 */
// Note that we don’t use the typedef "ForageFraction" as the type of parameter
// "fractions" here because Doxygen (1.8.16) does not recognize it and throws a
// warning.
ForageValues<ForageValueTag::PositiveAndZero> foragefractions_to_foragevalues(
    const ForageValues<ForageValueTag::ZeroToOne>& fractions);

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
// Note that we don’t use the typedefs "ForageFraction" and "ForageEnergy" as
// the parameter types here because Doxygen (1.8.16) does not recognize it and
// throws a warning.
ForageFraction convert_mj_to_kg_proportionally(
    const ForageValues<ForageValueTag::PositiveAndZero>& mj_per_kg,
    const ForageValues<ForageValueTag::ZeroToOne>& mj_proportions);

}  // namespace Fauna
#endif  // FAUNA_FORAGE_VALUES_H
