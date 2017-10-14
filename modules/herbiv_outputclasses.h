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

		/// Constructor, initializing with zero values.
		HabitatData():
			available_forage(), // already zeros by default.
			eaten_forage()      // already zeros by default.
		{}

		/// Available forage in the habitat.
		Fauna::HabitatForage available_forage;

		/// Forage mass [kgDM/km²/day] eaten by herbivores.
		/** This equals the sum of \ref HerbivoreData::eaten_forage over
		 * all HFTs */
		Fauna::ForageMass eaten_forage;

		//------------------------------------------------------------
		/** @{ \name Aggregation Functionality */
		/// Aggregate data of this object with another one.
		/** 
		 * \param other The other object to be merged into this one.
		 * \param this_weight Weight of this object in average building.
		 * \param other_weight Weight of `other` in average building.
		 * \return This object. 
		 * \see \ref Fauna::average(),
		 *      \ref Fauna::HabitatForage::merge(), 
		 *      \ref Fauna::ForageValues::merge()
		 * \throw std::invalid_argument If either weight is not a positive
		 * number or if both are zero.
		 */
		HabitatData& merge(const HabitatData& other,
				const double this_weight,
				const double other_weight);

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
		/// Constructor, initializing with zero values.
		HerbivoreData():
			age_years(0.0),
			bodyfat(0.0),
			eaten_forage(0.0),
			energy_intake(0.0),
			expenditure(0.0),
			inddens(0.0),
			massdens(0.0)
		{}

		//------------------------------------------------------------
		/** @{ \name Per-individual variables */

		/// Age in years.
		double age_years;

		/// Body fat [fraction].
		double bodyfat;

		/// Energy expenditure [MJ/ind/day].
		double expenditure;

		/** @} */ // Per-Individual variables

		//------------------------------------------------------------
		/** @{ \name Per-habitat variables */

		/// Individual density [ind/km²].
		double inddens;

		/// Mass density [kg/km²].
		double massdens;

		/// Daily mortality rate [ind/ind/day].
		std::map<Fauna::MortalityFactor, double> mortality;

		/// Eaten forage [kgDM/ind/km²].
		Fauna::ForageMass eaten_forage;

		/// Intake of net energy in forage [MJ/ind/km²]
		Fauna::ForageEnergy energy_intake;

		/** @} */ // Per-habitat variables

		//------------------------------------------------------------
		/** @{ \name Aggregation Functionality */
		/// Aggregate data of this object with another one.
		/** 
		 * This function builds **averages** for all member variables.
		 *
		 * \ref mortality : Only those factors are included in the 
		 * result that are present in both objects (intersection).
		 * All other map entries are deleted. This is necessary because
		 * the statistical weight is the same for *all* variables.
		 *
		 * \param other The other object to be merged into this one.
		 * \param this_weight Weight of this object in average building.
		 * \param other_weight Weight of `other` in average building.
		 * \return This object. 
		 * \see \ref Fauna::average(), \ref Fauna::ForageValues::merge()
		 * \throw std::invalid_argument If either weight is not a positive
		 * number or if both are zero.
		 */
		HerbivoreData& merge(const HerbivoreData& other,
				const double this_weight,
				const double other_weight);

		/// Reset to initial values.
		void reset(){
			// Simply call the copy assignment operator
			this->operator=(HerbivoreData());
		}

		/// Aggregate herbivore data *within one habitat*.
		/**
		 * As opposed to \ref merge(), this function is intended to combine
		 * data of *one habitat* in *one point of time* into a single data 
		 * point. 
		 * This can then be merged with other data points across space and 
		 * time, using \ref merge().
		 *
		 * For variables *per individual*, this function creates the 
		 * **average** (just like \ref merge()).
		 * For variables *per area* or *per habitat*, this function creates 
		 * the **sum**, adding up the numbers in the habitat.
		 *
		 * In contrast to \ref merge(), \ref mortality is summed up, and
		 * all mortality factors are included because all merged datapoints
		 * have the same weight.
		 *
		 * \throw std::invalid_argument If length of vector `data` is zero.
		 */
		static HerbivoreData create_datapoint(
				const std::vector<HerbivoreData> data);

		/** @} */ // Aggregation Functionality
	};

	/// Output data for herbivores and habitat(s).
	/** 
	 * This can be data for one \ref SimulationUnit (possibly aggregated 
	 * over a period of time) or for a set of spatial units (aggregated over
	 * time and space).
	 * \see \ref sec_herbiv_outputclasses 
	 */
	struct CombinedData{
		/// Constructor.
		CombinedData():
			datapoint_count(0)
		{}

		/// How many data points are merged in this object.
		unsigned int datapoint_count;

		/// Habitat output data.
		HabitatData habitat_data;

		/// Herbivore output data aggregated by HFT.
		std::map<const Fauna::Hft*, HerbivoreData> hft_data;

		/// Merge other data into this object.
		/**
		 * Use this to aggregate (=build averages) over space and time.
		 * \ref datapoint_count is used to weigh the values in 
		 * average-building.
		 *
		 * If the other object contains no data (\ref datapoint_count == 0),
		 * this function does nothing.
		 *
		 * For herbivore data (\ref hft_data), the merge routine creates an
		 * empty \ref HerbivoreData object as a ‘stub’ if it the HFT is found
		 * in one of the merge partners, but not in the other one. This way, 
		 * the averages are built correctly across habitats even if in one
		 * habitat, there are no herbivores of one type.
		 *
		 * \return This object after merging.
		 *
		 * \see \ref HerbivoreData::merge()
		 * \see \ref HabitatData::merge()
		 */
		CombinedData& merge(const CombinedData&);

		/// Retrieve aggregated data and reset object.
		CombinedData reset(){
			// copy old object
			CombinedData result = *this; 
			// reset with copy assignment operator
			this->operator=(CombinedData());
			return result;
		}
	};
}

#endif // HERBIV_OUTPUT_H
