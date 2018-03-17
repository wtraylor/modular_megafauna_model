Tutor for the Large Herbivore Module {#page_herbiv_tutor}
=========================================================
<!-- For doxygen, this is the *page* header -->
\brief Instructions how to use the herbivore module and expand the code base for one’s needs.

Tutor for the Large Herbivore Module {#sec_herbiv_tutor}
=========================================================
<!-- For doxygen, this is the *section* header -->
\tableofcontents

Before starting with your programming work, please familiarize yourself with the parts of the [module design](\ref page_herbiv_design) that are relevant to your project.
Make sure you have understood the concepts of [object-oriented software design](\ref sec_object_orientation).
In particular, observe the [Open-closed Principle](\ref sec_open_closed) wherever possible: Only change a class *if necessary.* Try to extend it instead or—even better—write a new class with your functionality.

Please keep the documentation of the [design](\ref page_herbiv_design) and the [model explanations](\ref page_herbiv_model) updated.

Come into the practice of **test-driven development** (see \ref page_herbiv_tests).
Write a test for each one of your new classes!
Check all of its public methods and see what example test scenarios it has to fulfill.
Also check if exceptions are thrown correctly.

Herbivores Tutorials {#sec_herbiv_tutor_herbivores}
---------------------------------------------------

### How to add a new herbivore class {#sec_herbiv_new_herbivore_class}

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

@startuml "Relationships for a new herbivore type."
	!include herbiv_diagrams.iuml!new_herbivore_type
@enduml

### How to add a new energy expenditure component {#sec_herbiv_new_expenditure_component}
- Add a new entry in \ref Fauna::ExpenditureComponent.
- Parameters:
	+ Add new possible string value for the HFT parameter `expenditure_components` in \ref Fauna::ParamReader::declare_parameters().
	+ Adjust \ref Fauna::ParamReader::callback() under `callback == CB_EXPENDITURE_COMPONENTS`.
	+ Expand the comment in the instruction file `data/ins/herbivores.ins`.
- Implement your algorithm as a free function or a class. See \ref herbiv_energetics.h for examples.
- Call your model in \ref Fauna::HerbivoreBase::get_todays_expenditure().
- Update the UML diagram in \ref sec_herbiv_herbivorebase.

### How to add a new foraging limit {#sec_herbiv_new_foraging_limit}
A foraging limit constrains the daily uptake of forage mass by a herbivore individual.
Foraging limits are implemented as functors (without using the [strategy design pattern](\ref sec_strategy), though).
Which ones are activated is defined by \ref Fauna::Hft::foraging_limits.
They are called in \ref Fauna::GetForageDemands::get_max_foraging().

- Add a new enum entry in \ref Fauna::ForagingLimit.
- Parameters:
	+ In \ref Fauna::ParamReader::callback() add a string identifier for your implementation under `CB_FORAGING_LIMITS` (also in the error message).
	+ Add that identifier in the help message under \ref Fauna::ParamReader::declare_parameters().
	+ Document your option in the example instruction file `data/ins/herbivores.ins`.
- Implement your foraging limit (preferably as a function object in the file \ref herbiv_foraging.h, but you can do as you wish). 
Make sure that an exception is thrown if it is called with an unknown forage type.
- Call your implementation in \ref Fauna::GetForageDemands::get_max_foraging().
- Update the UML diagram in \ref sec_herbiv_herbivorebase.

### How to add a new digestive limit {#sec_herbiv_new_digestive_limit}
On top of foraging limitations, the daily forage uptake can be constrained by maximum digestive throughput.
The implementation is almost parallel to a [foraging limit](\ref sec_herbiv_new_foraging_limit).
You can choose 

- Add a new enum entry in \ref Fauna::DigestiveLimit.
- Parameters:
	+ In \ref Fauna::ParamReader::callback() add a string identifier for your implementation under `CB_DIGESTIVE_LIMIT` (also in the error message).
	+ Add that identifier in the help message under \ref Fauna::ParamReader::declare_parameters().
	+ Document your option in the example instruction file `data/ins/herbivores.ins`.
- Implement your digestive limit algorithm as a free function or an object.
Make sure that an exception is thrown if it is called with an unknown forage type.
- Call your implementation in \ref Fauna::GetForageDemands::get_max_digestion().
- Update the UML diagram in \ref sec_herbiv_herbivorebase.


### How to add a new reproduction model {#sec_herbiv_new_reproduction_model}
A reproduction model defines the offspring per female individual for each simulation day.

- Parameters:
	+ Create a new enum entry in \ref Fauna::ReproductionModel.
	+ Parse the parameter in \ref Fauna::ParamReader::callback() under `CB_REPRODUCTION_MODEL` and expand the error message with your string identifier.
	+ Add your string identifier to the help output in \ref Fauna::ParamReader::declare_parameters().
	+ Document your model in the comments of the example instruction file: `data/ins/herbivores.ins`.
- Create your class or function somewhere.
- Call your model in \ref Fauna::HerbivoreBase::get_todays_offspring_proportion().
- Update the UML diagram in \ref sec_herbiv_herbivorebase.

### How to add a new diet composer {#sec_herbiv_new_diet}
In a scenario with multiple forage types, the herbivore decides what to include in its diet.
This decision is modelled by an implementation of a so called “diet composer model”: \ref Fauna::DietComposer.
You can implement your own model as a new class or a simple function; just call it in \ref Fauna::GetForageDemands::get_diet_composition().

- Parameters:
	+ Create a new enum entry in \ref Fauna::DietComposer.
	+ Parse the parameter in \ref Fauna::ParamReader::callback() under `CB_DIET_COMPOSER` and expand the error message with your string identifier.
	+ Add your string identifier to the help output in \ref Fauna::ParamReader::declare_parameters().
	+ Document your model in the comments of the example instruction file: `data/ins/herbivores.ins`.
- Call your model in \ref Fauna::GetForageDemands::get_diet_composition().
- Update the UML diagram in \ref sec_herbiv_herbivorebase.

### How to add a new mortality factor {#sec_herbiv_new_mortality}
<!-- TODO -->
If your new mortality factor is starvation-based, it might make sense to add it to the “starvation” output variable in \ref GuessOutput::HerbivoryOutput in the private method `write_datapoint()`.

Forage Tutorials {#sec_herbiv_tutor_forage}
-------------------------------------------

### How to add a new forage type {#sec_herbiv_new_forage_type}

- Create new enum entry in \ref Fauna::ForageType and add it to \ref Fauna::FORAGE_TYPES by expanding the initializing function `get_all_forage_types()`, which is declared in local namespace in \ref herbiv_forageclasses.cpp.

- Add a short name without blanks in \ref Fauna::get_forage_type_name().

- Instruction file parameters (\ref herbiv_paramreader.cpp):
	+ \ref ParamReader::declare_parameters() : Add the new forage type to parameter description of parameter `forage_type`.
	+ \ref ParamReader::callback() : Add forage type under \ref Fauna::CB_FORAGE_TYPE.

- Derive new class from \ref Fauna::ForageBase.
	+ Implement a `merge()` method, like \ref Fauna::GrassForage::merge().

- Add a new member variable in \ref Fauna::HabitatForage of that class.
	+ Add it in \ref Fauna::HabitatForage::operator[]().
	+ Call your `merge()` function in \ref Fauna::HabitatForage::merge().

- Adjust \ref Individual::get_forage_mass() and \ref Individual::reduce_forage_mass().

- Adjust \ref Fauna::PatchHabitat::get_available_forage() to define values for your new forage-specific properties.
(\ref Fauna::PatchHabitat::remove_eaten_forage() is generic for all forage types and does usually not need to be altered.)

- Perhaps adjust the digestibility in your chosen \ref Fauna::GetDigestibility implementation.

- Extend \ref Fauna::GetNetEnergyContentDefault.

- Herbivores
	+ Check all foraging and digestion limits (\ref herbiv_foraging.h) whether they need to be expanded.
	+ Check also all models for net energy content (\ref herbiv_forageenergy.h).
	+ Probably you will need to implement [a new diet composer](\ref sec_herbiv_new_diet) or adjust existing ones.

- Test Simulations
	+ If you want to use your forage type in the herbivory test simulations, expand \ref FaunaSim::SimpleHabitat by a new growth model (analoguous to \ref FaunaSim::LogisticGrass).
	Also update the UML diagram in the class documentation of SimpleHabitat.

@startuml "Relationships for a new forage type."
	!include herbiv_diagrams.iuml!new_forage_type
@enduml

### How to change forage net energy content {#sec_herbiv_change_netenergy}

Forage net energy content is implemented with the [strategy design pattern](\ref sec_strategy):
@startuml "Net energy content design."
	!include herbiv_diagrams.iuml!net_energy_content
@enduml
- Derive a new class from \ref Fauna::GetNetEnergyContentInterface and implement the `operator()`.
- Parameters:
	+ Add a new enum item in \ref Fauna::NetEnergyModel.
	+ Select it in \ref Fauna::ParamReader::callback() under `CB_NET_ENERGY_MODEL`.
	- Update help message in \ref Fauna::ParamReader::declare_parameters().
	- Update instruction file comments in `data/ins/herbivores.ins`.
- Let your new class be created in \ref Fauna::HerbivoreBase::get_net_energy_content() if it is selected in the parameters.
- Update the UML diagram in \ref sec_herbiv_herbivorebase and the diagram above.

### How to add a new forage distribution algorithm {#sec_herbiv_new_forage_distribution}
- Derive a new class from \ref Fauna::DistributeForage and implement your algorithm.

- Return a reference to an object of your class in \ref Fauna::Simulator::create_distribute_forage().

- Add an identifier in \ref Fauna::ForageDistributionAlgorithm and add your string identifier in \ref Fauna::ParamReader::callback() under `CB_FORAGE_DISTRIBUTION`.

- Don’t forget to add your identifier as possible values in the message output in \ref Fauna::ParamReader::declare_parameters() and \ref Fauna::ParamReader::callback(), as well as in the example instruction file `data/ins/herbivores.ins`.

LPJ-GUESS–Herbivory Interface {#sec_herbiv_tutor_guess_herbivory_interface}
---------------------------------------------------------------------------

### How to add a new snow depth model {#sec_herbiv_new_snow_depth_model}
LPJ-GUESS accounts for snow as snow water equivalent (SWE, see \ref Soil::snowpack).
A snow depth model converts that into the actual snow depth for the herbivory module.

- Implement your algorithm by deriving a new [function class](\ref sec_functors) from \ref Fauna::GetSnowDepth.

- Declare a new enum item in \ref Fauna::SnowDepthModel as an identifier for your model.

- Create an instance of your class in \ref Fauna::Simulator::create_snow_depth_model().

- Add your string identifier in \ref Fauna::ParamReader::callback() under `CB_SNOW_DEPTH_MODEL`.

- Don’t forget to add your identifier as a possible value in the message output in \ref Fauna::ParamReader::declare_parameters() and \ref Fauna::ParamReader::callback(), as well as in the example instruction file `data/ins/herbivores.ins`.

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
- Call the plib function \ref declareitem() in \ref Fauna::ParamReader::declare_parameters() (possibly with your own CB_* code).
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
- Call the plib function \ref declareitem() in \ref Fauna::ParamReader::declare_parameters() (possibly with your own CB_* code).
- If you wish, add it to `mandatory_hft_params` in \ref Fauna::ParamReader::callback() so that it must not be omitted. 
- Extend the example instruction file `data/ins/herbivores.ins`.


\see \ref sec_herbiv_parameters
Output Tutorials {#sec_herbiv_output_tutor}
-------------------------------------------

### How to add a new output variable {#sec_herbiv_new_output}
- Create a new `TableFile` member variable in \ref GuessOutput::HerbivoryOutput.
	+ Initialize it in the constructor.
	(The parameter and variable names must not contain a dot, space, or \ref GuessOutput::HerbivoryOutput::CAPTION_SEPARATOR.)
	+ Add it in the private function `HerbivoryOutput::init_tablefiles()`.
	- The instruction file parameter name for the output file name will be autogenerated in \ref GuessOutput::HerbivoryOutput::HerbivoryOutput().
	Add this parameter to your instruction file and to the default example file: `data/ins/herbiv_output.ins`.
- Extend the appropriate container: \ref FaunaOut::HabitatData or \ref FaunaOut::HerbivoreData by a new member variable and initialize it in the constructor with zero values.
	+ Implement average building in the appropriate `merge()` function.
	+ For herbivore data, you need to add it to \ref FaunaOut::HerbivoreData::create_datapoint(). If your value is *per individual*, you don’ TODO
	+ Assign a value to the variable somewhere in daily simulation.
- Map this container variable to the `TableFile` by actually writing output in \ref GuessOutput::HerbivoryOutput::write_datapoint().

\note In the case of variables for the whole habitat (not specific to forage type, HFTs etc.), use the existing `TableFile` object `TBL_HABITAT`. 
Add your new variable as a column to the table in \ref GuessOutput::HerbivoryOutput::get_columns() and add the value in \ref GuessOutput::HerbivoryOutput::write_datapoint(), *in the same order as the columns*.

\note If you want to add a variable that is not *per herbivore mass*, you would have to use mass density as weight.

### How to limit output to a specific time period {#sec_herbiv_limit_output}

Adjust the return value of \ref GuessOutput::HerbivoryOutput::is_today_included() to your needs.
For comparison, see the local function `outlimit()` in \ref commonoutput.cpp.

\see \ref sec_herbiv_output


How to add a new test vegetation model {#sec_herbiv_new_testhabitat}
--------------------------------------

By default, the class \ref FaunaSim::SimpleHabitat is used for all test simulations.
If you want to run your own vegetation model, create it in \ref FaunaSim::Framework::create_habitat().

Re-using the code for other vegetation models {#sec_herbiv_reusing_code}
---------------------------------------------

<!--TODO-->


------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see \ref page_herbiv_design
\see \ref page_herbiv_model
\see \ref page_herbiv_tests
\see \ref group_herbivory
