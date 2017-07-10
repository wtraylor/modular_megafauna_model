///////////////////////////////////////////////////////////////////////////////////////
/// \file 
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
	class Habitat;
	class HabitatOutputData;
	class HftList;
}

namespace GuessOutput {

	/// Abstract helper class to limit output to a specific time.
	/**
	 * \see \ref sec_herbiv_limit_output 
	 */
	struct OutputLimiter{
		/// Check whether the date shall be included in the output.
		/** 
		 * \param day_of_year Day of year (0=Jan 1st).
		 * \param year Simulation year (0=first year).
		 * \return True if the given year/date shall be included.
		 * \note This is equivalent to the check in \ref outlimit() in \ref commonoutput.cpp, 
		 * but implemented as a class member function.
		 */
		virtual bool include_date(const int year, 
				const int day_of_year) const = 0; 
	};

	/// Limits output to the time after \ref nyear_spinup.
	struct NoSpinupLimiter: public OutputLimiter{
		virtual bool include_date(const int year, 
				const int day_of_year) const;
	};

	/// Output module for the herbivory module.
	/** 
	 * If \ref deactivate() is called, all public methods will not
	 * do anything anymore.
	 * \see \ref sec_herbiv_output
	 * \see \ref sec_herbiv_new_output
	 * \see \ref sec_herbiv_limit_output
	 */
	class HerbivoryOutput : public OutputModule {
		public:

			/// Constructor, declaring parameters, setting global instance
			/**
			 * There is only one single instance of this class in the
			 * program.
			 * \throw std::logic_error If the constructor is called 
			 * a second time.
			 */
			HerbivoryOutput();

			/// Returns the one global instance of this class
			/**
			 * \throw std::logic_error If no global instance has been
			 * created yet.
			 */
			static HerbivoryOutput& get_instance(); 

			/// Initialize, defines output tables.
			void init();

			/// Write output of one year for a gridcell.
			/** \see \ref OutputModule::outannual() 
			 * \see \ref outannual() */
			void outannual(Gridcell& gridcell);

			/// Write output of one year for a number of habitats.
			/** 
			 * Depending on \ref interval, for each day, each month,
			 * or the one year one output row is written.
			 * \note This function is independent of any global LPJ-GUESS
			 * variables (except for \ref Date::ndaymonth).
			 * \param longitude Value for the longitude column.
			 * \param latitude  Value for the latitude column.
			 * \param year Simulation year (starting with 0).
			 * \param habitats A vector of pointers to habitats whose
			 * output will be merged.
			 */
			void outannual( const double longitude, const double latitude,
					const int year,
					const std::vector<const Fauna::Habitat*> habitats) const;

			/// Write daily output for gridcell (not used).
			/** Also daily output is done in \ref outannual().
			 * \see \ref OutputModule::outdaily() */
			void outdaily(Gridcell& gridcell){}

			/// Disable any activity all together.
			void deactivate(){ isactive = false; }

			/// Set the list of HFTs for the output tables
			void set_hftlist(const Fauna::HftList& _hftlist){hftlist = &_hftlist;}

			/// Set an object that limits the output to a time.
			/** \param l Reference Limiter object. Make sure this object
			 * is not released over the whole program run! 
			 * Pass NULL in order to have no limits.*/
			void set_limiter(OutputLimiter* l){ limiter = l; }
		protected:
			/// Temporal aggregation interval (monthly, yearly, ...)
			enum Interval{
				/// Daily output
				DAILY,
				/// Monthly output
				MONTHLY,
				/// Annual output
				ANNUAL
			};

			/// Create a column descriptor for each forage type.
			const ColumnDescriptors get_forage_columns() const;

			/// Create a column descriptor for each \ref Fauna::Hft
			/** \throw std::logic_error if \ref hftlist not defined. */
			const ColumnDescriptors get_hft_columns() const;

			/// Add one line to each output table
			/**
			 * \param out The Outputrows object manages the next row of output for 
			 * each output table. Create a new one for each function call with
			 * the right lon/lat and time info.
			 * \param data The aggregated output data for the whole gridcell.
			 */
			void add_output_object(OutputRows out, const Fauna::HabitatOutputData& data) const;

			/// Helper function to check if \ref limiter includes the day.
			/** \see GuessOutput::OutputLimiter::include_date() */
			bool include_date(const int day, const int year) const{
				if (limiter != NULL) 
					return limiter->include_date(day, year);
				else
					return true;
			}

			/// Width of one column in the output table.
			static const int column_width = 8;

			/// Temporal aggregation interval
			Interval get_interval()const{return interval;}

			/// Decimal precision for the values in the columns
			double get_precision()const{return precision;}

		private: 
			Fauna::HftList const* hftlist;
			static HerbivoryOutput* global_instance;

			Interval interval;
			/// Interval parameter string as read from instruction file.
			xtring interval_xtring;

			bool isactive;
			OutputLimiter* limiter;
			int precision; 

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
			std::string file_forage_avail, file_forage_eaten, 
				file_digestibility;
			/**@} */ //Output file names

			/**@{ \name Output tables */
			/// forage output tables
			Table out_forage_avail, out_forage_eaten, out_digestibility;
			/**@} */ // Output tables
	};
}

#endif // HERBIV_OUTPUT_MODULE_H
