#ifndef TESTS_DUMMY_HFT_H
#define TESTS_DUMMY_HFT_H

#include <sstream>
#include "hft.h"
#include "hft_list.h"

namespace Fauna {
/// Create a simple, valid HftList
inline HftList create_hfts(const int count, const Parameters& params) {
  HftList hftlist;
  for (int i = 0; i < count; i++) {
    Hft hft;
    hft.is_included = true;
    // construct name for HFT
    std::ostringstream stream;
    stream << "hft" << i;
    hft.name = stream.str();

    std::string msg;
    if (!hft.is_valid(params, msg))
      FAIL("create_hfts(): HFT is not valid:\n" << msg);

    hftlist.insert(hft);
  }
  REQUIRE(hftlist.size() == count);
  return hftlist;
}
}  // namespace Fauna

#endif  // TESTS_DUMMY_HFT_H
