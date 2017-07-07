///////////////////////////////////////////////////////////////////////////////////////
/// \file 
/// \brief Herbivore Functional Type.
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date May 2017
///
///////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_hft.h"
#include <sstream>

using namespace Fauna;

Hft::Hft():
	name(""), // not valid
	is_included(false) 
	// SIMULATION PARAMETERS:
	// add more initializiations in alphabetical order
{}

bool Hft::is_valid(std::string& msg) const{
	bool is_valid = true;
	// TODO: make validity check dependent on HerbivoreType

	// The message text is written into an output string stream
	std::ostringstream stream;

	if (name=="") {
		stream << "name is empty" << std::endl;
		is_valid = false;
	}


	if (bodymass_female < 1) {
		stream << "bodymass_female must be >=1 ("<<bodymass_female<<")"<<std::endl;
		is_valid = false;
	}

	if (bodymass_male < 1) {
		stream << "bodymass_male must be >=1 ("<<bodymass_male<<")"<<std::endl;
		is_valid = false;
	}

	if (lifespan < 1) {
		stream << "lifespan must be >=1 ("<<lifespan<<")"<<std::endl;
		is_valid = false;
	}

	if (maturity < 1) {
		stream << "maturity must be >=1 ("<<maturity<<")"<<std::endl;
		is_valid = false;
	}

	if (lifespan <= maturity) {
		stream << "lifespan ("<<lifespan<<") should be greater than "
			<< "maturity ("<<maturity<<")"<<std::endl;
		is_valid = false;
	}

	if (mortality < 0.0 || mortality >= 1.0) {
		stream << "mortality must be between >=0.0 and <1.0 "
			"("<<mortality<<")"<<std::endl;
		is_valid = false;
	}

	if (mortality_juvenile < 0.0 || mortality_juvenile >= 1.0) {
		stream << "mortality_juvenile must be between >=0.0 and <1.0 "
			"("<<mortality_juvenile<<")"<<std::endl;
		is_valid = false;
	}

	if (reproduction_max <= 0.0) {
		stream << "reproduction_max must be >0.0 ("
			<<reproduction_max<<")"<<std::endl;
		is_valid = false; 
	}
	// add more checks in alphabetical order

	// convert stream to string
	msg = stream.str();

	return is_valid;
}
