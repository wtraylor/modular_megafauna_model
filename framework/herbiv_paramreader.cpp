////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Reading instruction file parameters for the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date July 2017
////////////////////////////////////////////////////////////

#include "config.h"
#include "herbiv_paramreader.h"
#include "guess.h"      // for Pft
#include "parameters.h" // read instruction file
#include "plib.h"       // read instruction file
#include "shell.h"      // for dprintf()
#include <algorithm>    // for std::transform
#include <cfloat>       // for DBL_MAX, DBL_MIN
#include <climits>      // for INT_MAX
#include <sstream>       // for ostringstream


using namespace Fauna;
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

void ParamReader::callback(const int callback, Pft* ppft){

	// Turn the string parameter to upper case to make
	// comparison case-insensitive.
	std::transform(strparam.begin(), strparam.end(), 
			strparam.begin(), ::toupper);

	// add checkback codes in alphabetical order
	
	if (callback == CB_CHECKHFT) {

		if (params.ifherbivory) {
			// compile and check mandatory parameters
			MandatoryParamList mandatory_hft_params;

			mandatory_hft_params.push_back(MandatoryParam("include",""));

			if (params.herbivore_type == HT_INDIVIDUAL ||
					params.herbivore_type == HT_COHORT) {
				mandatory_hft_params.push_back(MandatoryParam(
							"bodymass_female",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"bodymass_male",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"digestion_type",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"establishment_density",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"lifespan",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"maturity",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"mortality",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"mortality_juvenile",
							"herbivore_type=(cohort|individual)"));
				mandatory_hft_params.push_back(MandatoryParam(
							"reproduction_max",
							"herbivore_type=(cohort|individual)"));
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
			mandatory_global_params.push_back(MandatoryParam(
						"dead_herbivore_threshold", ""));
			mandatory_global_params.push_back(MandatoryParam(
						"digestibility_model", ""));
			mandatory_global_params.push_back(MandatoryParam(
						"forage_distribution", ""));
			mandatory_global_params.push_back(MandatoryParam(
						"free_herbivory_years", ""));
			mandatory_global_params.push_back(MandatoryParam(
						"herbivore_type", ""));

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
	} 

	if (callback == CB_DIG_MODEL) {
		if (strparam == "PFT_FIXED")
			params.dig_model = DM_PFT_FIXED;
		// add other digestibility models here
		else {
			sendmessage("Error",
					"Unknown digestibility model; valid types: "
					"\"pft_fixed\"");
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

	if (callback == CB_FORAGE_TYPE) {
		// TODO use get_forage_type_name()
		if (strparam == "INEDIBLE") 
			ppft->herbiv_params.forage_type = Fauna::FT_INEDIBLE;
		else if (strparam == "GRASS") 
			ppft->herbiv_params.forage_type = Fauna::FT_GRASS;
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

	if (callback == CB_PFT) {
		assert (ppft != NULL);
		const Pft& pft = *ppft;

		if (params.ifherbivory && pft.herbiv_params.is_edible()) {

			MandatoryParamList mandatory_pft_params;

			// add parameters to the list
			
			if (params.dig_model == DM_PFT_FIXED) {
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
				"Minimum mass density [kg/km²] for a living herbivore object.");

		declareitem("digestibility_model",
				&strparam,
				128, // max length of string
				CB_DIG_MODEL,
				"Digestibility model for herbivore forage. "
				"Possible values: "
				"\"pft_fixed\"");

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
				"Habitat population mass density for initial establishment [kg/km²].");

		declareitem("lifespan",
				&current_hft.lifespan,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Maximum age in years [1–∞).");

		declareitem("maturity",
				&current_hft.maturity,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Age of physical and sexual (female) maturity in years.");

		declareitem("mortality",
				&current_hft.mortality,
				0.0, 1.0-DBL_MIN, // min, max
				1,                // number of parameters
				CB_NONE,
				"Annual mortality rate [0.0–1.0) after first year of life.");

		declareitem("mortality_juvenile",
				&current_hft.mortality_juvenile,
				0.0, 1.0-DBL_MIN, // min, max
				1,                // number of parameters
				CB_NONE,
				"Annual mortality rate [0.0–1.0) in the first year of life.");

		declareitem("reproduction_max",
				&current_hft.reproduction_max,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Average number of offspring per year produced by one "
				"female individual under optimal nutritional conditions.");

		// let plib call function plib_callback() with given code
		callwhendone(CB_CHECKHFT);
	}

	if (id == BLOCK_PFT) {

		if (!is_help)
			assert(ppft != NULL);

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

