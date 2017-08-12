///////////////////////////////////////////////////////////////////
/// \file 
/// \brief Output classes of the herbivory module.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date August 2017
/// \see \ref sec_herbiv_output
////////////////////////////////////////////////////////////////////
#ifndef HERBIV_OUTPUT_H
#define HERBIV_OUTPUT_H

#include "herbiv_forageclasses.h"
#include "herbiv_hft.h" // for MortalityFactor

namespace Fauna {
	// forward declarations
	class Hft;

	/// Helper function to see if a day is the first of a month.
	/**
	 * \param day Day of year (0=Jan 1st).
	 * \return True if `day` (0–364) is first day of a month.
	 * \throw std::invalid_argument If `day` not in [0,364].
	 * \todo This is misplaced.
	 */
	bool is_first_day_of_month(int day);
}

namespace FaunaOut {
	class HerbivoreData; // forward declaration

	/// Habitat output data for one time unit.
	/**
	 * \see \ref sec_herbiv_outputclasses 
	 */
	struct HabitatData{
		/// Default constructor
		HabitatData(): datapoint_count(0){}

		/// Available forage in the habitat.
		Fauna::HabitatForage available_forage;

		/// Forage mass [kgDM/km²/day] eaten by herbivores.
		Fauna::ForageMass eaten_forage;

		//------------------------------------------------------------
		/** @{ \name Aggregation Functionality */
		/// Aggregate data of this object with another one.
		/** \return This object. 
		 * \see \ref Fauna::average(),
		 *      \ref Fauna::HabitatForage::merge(), 
		 *      \ref Fauna::ForageValues::merge()
		 */
		HabitatData& merge(const HabitatData&);

		/// How many data points are merged in this object.
		unsigned int datapoint_count;

		/// Reset to initial values.
		void reset(){
			// Simply call the copy assignment operator
			this->operator=(HabitatData());
		}
		/** @} */ // Aggregation Functionality
	};

	/// Herbivore output data for one time unit.
	/** \see \ref sec_herbiv_outputclasses */
	struct HerbivoreData{
		/// Default Constructor
		HerbivoreData(): datapoint_count(0){}

		/// Age in years.
		double age_years;

		/// Body fat [fraction].
		double bodyfat;

		/// Individual density [ind/km²].
		double inddens;

		/// Mass density [kg/km²].
		double massdens;

		/// Daily mortality rate [ind/ind/day].
		std::map<Fauna::MortalityFactor, double> mortality;

		/// Eaten forage [kgDM/ind/day].
		Fauna::ForageMass eaten_forage;

		/// Energy expenditure [MJ/ind/day].
		double expenditure;

		//------------------------------------------------------------
		/** @{ \name Aggregation Functionality */
		/// Aggregate data of this object with another one.
		/** 
		 * \ref mortality :
		 * Only those factors are included in the 
		 * result that are present in both objects (intersection).
		 * All other map entries are deleted.
		 * \return This object. 
		 * \see \ref Fauna::average(), \ref Fauna::ForageValues::merge()
		 */
		HerbivoreData& merge(const HerbivoreData&);

		/// How many data points are merged in this object.
		unsigned int datapoint_count;

		/// Reset to initial values.
		void reset(){
			// Simply call the copy assignment operator
			this->operator=(HerbivoreData());
		}
		/** @} */ // Aggregation Functionality
	};

	/// All output data of the herbivory simulation for one time unit.
	/** \see \ref sec_herbiv_outputclasses */
	struct CombinedData{
		/// Habitat output data.
		HabitatData habitat_data;

		/// Herbivore output data aggregated by HFT.
		std::map<const Fauna::Hft*, HerbivoreData> hft_data;

		/// Reset to empty state.
		void reset(){
			habitat_data.reset();
			hft_data.clear();
		}
	};

	/// Aggregates herbivory output data over time and across habitats.
	/** 
	 * \see \ref sec_herbiv_outputclasses 
	 */
	class Aggregator{
		public:
			/// @{ Add output data to the aggregation. 
			void add(const HabitatData& data){
				aggregation.habitat_data.merge(data);
			}
			void add(const Fauna::Hft& hft, const HerbivoreData& data){
				aggregation.hft_data[&hft].merge(data);
			}
			/** @} */

			/// Retrieve aggregated data and reset object.
			CombinedData reset(){
				CombinedData result(aggregation); // copy constructor
				aggregation.reset();
				return result;
			}
		private:
			CombinedData aggregation;
	};
}

#endif // HERBIV_OUTPUT_H
