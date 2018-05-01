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
				 msg_stream << "HFT \"" << hft_itr->name << "\": " << tmp_msg;
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

	if (callback == CB_CONDUCTANCE) {
		if (strparam == "BRADLEY_DEAVERS_1980")
			current_hft.conductance = CM_BRADLEY_DEAVERS_1980;
		if (strparam == "CUYLER_OERITSLAND_2004")
			current_hft.conductance = CM_CUYLER_OERITSLAND_2004;
		// add other conductance models here
		else {
			sendmessage("Error",
					"Unknown conductance model; valid types: "
					"\"bradley_deavers_1980\", \"cuyler_oeritsland_2004\"");
			plibabort();
		}
	}
	
	if (callback == CB_CHECKHFT) {

    // First check for `include` parameter
    if (!itemparsed("include")){
      sendmessage("Error", std::string(
            "Parameter `include` is missing in HFT " 
            + current_hft.name + ".").c_str());
      plibabort();
    }

		// We need to know which kind of herbivore we are parametrizing.
		// (Note that itemparsed() doesn’t work here because of the local 
		//  scope within the HFT parameters block.)
		if (!herbivore_type_defined){
			sendmessage("Error",
					"Global parameter `herbivore_type` must be defined before "
					"any HFTs can be parametrized.");
			plibabort();
		}

    if (current_hft.is_included) {
      // compile and check mandatory parameters
      MandatoryParamList mandatory_hft_params;

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
              "digestive_limit", req_str));
        mandatory_hft_params.push_back(MandatoryParam(
              "establishment_age_range", req_str));
        mandatory_hft_params.push_back(MandatoryParam(
              "establishment_density", req_str));
        mandatory_hft_params.push_back(MandatoryParam(
              "expenditure_components", req_str));
        mandatory_hft_params.push_back(MandatoryParam(
              "gestation_months", req_str));
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
        if (current_hft.digestive_limit == DL_ALLOMETRIC){
          mandatory_hft_params.push_back(MandatoryParam(
                "digestive_limit_allometry", req_str +
                " and \"allometric\" is digestive limit."));
        }
        if (current_hft.digestive_limit == DL_FIXED_FRACTION){
          mandatory_hft_params.push_back(MandatoryParam(
                "digestive_limit_fixed", req_str +
                " and \"fixed_fraction\" is digestive limit."));
        }
        if (current_hft.expenditure_components.count(EC_ALLOMETRIC)) {
          mandatory_hft_params.push_back(MandatoryParam(
                "expenditure_allometry", req_str +
                " and \"allometric\" is an expenditure component."));
        }
        if (current_hft.expenditure_components.count(EC_THERMOREGULATION)) {
          mandatory_hft_params.push_back(MandatoryParam(
                "conductance", req_str +
                " and \"thermoregulation\" is an expenditure component."));
        }
        if (current_hft.foraging_limits.count(FL_ILLIUS_OCONNOR_2000)){
          mandatory_hft_params.push_back(MandatoryParam(
                "half_max_intake_density", req_str +
                " and \"illius_oconnor_2000\" in foraging_limits"));
        }
        if (current_hft.mortality_factors.count(MF_STARVATION_ILLIUS_OCONNOR_2000)){
          mandatory_hft_params.push_back(MandatoryParam(
                "bodyfat_deviation", req_str +
                " and \"starvation_illius_oconnor_2000\" in mortality_factors"));
        }
        if (current_hft.reproduction_model == RM_ILLIUS_OCONNOR_2000 ||
            current_hft.reproduction_model == RM_CONST_MAX ||
            current_hft.reproduction_model == RM_LINEAR)
        {
          mandatory_hft_params.push_back(MandatoryParam(
                "breeding_season_length", req_str +
                " and reproduction_model=illius_oconnor_2000|const_max|linear"));
          mandatory_hft_params.push_back(MandatoryParam(
                "breeding_season_start", req_str +
                " and reproduction_model=illius_oconnor_2000|const_max|linear"));
          mandatory_hft_params.push_back(MandatoryParam(
                "reproduction_max", req_str +
                " and reproduction_model=illius_oconnor_2000|const_max|linear"));
        }
      }

			// Set `dead_herbivore_threshold` to default value.
			if (!itemparsed("dead_herbivore_threshold")){
				current_hft.dead_herbivore_threshold = 
					current_hft.get_max_dead_herbivore_threshold() * 0.9;
				std::ostringstream msg_stream;
				msg_stream << "Setting `dead_herbivore_threshold` to 0.9 of "
					<< "maximum value: " << current_hft.dead_herbivore_threshold
					<< " ind/km²";
				sendmessage("Info", msg_stream.str().c_str());
			}

      if (!check_mandatory(mandatory_hft_params,
            "HFT \""+current_hft.name+"\""))
        plibabort();

      // Now everything seems okay, and we can add the HFT.
      hftlist.insert(current_hft);
    }
	}

	if (callback == CB_CHECKGLOBAL) {

		if (!itemparsed("ifherbivory"))
			dprintf("Notice: ifherbivory was not declared. "
					"It is disabled by default.\n");

		if (params.ifherbivory) {
			MandatoryParamList mandatory_global_params;

			// Add mandatory parameters 
#ifndef NO_GUESS_PARAMETERS
			// The following parameters only apply to LPJ-GUESS.
			mandatory_global_params.push_back(MandatoryParam(
						"digestibility_model"));
			mandatory_global_params.push_back(MandatoryParam(
						"snow_depth_model"));
#endif
			mandatory_global_params.push_back(MandatoryParam(
						"forage_distribution"));
			mandatory_global_params.push_back(MandatoryParam(
						"free_herbivory_years"));
			mandatory_global_params.push_back(MandatoryParam(
						"herbivore_establish_interval"));
			mandatory_global_params.push_back(MandatoryParam(
						"herbivore_type"));

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
								"Messages in the herbivory module:\n"
								+ msg).c_str());
		}

		// This is the very end of the plib checks
		completed = true;
	} 

	if (callback == CB_DIG_MODEL) {
		if (strparam == "PFT_FIXED")
			params.digestibility_model = DM_PFT_FIXED;
		else if (strparam == "NPP")
			params.digestibility_model = DM_NPP;
		else if (strparam == "PACHZELT_2013")
			params.digestibility_model = DM_PFT_PACHZELT2013;
		// add other digestibility models here
		else {
			sendmessage("Error",
					"Unknown digestibility model; valid types: "
					"\"npp\", \"pachzelt_2013\", \"pft_fixed\"");
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

	if (callback == CB_DIGESTION_LIMIT_ALLOMETRY) {
		current_hft.digestive_limit_allometry.coefficient = double_pair[0];
		current_hft.digestive_limit_allometry.exponent    = double_pair[1];
	}

	if (callback == CB_DIGESTIVE_LIMIT) {
		if (strparam == "NONE")
			current_hft.digestive_limit = DL_NONE;
		else if (strparam == "ALLOMETRIC")
			current_hft.digestive_limit = DL_ALLOMETRIC;
		else if (strparam == "FIXED_FRACTION")
			current_hft.digestive_limit = DL_FIXED_FRACTION;
		else if (strparam == "ILLIUS_GORDON_1992")
			current_hft.digestive_limit = DL_ILLIUS_GORDON_1992;
		// add new digestive limits here
		else {
			sendmessage("Error", std::string(
						"Unknown digestive limit: \""
						+strparam+"\". "
						"Valid types: "
						"\"none\", \"allometric\", \"illius_gordon_1992\"").c_str());
			plibabort();
		} 
	}

	if (callback == CB_ESTABLISHMENT_AGE_RANGE) {
		current_hft.establishment_age_range.first  = integer_pair[0];
		current_hft.establishment_age_range.second = integer_pair[1];
	}

	if (callback == CB_EXPENDITURE_ALLOMETRY) {
		current_hft.expenditure_allometry.coefficient = double_pair[0];
		current_hft.expenditure_allometry.exponent    = double_pair[1];
	}

	if (callback == CB_EXPENDITURE_COMPONENTS) {
		const std::list<std::string> token_list = 
			parse_comma_separated_param(strparam);

		std::list<std::string>::const_iterator itr;
		for (itr = token_list.begin(); itr != token_list.end(); itr++){
			if (*itr == "TAYLOR_1981") 
				current_hft.expenditure_components.insert(EC_TAYLOR_1981);
			else if (*itr == "ALLOMETRIC") 
				current_hft.expenditure_components.insert(EC_ALLOMETRIC);
			else if (*itr == "THERMOREGULATION") 
				current_hft.expenditure_components.insert(EC_THERMOREGULATION);
			else if (*itr == "ZHU_2018") 
				current_hft.expenditure_components.insert(EC_ZHU_2018);
			// add new expenditure components here
			else {
				sendmessage("Error", std::string(
							"Unknown expenditure component: \""
							+*itr+"\". "
							"Valid types: "
							"\"allometric\", \"taylor_1981\", "
							"\"thermoregulation\", \"zhu_2018\"").c_str()); // add more here
				plibabort();
			} 
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
			if (*itr == "ILLIUS_OCONNOR_2000") 
				current_hft.foraging_limits.insert(FL_ILLIUS_OCONNOR_2000);
			// add new foraging limits here
			else {
				sendmessage("Error", std::string(
							"Unknown foraging limit: \""
							+*itr+"\". "
							"Valid types: "
							"\"illius_oconnor_2000\"").c_str());
				plibabort();
			} 
		}
	}

	if (callback == CB_FORAGE_TYPE) {
		if (strparam == to_upper(get_forage_type_name(FT_INEDIBLE)))
			ppft->herbiv_params.forage_type = FT_INEDIBLE;
		else if (strparam == to_upper(get_forage_type_name(FT_GRASS))) 
			ppft->herbiv_params.forage_type = FT_GRASS;
		// ** add new forage types here **
		else {
			sendmessage("Error", std::string(
					"Unknown forage type: \"" + strparam + "\". "
					"Valid types: "
					"\"inedible\", \"grass\"").c_str());
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
		herbivore_type_defined = true;
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
			else if (*itr == "STARVATION_ILLIUS_OCONNOR_2000") 
				current_hft.mortality_factors.insert(MF_STARVATION_ILLIUS_OCONNOR_2000);
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
							"\"starvation_illius_oconnor_2000\", "
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
			
			if (params.digestibility_model == DM_PFT_FIXED ||
					params.digestibility_model == DM_NPP) {
				mandatory_pft_params.push_back(MandatoryParam(
							"digestibility",
							"digestibility_model=PFT_FIXED|NPP"));
			}

			if (params.digestibility_model == DM_NPP) {
				mandatory_pft_params.push_back(MandatoryParam(
							"digestibility_dead",
							"digestibility_model=NPP"));
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
		else if (strparam == "LINEAR")
			current_hft.reproduction_model = RM_LINEAR;
		// Add new reproduction models here.
		else {
			sendmessage("Error", std::string(
					"Unknown value for `reproduction_model` "
					"in HFT \""+current_hft.name+"\"; valid types: "
					"\"illius_oconnor_2000\", \"const_max\", \"linear\"").c_str());
			plibabort();
		}
	}

	if (callback == CB_SNOW_DEPTH_MODEL) {
		if (strparam == "10_TO_1")
			params.snow_depth_model = SD_TEN_TO_ONE;
		else {
			sendmessage("Error", std::string(
					"Unknown value for `snow_depth_model` "
					"in HFT \""+current_hft.name+"\"; valid types: "
					"\"10_to_1\"").c_str());
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

#ifndef NO_GUESS_PARAMETERS
		// The following models are only needed in LPJ-GUESS.
		declareitem("digestibility_model",
				&strparam,
				128, // max length of string
				CB_DIG_MODEL,
				"Digestibility model for herbivore forage. "
				"Possible values: "
				"\"npp\", \"pft_fixed\", \"pachzelt_2013\"");
		declareitem("snow_depth_model",
				&strparam,
				128, // max length of string
				CB_SNOW_DEPTH_MODEL,
				"Algorithm to convert snow water equivalent to snow depth. "
				"Possible values: "
				"\"10_to_1\"");
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
				0, INT_MAX, // min, max
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
		herbivore_type_defined = false;

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
				current_hft.expenditure_components.clear();
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

		declareitem("bodyfat_deviation",
				&current_hft.bodyfat_deviation,
				0.0, 1.0, // min, max
				1,                // number of parameters
				CB_NONE,
				"Standard deviation of body condition within a herbivore cohort/population.");

		declareitem("bodyfat_max",
				&current_hft.bodyfat_max,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Maximum proportional fat mass [kg/kg].");

		declareitem("bodyfat_max_daily_gain",
				&current_hft.bodyfat_max_daily_gain,
				0, 1.0, // min, max
				1,          // number of parameters
				CB_NONE,
				"Maximum body fat gain rate [kg/kg/day].");

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

		declareitem("conductance",
				&strparam,
				64,
				CB_CONDUCTANCE,
				"Conductance model for thermoregulation: "
				"\"bradley_deavers_1980\", \"cuyler_oeritsland_2004\"");

		declareitem("core_temperature",
				&current_hft.core_temperature,
				DBL_MIN, DBL_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Body core temperature [°C].");

		declareitem("dead_herbivore_threshold",
				&(current_hft.dead_herbivore_threshold),
				0.0, DBL_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Minimum density [ind/km²] for a living herbivore cohort. "
				"Default: 0.9 of maximum value");

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

		declareitem("digestive_limit",
				&strparam,
				256, // max length of string
				CB_DIGESTIVE_LIMIT,
				"Digestive constraint for daily herbivore food intake."
				"Possible values: "
				"\"none\", \"allometric\", \"fixed_fraction\", "
				"\"illius_gordon_1992\"");

		declareitem("establishment_age_range",
				integer_pair,
				0, INT_MAX, // min, max
				2,          // number of parameters
				CB_ESTABLISHMENT_AGE_RANGE,
				"Youngest and oldest age [years] for herbivore establishment.");

		declareitem("establishment_density",
				&current_hft.establishment_density,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Habitat population density for initial establishment [ind/km²].");

		declareitem("expenditure_allometry",
				double_pair,
				0.0, DBL_MAX, // min, max
				2,            // number of parameters
				CB_EXPENDITURE_ALLOMETRY,
				"Coefficient and exponent for allometric expenditure component.");

		declareitem("expenditure_components",
				&strparam,
				128, // max length of string
				CB_EXPENDITURE_COMPONENTS,
				"Comma-separated list of energy expenditure model for herbivores."
				"Possible values: "
				"\"taylor_1981\", \"allometric\", \"thermoregulation\", \"zhu_2018\""); 

		declareitem("digestive_limit_allometry",
				double_pair,
				-DBL_MAX, DBL_MAX, // min, max
				2,            // number of parameters
				CB_DIGESTION_LIMIT_ALLOMETRY,
				"Allometric coefficient and exponent for digestive limit \"allometric\".");

		declareitem("digestive_limit_fixed",
				&current_hft.digestive_limit_fixed,
				DBL_MIN, 1.0, // min, max
				1,            // number of parameters
				CB_NONE,
				"Digestive limit as body mass fraction for digestive limit \"fixed_fraction\".");

		declareitem("foraging_limits",
				&strparam,
				256, // max length of string
				CB_FORAGING_LIMITS,
				"Comma-separated list of constraints of herbivore forage intake. "
				"Possible values: "
				"\"illius_oconnor_2000\"");

		declareitem("gestation_months",
				&current_hft.gestation_months,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Duration of pregnancy in months.");

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
				"\"starvation_illius_oconnor_2000\" (only for cohorts)");  

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
				"Possible values: \"illius_oconnor_2000\", \"const_max\", \"linear\"");  

		declareitem("shift_body_condition_for_starvation",
				&(current_hft.shift_body_condition_for_starvation),
				1, // number of parameters
				CB_NONE,
				"Whether to shift mean cohort body condition with starvation "
				"mortality. Defaults to true.");  

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
				"Herbivory: Fractional digestibility of herbivore forage (constant or fresh).");

		declareitem("digestibility_dead",
				&ppft->herbiv_params.digestibility_dead,
				DBL_MIN, 1.0, // min, max
				1,            // number of parameters
				CB_NONE,
				"Herbivory: Fractional digestibility of dead herbivore forage.");

		declareitem("inaccessible_forage",
				&ppft->herbiv_params.inaccessible_forage,
				0.0, DBL_MAX, // min, max
				1,            // number of parameters
				CB_NONE,
				"Herbivory: Carbon mass of edible plant compartment that is "
				"inaccessible for herbivores [kgC/m²].");

		declareitem("forage_type",
				&strparam,
				64, // max length of string
				CB_FORAGE_TYPE,
				"Forage Type (\"inedible\",\"grass\")"); // Add more forage types here

	} 
}

