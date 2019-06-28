#include <catch2/catch.hpp>
#include "hft.h"
#include "parameters.h"
using namespace Fauna;

TEST_CASE("Fauna::Hft", "") {
  Hft hft = Hft();
  std::string msg;

  SECTION("not valid without name") {
    hft.name = "";
    CHECK_FALSE(hft.is_valid(Fauna::Parameters(), msg));
  }
}

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
  hft1.is_included = true;
  REQUIRE_NOTHROW(hftlist.insert(hft1));
  REQUIRE(hftlist.size() == 1);
  REQUIRE(hftlist[0].name == "hft1");
  REQUIRE(hftlist.begin()->name == "hft1");

  Hft hft2;
  hft2.name = "hft2";
  hft2.is_included = false;
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
  hft2.lifespan += 2;  // change a property outside list
  REQUIRE(hftlist[hft2.name].lifespan != hft2.lifespan);
  hftlist.insert(hft2);  // replace existing
  CHECK(hftlist[hft2.name].lifespan == hft2.lifespan);

  // remove excluded
  hftlist.remove_excluded();
  CHECK(hftlist.size() == 1);
  CHECK(hftlist.contains(hft1.name));        // hft1 included
  CHECK_FALSE(hftlist.contains(hft2.name));  // hft2 NOT included
}

