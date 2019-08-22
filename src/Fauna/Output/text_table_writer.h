#ifndef TEXT_TABLE_WRITER_H
#define TEXT_TABLE_WRITER_H

#include "output_writer_interface.h"
#include "parameters.h"

namespace Fauna {
namespace Output {

/// Writes output data to tabular plaintext files.
class TextTableWriter : public OutputWriterInterface {
 public:
  /// Constructor
  /**
   * Create all files (selected in options) with table headers.
   * \param interval Selector if output is daily/monthly/annual/...
   * \param options Specific user-defined options for this class.
   */
  TextTableWriter(const OutputInterval interval,
                  const Parameters::TextTableWriterOptions& options);

  /// Append spatially & temporally aggregated output data to table files.
  void write_datapoint(const DataPoint& datapoint);
 private:
  const OutputInterval interval;
  const Parameters::TextTableWriterOptions options;
};
}  // namespace Output
}  // namespace Fauna

#endif  // TEXT_TABLE_WRITER_H
