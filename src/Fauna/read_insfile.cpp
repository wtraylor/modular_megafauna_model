#include "read_insfile.h"
#include "hft.h"
#include "makeshift.h"

using namespace Fauna;

InsfileContent Fauna::read_instruction_file(const std::string filename) {
  return InsfileContent({construct_makeshift_hfts(), Parameters()});
}
