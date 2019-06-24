//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Models to calculate snow depth for herbivores in LPJ-GUESS.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date November 2017
//////////////////////////////////////////////////////////////////////////

#include "snowdepth.h"
#include "config.h"
#include "guess.h"

using namespace Fauna;

double SnowDepthTenToOne::operator()(const Patch& patch) const {
  // convert from mm to cm and multiply by 10
  return (patch.soil.snowpack / 10.0) * 10.0;
}
