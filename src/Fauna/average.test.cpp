/**
 * \file
 * \brief Unit test for output aggregation helpers.
 * \copyright ...
 * \date 2019
 */
#include "average.h"
#include "catch.hpp"

using namespace Fauna;

TEST_CASE("Fauna::average()", "") {
  CHECK_THROWS(average(1.0, 2.0, -1.0, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, -1.0));
  CHECK_THROWS(average(1.0, 2.0, 0.0, 0.0));
  CHECK_THROWS(average(1.0, 2.0, NAN, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, NAN));
  CHECK_THROWS(average(1.0, 2.0, INFINITY, 1.0));
  CHECK_THROWS(average(1.0, 2.0, 1.0, INFINITY));
  CHECK(average(1.0, 3.0) == Approx(2.0));
  CHECK(average(1.0, 1.0) == Approx(1.0));
  CHECK(average(-1.0, 1.0) == Approx(0.0));
}

TEST_CASE("Fauna::PeriodAverage") {
  CHECK_THROWS(PeriodAverage(-1));
  CHECK_THROWS(PeriodAverage(0));

  const int COUNT = 3;
  PeriodAverage pa(COUNT);

  CHECK_THROWS(pa.get_average());
  CHECK_THROWS(pa.get_first());

  const double A = .1;
  const double B = .2;
  const double C = .4;
  const double D = .5;
  const double E = .6;

  pa.add_value(A);
  CHECK(pa.get_average() == A);
  CHECK(pa.get_first() == A);

  pa.add_value(B);
  CHECK(pa.get_average() == Approx((A + B) / 2.0));
  CHECK(pa.get_first() == A);

  pa.add_value(C);
  CHECK(pa.get_average() == Approx((A + B + C) / 3.0));
  CHECK(pa.get_first() == A);

  pa.add_value(D);
  CHECK(pa.get_average() == Approx((B + C + D) / 3.0));
  CHECK(pa.get_first() == B);

  pa.add_value(E);
  CHECK(pa.get_average() == Approx((C + D + E) / 3.0));
  CHECK(pa.get_first() == C);
}
