//////////////////////////////////////////////////////////
/// \file
/// \brief Functions and classes shared across parts of the herbivory
///        module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date October 2017
///////////////////////////////////////////////////////////
#include "utils.h"
#include <cassert>  // for assert()
#include <cmath>    // for NAN and INFINITY
#include <stdexcept>

using namespace Fauna;

namespace Fauna {
// Days within each month, assuming a 365 days year.
const int MONTH_LENGTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

//////////////////////////////////////////////////////////////////////
// Free Functions
//////////////////////////////////////////////////////////////////////

double average(const double a, const double b, const double weight_a,
               const double weight_b) {
  if (weight_a < 0.0 || weight_b < 0.0)
    throw std::invalid_argument(
        "Fauna::average() "
        "Weight must be >=0.0");
  if (weight_a != weight_a || weight_b != weight_b)
    throw std::invalid_argument(
        "Fauna::average() "
        "Weight is NAN");
  if (weight_a == INFINITY || weight_b == INFINITY)
    throw std::invalid_argument(
        "Fauna::average() "
        "Weight is INFINITY");
  if (weight_a + weight_b == 0.0)
    throw std::invalid_argument(
        "Fauna::average() "
        "Sum of weights is zero.");
  return (a * weight_a + b * weight_b) / (weight_a + weight_b);
}

int get_day_of_month(const int day_of_year) {
  if (day_of_year < 0)
    throw std::invalid_argument(
        "Fauna::get_day_of_month()"
        "Parameter `day_of_year` is negative.");
  if (day_of_year >= 365)
    throw std::invalid_argument(
        "Fauna::get_day_of_month()"
        "Parameter `day_of_year` is greater than 364.");

  int day_of_month = -1;
  int month = 0;
  for (int day = 0; day <= day_of_year; day++) {
    if (day_of_month >= MONTH_LENGTH[month] - 1) {
      day_of_month = 0;
      month++;
    } else
      day_of_month++;
  }
  assert(day_of_month >= 0);
  assert(month >= 0);
  assert(month <= 11);
  assert(day_of_month < MONTH_LENGTH[month]);
  return day_of_month;
}
}  // namespace Fauna

////////////////////////////////////////////////////////////////////////
// PeriodAverage
////////////////////////////////////////////////////////////////////////

PeriodAverage::PeriodAverage(const int count) : count(count) {
  if (count <= 0)
    throw std::invalid_argument(
        "Fauna:: PeriodAverage::PeriodAverage() "
        "Parameter `count` is zero or negative.");
}

void PeriodAverage::add_value(const double v) {
  assert(deque.size() <= count);

  // Add new value to the front.
  deque.push_front(v);

  // Remove old value in the back.
  if (deque.size() > count) deque.pop_back();
}

double PeriodAverage::get_average() const {
  assert(deque.size() <= count);

  if (deque.empty())
    throw std::logic_error(
        "Fauna::PeriodAverage::get_average() "
        "No values have been added yet. Cannot build average.");

  double sum = 0.0;
  for (std::deque<double>::const_iterator itr = deque.begin();
       itr != deque.end(); itr++) {
    sum += *itr;
  }
  assert(deque.size() > 0);
  return sum / (double)deque.size();
}
