Tutor for the Large Herbivore Module {#page_herbiv_tutor}
=========================================================
<!-- For doxygen, this is the *page* header -->
\brief Instructions how to use the herbivore module and adapt the 
code it to one’s needs.

Tutor for the Large Herbivore Module {#sec_herbiv_tutor}
=========================================================
<!-- For doxygen, this is the *section* header -->
\tableofcontents

Herbivores Tutorials {#sec_herbiv_tutor_herbivores}
---------------------------------------------------

### How to add a new herbivore class {#sec_herbiv_new_herbivore_class}

@startuml "Relationships for a new herbivore type"
namespace Fauna{
	hide members
	hide methods
	interface HerbivoreInterface
	abstract HerbivoreBase
	HerbivoreInterface  <|-- HerbivoreBase
	HerbivoreBase       <|-- HerbivoreIndividual
	HerbivoreBase       <|-- HerbivoreCohort
	HerbivoreInterface  <|-- NewHerbivore
	interface PopulationInterface
	PopulationInterface <|-- NewPopulation
	NewPopulation        ..> NewHerbivore : create & manage
}
@enduml

The model design allows a complete substitution of the herbivore
class.
If you want to implement a completely new model behaviour, you
can derive your new class from \ref Fauna::HerbivoreInterface
and write it from scratch.
If you want to build upon the base functionality, derive it
from \ref Fauna::HerbivoreBase.

Add a new ID enum element in \ref Fauna::HerbivoreType and make it
accessible to the instruction file parser by checking for your
string identifier in \ref Fauna::ParamReader::callback() under
`CB_HERBIVORE_TYPE`.

Then, derive a new class from \ref Fauna::PopulationInterface to manage and construct your object instances.
In \ref Fauna::Simulator::create_populations(), create that
population class.

Forage Tutorials {#sec_herbiv_tutor_forage}
-------------------------------------------

### How to add a new forage type {#sec_herbiv_new_forage_type}

@startuml "Relationships for a new forage type"
namespace Fauna{
	enum ForageType {
		FT_GRASS,
		FT_NEWFORAGE
	}
	ParamReader ..> ForageType : <<use>>
	class ForageMass{
		double grass
		double new_forage
		double sum()
	}
	abstract ForageBase
	class NewForage{
		your_custom_members
	}
	NewForage <|-- ForageBase
	class HabitatForage{
		get_total()
		merge()
	}
	HabitatForage *--> NewForage
	interface GetDigestibility
	class PatchHabitat{
		get_available_forage()
		reduce_forage_mass()
	}
	PatchHabitat ..> HabitatForage : <<use>>
	TestHabitat  ..> HabitatForage : <<use>>
}
class Individual{
	get_forage_mass()
}
Fauna.PatchHabitat          ..> Individual       : <<use>>
Fauna.GetDigestibility      ..> Individual       : <<use>>
Individual                  ..> Fauna.ForageMass : <<use>>
GuessOutput.HerbivoryOutput ..> Fauna.NewForage  : <<use>>
@enduml

- Create new enum entry in \ref Fauna::ForageType.

- Increase \ref Fauna::FORAGE_TYPE_COUNT.

- Add a short name for it in \ref Fauna::get_forage_type_name().

- Instruction file (\ref herbiv_parameters.cpp):
	+ ParamReader::declare_parameters():
	Add parameter description
	+ ParamReader::callback():
	Add forage type under \ref Fauna::CB_FORAGE_TYPE.

- Create new member variable in \ref Fauna::ForageMass and include it in the constructor, in \ref Fauna::ForageMass::sum() and in all overloaded operators.

- Derive new class from \ref Fauna::ForageBase;

- Add a new member variable in \ref Fauna::HabitatForage of that class and include it in \ref Fauna::HabitatForage::get_total().

- Implement average building in \ref Fauna::HabitatForage::merge().

- Adjust \ref Individual::get_forage_mass() and \ref Individual::reduce_forage_mass().

- Adjust \ref Fauna::PatchHabitat::get_available_forage() and \ref Fauna::PatchHabitat::remove_eaten_forage().

- Output:
	+ Add a new column descriptor in \ref GuessOutput::HerbivoryOutput::get_forage_columns().
	+ Add output file names and tables as member variables in \ref GuessOutput::HerbivoryOutput.
	+ Declare output file parameters in \ref GuessOutput::HerbivoryOutput::HerbivoryOutput().
	+ Define output tables in \ref GuessOutput::HerbivoryOutput::define_output_tables().

- Perhaps adjust the digestibility in your chosen \ref Fauna::GetDigestibility.

\todo Add Herbivore feeding and test simulations

### How to add a new forage distribution algorithm {#sec_herbiv_new_forage_distribution}
- Derive a new class from \ref Fauna::DistributeForage and
  implement your algorithm.

- Return a reference to an object of your class in
  \ref Fauna::Simulator::distribute_forage().

- Add an identifier in \ref Fauna::ForageDistributionAlgorithm and
  add your string identifier in \ref Fauna::ParamReader::callback()
	under `CB_FORAGE_DISTRIBUTION`.

- Don’t forget to add your identifier as possible values in
  the message output in \ref Fauna::ParamReader::declare_parameters()
	and \ref Fauna::ParamReader::callback(), as well as in the
	example instruction file `data/ins/herbivores.ins`.

Parameters Tutorials {#sec_herbiv_tutor_parameters}
---------------------------------------------------

### How to add a new global parameter {#sec_herbiv_new_global_parameter}

Global parameters of the herbivory module are declared and parsed by \ref Fauna::ParamReader, but initialized and checked in \ref Fauna::Parameters.

- Declare your parameter as a member variable in \ref Fauna::Parameters.
- Initialize it with a valid default value in \ref Fauna::Parameters::Parameters().
- Write a validity check in \ref Fauna::Parameters::is_valid().
- If the parameter needs to be parsed from a string, add your
	own callback:
	+ add a new enum item CB_* in \ref parameters.h.
	+ add a new if statement in \ref Fauna::ParamReader::callback().
- Call the plib function \ref declareitem() in 
	\ref Fauna::ParamReader::declare_parameters()
	(possibly with your own CB_* code).
- If you wish, add it to `mandatory_global_params` in \ref Fauna::ParamReader::callback() so that it must not be omitted. 
- Extend the example instruction file `data/ins/herbivores.ins`.

### How to add a new PFT parameter {#sec_herbiv_new_pft_parameter}

Herbivory-related PFT parameters are declared and parsed by \ref Fauna::ParamReader, but initialized and checked in \ref Fauna::PftParams.

- Create the member variable in \ref Fauna::PftParams.
- If the parameters needs its own callback:
	+ add a new enum item both in \ref parameters.h.
	+ add a new if statement in \ref Fauna::ParamReader::callback().
- Declare the parameter in \ref Fauna::ParamReader::declare_parameters()
	(possibly with your own CB_* code).
- You can initialize it in \ref Fauna::PftParams::PftParams().
- Check if the parameter was *parsed* in \ref Fauna::ParamReader::callback(). There, add it to `mandatory_pft_params` to ensure it is not omitted. If it is *not* mandatory, make sure it is initialized with a valid value!
- Check if the parameter is *valid* in \ref Fauna::PftParams::is_valid().
- Extend the example instruction files in the directory `data/ins`.


### How to add a new HFT parameter {#sec_herbiv_new_hft_parameter}

- Declare your member variable in \ref Fauna::Hft 
  (observe alphabetical order, please)
- Initialize it with a (valid!) default value in \ref Fauna::Hft::Hft().
- Write a validity check in \ref Fauna::Hft::is_valid().
- If the parameter needs to be parsed from a string, add your
	own callback:
	+ add a new enum item CB_* in \ref parameters.h.
	+ add a new if statement in \ref Fauna::ParamReader::callback().
- Call the plib function \ref declareitem() in 
	\ref Fauna::ParamReader::declare_parameters()
	(possibly with your own CB_* code).
- If you wish, add it to `mandatory_hft_params` in \ref Fauna::ParamReader::callback() so that it must not be omitted. 
- Extend the example instruction file `data/ins/herbivores.ins`.


Output Tutorials {#sec_herbiv_output_tutor}
-------------------------------------------

\see \ref sec_herbiv_output

### How to add a new output variable {#sec_herbiv_new_output}


- Add a new variable in \ref Fauna::HabitatOutputData, either as an accumulated or
averaged value.
	+ Initialize it for each day in \ref Fauna::Habitat::init_todays_output().
	+ Fill it with data somehow.
	+ Add it in the merge function: \ref Fauna::HabitatOutputData::merge().
- In class \ref GuessOutput::HerbivoryOutput :
	+ Add new member variables for a file name and a \ref GuessOutput::Table object.
	+ Call \ref declare_parameter() in \ref GuessOutput::HerbivoryOutput::HerbivoryOutput()
	for your new output file.
	+ Create the \ref GuessOutput::Table object in \ref GuessOutput::HerbivoryOutput::define_output_tables().
	+ Write the data of one row in \ref GuessOutput::HerbivoryOutput::add_output_object().
- Add the file name in your instruction script.

### How to limit output to a specific time period {#sec_herbiv_limit_output}

Declare in any header file a class derived from 
\ref GuessOutput::OutputLimiter and implement the inherited
pure virtual method \ref GuessOutput::OutputLimiter::include_date().
In the \ref framework() function call
\ref GuessOutput::HerbivoryOutput::set_limiter() passing a 
persistent instantiation of your class.


\todo How to add a new test vegetation model.

------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see \ref page_herbiv_design
\see \ref page_herbiv_model
\see \ref page_herbiv_tests
\see \ref group_herbivory
