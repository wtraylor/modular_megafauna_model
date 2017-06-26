

/** \page page_herbiv_tutor Tutor for the Large Herbivore Module
\brief Instructions how to use the herbivore module and adapt it to one’s needs.

\section sec_herbiv_new_forage_type How to add a new forage type

- Create new enum entry in \ref Fauna::ForageType.
- Increase \ref Fauna::FORAGE_TYPE_COUNT.
- Add a short name for it in \ref Fauna::get_forage_type_name().
- Instruction file (\ref parameters.cpp):
	+ \ref plib_declarations(): Add parameter description
	+ \ref plib_callback(): Add forage type under \ref CB_FORAGE_TYPE
- Create new member variable in \ref Fauna::ForageMass and include it in 
the constructor, in \ref Fauna::ForageMass::sum() and in all overloaded operators.
- Derive new class from \ref Fauna::ForageBase.
- Add a new member variable in \ref Fauna::HabitatForage of that class and include
it in \ref Fauna::HabitatForage::get_total().
- Add it in \ref Fauna::HabitatForage::get_forage_type_name().
- Implement average building in \ref Fauna::HabitatForage::merge().
- Adjust \ref Individual::get_forage_mass() \ref Individual::reduce_forage_mass().
- Adjust \ref Fauna::PatchHabitat::get_available_forage() and 
\ref Fauna::Habitat::remove_eaten_forage().
- Output:
	+ Add a new column descriptor in \ref GuessOutput::HerbivoryOutput::get_forage_columns().
	+ Add output file names and tables as member variables in \ref GuessOutput::HerbivoryOutput.
	+ Declare output file parameters in \ref GuessOutput::HerbivoryOutput::HerbivoryOutput().
	+ Define output tables in \ref GuessOutput::HerbivoryOutput::define_output_tables().
- Perhaps adjust the digestibility in your chosen \ref Fauna::DigestibilityModel.



\section sec_herbiv_output_tutor Output

\subsection sec_herbiv_new_output How to add a new output variable

- Add a new variable in \ref Fauna::HabitatOutputData, either as an accumulated or
averaged value.
	+ Add it in the merge function: \ref Fauna::HabitatOutputData::merge().
- Add new member variables \ref GuessOutput::HerbivoryOutput for a file name and a
\ref Table object.
- 

\subsection sec_herbiv_limit_output How to limit output to a specific time period
Simply adjust \ref GuessOutput::HerbivoryOutput::include_date().

\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see The page \ref group_herbivory gives implementation details.
\see The page \ref page_herbiv_model gives background on the science.
\see The page \ref page_herbiv_tests explains the testing %framework.
*/
