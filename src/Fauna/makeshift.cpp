/**
 * \file
 * \brief Some functions for testing purposes while some functions are missing.
 * \copyright ...
 * \date 2019
 */
#include "makeshift.h"
#include "hft.h"
#include "hft_list.h"

using namespace Fauna;

HftList Fauna::construct_makeshift_hfts() {
  HftList list;

  Hft bison;
  bison.name = "Bison";
  bison.body_mass.male = 400;
  bison.body_mass.female = 350;

  list.insert(bison);

  return list;
}
