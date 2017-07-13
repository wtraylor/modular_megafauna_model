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
	 * @startuml "Construction of PatchHabitat"
	 * participant "framework()" as framework
	 * framework    -> "Fauna::Simulator" : <<create>>
	 * activate "Fauna::Simulator"
	 * framework    -> Gridcell : <<create>>
	 * activate Gridcell
	 * Gridcell     -> Patch : <<create>>
	 * activate Patch
	 * framework    -> "Fauna::Simulator" : create_populations()
	 * framework  <--  "Fauna::Simulator" : populations
	 * framework    -> "Fauna::Simulator" : create_digestibility_model()
	 * framework  <--  "Fauna::Simulator" : digestibility_model
	 * framework    -> "Fauna::PatchHabitat" : <<create>> (populations, digestibility_model)
	 * note right : constructor injection
	 * activate "Fauna::PatchHabitat"
	 * framework    -> Patch : set_habitat()
	 * note right : setter injection
	 * @enduml
	 * \see \ref sec_inversion_of_control
	 */
	class Simulator{
	public:
		/// Constructor, initializing simulation settings.
		Simulator(const Parameters& params, const HftList& hftlist);

		/// Construct a digestibility model object according to parameters
		/** \throw std::logic_error if \ref Parameters::dig_model is
		 * not implemented.
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
		 * \param do_herbivores Whether to perform herbivore simulations.
		 * If false, only the output data of the habitats are updated.
		 * \throw std::invalid_argument if day_of_year not in [0,364]
		 */
		void simulate_day(const int day_of_year, Habitat& habitat,
				const bool do_herbivores);
	private:
		const HftList& hftlist;
		const Parameters& params;
		
		/// Get the forage distribution algorithm class chosen in parameters
		/**\return a function object (see \ref sec_functors) that can
		 * be called directly with the `()` operator.
		 * \throw std::logic_error if 
		 * \ref Parameters::forage_distribution not implemented */
		DistributeForage& distribute_forage();
	};

	/// Map defining which herbivore gets what to eat.
	typedef std::map<HerbivoreInterface*, ForageMass> ForageDistribution;

	/// Interface for a forage distribution algorithm
	/** \see \ref sec_strategy */
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
