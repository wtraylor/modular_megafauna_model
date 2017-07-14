//////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////
#ifndef HERBIV_DIGESTIBILITY_H
#define HERBIV_DIGESTIBILITY_H

// Forward declarations of LPJ-GUESS classes
class Individual;

namespace Fauna {

	/// Abstract base class (interface) for calculating forage digestibility
	/**
	 * This is a function object and implementing the 
	 * \ref sec_strategy design pattern.
	 */
	struct GetDigestibility{
		/// Get current forage digestibility for one plant individual.
		virtual double operator()(const Individual& Individual)const=0;
	};

	/// Digestibility model using \ref PftParams::digestibility.
	/** 
	 * Digestibility is a fixed value for each \ref Pft.
	 */
	class PftDigestibility : public GetDigestibility{
	public:
		/// Returns \ref PftParams::digestibility.
		virtual double operator()(const Individual& individual) const;
	};

}
#endif // HERBIV_DIGESTIBILITY_H
