#ifndef READ_INSFILE_H
#define READ_INSFILE_H

namespace Fauna {
// Forward declarations
struct Parameters;
class HftList;

/// Read parameters and HFTs from given instruction file.
/**
 * \param[in] filename Relative or absolute file path to the instruction file.
 * \param[out] params Object to store global parameters in. Existing values will
 * be overwritten.
 * \param[out] hfts Object to store herbivore parameters in. Existing values
 * will be overwritten.
 */
void read_instruction_file(const std::string filename, Parameters& params,
                           HftList& hfts);
}  // namespace Fauna

#endif  // READ_INSFILE_H
