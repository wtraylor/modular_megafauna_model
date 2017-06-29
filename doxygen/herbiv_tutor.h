

/** \page page_herbiv_tutor Tutor for the Large Herbivore Module
\brief Instructions how to use the herbivore module and adapt it to one’s needs.







\section sec_herbiv_tutor_forage Forage

\subsection sec_herbiv_new_forage_type How to add a new forage type

- Create new enum entry in \ref Fauna::ForageType.
- Increase \ref Fauna::FORAGE_TYPE_COUNT.
- Add a short name for it in \ref Fauna::get_forage_type_name().
- Instruction file (\ref herbiv_parameters.cpp):
	+ \ref Fauna::Parameters::declare_parameters(): Add parameter description
	+ \ref Fauna::Parameters::callback(): Add forage type under \ref CB_FORAGE_TYPE
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

\subsection sec_herbiv_new_digestibility_model How to add a new model for forage digestibility
- Derive a new class from \ref Fauna::DigestibilityModel 
(it does not need to be in \ref herbiv_digestibility.h).
- Implement \ref Fauna::DigestibilityModel::get_digestibility() 
in your new class.
- To comply with the singleton design pattern (\ref sec_singleton),
hide the constructors as protected.
- Add a new enum entry in \ref Fauna::DigestibilityModelType.
- Construct an instance of the new class in 
\ref Fauna::Simulator::Simulator().  
- \ref Fauna::Parameters:
    + Add description in \ref Fauna::Parameters::declare_parameters().
    + Implement parameter parsing for CB_DIG_MODEL in
    \ref Fauna::Parameters::callback(). (Also add your model name
    in the error message.)
    + If your model needs PFT parameters, make sure to check
    them in \ref Fauna::Parameters::callback() unter CB_PFT.
- Add a description of your model in `data/ins/herbivores.ins`.






\section sec_herbiv_tutor_parameters Parameters

\subsection sec_herbiv_new_pft_parameter How to add a new PFT parameter
Pft parameters are declared and parsed outside of the core LPJ-GUESS functions of
\ref parameters.cpp (see \ref sec_herbiv_parameters).

- Create the member variable in \ref Pft. Place it with the other herbivory variables.
- Declare the parameter in \ref Fauna::Parameters::declare_parameters().
- Check if the parameter was parsed and is okay in \ref Fauna::Parameters::callback().
- If the parameters needs its own callback:
	+ add a new enum item both in \ref parameters.cpp and (as static const int) in 
	\ref Fauna::Parameters. 
	+ Initialise the latter in \ref parameters.cpp with the value from the enum value.
	+ add a new case block in \ref Fauna::Parameters::callback().


\subsection sec_herbiv_new_hft_parameter How to add a new HFT parameter






















\section sec_herbiv_output_tutor Output

\see \ref sec_herbiv_output
\subsection sec_herbiv_new_output How to add a new output variable

- Add a new variable in \ref Fauna::HabitatOutputData, either as an accumulated or
averaged value.
	+ Initialize it for each day in \ref Fauna::Habitat::init_todays_output().
	+ Fill it with data somehow.
	+ Add it in the merge function: \ref Fauna::HabitatOutputData::merge().
- In class \ref GuessOutput::HerbivoryOutput :
	+ Add new member variables for a file name and a \ref Table object.
	+ Call \ref declare_parameter() in \ref GuessOutput::HerbivoryOutput::HerbivoryOutput()
	for your new output file.
	+ Create the \ref Table object in \ref GuessOutput::HerbivoryOutput::define_output_tables().
	+ Write the data of one row in \ref GuessOutput::HerbivoryOutput::add_output_object().
- Add the file name in your instruction script.

\subsection sec_herbiv_limit_output How to limit output to a specific time period
Declare in any header file a class derived from 
\ref GuessOutput::OutputLimiter and implement the inherited
pure virtual method \ref GuessOutput::OutputLimiter::include_date().
In the \ref framework() function call
\ref GuessOutput::HerbivoryOutput::set_limiter() passing a 
persistent instantiation of your class.




\todo How to add a new test vegetation model.


















\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see The page \ref group_herbivory gives implementation details.
\see The page \ref page_herbiv_model gives background on the science.
\see The page \ref page_herbiv_tests explains the testing %framework.
*/
