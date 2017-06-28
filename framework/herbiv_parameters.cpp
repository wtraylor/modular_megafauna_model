
#include "config.h"
#include "plib.h"              // read instruction file
#include "guess.h"             // for Pft
#include "herbiv_parameters.h" // read global parameters
#include "herbiv_output.h"     // for HerbivoryOutput::deactivate()

#include <climits>   // for INT_MAX
#include <cfloat>    // for DBL_MAX, DBL_MIN
#include <algorithm> // for std::transform


using namespace Fauna;

// initialization of static member variables
std::string Parameters::strparam      = std::string();

void Parameters::declare_parameters(
		const int id, 
		const std::string& setname,
		Pft* ppft)
{
	// (We use if-else instead of switch here, because the static
	// members are initialized in parameters.cpp and cannot be
	// used in as constant values in a switch-case statement.)
	
	if (id == BLOCK_GLOBAL) {
		declareitem("ifherbivory", 
				&(get_global().ifherbivory),
				1, // number of parameters
				CB_NONE, 
				"Whether herbivory is enabled");

		declareitem("free_herbivory_years",
				&(get_global().free_herbivory_years),
				0, INT_MAX, // min, max
				1,          // number of parameters
				CB_NONE,
				"Number of years without herbivory, as part of vegetation spinup.");

		declareitem("digestibility_model",
				&strparam,
				128, // max length of string
				CB_DIG_MODEL,
				"Digestibility model for herbivore forage. "
				"Possible values: \"PFT_FIXED\"");

	} else if (id == BLOCK_PFT) {

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

	if (callback == CB_CHECKGLOBAL) {

		if (!itemparsed("ifherbivory"))
			sendmessage("Notice", "ifherbivory was not declared. "
					"It is disabled by default.");

		if (!get_global().ifherbivory) {
			sendmessage("Notice", "ifherbivory is disabled. "
					"The herbivory output module will be deactivated.");
			GuessOutput::HerbivoryOutput::deactivate();
		}
				

		if (get_global().ifherbivory) {

			if (!itemparsed("free_herbivory_years")) {
				sendmessage("Error", "free_herbivory_years needs "
						"to be defined if ifherbivory is true.");
				plibabort();
			}

			if (!itemparsed("digestibility_model")) {
				sendmessage("Error", "digestibility_model needs "
						"to be defined if ifherbivory is true.");
				plibabort();
			}
		}
	} else if (callback == CB_PFT) {
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
			}
		}
	} else if (callback == CB_FORAGE_TYPE) {
		if (strparam == "INEDIBLE") 
			ppft->forage_type = Fauna::FT_INEDIBLE;
		else if (strparam == "GRASS") 
			ppft->forage_type = Fauna::FT_GRASS;
		else {
			sendmessage("Error",
					"Unknown forage type; valid types: "
					"\"INEDIBLE\", \"GRASS\"");
			plibabort();
		}
	} else if (callback == CB_DIG_MODEL) {
		if (strparam == "PFT_FIXED")
			get_global().dig_model = DM_PFT_FIXED;
		// add other digestibility models here
		else {
			sendmessage("Error",
					"Unknown digestibility model; valid types: "
					"\"DM_PFT_FIXED\"");
			plibabort();
		}
	}
}


void Parameters::init_pft(Pft& pft){
	pft.forage_type=Fauna::FT_INEDIBLE;
}

