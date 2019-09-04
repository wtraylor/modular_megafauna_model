/**
 * \file
 * \brief Unit test for energy expenditure algorithms.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "expenditure_components.h"
using namespace Fauna;

TEST_CASE("Fauna::get_expenditure_taylor_1981()") {
  const double CURRENT = 90;  // [kg]
  const double ADULT = 100;   // [kg]
  CHECK(get_expenditure_taylor_1981(CURRENT, ADULT) ==
        Approx(0.4 * CURRENT * pow(ADULT, -0.27)));
}

TEST_CASE("Fauna::get_expenditure_zhu_et_al_2018()") {
  const double M = 100;  // [kg]
  const double T = 20;   // [°C]
  CHECK_THROWS(get_expenditure_zhu_et_al_2018(-4, T));
  CHECK_THROWS(get_expenditure_zhu_et_al_2018(0, T));
  // Check formula:
  CHECK(get_expenditure_zhu_et_al_2018(M, T) ==
        0.36 / exp(0.0079 * T) * pow(M, 0.75));
  // More energy costs in lower temperature:
  CHECK(get_expenditure_zhu_et_al_2018(M, T) <
        get_expenditure_zhu_et_al_2018(M, T - 20));
}
