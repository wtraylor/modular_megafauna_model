////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Reading instruction file parameters for the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_paramreader.h"
#include "guess.h"      // for Pft and Pftlist
#include "guessstring.h" // for trim() and to_upper()
#include "parameters.h" // read instruction file
#include "plib.h"       // read instruction file
#include "shell.h"      // for dprintf()
#include <algorithm>    // for std::transform
#include <cfloat>       // for DBL_MAX, DBL_MIN
#include <climits>      // for INT_MAX
#include <sstream>      // for ostringstream and istringstream


using namespace Fauna;

std::list<std::string> Fauna::parse_comma_separated_param(
			const std::string& strparam){
	std::list<std::string> result;

	std::istringstream stream(strparam);
	while (stream.good()){
		std::string token; // one of the comma-separated items
		std::getline(stream, token, ','); // read until comma
		token = trim(token); // remove whitespaces
		if (token == "")
			continue; // skip empty tokens
		result.push_back(token);
	}
	return result;
}

bool ParamReader::check_mandatory(const MandatoryParamList& list,
		const std::string& prefix){
	// Check all parameters marked as mandatory
	bool okay = true;
	std::ostringstream msg_stream;

	MandatoryParamList::const_iterator itr = list.begin();
	while (itr != list.end()){

		if (!itemparsed(itr->param.c_str())){
			// create error message
			msg_stream << "Mandatory parameter \""<<itr->param<<"\" "
				<< "is missing.";
			if (itr->required_by != "")
				msg_stream << " Required by: " << itr->required_by;
			msg_stream << std::endl;

			okay = false;
		} 
		itr++;
	}

	if (!msg_stream.str().empty()){
		// print all error messages together
		dprintf("Error while reading herbivory parameters:\n");
		if (prefix != "") {
			dprintf(prefix.c_str());
			dprintf("\n");
		}
		dprintf(msg_stream.str().c_str());
	}

	return okay;
}

void ParamReader::check_all_params(bool& fatal, std::string& msg)const{
	fatal = false;

	// We write into a string stream for convenience and convert it later
	// to a std::string.
	std::ostringstream msg_stream;

	{ // GLOBAL PARAMETERS
		std::string tmp_msg;
		if (!params.is_valid(tmp_msg))
			fatal = true;
		msg_stream << tmp_msg;
	}

	{ // HFT PARAMETERS

		// Now check each HFT if it is valid also in the context of the global
		// parameters.
		for (HftList::const_iterator hft_itr = hftlist.begin();
				hft_itr != hftlist.end();
				hft_itr++)
		{
			std::string tmp_msg;
			if (!hft_itr->is_valid(params, tmp_msg))
				fatal = true;
			// Print message even if HFT might be valid. Maybe the message is
			// just a warning, but no fatal error.
			if (!tmp_msg.empty())
				 msg_stream << "HFT \"" << hft_itr->name << "\":" << tmp_msg;
		}
	}

	// Convert stringstream to string
	msg = msg_stream.str();
}

void ParamReader::check_all_params(
		Pftlist& pftlist, // is actually const, but container functions are not const.
		bool& fatal, std::string& msg)const
{
	// Call other overloaded function.
	check_all_params(fatal, msg);

	//------------------------------------------------------------------
	// We write into a string stream for convenience and convert it later
	// to a std::string.
	std::ostringstream msg_stream;

	// Here, we check only the PFTs in addition.

	// For each forage type: Whether at least one PFT is representing it.
	std::set<Fauna::ForageType> edible_forage_types;

	for (pftlist.firstobj();
			pftlist.isobj;
			pftlist.nextobj())
	{
		const Pft& pft = pftlist.getobj();
		const PftParams& pft_params = pft.herbiv_params;
		std::string tmp_msg;

		// Check the individual PFT.
		if (!pft_params.is_valid(params, tmp_msg))
			fatal = true;
		// Print message even if PFT might be valid. Maybe the message is
		// just a warning, but no fatal error.
		if (!tmp_msg.empty())
			msg_stream << "PFT \"" << pft.name << "\":" << tmp_msg;

		if (pft_params.forage_type != FT_INEDIBLE)
			edible_forage_types.insert(pft_params.forage_type);
	}

	// Check if at least one edible forage type is there for the herbivores.
	if (params.ifherbivory && 
			hftlist.size() != 0 &&
			edible_forage_types.empty())
	{
		msg_stream << "There is no edible forage for the herbivores. "
			<< "Please define in at least one PFT an endible `forage_type`."
			<< std::endl;
		fatal = true;
	}

	// Append new messages
	msg += msg_stream.str();
}

void ParamReader::callback(const int callback, Pft* ppft){

	// Turn the string parameter to upper case to make
	// comparison case-insensitive.
	strparam = to_upper(strparam);

	// add checkback codes in alphabetical order
	
	if (callback == CB_CHECKHFT) {

		if (params.ifherbivory) {
			// compile and check mandatory parameters
			MandatoryParamList mandatory_hft_params;

			mandatory_hft_params.push_back(MandatoryParam("include"));

			if (params.herbivore_type == HT_INDIVIDUAL ||
					params.herbivore_type == HT_COHORT) {
				// string message why parameter is required
				const std::string req_str = "herbivore_type=(cohort|individual)";
				mandatory_hft_params.push_back(MandatoryParam(
							"bodyfat_birth", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"bodyfat_max", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"bodymass_birth", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"bodymass_female", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"bodymass_male", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"diet_composer", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"digestion_type", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"establishment_density", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"expenditure_model", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"net_energy_model", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"maturity_age_phys_female", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"maturity_age_phys_male", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"maturity_age_sex", req_str));
				mandatory_hft_params.push_back(MandatoryParam(
							"reproduction_model", req_str));
				if (current_hft.mortality_factors.count(MF_LIFESPAN))
					mandatory_hft_params.push_back(MandatoryParam(
								"lifespan", req_str +
								" and lifespan in mortality_factors"));
				if (current_hft.mortality_factors.count(MF_BACKGROUND)){
					mandatory_hft_params.push_back(MandatoryParam(
								"mortality", req_str +
								" and \"background\" in mortality_factors"));
					mandatory_hft_params.push_back(MandatoryParam(
								"mortality_juvenile", req_str +
								" and \"background\" in mortality_factors"));
				}
				if (current_hft.foraging_limits.count(FL_ILLIUS_OCONNOR_2000)){
					mandatory_hft_params.push_back(MandatoryParam(
								"half_max_intake_density", req_str +
								" and \"illius_oconnor_2000\" in foraging_limits"));
				}
				if (current_hft.reproduction_model == RM_ILLIUS_OCONNOR_2000 ||
						current_hft.reproduction_model == RM_CONST_MAX)
				{
					mandatory_hft_params.push_back(MandatoryParam(
								"breeding_season_length", req_str +
								" and reproduction_model=illius_oconnor_2000"));
					mandatory_hft_params.push_back(MandatoryParam(
								"breeding_season_start", req_str +
								" and reproduction_model=illius_oconnor_2000"));
					mandatory_hft_params.push_back(MandatoryParam(
								"reproduction_max", req_str +
								" and reproduction_model=illius_oconnor_2000"));
				}
			}

			if (!check_mandatory(mandatory_hft_params,
						"HFT \""+current_hft.name+"\""))
				plibabort();
		}
		// Now everything seems okay, and we can add the HFT
		hftlist.insert(current_hft);
	}

	if (callback == CB_CHECKGLOBAL) {

		if (!itemparsed("ifherbivory"))
			dprintf("Notice: ifherbivory was not declared. "
					"It is disabled by default.\n");

		if (params.ifherbivory) {
			MandatoryParamList mandatory_global_params;

			// Add mandatory parameters 
#ifndef NO_GUESS_PARAMETERS
			// digestibility model only applies to LPJ-GUESS.
			mandatory_global_params.push_back(MandatoryParam(
						"digestibility_model"));
#endif
			mandatory_global_params.push_back(MandatoryParam(
						"forage_distribution"));
			mandatory_global_params.push_back(MandatoryParam(
						"free_herbivory_years"));
			mandatory_global_params.push_back(MandatoryParam(
						"herbivore_establish_interval"));
			mandatory_global_params.push_back(MandatoryParam(
						"herbivore_type"));

			if (params.herbivore_type == HT_COHORT) 
				mandatory_global_params.push_back(MandatoryParam(
							"dead_herbivore_threshold", 
							"herbivore_type=\"cohort\""));

			if (params.herbivore_type == HT_INDIVIDUAL)
				mandatory_global_params.push_back(MandatoryParam(
							"habitat_area_km2",
							"herbivore_type=\"individual\""));

			// check the list
			if (!check_mandatory(mandatory_global_params,
						"Global herbivory settings"))
				plibabort();

		}

		// Finish HFT list
		hftlist.remove_excluded();

		if (params.ifherbivory && hftlist.size()==0) {
			dprintf("Warning: ifherbivory is true, but "
					"no HFTs were included. The herbivory output "
					"module will be active, but no herbivory simulation "
					"will be done.\n");
		}

		{
			// Check again the parameters of the herbivory module.
			bool fatal;
			std::string msg;

#ifdef NO_GUESS_PARAMETERS
			// Check parameters for test simulation independent of LPJ-GUESS.
			// Here we disregard the PFTs.
			check_all_params(fatal, msg);
#else
			// Check parameters including the PFTs of LPJ-GUESS.
			check_all_params(pftlist, fatal, msg);
#endif // NO_GUESS_PARAMETERS

			if (fatal){
				sendmessage("Error", std::string(
							"Parameters of the herbivory module are not valid:\n"
							+ msg).c_str());
				plibabort();
			} else
				if (!msg.empty())
					sendmessage("Warning", std::string(
								"Warning messages in the herbivory module:\n"
								+ msg).c_str());
		}

		// This is the very end of the plib checks
		completed = true;
	} 

	if (callback == CB_DIG_MODEL) {
		if (strparam == "PFT_FIXED")
			params.digestibility_model = DM_PFT_FIXED;
		if (strparam == "PACHZELT_2013")
			params.digestibility_model = DM_PFT_PACHZELT2013;
		// add other digestibility models here
		else {
			sendmessage("Error",
					"Unknown digestibility model; valid types: "
					"\"pft_fixed\"");
			plibabort();
		}
	} 

	if (callback == CB_DIET_COMPOSER) {
		if (strparam == "PURE_GRAZER")
			current_hft.diet_composer = DC_PURE_GRAZER;
		else {
			sendmessage("Error", std::string(
					"Unknown value for parameter \"diet_composer\" "
					"in HFT \""+current_hft.name+"\"; valid types: "
					"\"pure_grazer\"").c_str());
			plibabort();
		} 
	}

	if (callback == CB_DIGESTION_TYPE) {
		if (strparam == "RUMINANT")
			current_hft.digestion_type = DT_RUMINANT;
		else if (strparam == "HINDGUT")
			current_hft.digestion_type = DT_HINDGUT;
		else {
			sendmessage("Error", std::string(
					"Unknown value for parameter \"digestion_type\" "
					"in HFT \""+current_hft.name+"\"; valid types: "
					"\"ruminant\", \"hindgut\"").c_str());
			plibabort();
		} 
	}

	if (callback == CB_EXPENDITURE_MODEL) {
		if (strparam == "TAYLOR_1981")
			current_hft.expenditure_model = EM_TAYLOR_1981;
		// add other models here
		else {
			sendmessage("Error", std::string(
					"Unknown expenditure model"
					"in HFT \""+current_hft.name+"\"; valid types: "
					"\"taylor_1981\"").c_str()); // add more here
			plibabort();
		}
	} 

	if (callback == CB_FORAGE_DISTRIBUTION) {
		if (strparam == "EQUALLY")
			params.forage_distribution = FD_EQUALLY;
		else {
			sendmessage("Error", std::string(
					"Unknown value for parameter \"forage_distribution\" "
					"Possible values: "
					"\"equally\"").c_str());
			plibabort();
		}
	}

	if (callback == CB_FORAGING_LIMITS) {
		const std::list<std::string> token_list = 
			parse_comma_separated_param(strparam);
				
		std::list<std::string>::const_iterator itr;
		for (itr = token_list.begin(); itr != token_list.end(); itr++){
			if (*itr == "DIGESTION_ILLIUS_1992") 
				current_hft.foraging_limits.insert(FL_DIGESTION_ILLIUS_1992);
			else if (*itr == "ILLIUS_OCONNOR_2000") 
				current_hft.foraging_limits.insert(FL_ILLIUS_OCONNOR_2000);
			// add new foraging limits here
			else {
				sendmessage("Error", std::string(
							"Unknown foraging limit: \""
							+*itr+"\". "
							"Valid types: "
							"\"digestion_illius_1992\", \"illius_oconnor_2000\"").c_str());
				plibabort();
			} 
		}
	}

	if (callback == CB_FORAGE_TYPE) {
		if (strparam == to_upper(get_forage_type_name(FT_INEDIBLE)))
			ppft->herbiv_params.forage_type = FT_INEDIBLE;
		else if (strparam == to_upper(get_forage_type_name(FT_GRASS))) 
			ppft->herbiv_params.forage_type = FT_GRASS;
		else {
			sendmessage("Error",
					"Unknown forage type; valid types: "
					"\"inedible\", \"grass\"");
			plibabort();
		}
	}

	if (callback == CB_HERBIVORE_TYPE) {
		if (strparam == "COHORT")
			params.herbivore_type = HT_COHORT;
		else if (strparam == "INDIVIDUAL")
			params.herbivore_type = HT_INDIVIDUAL;
		else {
			sendmessage("Error",
					"Unknown value for herbivore_type; valid types: "
					"\"cohort\", \"individual\"");
			plibabort();
		}
	}

	if (callback == CB_MORTALITY_FACTORS) {
		const std::list<std::string> token_list = 
			parse_comma_separated_param(strparam);
				
		std::list<std::string>::const_iterator itr;
		for (itr = token_list.begin(); itr != token_list.end(); itr++){
			if (*itr == "BACKGROUND") 
				current_hft.mortality_factors.insert(MF_BACKGROUND);
			else if (*itr == "LIFESPAN") 
				current_hft.mortality_factors.insert(MF_LIFESPAN);
			else if (*itr == "STARVATION_ILLIUS_2000") 
				current_hft.mortality_factors.insert(MF_STARVATION_ILLIUS2000);
			else if (*itr == "STARVATION_THRESHOLD") 
				current_hft.mortality_factors.insert(MF_STARVATION_THRESHOLD);
			// add new mortality factors here
			else {
				sendmessage("Error", std::string(
							"Unknown mortality factor: \""
							+*itr+"\". "
							"Valid types: "
							"\"background\", "
							"\"lifespan\", "
							"\"starvation_illius_2000\", "
							"\"starvation_threshold\"").c_str());
				plibabort();
			} 
		}
	}

	if (callback == CB_NET_ENERGY_MODEL) {
		if (strparam == "DEFAULT")
			current_hft.net_energy_model = NE_DEFAULT;
		else {
			sendmessage("Error", std::string(
					"Unknown value for net_energy_model "
					"in HFT \""+current_hft.name+"\"; valid types: "
					"\"default\"").c_str());
			plibabort();
		}
	}

	if (callback == CB_PFT) {
		if (ppft == NULL)
			throw std::invalid_argument("Fauna::ParamReader::callback() "
					"PFT pointer ppft is NULL. Error in parameters.h?");
		const Pft& pft = *ppft;

		if (params.ifherbivory && pft.herbiv_params.is_edible()) {

			MandatoryParamList mandatory_pft_params;

			// add parameters to the list
			
			if (params.digestibility_model == DM_PFT_FIXED) {
				mandatory_pft_params.push_back(MandatoryParam(
							"digestibility",
							"digestibility_model=PFT_FIXED"));
			}

			// check through the list
			if (!check_mandatory(mandatory_pft_params,
						"PFT \""+ std::string(pft.name) +"\""))
				plibabort();

		}
	}

	if (callback == CB_REPRODUCTION_MODEL) {
		if (strparam == "ILLIUS_OCONNOR_2000")
			current_hft.reproduction_model = RM_ILLIUS_OCONNOR_2000;
		else if (strparam == "CONST_MAX")
			current_hft.reproduction_model = RM_CONST_MAX;
		// Add new reproduction models here.
		else {
			sendmessage("Error", std::string(
					"Unknown value for reproduction_model "
					"in HFT \""+current_hft.name+"\"; valid types: "
					"\"ILLIUS_OCONNOR_2000\", \"const_max\"").c_str());
			plibabort();
		}
	}
}

void ParamReader::declare_parameters(
		const int id, 
		const std::string& setname,
		Pft* ppft,
		const bool is_help)
{
	// (We use if-else instead of switch here, because the static
	// members are initialized in parameters.cpp and cannot be
	// used in as constant values in a switch-case statement.)
	
	// add new parameters in alphabetical order
	
	if (id == BLOCK_GLOBAL) {

		declareitem("dead_herbivore_threshold",
				&(params.dead_herbivore_threshold),
				0.0, DBL_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Minimum density [ind/km²] for a living herbivore cohort.");

#ifndef NO_GUESS_PARAMETERS
		// Digestibility model only applies to LPJ-GUESS runs.
		declareitem("digestibility_model",
				&strparam,
				128, // max length of string
				CB_DIG_MODEL,
				"Digestibility model for herbivore forage. "
				"Possible values: "
				"\"pft_fixed\", \"pachzelt_2013\"");
#endif

		declareitem("forage_distribution",
				&strparam,
				128, // max length of string
				CB_FORAGE_DISTRIBUTION,
				"Algorithm for how to distribute available forage among "
				"herbivores. Possible values: "
				"\"equally\"");

		declareitem("free_herbivory_years",
				&(params.free_herbivory_years),
				0, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Number of years without herbivory, as part of vegetation spinup.");

		declareitem("habitat_area_km2",
				&(params.habitat_area_km2),
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Area size of one patch [km²] for herbivore individual mode.");

		declareitem("herbivore_establish_interval",
				&(params.herbivore_establish_interval),
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Number of days between herbivore establishment.");

		declareitem("herbivore_type",
				&strparam,
				128, // max length of string
				CB_HERBIVORE_TYPE,
				"Which kind of herbivore class to use."
				"Possible values: "
				"\"cohort\", \"individual\"");

		declareitem("hft",
				BLOCK_HFT,
				CB_NONE,
				"Header for block defining HFT (Herbivore Functional Type)");

		declareitem("ifherbivory", 
				&(params.ifherbivory),
				1, // number of parameters
				CB_NONE, 
				"Whether herbivory is enabled");

	} 

	if (id == BLOCK_HFT) {
		if (!is_help) {
			// Get existing Hft object or create new one
			if (hftlist.contains(setname))
				current_hft = hftlist[setname]; // copy object
			else {
				Hft new_hft;// create new object
				new_hft.name=setname;
				current_hft = new_hft; 
			}
		}

		declareitem("include",
				&current_hft.is_included,
				1, // number of parameters
				CB_NONE,
				"Include HFT in simulation.");

		// ------------------------ 
		// alphabetical order now
		
		declareitem("bodyfat_birth",
				&current_hft.bodyfat_birth,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Body mass [kg] at birth for both sexes.");

		declareitem("bodyfat_max",
				&current_hft.bodyfat_max,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Maximum proportional fat mass [kg/kg].");

		declareitem("bodymass_birth",
				&current_hft.bodymass_birth,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Body mass [kg] at birth for both sexes.");

		declareitem("bodymass_female",
				&current_hft.bodymass_female,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Body mass [kg] of an adult female individual.");

		declareitem("bodymass_male",
				&current_hft.bodymass_male,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Body mass [kg] of an adult male individual.");

		declareitem("breeding_season_length",
				&current_hft.breeding_season_length,
				0, 365, // min, max
				1,      // number of parameters
				CB_NONE,
				"Length of breeding season in days.");

		declareitem("breeding_season_start",
				&current_hft.breeding_season_start,
				0, 364, // min, max
				1,      // number of parameters
				CB_NONE,
				"First day of breeding season (0=Jan 1st).");

		declareitem("diet_composer",
				&strparam,
				64,
				CB_DIET_COMPOSER,
				"Herbivore diet: \"pure_grazer\"");

		declareitem("digestion_type",
				&strparam,
				64,
				CB_DIGESTION_TYPE,
				"Digestion type: \"ruminant\", \"hindgut\"");

		declareitem("establishment_density",
				&current_hft.establishment_density,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Habitat population density for initial establishment [ind/km²].");

		declareitem("expenditure_model",
				&strparam,
				128, // max length of string
				CB_EXPENDITURE_MODEL,
				"Energy expenditure model for herbivores."
				"Possible values: "
				"\"taylor_1981\", "); 

		declareitem("foraging_limits",
				&strparam,
				256, // max length of string
				CB_FORAGING_LIMITS,
				"Comma-separated list of constraints of herbivore forage intake. "
				"Possible values: "
				"\"digestion_illius_1992\", \"illius_oconnor_2000\"");

		declareitem("half_max_intake_density",
				&current_hft.half_max_intake_density,
				0.0, DBL_MAX, // min, max
				1,            // number of parameters
				CB_NONE,
				"Grass density [gDM/m²] where intake rate is half of its maximum.");

		declareitem("lifespan",
				&current_hft.lifespan,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Maximum age in years [1–∞).");

		declareitem("maturity_age_phys_female",
				&current_hft.maturity_age_phys_female,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Age of physical maturity [years] of females.");

		declareitem("maturity_age_phys_male",
				&current_hft.maturity_age_phys_male,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Age of physical maturity [years] of males.");

		declareitem("maturity_age_sex",
				&current_hft.maturity_age_sex,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Age of female sexual maturity in years.");

		declareitem("mortality",
				&current_hft.mortality,
				0.0, 1.0-DBL_MIN, // min, max
				1,                // number of parameters
				CB_NONE,
				"Annual mortality rate [0.0–1.0) after first year of life.");

		declareitem("mortality_factors",
				&strparam,
				256, // max length of string
				CB_MORTALITY_FACTORS,
				"Comma-separated list of death causes for herbivores. "
				"Possible values: "
				"\"background\", "
				"\"lifespan\", "
				"\"starvation_threshold\", "
				"\"starvation_illius_2000\" (only for cohorts)");  

		declareitem("mortality_juvenile",
				&current_hft.mortality_juvenile,
				0.0, 1.0-DBL_MIN, // min, max
				1,                // number of parameters
				CB_NONE,
				"Annual mortality rate [0.0–1.0) in the first year of life.");

		declareitem("net_energy_model",
				&strparam,
				128, // max length of string
				CB_NET_ENERGY_MODEL,
				"Model to calculate net energy content of herbivore forage. "
				"Possible values: "
				"\"defualt\"");

		declareitem("reproduction_max",
				&current_hft.reproduction_max,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Average number of offspring per year produced by one "
				"female individual under optimal nutritional conditions.");

		declareitem("reproduction_model",
				&strparam,
				256, // max length of string
				CB_REPRODUCTION_MODEL,
				"Reproduction model for the HFT."
				"Possible values: \"illius_oconnor_2000\", \"const_max\"");  

		// let plib call function plib_callback() with given code
		callwhendone(CB_CHECKHFT);
	}

	if (id == BLOCK_PFT) {

		if (!is_help && ppft==NULL)
			throw std::invalid_argument("Fauna::ParamReader::declare_parameters() "
					"PFT pointer ppft is NULL. Error in parameters.h?");

		declareitem("c_in_dm_forage",
				&ppft->herbiv_params.c_in_dm_forage,
				DBL_MIN, 1.0, // min, max
				1,            // number of parameters
				CB_NONE,
				"Herbivory: Fractional carbon content in dry matter forage.");

		declareitem("digestibility",
				&ppft->herbiv_params.digestibility,
				DBL_MIN, 1.0, // min, max
				1,            // number of parameters
				CB_NONE,
				"Herbivory: Fractional digestibility of herbivore forage for ruminants");

		declareitem("forage_type",
				&strparam,
				64, // max length of string
				CB_FORAGE_TYPE,
				"Forage Type (\"inedible\",\"grass\")"); // Add more forage types here

	} 
}

