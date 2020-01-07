/**
 * \file
 * \brief Unit test for forage energy content models.
 * \copyright ...
 * \date 2019
 */
#include <sstream>
#include "catch.hpp"
#include "hft.h"
#include "net_energy_models.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::get_net_energy_from_gross_energy()", "") {
  static const Digestibility DMD(0.5);
  static const ForageEnergyContent GE = Parameters().forage_gross_energy;
  static const double ME_COEFF = Hft().digestion_me_coefficient;
  static const double NE_COEFF = Hft().digestion_k_maintenance;

  REQUIRE_NOTHROW(
      get_net_energy_from_gross_energy(GE, DMD, ME_COEFF, NE_COEFF));

  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, -0.1, NE_COEFF));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, 0.0, NE_COEFF));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, 1.0, NE_COEFF));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, 1.1, NE_COEFF));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, 1.1, NE_COEFF));

  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, ME_COEFF, -0.1));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, ME_COEFF, 0.0));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, ME_COEFF, 1.0));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, ME_COEFF, 1.1));
  CHECK_THROWS(get_net_energy_from_gross_energy(GE, DMD, ME_COEFF, 1.1));

  // It doesn’t seem worth to check the formula itself since it is very simple.
}
