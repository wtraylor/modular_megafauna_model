///////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Models for digestibility of herbivore forage.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_digestibility.h"

#include "guess.h"

using namespace Fauna;

// initialize static member
DigestibilityModel* DigestibilityModel::global_model = NULL;

double PftDigestibility::get_digestibility(const Individual& individual) const{
	return individual.pft.herbiv_params.digestibility;
}
