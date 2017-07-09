///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Central management of the herbivory simulation.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2017-05-10
///////////////////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FRAMEWORK_H
#define HERBIV_FRAMEWORK_H

#include "herbiv_digestibility.h" // for DigestibilityModelType
#include "herbiv_foraging.h"      // for ForageMass
#include "herbiv_population.h"    // for HftPopulationsMap
#include <vector>                 // for herbivore list
#include <map>                    // for ForageDistribution

namespace Fauna{
	// Forward declarations
	class DistributeForageEqually;
	class DistributeForage;
	class Habitat;
	class HerbivoreInterface;
	class HftList;
	class Parameters;

	/// Central herbivory framework class.
	class Simulator{
	public:
		/// Constructor, initializing simulation settings.
		Simulator(const Parameters& params, const HftList& hftlist);

		/// Simulate all herbivore interactions for the current day.
		/**
		 * Call this even if you don’t want herbivores in your model
		 * because it prepares the output data which might be
		 * used by \ref GuessOutput::HerbivoryOutput.
		 * \param day_of_year Current day of year (0 = Jan 1st)
		 * \param habitat The Habitat to simulate
		 * \param do_herbivores Whether to perform herbivore simulations.
		 * If false, only the output data of the habitats are updated.
		 * \throw std::invalid_argument if day_of_year not in [0,364]
		 */
		void simulate_day(const int day_of_year, Habitat& habitat,
				const bool do_herbivores);

		/// Get simulation parameters
		const Parameters& get_params(){return params;}
	private:
		const HftList& hftlist;
		const Parameters& params;
		
		/// Instantiate populations for one \ref Habitat.
		/** \throw std::logic_error if \ref Parameters::herbivore_type
		 * is not implemented */
		HftPopulationsMap create_populations();

		/// Get the forage distribution algorithm class chosen in parameters
		/**\return a function object (see \ref sec_functor) that can
		 * be called directly with the `()` operator.
		 * \throw std::logic_error if 
		 * \ref Parameters::forage_distribution not implemented */
		DistributeForage& distribute_forage();
	};
	
	/// Map defining which herbivore gets what to eat.
	typedef std::map<HerbivoreInterface*, ForageMass> ForageDistribution;

	/// Interface for a forage distribution algorithm
	/** \see \ref sec_functor, \ref sec_strategy */
	struct DistributeForage{
		/// Distribute forage equally among herbivores
		/**
		 * No direct competition.
		 * Under forage scarcity, each herbivore gets its share in
		 * proportion to its demanded forage.
		 * \param[in]  available Available forage in the habitat.
		 * \param[in]  populations All the herbivores
		 * \param[out] forage_distribution resultant distribution of forage
		 * \param[out] forage_sum sum of `forage_distribution`
		 */
		virtual void operator()(
				const HabitatForage& available,
				const HftPopulationsMap& populations,
				ForageDistribution& forage_distribution,
				ForageMass forage_sum) const = 0;
	};

	/// Equal forage distribution algorithm
	/**
	 * No direct competition.
	 * Under forage scarcity, each herbivore gets its share in
	 * proportion to its demanded forage.
	 * \ingroup group_herbivory
	 */
	struct DistributeForageEqually: public DistributeForage{
		virtual void operator()(
				const HabitatForage& available,
				const HftPopulationsMap& populations,
				ForageDistribution& forage_distribution,
				ForageMass forage_sum) const;
	};
}
#endif // HERBIV_FRAMEWORK_H
