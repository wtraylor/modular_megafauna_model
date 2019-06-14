//////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Nitrogen uptake and excretion by herbivores
/// \ingroup group_herbivory
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date April 2018
//////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "nitrogen.h"
#include "forageclasses.h"

using namespace Fauna;

const double Fauna::NitrogenInHerbivore::N_CONTENT_IN_TISSUE = .03;

double Fauna::get_retention_time(const double bodymass) {
	if (bodymass <= 0.0)
		throw std::invalid_argument("Fauna::get_retention_time() "
				"Parameter `bodymass` must be positive.");
	return 32.8 * pow(bodymass, 0.07); // [h]
}

void NitrogenInHerbivore::digest_today(const double retention_time,
		const double massdens)
{
	if (retention_time <= 0.0)
		throw(std::invalid_argument("Fauna::NitrogenInHerbivore::digest_today() "
					"Parameter `retention_time` <= 0.0"));

	if (massdens < 0.0)
		throw(std::invalid_argument("Fauna::NitrogenInHerbivore::digest_today() "
					"Parameter `massdens` < 0.0"));

	// Maximum amount of nitrogen in the guts of an animal [kgN/km²].
	const double max_in_guts =  
		ingested // [kgN/km²/day]
		* retention_time/24.0; // [day]

	assert(N_CONTENT_IN_TISSUE >= 0.0 && N_CONTENT_IN_TISSUE < 1.0);

	// Maximum amount of nitrogen in the population [kgN/km²].
	const double max_bound = max_in_guts + massdens * N_CONTENT_IN_TISSUE;


	// Add nitrogen that “overflows” the limits of gut (and tissue) capacity
	// to the excreta and update the bound nitrogen pool without exceeding 
	// the maximum.
	const double new_excreta = max(0.0, bound + ingested - max_bound);
	excreta += new_excreta;
	bound = min(max_bound, bound + ingested);

	// Reset ingested nitrogen because it is now accounted for 
	ingested = 0.0; 
}

void NitrogenInHerbivore::ingest(const double eaten_nitrogen) {
	if (eaten_nitrogen < 0.0)
		throw(std::invalid_argument("Fauna::NitrogenInHerbivore::ingest() "
					"Parameter `eaten_nitrogen` < 0.0"));

	this->ingested += eaten_nitrogen; 
}

void NitrogenInHerbivore::merge(const NitrogenInHerbivore& other){
	this->excreta  += other.excreta;
	this->ingested += other.ingested;
	this->bound    += other.bound;
}

double NitrogenInHerbivore::reset_excreta(){
	const double result = get_excreta();
	excreta = 0.0;
	return result;
}

double NitrogenInHerbivore::reset_total(){
	const double result = get_unavailable() + get_excreta();
	bound = excreta = 0.0;
	return result;
}
