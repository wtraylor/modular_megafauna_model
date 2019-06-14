//////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Function objects to distribute forage among herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date 2018-06-16
//////////////////////////////////////////////////////////////////////////
#ifndef HERBIV_FEED_H
#define HERBIV_FEED_H

#include "forageclasses.h" // for ForageDistribution
#include <memory>                 // for std::auto_ptr
#include <vector>                 // for HerbivoreVector

namespace Fauna{
	// forward declarations
	class HerbivoreInterface;

	/// A vector of herbivore pointers.
	/** Originally defined in \ref herbiv_population.h */
	typedef std::vector<HerbivoreInterface*> HerbivoreVector;

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

		/// Virtual destructor.
		/** The virtual destructor is necessary so that the object is properly
		 * released from memory.*/
		virtual ~DistributeForage(){}
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

} // namespace Fauna
#endif // HERBIV_FEED_H
