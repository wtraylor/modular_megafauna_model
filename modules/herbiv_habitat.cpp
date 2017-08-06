///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Classes for the spatial units where herbivores live.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///////////////////////////////////////////////////////////////////////////////////////
#include "config.h"
#include "herbiv_habitat.h"
#include "herbiv_forageclasses.h" // for HabitatForage and ForageMass

using namespace Fauna;

Habitat::Habitat(std::auto_ptr<HftPopulationsMap> populations):
	populations(populations)
{
	if (this->populations.get() == NULL)
		throw std::invalid_argument("Fauna::Habitat::Habitat() "
				"Argument \"populations\" is NULL.");
}

void Habitat::init_day(const int today){
	if (today<0 || today >= 365)
		throw std::invalid_argument( "Fauna::Habitat::init_day() "
				"Parameter \"today\" is out of range.");
	// TODO
}

void Habitat::remove_eaten_forage(const ForageMass& eaten_forage){

	// TODO
	// actual forage removal in derived classes
}
