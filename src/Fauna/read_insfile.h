#ifndef READ_INSFILE_H
#define READ_INSFILE_H

#include "hft.h"
#include "parameters.h"

namespace Fauna {

/// All global and herbivore parameters in an instruction file.
struct InsfileContent {
  /// Herbivore functional types from the instruction file.
  HftList hftlist;
  /// Global parameters from the instruction file.
  Parameters params;
};

/// Read parameters and HFTs from given instruction file.
/**
 * \param[in] filename Relative or absolute file path to the instruction file.
 * \return The HFTs and parameters from the instruction file.
 * \todo Implement this.
 */
InsfileContent read_instruction_file(const std::string filename);
}  // namespace Fauna

#endif  // READ_INSFILE_H
