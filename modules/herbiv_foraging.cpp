///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \ingroup group_herbivory
/// \brief Foraging models of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_foraging.h"

using namespace Fauna;


const ForageEnergy GetDigestiveLimitIllius1992::operator()(
		const double bodymass,
		const Digestibility& digestibility)const{
	// TODO
	return ForageEnergy();
}
