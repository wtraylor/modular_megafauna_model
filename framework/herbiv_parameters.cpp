////////////////////////////////////////////////////////////
/// \file
/// \ingroup group_herbivory
/// \brief Instruction file parameters of the herbivory module.
/// \author Wolfgang Pappa, Senckenberg BiK-F
/// \date June 2017
////////////////////////////////////////////////////////////

#include "config.h"
#include "plib.h"              // read instruction file
#include "guess.h"             // for Pft
#include "herbiv_parameters.h" // read global parameters
#include "herbiv_output.h"     // for HerbivoryOutput::deactivate()
#include "herbiv_hft.h"        // for Hft

#include <climits>   // for INT_MAX
#include <cfloat>    // for DBL_MAX, DBL_MIN
#include <algorithm> // for std::transform


using namespace Fauna;

// initialization of static member variables
std::string Parameters::strparam      = std::string();
std::vector<std::string> Parameters::mandatory_hft_params;
Hft Parameters::current_hft = Hft();

void Parameters::declare_parameters(
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
				&(get_global().dead_herbivore_threshold),
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
				&(get_global().free_herbivory_years),
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
				&(get_global().ifherbivory),
				1, // number of parameters
				CB_NONE, 
				"Whether herbivory is enabled");

	} 

	if (id == BLOCK_HFT) {
		HftList& hftlist = HftList::get_global();

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
		mandatory_hft_params.push_back("include");

		// ------------------------
		
		declareitem("bodymass_female",
				&current_hft.bodymass_female,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Body mass [kg] of an adult female individual.");
		mandatory_hft_params.push_back("bodymass_female");

		declareitem("bodymass_male",
				&current_hft.bodymass_male,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Body mass [kg] of an adult male individual.");
		mandatory_hft_params.push_back("bodymass_male");

		declareitem("digestion_type",
				&strparam,
				64,
				CB_DIGESTION_TYPE,
				"Digestion type: \"ruminant\", \"hindgut\"");
		mandatory_hft_params.push_back("digestion_type");

		declareitem("lifespan",
				&current_hft.lifespan,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Maximum age in years [1–∞).");
		mandatory_hft_params.push_back("lifespan");

		declareitem("maturity",
				&current_hft.maturity,
				1, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Age of physical and sexual (female) maturity in years.");
		mandatory_hft_params.push_back("maturity");

		declareitem("mortality",
				&current_hft.mortality,
				0.0, 1.0-DBL_MIN, // min, max
				1,                // number of parameters
				CB_NONE,
				"Annual mortality rate [0.0–1.0) after first year of life.");
		mandatory_hft_params.push_back("mortality");

		declareitem("mortality_juvenile",
				&current_hft.mortality_juvenile,
				0.0, 1.0-DBL_MIN, // min, max
				1,                // number of parameters
				CB_NONE,
				"Annual mortality rate [0.0–1.0) in the first year of life.");
		mandatory_hft_params.push_back("mortality_juvenile");

		declareitem("reproduction_max",
				&current_hft.reproduction_max,
				DBL_MIN, DBL_MAX, // min, max
				1,                // number of parameters
				CB_NONE,
				"Average number of offspring per year produced by one "
				"female individual under optimal nutritional conditions.");
		mandatory_hft_params.push_back("reproduction_max");

		// let plib call function plib_callback() with given code
		callwhendone(CB_CHECKHFT);
	}

	if (id == BLOCK_PFT) {

		if (!is_help)
			assert(ppft != NULL);

		declareitem("c_in_dm_forage",
				&ppft->c_in_dm_forage,
				DBL_MIN, 1.0, // min, max
				1,            // number of parameters
				CB_NONE,
				"Herbivory: Fractional carbon content in dry matter forage.");

		declareitem("digestibility",
				&ppft->digestibility,
				DBL_MIN, 1.0, // min, max
				1,            // number of parameters
				CB_NONE,
				"Herbivory: Fractional digestibility of herbivore forage for ruminants");

		declareitem("forage_type",
				&strparam,
				64, // max length of string
				CB_FORAGE_TYPE,
				"Forage Type (\"INEDIBLE\",\"GRASS\")"); // Add more forage types here

	} 
}

void Parameters::callback(const int callback, Pft* ppft){

	// Turn the string parameter to upper case to make
	// comparison case-insensitive.
	std::transform(strparam.begin(), strparam.end(), 
			strparam.begin(), ::toupper);

	// add checkback codes in alphabetical order
	
	if (callback == CB_CHECKHFT) {

		// COMPLETION CHECK

		// Check all parameters marked as mandatory
		bool abort = false; // whether to abort
		for (int i=0; i<mandatory_hft_params.size(); i++){
			const std::string param = mandatory_hft_params[i];

			// send all messages first before aborting
			if (!itemparsed(param.c_str())){
				dprintf(std::string("Error: "
							"HFT \""+current_hft.name+"\""
							"is missing parameter "
							"\""+param+"\"\n").c_str());
				abort = true;
			} 
		}
		if (abort)
			plibabort();

		// VALIDITY CHECK

		// Let the Hft class do its own checks for validity
		std::string hft_messages; 
		const bool hft_valid = current_hft.is_valid(hft_messages);
		// print warnings and error messages
		if (hft_messages != "")
			dprintf(std::string(
						"HFT \""+current_hft.name+"\": \n"
						+ hft_messages).c_str());
		if (!hft_valid)
			plibabort();

		// Now everything seems okay, and we can add the HFT
		HftList::get_global().insert(current_hft);
	}

	if (callback == CB_CHECKGLOBAL) {

		if (!itemparsed("ifherbivory"))
			dprintf("Notice: ifherbivory was not declared. "
					"It is disabled by default.\n");

		if (!get_global().ifherbivory) {
			dprintf("Notice: ifherbivory is disabled. "
					"The herbivory output module will be deactivated.\n");
			GuessOutput::HerbivoryOutput::deactivate();
		}

		if (get_global().ifherbivory) {

			if (!itemparsed("free_herbivory_years")) {
				dprintf("Error: free_herbivory_years needs "
						"to be defined if ifherbivory is true.\n");
				plibabort();
			}

			if (!itemparsed("digestibility_model")) {
				dprintf("Error: digestibility_model needs "
						"to be defined if ifherbivory is true.\n");
				plibabort();
			}

			if (!itemparsed("forage_distribution")) {
				dprintf("Notice: forage_distribution not defined. "
						"It defaults to \"equally\".");
			}

			if (!itemparsed("dead_herbivore_threshold")) {
				dprintf("Error: dead_herbivore_threshold needs "
						"to be defined if ifherbivory is true.\n");
				plibabort();
			} 

			if (!itemparsed("herbivore_type")) {
				dprintf("Error: herbivore_type needs "
						"to be defined if ifherbivory is true.\n");
				plibabort();
			}

		}

		// Finish and close HFT list
		HftList& hftlist = HftList::get_global();
		hftlist.remove_excluded();
		hftlist.close(); // no further changes possible

		if (get_global().ifherbivory && hftlist.size()==0) {
			dprintf("Warning: ifherbivory is true, but "
					"no HFTs were included. The herbivory output "
					"module will be active, but no herbivory simulation "
					"will be done.\n");
		}
	} 

	if (callback == CB_DIG_MODEL) {
		if (strparam == "PFT_FIXED")
			get_global().dig_model = DM_PFT_FIXED;
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
			get_global().forage_distribution = FD_EQUALLY;
		else {
			sendmessage("Error", std::string(
					"Unknown value for parameter \"forage_distribution\" "
					"Possible values: "
					"\"equally\"").c_str());
			plibabort();
		}
	}

	if (callback == CB_FORAGE_TYPE) {
		if (strparam == "INEDIBLE") 
			ppft->forage_type = Fauna::FT_INEDIBLE;
		else if (strparam == "GRASS") 
			ppft->forage_type = Fauna::FT_GRASS;
		else {
			sendmessage("Error",
					"Unknown forage type; valid types: "
					"\"inedible\", \"grass\"");
			plibabort();
		}
	}

	if (callback == CB_HERBIVORE_TYPE) {
		if (strparam == "COHORT")
			get_global().herbivore_type = HT_COHORT;
		else if (strparam == "INDIVIDUAL")
			get_global().herbivore_type = HT_INDIVIDUAL;
		else {
			sendmessage("Error",
					"Unknown value for herbivore_type; valid types: "
					"\"cohort\", \"individual\"");
			plibabort();
		}
	}

	if (callback == CB_PFT) {
		assert (ppft != NULL);

		if (get_global().ifherbivory && ppft->is_edible()) {

			if (!itemparsed("digestibility") &&
					get_global().dig_model == DM_PFT_FIXED) 
			{
				sendmessage("Error", 
						(xtring) "Edible Pft " + ppft->name +
						" is missing \"digestibility\". "
						"It needs to be defined with ifherbivory=true and "
						"the chosen digestibility model.");
				plibabort();
			}

			if (!itemparsed("c_in_dm_forage")) {
				sendmessage("Error", 
						(xtring) "Edible Pft " + ppft->name +
						" is missing \"c_in_dm_forage\". "
						"It needs to be defined with ifherbivory=true.");
				plibabort();
			}
		}
	}

}

void Parameters::init_pft(Pft& pft){
	pft.forage_type=Fauna::FT_INEDIBLE;
}

