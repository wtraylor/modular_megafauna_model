#ifndef TEXT_TABLE_WRITER_H
#define TEXT_TABLE_WRITER_H

#include "output_writer_interface.h"
#include "parameters.h"

namespace Fauna {
namespace Output {

/// Writes output data to tabular plaintext files.
/**
 * \ref Parameters::TextTableWriterOptions contains boolean switches to enable
 * and disable the creation of different data tables in plaintext files. The
 * files have the same name as the corresponding boolean variable in
 * \ref Parameters::TextTableWriterOptions.
 * All files are created in a directory specified by
 * \ref Parameters::TextTableWriterOptions::output_directory.
 */
class TextTableWriter : public OutputWriterInterface {
 public:
  /// Constructor
  /**
   * Create all files that are selected in `options` as empty files.
   * \param interval Selector if output is daily/monthly/annual/...
   * \param options Specific user-defined options for this class.
   */
  TextTableWriter(const OutputInterval interval,
                  const Parameters::TextTableWriterOptions& options);

  /// Append spatially & temporally aggregated output data to table files.
  /**
   * \param datapoint The output data to write.
   * \throw std::invalid_argument If an HFT name contains whitespaces or the
   * \ref FIELD_SEPARATOR.
   * \throw std::invalid_argument If `datapoint.combined_data.datapoint_count`
   * is zero.
   * \throw std::invalid_argument If `datapoint.interval` does not match the
   * given \ref OutputInterval.
   */
  virtual void write_datapoint(const Datapoint& datapoint);

  /// Character to separate columns.
  static const char FIELD_SEPARATOR = '\t';

  /// File extension for tabular plaintext files.
  static const char* FILE_EXTENSION;

 private:
  const OutputInterval interval;
  const Parameters::TextTableWriterOptions options;
};
}  // namespace Output
}  // namespace Fauna

#endif  // TEXT_TABLE_WRITER_H
