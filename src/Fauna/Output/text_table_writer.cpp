#include "text_table_writer.h"
#include "datapoint.h"
#include "fileystem.h"

using namespace Fauna;
using namespace Fauna::Output;

const char* TextTableWriter::FILE_EXTENSION = ".tsv";

TextTableWriter::TextTableWriter(
    const OutputInterval interval,
    const Parameters::TextTableWriterOptions& options)
    : interval(interval), options(options) {
  const std::string& dir = options.output_directory;

  create_directories(dir);

  // Add all selected output files to list of file streams.
  if (options.mass_density_per_hft){
    file_streams.push_back(&mass_density_per_hft);
    mass_density_per_hft.open(dir + "mass_density_per_hft");
  }
  // -> Add new output files here.
}

void TextTableWriter::write_datapoint(const Datapoint& datapoint) {
  const CombinedData& data = datapoint.data;

  /* TODO: \throw std::invalid_argument If an HFT name contains
   * whitespaces or the \ref FIELD_SEPARATOR.
   *
   * \throw std::invalid_argument If
   * `datapoint.combined_data.datapoint_count` is zero.
   *
   * \throw std::invalid_argument If `datapoint.interval` does not
   * match the given \ref OutputInterval.
   *
   * \throw std::invalid_argument If `datapoint.aggregation_unit`
   * contains \ref FIELD_SEPARATOR
   *
   * \throw std::logic_error If the \ref OutputInterval is not
   * implemented.
   */

  if (!captions_written) {
    write_captions(datapoint);
    captions_written = true;
  }

  // Write common information for all output tables.
  for (auto& f : file_streams) {
    switch (interval) {
      case OutputInterval::OI_DAILY:
        // TODO
        break;
      case OutputInterval::OI_MONTHLY:
        // TODO
        break;
      case OutputInterval::OI_ANNUAL:
        *f << datapoint.interval.get_first().get_year() << FIELD_SEPARATOR;
        break;
      case OutputInterval::OI_DECADAL:
        // TODO
        break;
      default:
        std::logic_error(
            "Fauna::TextTableWriter::write_datapoint() Output time interval is "
            "not implemented.");
    }
    // TODO: Check that aggregation unit has no field separator in it.
    *f << datapoint.aggregation_unit << FIELD_SEPARATOR;
  }

  // Per HFT Tables
  // TODO: To what precision will be rounded?
  for (const auto i : datapoint.data.hft_data){
    if (mass_density_per_hft.is_open())
      mass_density_per_hft << i.second.massdens;
  }

}

void TextTableWriter::write_captions(const Datapoint& datapoint) {
  // Write common column captions for all output tables.
  for (auto& f : file_streams) {
    switch (interval) {
      case OutputInterval::OI_DAILY:
        // TODO
        break;
      case OutputInterval::OI_MONTHLY:
        // TODO
        break;
      case OutputInterval::OI_ANNUAL:
        *f << "year" << FIELD_SEPARATOR;
        break;
      case OutputInterval::OI_DECADAL:
        // TODO
        break;
      default:
        std::logic_error(
            "Fauna::TextTableWriter::write_datapoint() Output time interval is "
            "not implemented.");
    }
    *f << "agg_unit" << FIELD_SEPARATOR;
  }

  // Per HFT Tables
  for (const auto i : datapoint.data.hft_data){
    const std::string& hft_name = i.first->name;

    if (mass_density_per_hft.is_open())
      mass_density_per_hft << hft_name << FIELD_SEPARATOR;
  }

  for (auto& f : file_streams)
    *f << std::endl;
}
