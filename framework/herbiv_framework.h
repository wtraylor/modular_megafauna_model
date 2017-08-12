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
#include <vector>                 // for herbivore list
#include <map>                    // for ForageDistribution
#include <memory>                 // for std::auto_ptr

namespace Fauna{
	// Forward declarations
	class DistributeForageEqually;
	class DistributeForage;
	class GetDigestibility;
	class Habitat;
	class HerbivoreInterface;
	class HftList;
	class HftPopulationsMap;
	class Parameters;

	/// Central herbivory framework class.
	/**
	 * \see \ref sec_inversion_of_control
	 */
	class Simulator{
		public:
			/// Constructor, initializing simulation settings.
			/** 
			 * \param params **valid** global simulation parameters
			 * \param hftlist set of **valid** herbivore functional types
			 * \throw std::invalid_argument if any parameter not valid.
			 */
			Simulator(const Parameters& params, const HftList& hftlist);

			/// Construct a digestibility model object according to parameters
			/** \throw std::logic_error if 
			 * \ref Parameters::digestibility_model is not implemented.
			 * \return Pointer to newly constructed object */
			std::auto_ptr<GetDigestibility> create_digestibility_model()const;

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
			 * \param day_of_year Current day of year (0 = Jan 1st)
			 * \param habitat The Habitat to simulate
			 * \param do_herbivores Whether to perform herbivore 
			 * simulations.
			 * If false, only the output data of the habitats are updated.
			 * \throw std::invalid_argument if day_of_year not in [0,364]
			 */
			void simulate_day(const int day_of_year, Habitat& habitat,
					const bool do_herbivores);
		private:
			const HftList& hftlist;
			const Parameters& params;
			int days_since_last_establishment;

			/// Get the forage distribution algorithm class chosen in parameters
			/**\return a function object (see \ref sec_functors) that can
			 * be called directly with the `()` operator.
			 * \throw std::logic_error if 
			 * \ref Parameters::forage_distribution not implemented */
			DistributeForage& distribute_forage();
	};

	/// Interface for a forage distribution algorithm
	/** \see \ref sec_strategy */
	class DistributeForage{
		public:
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
