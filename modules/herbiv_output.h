///////////////////////////////////////////////////////////////////////////////////////
//
/// \file herbiv_output.h
/// \brief Output module for the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#ifndef HERBIV_OUTPUT_MODULE_H
#define HERBIV_OUTPUT_MODULE_H

#include "outputmodule.h"
#include "gutil.h" // for xtring

#include "herbiv_foraging.h" // for Fauna::ForageType

// forward declarations of referenced classes
class Date;
namespace Fauna { 
	class HabitatOutputData;
	class Habitat;
}

namespace GuessOutput {

	/// Output module for the herbivory module.
	/** 
	 * \ingroup group_herbivory 
	 * \see sec_herbiv_new_output
	 * \see sec_herbiv_limit_output
	 */
	class HerbivoryOutput : public OutputModule {
		public:

			/// Constructor, declaring parameters
			HerbivoryOutput();

			/// Initialize, defines output tables.
			void init();

			/// Write annual output for gridcell.
			/** \see \ref OutputModule::outannual() */
			void outannual(Gridcell& gridcell);

			/// Write annual output for a number of habitats.
			/** 
			 * This function is independent of any global LPJ-GUESS
			 * variables (except for \ref Date::ndaymonth).
			 * \param longitude Value for the longitude column.
			 * \param latitude  Value for the latitude column.
			 * \param day_of_year Day of year (0 = Jan 1st).
			 * \param year Simulation year (starting with 0).
			 * \param habitats A vector of pointers to habitats whose
			 * output will be merged.
			 */
			void outannual( const double longitude, const double latitude,
					const int day_of_year, const int year,
					const std::vector<const Fauna::Habitat*> habitats) const;

			/// Write daily output for gridcell (currently not used).
			/** \see \ref OutputModule::outdaily() */
			void outdaily(Gridcell& gridcell){}

		protected:

			/// Create a column descriptor for each forage type.
			const ColumnDescriptors get_forage_columns(
					const int width, const int precision) const;

			/// Create a column descriptor for each \ref Fauna::Hft
			const ColumnDescriptors get_hft_columns(const int width, const int precision) const;

			/// Check whether the date shall be included in the output.
			/** 
			 * \param d The %date.
			 * \return True if the given year/date shall be included.
			 * \note This is equivalent to the check in \ref outlimit() in \ref commonoutput.cpp, 
			 * but implemented as a class member function.
			 */
			virtual bool include_date(const Date& d) const;

			/// Add one line to each output table
			/**
			 * \param out The Outputrows object manages the next row of output for 
			 * each output table. Create a new one for each function call with
			 * the right lon/lat and time info.
			 * \param data The aggregated output data for the whole gridcell.
			 */
			void add_output_object(OutputRows out, const Fauna::HabitatOutputData& data) const;

			/// Temporal aggregation interval (monthly, yearly, ...)
			enum {
				// TODO: DAILY
				MONTHLY,
				ANNUAL
			} interval;

			enum ColumnType {
				CS_HFT,
				CS_FORAGE,
			};

			// TODO: Document
			struct Field{
				std::string identifier;
				std::string help;
				ColumnType column_type;
				std::string filename;
				Table table;
			};
			std::vector<Field> fields;
		private: 
			/// Interval parameter string as read from instruction file.
			xtring interval_xtring;

			/// Defines all output tables and their formats.
			/** This function specifies all columns in all output tables, their names,
			 *  column widths and precision.
			 *
			 *  For each table a TableDescriptor object is created which is then sent to
			 *  the output channel to create the table.
			 */
			void define_output_tables();

			/**@{ \name Output file names */
			/// forage output files
			std::string file_forage_avail, file_forage_eaten, file_digestibility;
			/**@} */ //Output file names

			/**@{ \name Output tables */
			/// forage output tables
			Table out_forage_avail, out_forage_eaten, out_digestibility;
			/**@} */ // Output tables
	};

}

#endif // HERBIV_OUTPUT_MODULE_H
