///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FRAMEWORK_H
#define HERBIV_FRAMEWORK_H

#include "herbiv_forageclasses.h" // for ForageMass
#include "herbiv_outputclasses.h" // for FaunaOut::CombinedData
#include <vector>                 // for herbivore list
#include <map>                    // for ForageDistribution
#include <memory>                 // for std::auto_ptr

namespace Fauna{
	// Forward declarations
	class DistributeForage;
	class GetDigestibility;
	class GetSnowDepth;
	class FeedHerbivores;
	class Habitat;
	class HerbivoreInterface;
	class HftList;
	class HftPopulationsMap;
	class Parameters;

	/// A vector of herbivore pointers.
	/** Originally defined in \ref herbiv_population.h */
	typedef std::vector<HerbivoreInterface*> HerbivoreVector;


	/// Function object to feed herbivores.
	class FeedHerbivores{
		public:
			/// Constructor.
			/** 
			 * \param distribute_forage Strategy object for 
			 * calculating the forage portions.
			 * \throw std::invalid_argument If `distribute_forage==NULL`. */
			FeedHerbivores(std::auto_ptr<DistributeForage> distribute_forage);

			/// Feed the herbivores.
			/**
			 * \param[in,out] available Available forage mass in the
			 * habitat. This will be reduced by the amount of eaten 
			 * forage.
			 * \param[in,out] herbivores Herbivore objects that are
			 * being fed by calling \ref HerbivoreInterface::eat().
			 */
			void operator()(
					HabitatForage& available,
					const HerbivoreVector& herbivores) const;
		private:
			std::auto_ptr<DistributeForage> distribute_forage;
	};

	/// A habitat with the herbivores that live in it.
	/** \see \ref sec_herbiv_designoverview */
	class SimulationUnit{
		public:
			/// Constructor
			/**
			 * \throw std::invalid_argument If one of the parameters
			 * is NULL.
			 */
			SimulationUnit( std::auto_ptr<Habitat>,
					std::auto_ptr<HftPopulationsMap>);

			/// The habitat where the populations live.
			/** \throw std::logic_error If the private pointer is NULL. */
			Habitat& get_habitat(){
				if (habitat.get() == NULL)
					throw std::logic_error("Fauna::SimulationUnit::get_habitat() "
							"The unique pointer to habitat is NULL. "
							"The SimulationUnit object lost ownership "
							"of the Habitat object."); 
				return *habitat;
			};

			/// The herbivores that lives in the habitat.
			/** \throw std::logic_error If the private pointer is NULL. */
			HftPopulationsMap& get_populations(){
				if (populations.get() == NULL)
					throw std::logic_error("Fauna::SimulationUnit::get_populations() "
							"The unique pointer to populations is NULL. "
							"The SimulationUnit object lost ownership "
							"of the HftPopulationsMap object."); 
				return *populations;
			}

			/// Whether the flag for initial establishment has been set.
			bool is_initial_establishment_done()const{return initial_establishment_done;}

			/// Set the flag that initial establishment has been performed.
			void set_initial_establishment_done(){initial_establishment_done = true;}

			/// @{ \brief Get temporally aggregated habitat and herbivore output. 
			FaunaOut::CombinedData& get_output(){ return current_output; }
			const FaunaOut::CombinedData& get_output()const{
				return current_output;
			}
			/**@}*/ // Output Functions
		private:
			FaunaOut::CombinedData current_output;
			std::auto_ptr<Habitat> habitat;
			bool initial_establishment_done;
			std::auto_ptr<HftPopulationsMap> populations;
	};

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
			 * \param hftlist set of **valid** herbivore functional types
			 * \throw std::invalid_argument if any parameter not valid.
			 * \throw std::logic_error if 
			 * \ref Parameters::forage_distribution not implemented 
			 */
			Simulator(const Parameters& params, const HftList& hftlist);

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

			/// Instantiate populations for one \ref Habitat.
			/** \throw std::logic_error if \ref Parameters::herbivore_type
			 * is not implemented 
			 * \return Pointer to new object
			 */
			std::auto_ptr<HftPopulationsMap> create_populations()const;

			/// Get simulation parameters
			const Parameters& get_params(){return params;}

			/// Simulate all herbivore interactions for the current day.
			/**
			 * Call this even if you don’t want herbivores in your model
			 * because it prepares the output data which might be
			 * used by \ref GuessOutput::HerbivoryOutput.
			 *
			 * \param day_of_year Current day of year (0 = Jan 1st)
			 * \param simulation_unit The habitat and herbivores to
			 * simulate.
			 * \param do_herbivores Whether to perform herbivore 
			 * simulations.
			 * If false, only the output data of the habitats are updated.
			 * \throw std::invalid_argument if day_of_year not in [0,364]
			 */
			void simulate_day(const int day_of_year, 
					SimulationUnit& simulation_unit,
					const bool do_herbivores);
		private:
			/// Create new \ref DistributeForage object according to
			/// parameters.
			std::auto_ptr<DistributeForage> create_distribute_forage();

			const HftList& hftlist;
			const Parameters& params;
			FeedHerbivores feed_herbivores; 
			int days_since_last_establishment;
	};

	/// Interface for a forage distribution algorithm
	/** \see \ref sec_strategy */
	struct DistributeForage{
		/// Distribute forage equally among herbivores
		/**
		 * No direct competition.
		 * Under forage scarcity, each herbivore gets its share in
		 * proportion to its demanded forage.
		 * \param[in]  available Available forage in the habitat.
		 * \param[in,out] forage_distribution As input: Demanded
		 * forage of each herbivore 
		 * (see \ref HerbivoreInterface::get_forage_demands()).
		 * As output: Forage portion for each herbivore.
		 * Unit is kgDM/km².
		 * The sum of all portions must not exceed the available
		 * forage!
		 */
		virtual void operator()(
				const HabitatForage& available,
				ForageDistribution& forage_distribution) const = 0;
	};

	/// Equal forage distribution algorithm
	/**
	 * No direct competition.
	 * Under forage scarcity, each herbivore gets its share in
	 * proportion to its demanded forage.
	 *
	 * Under food scarcity, the following equation holds:
	 * \f[
	 * \frac{P_{ind}}{A} = \frac{D_{ind}}{D_{total}}
	 * \f]
	 * - \f$P_{ind}\f$: individual portion of one herbivore
	 * - \f$D_{ind}\f$: forage demanded by that herbivore
	 * - A: total available forage
	 * - \f$D_{total}\f$: sum of all forage demands
	 *
	 * \note There could occur precision errors leading to the sum of
	 * forage being greater than what is available.
	 * To counteract this, only a fraction of 99.9\% of the available
	 * forage gets actually distributed.
	 */
	struct DistributeForageEqually: public DistributeForage{
		virtual void operator()(
				const HabitatForage& available,
				ForageDistribution& forage_distribution) const;
	};
}
#endif // HERBIV_FRAMEWORK_H
