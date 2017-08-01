///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_digestibility.h"

#include "guess.h" // for Individual

using namespace Fauna;

double PftDigestibility::operator()(const Individual& individual) const{
	const double result = individual.pft.herbiv_params.digestibility;
	assert( result > 0.0 && result <= 1.0 );
	return result;
}
