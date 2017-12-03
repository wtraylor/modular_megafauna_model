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
#include "herbiv_outputmodule.h" // for GuessOutput::HerbivoryOutput::CAPTION_SEPARATOR
#include "herbiv_parameters.h"
#include <sstream> // for is_valid() messages

using namespace Fauna;

Hft::Hft():
	name("hft"), 
	is_included(false), 
	// SIMULATION PARAMETERS:
	// add more initializiations in alphabetical order
	bodyfat_birth(0.1),
	bodyfat_deviation(0.125),
	bodyfat_max(0.3),
	bodymass_birth(5.0),
	bodymass_male(60.0),
	bodymass_female(50.0),
	breeding_season_length(60),
	breeding_season_start(120),
	establishment_age_range(std::pair<int, int>(1, 5)),
	establishment_density(10.0),
	diet_composer(DC_PURE_GRAZER),
	digestion_type(DT_RUMINANT),
	digestive_limit(DL_NONE),
	expenditure_model(EM_TAYLOR_1981),
	digestion_bodymass_fraction(0.02),
	gestation_months(8),
	half_max_intake_density(20),
	lifespan(10),
	maturity_age_phys_female(3),
	maturity_age_phys_male(3),
	maturity_age_sex(2),
	mortality(0.05),
	mortality_juvenile(0.3),
	net_energy_model(NE_DEFAULT),
	reproduction_max(0.7),
	reproduction_model(RM_ILLIUS_OCONNOR_2000) 
{
}

bool Hft::is_valid(const Parameters& params, std::string& msg) const{
	bool is_valid = true;
	
	// The message text is written into an output string stream
	std::ostringstream stream;

	if (name=="") {
		stream << "name is empty." << std::endl;
		is_valid = false;
	}
	if (name.find(' ') != std::string::npos ||
			name.find(',') != std::string::npos ||
			name.find(GuessOutput::HerbivoryOutput::CAPTION_SEPARATOR)
			!= std::string::npos)
	{
		stream << "name contains a forbidden character. " << 
			"(' ', ',', or '"<<
			GuessOutput::HerbivoryOutput::CAPTION_SEPARATOR<<"')" 
			<< std::endl;
		is_valid = false;
	}

	//------------------------------------------------------------
	if (params.herbivore_type == HT_COHORT || 
			params.herbivore_type == HT_INDIVIDUAL ){

		if (bodyfat_birth <= 0.0) {
			stream << "bodyfat_birth must be >0.0 ("<<bodyfat_birth<<")"<<std::endl;
			is_valid = false;
		}

		if (bodyfat_birth > bodyfat_max) {
			stream << "bodyfat_birth must not exceed bodyfat_max ("
				<<bodyfat_birth<<")"<<std::endl;
			is_valid = false;
		}

		if (bodyfat_deviation < 0.0 || bodyfat_deviation > 1.0){
			stream << "bodyfat_deviation is out of bounds. (Current value: "
				<<bodyfat_deviation << ")" << std::endl;
			is_valid = false;
		}

		if (bodyfat_max <= 0.0 || bodyfat_max >= 1.0) {
			stream << "bodyfat_max must be between 0.0 and 1.0"
				<<bodyfat_max<<")"<<std::endl;
			is_valid = false;
		}

		if (bodymass_birth <= 0.0) {
			stream << "bodymass_birth must be > 0.0 ("
				<<bodyfat_birth<<")"<<std::endl;
			is_valid = false;
		}

		if (bodymass_birth >= bodymass_male || bodymass_birth >= bodymass_female) {
			stream << "bodymass_birth must not be greater than either "
				<<"bodymass_male or bodymass_female ("
				<<bodymass_birth<<")"<<std::endl;
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

		if (digestive_limit == DL_NONE) {
			stream << "Warning: No digestive limit defined."<<std::endl;
			// the HFT is still valid (e.g. for testing purpose)
		}

		if (establishment_age_range.first < 0 ||
				establishment_age_range.second < 0) {
			stream << "establishment_age_range must be 2 positive numbers ("
				<<establishment_age_range.first << ", "
				<<establishment_age_range.second<<")"<<std::endl;
			is_valid = false;
		}

		if (establishment_age_range.first  >= lifespan ||
				establishment_age_range.second >= lifespan) {
			stream << "establishment_age_range must be smaller than `lifespan` ("
				<<establishment_age_range.first << ", "
				<<establishment_age_range.second<<")"<<std::endl;
			is_valid = false;
		}

		if (establishment_age_range.first  > establishment_age_range.second) {
			stream << "First number of `establishment_age_range` must be smaller "
				<< " the second number ("
				<<establishment_age_range.first << ", "
				<<establishment_age_range.second<<")"<<std::endl;
			is_valid = false;
		}

		if (establishment_density <= 0.0) {
			stream << "establishment_density must be >=0.0 ("
				<<establishment_density<<")"<<std::endl;
			is_valid = false;
		}

		const double establishment_cohort_count =
			(2 * (establishment_age_range.second - establishment_age_range.first + 1));
		if (params.herbivore_type == HT_COHORT && 
				establishment_density/establishment_cohort_count <= params.dead_herbivore_threshold){
			stream << "establishment_density (" <<establishment_density<<" ind/km²) "
				<< "must not be smaller than minimum viable population density"
				<< " (dead_herbivore_threshold = "
				<< params.dead_herbivore_threshold << " ind/km²)"
				<< " for one sex and age in cohort mode." <<std::endl;
			is_valid = false;
		}

		if (params.herbivore_type == HT_INDIVIDUAL && 
				establishment_density <= 2.0/params.habitat_area_km2) {
			stream << "establishment_density (" <<establishment_density<<" ind/km²) "
				<< "must not be smaller than two individuals in a habitat"
				<< " (habitat_area_km2 = "
				<< params.habitat_area_km2 << " km²)." <<std::endl;
			is_valid = false;
		}

		if (foraging_limits.empty()) {
			stream << "Warning: No foraging limits defined."<<std::endl;
			// the HFT is still valid (e.g. for testing purpose)
		}

		if (foraging_limits.count(FL_ILLIUS_OCONNOR_2000) &&
				diet_composer != DC_PURE_GRAZER) {
			stream << "`ILLIUS_OCONNOR_2000` is set as a foraging limit and"
				"requires a pure grass diet." << std::endl;
			is_valid = false;
		}

		if (foraging_limits.count(FL_ILLIUS_OCONNOR_2000) &&
				!(half_max_intake_density > 0.0)){
			stream << "half_max_intake_density must be >0 "
				"if `ILLIUS_OCONNOR_2000` is set as a foraging limit."
				<< " (current value: "<<half_max_intake_density<<")"<<std::endl;
			is_valid = false;
		}

		if (gestation_months <= 0) {
			stream << "`gestation_months` must be a positive number."
				<< " (current value: "<<gestation_months<<")"<<std::endl;
			is_valid = false;
		}

		if (digestive_limit == DL_BODYMASS_FRACTION &&
				!(digestion_bodymass_fraction > 0.0 && digestion_bodymass_fraction < 1.0)){
			stream << "`digestion_bodymass_fraction` must be in interval (0,1) "
				"if `bodymass_fraction` is set as a digestive limit."
				<< " (current value: "<<digestion_bodymass_fraction<<")"<<std::endl;
			is_valid = false;
		}

		if (maturity_age_phys_female < 1) {
			stream << "maturity_age_phys_female must be >=1"
				<<" (current value: "<<maturity_age_phys_female<<")"<<std::endl;
			is_valid = false;
		}

		if (maturity_age_phys_male < 1) {
			stream << "maturity_age_phys_male must be >=1"
				<<" ("<<maturity_age_phys_male<<")"<<std::endl;
			is_valid = false;
		}

		if (maturity_age_sex < 1) {
			stream << "maturity_age_sex must be >=1"
				<<" ("<<maturity_age_sex<<")"<<std::endl;
			is_valid = false;
		}

		if (mortality_factors.empty()) {
			stream << "Warning: No mortality factors defined."<<std::endl;
			// it is still valid (mainly for testing purposes)
		}

		if (mortality_factors.count(MF_BACKGROUND)) {
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
		}

		if (mortality_factors.count(MF_LIFESPAN)) {
			if (lifespan < 1) {
				stream << "lifespan must be >=1 ("<<lifespan<<")"<<std::endl;
				is_valid = false;
			}

			if (maturity_age_phys_female >= lifespan) {
				stream << "maturity_age_phys_female must not exceed lifespan"
					<<" ("<<maturity_age_phys_female<<")"<<std::endl;
				is_valid = false;
			}

			if (maturity_age_phys_male >= lifespan) {
				stream << "maturity_age_phys_male must not exceed lifespan"
					<<" ("<<maturity_age_phys_male<<")"<<std::endl;
				is_valid = false;
			}

			if (maturity_age_sex >= lifespan) {
				stream << "maturity_age_sex must not exceed lifespan"
					<<" ("<<maturity_age_sex<<")"<<std::endl;
				is_valid = false;
			}

		}

		if (reproduction_model == RM_ILLIUS_OCONNOR_2000 ||
				reproduction_model == RM_CONST_MAX ||
				reproduction_model == RM_LINEAR){
			if (reproduction_max <= 0.0) {
				stream << "reproduction_max must be >0.0 ("
					<<reproduction_max<<")"<<std::endl;
				is_valid = false; 
			}
			
			if (breeding_season_length < 0 || breeding_season_length > 365) {
				stream << "breeding_season_length must be in [0,365]"
					<<" ("<<breeding_season_length<<")"<<std::endl;
				is_valid = false; 
			}
			
			if (breeding_season_start < 0 || breeding_season_start >= 365) {
				stream << "breeding_season_start must be in [0,364]"
					<<" ("<<breeding_season_start<<")"<<std::endl;
				is_valid = false; 
			}
		}
		// add more checks in alphabetical order
	}

	// convert stream to string
	msg = stream.str();

	return is_valid;
}
