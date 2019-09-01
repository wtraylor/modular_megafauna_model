// Temporary file with some classes/routines that serve testing purpose until
// the instruction file reader and output are done.
#ifndef FAUNA_MAKESHIFT_H
#define FAUNA_MAKESHIFT_H

#include "hft_list.h"

namespace Fauna {
/// Create some sample HFTs.
HftList construct_makeshift_hfts();
}  // namespace Fauna

#endif  // FAUNA_MAKESHIFT_H
