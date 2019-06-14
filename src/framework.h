//////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
//////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FRAMEWORK_H
#define HERBIV_FRAMEWORK_H

#include "feed.h"          // for FeedHerbivores & ForageMass
#include <memory>                 // for std::auto_ptr

namespace Fauna{
	// Forward declarations
	class GetDigestibility;
	class GetSnowDepth;
	class Hft;
	class HftList;
	class HftPopulationsMap;
	class Parameters;
	class PopulationInterface;
	class SimulationUnit;

	/// Central herbivory framework class.
	/**
	 * \see \ref sec_herbiv_designoverview
	 * \see \ref sec_inversion_of_control
	 */
	class Simulator{
		public:
			/// Constructor, initializing simulation settings.
			/** 
			 * \param params **valid** global simulation parameters
			 * \throw std::logic_error if 
			 * \ref Parameters::forage_distribution not implemented 
			 */
			Simulator(const Parameters& params);

			/// Construct a digestibility model object for LPJ-GUESS according to parameters
			/** \throw std::logic_error if 
			 * \ref Parameters::digestibility_model is not implemented.
			 * \return Pointer to newly constructed object. */
			std::auto_ptr<GetDigestibility> create_digestibility_model()const;

			/// Construct a snow depth model object for LPJ-GUESS according to parameters
			/** \throw std::logic_error if 
			 * \ref Parameters::snow_depth_model is not implemented.
			 * \return Pointer to newly constructed object. */
			std::auto_ptr<GetSnowDepth> create_snow_depth_model()const;

			/// Create one (empty) herbivore population for one HFT.
			/**
			 * \param phft Pointer to the Hft.
			 * \throw std::logic_error if \ref Parameters::herbivore_type
			 * is not implemented 
			 * \return Pointer to new object.
			 */
			std::auto_ptr<PopulationInterface> create_population(
					const Hft* phft)const;

			/// Instantiate populations for one \ref Habitat.
			/**
			 * \param hftlist For each HFT in the list, one population will be
			 * created.
			 * \throw std::logic_error if \ref Parameters::herbivore_type
			 * is not implemented 
			 * \return Pointer to new object
			 */
			std::auto_ptr<HftPopulationsMap> create_populations(
					const HftList& hftlist)const;

			/// Instantiate a populaton of only one \ref Hft for one \ref Habitat.
			/**
			 * \param phft Pointer to the one \ref Hft.
			 * \throw std::logic_error if \ref Parameters::herbivore_type
			 * is not implemented.
			 * \return Pointer to new object.
			 */
			std::auto_ptr<HftPopulationsMap> create_populations(
					const Hft* phft)const;

			/// Get simulation parameters
			const Parameters& get_params(){return params;}

			/// Simulate all herbivore interactions for the current day.
			/**
			 * Call this even if you don’t want herbivores in your model
			 * because it prepares the output data which might be
			 * used by \ref GuessOutput::HerbivoryOutput.
			 *
			 * This function does not much more than creating and calling a
			 * function object of \ref SimulateDay.
			 *
			 * \param day_of_year Current day of year (0 = Jan 1st)
			 * \param simulation_unit The habitat and herbivores to
			 * simulate.
			 * \param do_herbivores Whether to perform herbivore 
			 * simulations.
			 * If false, only the output data of the habitats are updated.
			 * \throw std::invalid_argument If day_of_year not in [0,364].
			 */
			void simulate_day(const int day_of_year, 
					SimulationUnit& simulation_unit,
					const bool do_herbivores);
		private:
			/// Create new \ref DistributeForage object according to
			/// parameters.
			std::auto_ptr<DistributeForage> create_distribute_forage();

			const Parameters& params;
			FeedHerbivores feed_herbivores; 
			int days_since_last_establishment;
	};
}
#endif // HERBIV_FRAMEWORK_H
