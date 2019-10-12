# Tutor {#page_tutor}
<!-- For doxygen, this is the *page* header -->
\brief Instructions how expand the code base for your own needs.

# Tutor {#sec_tutor}
<!-- For doxygen, this is the *section* header -->
\tableofcontents

## Herbivores Tutorials {#sec_tutor_herbivores}

### How to add a new herbivore class {#sec_new_herbivore_class}

The model design allows a complete substitution of the herbivore class.
If you want to implement a completely new model behaviour, you can derive your new class from \ref Fauna::HerbivoreInterface and write it from scratch.
If you want to build upon the base functionality, derive it from \ref Fauna::HerbivoreBase.

Then, derive a new class from \ref Fauna::PopulationInterface to manage and construct your object instances.
In \ref Fauna::WorldConstructor::create_population(), create that population class.

@startuml "Relationships for a new herbivore type."
	!include diagrams.iuml!new_herbivore_type
@enduml

### How to add a new energy expenditure component {#sec_new_expenditure_component}
- Add a new enum entry in \ref Fauna::ExpenditureComponent. <!--TODO: ref-->
- TOML instruction file: Add new possible string value for the HFT parameter `expenditure.components` in \ref Fauna::InsfileReader::read_hft(); include it in the error message.
- Implement your algorithm as a free function or a class. See \ref expenditure_components.h for examples.
- Call your model in \ref Fauna::HerbivoreBase::get_todays_expenditure().
- Update the UML diagram in Section \ref sec_herbivorebase.

### How to add a new foraging limit {#sec_new_foraging_limit}
A foraging limit constrains the daily uptake of forage mass by a herbivore individual.
Foraging limits are implemented as functors (without using the [strategy design pattern](\ref sec_strategy), though).
Which ones are activated is defined by `foraging.limits` in \ref Fauna::Hft.
They are called in \ref Fauna::GetForageDemands::get_max_foraging().

- Add a new enum entry in \ref Fauna::ForagingLimit<!--TODO: ref-->.
- TOML instruction file: Add a new possible string value for the HFT parameter `foraging.limits` in \ref Fauna::InsfileReader::read_hft()
- Implement your foraging limit (preferably as a function object in the file \ref foraging_limits.h, but you can do as you wish).
Make sure that an exception is thrown if it is called with an unknown forage type.
- Call your implementation in \ref Fauna::GetForageDemands::get_max_foraging().
- Update the UML diagram in \ref sec_herbivorebase.

### How to add a new digestive limit {#sec_new_digestive_limit}
On top of foraging limitations, the daily forage uptake can be constrained by maximum digestive throughput.
The implementation is almost parallel to a [foraging limit](\ref sec_new_foraging_limit).
You can choose

- Add a new enum entry in \ref Fauna::DigestiveLimit.
- Parameters:
	+ In \ref Fauna::ParamReader::callback() add a string identifier for your implementation under `CB_DIGESTIVE_LIMIT` (also in the error message).
	+ Add that identifier in the help message under \ref Fauna::ParamReader::declare_parameters().
	+ Document your option in the example instruction file `data/ins/herbivores.ins`.
- Implement your digestive limit algorithm as a free function or an object.
Make sure that an exception is thrown if it is called with an unknown forage type.
- Call your implementation in \ref Fauna::GetForageDemands::get_max_digestion().
- Update the UML diagram in \ref sec_herbivorebase.


### How to add a new reproduction model {#sec_new_reproduction_model}
A reproduction model defines the offspring per female individual for each simulation day.

- Create a new enum entry in \ref Fauna::ReproductionModel.
- Read the new value for `reproduction.model` from the instruction file in \ref Fauna::InsfileReader::read_hft().
- Create your class or function in \ref reproduction_models.h or in a separate file.
- Call your model in \ref Fauna::HerbivoreBase::get_todays_offspring_proportion().
- Update the UML diagram in \ref sec_herbivorebase.

### How to add a new diet composer {#sec_new_diet_composer}
In a scenario with multiple forage types, the herbivore decides what to include in its diet.
This decision is modelled by an implementation of a so called “diet composer model”: \ref Fauna::DietComposer.
You can implement your own model as a new class or a simple function; just call it in \ref Fauna::GetForageDemands::get_diet_composition().

- Create a new enum entry in \ref Fauna::DietComposer.
- Read the new value for `foraging.diet_composer` in \ref Fauna::InsfileReader::read_hft().
- Call your model in \ref Fauna::GetForageDemands::get_diet_composition().
- Update the UML diagram in \ref sec_herbivorebase.

### How to add a new mortality factor {#sec_new_mortality_factor}
Any death event of an herbivore is modelled by a mortality factor.
Whether you want to have herbivores die by for instance disease, drought, or predators, you should create a new mortality factor.

- Create a new enum entry in \ref Fauna::MortalityFactor.
- Parse the new possible value for the set `mortality.factors` in \ref Fauna::InsfileReader::read_hft().
- Implement your mortality model as a function or class in \ref mortality_factors.h or in a separate file (if it’s more complex).
- Call the mortality factor in \ref Fauna::HerbivoreBase::apply_mortality_factors_today().
- Update the UML diagram in \ref sec_herbivorebase.

## Forage Tutorials {#sec_tutor_forage}

### How to add a new forage type {#sec_new_forage_type}

- Create new enum entry in \ref Fauna::ForageType and add it to \ref Fauna::FORAGE_TYPES by expanding the initializing function `get_all_forage_types()`, which is declared in local namespace in \ref forage_types.cpp.

- Add a short name without blanks in \ref Fauna::get_forage_type_name().

- Instruction file parameters (\ref paramreader.cpp):
	+ \ref Fauna::ParamReader::declare_parameters() : Add the new forage type to parameter description of parameter `forage_type`.
	+ \ref Fauna::ParamReader::callback() : Add forage type under \ref Fauna::CB_FORAGE_TYPE.

- Derive new class from \ref Fauna::ForageBase.
	+ Implement a `merge()` method, like \ref Fauna::GrassForage::merge().

- Add a new member variable in \ref Fauna::HabitatForage of that class.
	+ Add it in \ref Fauna::HabitatForage::operator[]().
	+ Call your `merge()` function in \ref Fauna::HabitatForage::merge().

- Adjust \ref Individual::get_forage_mass() and \ref Individual::reduce_forage_mass().

- Adjust the implementation of \ref Fauna::Habitat::get_available_forage() and \ref Fauna::Habitat::remove_eaten_forage() in your vegetation model.

- Perhaps adjust the digestibility in your chosen \ref Fauna::GetDigestibility implementation.

- Extend \ref Fauna::GetNetEnergyContentDefault.

- Herbivores
	+ Check all foraging and digestion limits (\ref foraging_limits.h) whether they need to be expanded.
	+ Check also all models for net energy content (\ref net_energy_models.h).
	+ Probably you will need to implement [a new diet composer](\ref sec_new_diet_composer) or adjust existing ones.

- Test Simulations
	+ If you want to use your forage type in the herbivory test simulations, expand \ref FaunaSim::SimpleHabitat by a new growth model (analoguous to \ref FaunaSim::LogisticGrass).
	Also update the UML diagram in the class documentation of SimpleHabitat.

@startuml "Relationships for a new forage type."
	!include diagrams.iuml!new_forage_type
@enduml

### How to change forage net energy content {#sec_change_netenergy}

Forage net energy content is implemented with the [strategy design pattern](\ref sec_strategy):
@startuml "Net energy content design."
	!include diagrams.iuml!net_energy_content
@enduml
- Derive a new class from \ref Fauna::GetNetEnergyContentInterface and implement the `operator()`.
- Parameters:
	+ Add a new enum item in \ref Fauna::NetEnergyModel.
	+ Select it in \ref Fauna::ParamReader::callback() under `CB_NET_ENERGY_MODEL`.
	- Update help message in \ref Fauna::ParamReader::declare_parameters().
	- Update instruction file comments in `data/ins/herbivores.ins`.
- Let your new class be created in \ref Fauna::HerbivoreBase::get_net_energy_content() if it is selected in the parameters.
- Update the UML diagram in \ref sec_herbivorebase and the diagram above.

### How to add a new forage distribution algorithm {#sec_new_forage_distribution}
- Derive a new class from \ref Fauna::DistributeForage and implement your algorithm.

- Return a reference to an object of your class in \ref Fauna::Simulator::create_distribute_forage().

- Add an identifier in \ref Fauna::ForageDistributionAlgorithm and add your string identifier in \ref Fauna::ParamReader::callback() under `CB_FORAGE_DISTRIBUTION`.

- Don’t forget to add your identifier as possible values in the message output in \ref Fauna::ParamReader::declare_parameters() and \ref Fauna::ParamReader::callback(), as well as in the example instruction file `data/ins/herbivores.ins`.

## Parameters Tutorials {#sec_tutor_parameters}

### How to add a new global parameter {#sec_new_global_parameter}

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

### How to add a new HFT parameter {#sec_new_hft_parameter}

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

\see \ref sec_design_parameters

## Output Tutorials {#sec_output_tutor}

\todo Rewrite output tutorials.

### How to add a new output variable {#sec_new_output}
<!--
- Create a new `TableFile` member variable in \ref GuessOutput::HerbivoryOutput.
	+ Initialize it in the constructor.
	(The parameter and variable names must not contain a dot, space, or underscore.)
	+ Add it in the private function `HerbivoryOutput::init_tablefiles()`.
	- The instruction file parameter name for the output file name will be autogenerated in \ref GuessOutput::HerbivoryOutput::HerbivoryOutput().
	Add this parameter to your instruction file and to the default example file: `examples/output.ins`.
- Extend the appropriate container: \ref FaunaOut::HabitatData or \ref FaunaOut::HerbivoreData by a new member variable and initialize it in the constructor with zero values.
	+ Implement average building in the appropriate `merge()` function.
	+ For herbivore data, you need to add it to \ref FaunaOut::HerbivoreData::create_datapoint(). If your value is *per individual*, you don’ TODO
	+ Assign a value to the variable somewhere in daily simulation.
- Map this container variable to the `TableFile` by actually writing output in \ref GuessOutput::HerbivoryOutput::write_datapoint().

\note In the case of variables for the whole habitat (not specific to forage type, HFTs etc.), use the existing `TableFile` object `TBL_HABITAT`.
Add your new variable as a column to the table in \ref GuessOutput::HerbivoryOutput::get_columns() and add the value in \ref GuessOutput::HerbivoryOutput::write_datapoint(), *in the same order as the columns*.

\note If you want to add a variable that is not *per herbivore mass*, you would have to use mass density as weight.

### How to limit output to a specific time period {#sec_limit_output}

Adjust the return value of \ref GuessOutput::HerbivoryOutput::is_today_included() to your needs.
For comparison, see the local function `outlimit()` in \ref commonoutput.cpp.

\see \ref sec_design_output

-->


## How to add a new test vegetation model {#sec_new_testhabitat}

By default, the class \ref FaunaSim::SimpleHabitat is used for all test simulations.
If you want to run your own vegetation model, create it in \ref FaunaSim::Framework::create_habitat().

------------------------------------------------------------

\author Wolfgang Traylor, Senckenberg BiK-F
\date May 2017
\see \ref page_design
\see \ref page_model
\see \ref page_tests
