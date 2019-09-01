#include "makeshift.h"
#include "hft.h"

using namespace Fauna;

HftList Fauna::construct_makeshift_hfts() {
  HftList list;

  Hft bison;
  bison.name = "Bison";
  bison.is_included = true;
  bison.bodymass_male = 400;
  bison.bodymass_female = 350;

  list.insert(bison);

  return list;
}
