///////////////////////////////////////////////////////////////////
/// \file 
/// \brief Classes for abiotic conditions for herbivores.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date November 2017
////////////////////////////////////////////////////////////////////
#ifndef HERBIV_ENVIRONMENT_H
#define HERBIV_ENVIRONMENT_H

namespace Fauna{
	/// Variables describing the habitat that are not forage ⇒ abiotic environment.
	struct HabitatEnvironment{
		/// Depth of snow cover [cm].
		double snow_depth;
	};
}

#endif // HERBIV_ENVIRONMENT_H
