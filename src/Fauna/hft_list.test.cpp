/**
 * \file
 * \brief Unit test for Fauna::HftList.
 * \copyright ...
 * \date 2019
 */
#include "catch.hpp"
#include "hft.h"
#include "hft_list.h"
using namespace Fauna;

TEST_CASE("Fauna::HftList", "") {
  HftList hftlist;

  // check initial size
  REQUIRE(hftlist.size() == 0);

  // invalid access
  CHECK_THROWS(hftlist[1]);
  CHECK_THROWS(hftlist[-1]);
  CHECK_THROWS(hftlist["abc"]);

  // add Hft without name
  Hft noname;
  noname.name = "";
  CHECK_THROWS(hftlist.insert(noname));

  // add some real HFTs
  Hft hft1;
  hft1.name = "hft1";
  REQUIRE_NOTHROW(hftlist.insert(hft1));
  REQUIRE(hftlist.size() == 1);
  REQUIRE(hftlist[0].name == "hft1");
  REQUIRE(hftlist.begin()->name == "hft1");

  Hft hft2;
  hft2.name = "hft2";
  REQUIRE_NOTHROW(hftlist.insert(hft2));
  REQUIRE(hftlist.size() == 2);
  REQUIRE_NOTHROW(hftlist[1]);

  // find elements
  CHECK(hftlist["hft2"].name == "hft2");
  CHECK(hftlist["hft1"].name == "hft1");
  CHECK(hftlist.contains("hft1"));
  CHECK(hftlist.contains("hft2"));
  CHECK_FALSE(hftlist.contains("abc"));

  // substitute element
  hft2.life_history_lifespan += 2;  // change a property outside list
  REQUIRE(hftlist[hft2.name].life_history_lifespan !=
          hft2.life_history_lifespan);
  hftlist.insert(hft2);  // replace existing
  REQUIRE(hftlist.size() == 2);
  CHECK(hftlist[hft2.name].life_history_lifespan == hft2.life_history_lifespan);
}