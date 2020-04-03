# Tutor {#page_tutor}
\brief Instructions how expand the code base for your own needs.

\tableofcontents

## Herbivores Tutorials {#sec_tutor_herbivores}

### How to add a new herbivore class {#sec_new_herbivore_class}

The model design allows a complete substitution of the herbivore class.
If you want to implement a completely new model behaviour, you can derive your new class from \ref Fauna::HerbivoreInterface and write it from scratch.
If you want to build upon the base functionality, derive it from \ref Fauna::HerbivoreBase.

Then, derive a new class from \ref Fauna::PopulationInterface to manage and construct your object instances.
In \ref Fauna::WorldConstructor::create_populations(), create all instances of that population class for one habitat.

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

- Add a new enum entry in \ref Fauna::DigestiveLimit.
- Read the new value for `digestion.limit` from the TOML instruction file in \ref Fauna::InsfileReader::read_hft().
- Implement your digestive limit algorithm as a free function or an object. If it is not much code, put it in \ref foraging_limits.h, otherwise create a new file for it.
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

- Derive new class from \ref Fauna::ForageBase.
	+ Implement a `merge()` method, like \ref Fauna::GrassForage::merge().

- Add a new member variable in \ref Fauna::HabitatForage of that class.
	+ Add it in \ref Fauna::HabitatForage::operator[]().
	+ Call your `merge()` function in \ref Fauna::HabitatForage::merge().

- Adjust the implementation of \ref Fauna::Habitat::get_available_forage() and \ref Fauna::Habitat::remove_eaten_forage() in your vegetation model.

- For \ref Fauna::NetEnergyModel::GrossEnergyFraction you will need to define a value for the parameter `forage.gross_energy.new_forage_type` in the TOML file.

- Herbivores
	+ Check all foraging and digestion limits (\ref foraging_limits.h) whether they need to be expanded.
	+ Probably you will need to implement [a new diet composer](\ref sec_new_diet_composer) or adjust existing ones.

- Test Simulations
	+ If you want to use your forage type in the demo simulations, expand \ref Fauna::Demo::SimpleHabitat by a new growth model (analoguous to \ref Fauna::Demo::LogisticGrass).
	Also update the UML diagram in the class documentation of `SimpleHabitat`.

@startuml "Relationships for a new forage type."
	!include diagrams.iuml!new_forage_type
@enduml

### How to change forage net energy content {#sec_change_netenergy}

- Implement your model in a function or class in the file \ref net_energy_models.h.
- Add a new enum item in \ref Fauna::NetEnergyModel.
- Parse that value for the HFT parameter `digestion.net_energy_model` in \ref Fauna::InsfileReader::read_hft().
- Execute your function or class in \ref Fauna::HerbivoreBase::get_net_energy_content().
- Update the UML diagram in \ref sec_herbivorebase.

### How to add a new forage distribution algorithm {#sec_new_forage_distribution}

- Derive a new class from \ref Fauna::DistributeForage and implement your algorithm. If the algorithm is not too big, put it in \ref forage_distribution_algorithms.h, otherwise create a new set of files.
- Return a reference to an object of your class in \ref Fauna::WorldConstructor::create_distribute_forage() if it is selected in the parameters.
- Add a new enum entry in \ref Fauna::ForageDistributionAlgorithm.
- Parse your new value from the parameter `simulation.forage_distribution` in \ref Fauna::InsfileReader::read_hft().

## Parameters Tutorials {#sec_tutor_parameters}

### How to add a new global parameter {#sec_new_global_parameter}

- Declare your parameter as a member variable in \ref Fauna::Parameters and initialize it with a valid default value. The `enum class` type for Enum parameters should be declared in \ref parameters.h.
- Write a validity check in \ref Fauna::Parameters::is_valid().
- Parse the parameter from the TOML instruction file in \ref Fauna::InsfileReader. General parameters are parsed in Fauna::InsfileReader::read_table_simulation().
    + If you are creating a whole new set of parameters, it might make sense to group them in a TOML table. Existing tables are `output` and `simulation`. You should then write a new private member function similar to \ref Fauna::InsfileReader::read_table_simulation().

### How to add a new HFT parameter {#sec_new_hft_parameter}

- Declare a new member variable in \ref Fauna::Hft and initialize it with a valid default value. The `enum class` type for Enum parameters should be declared in \ref hft.h.
- Write a validity check in \ref Fauna::Hft::is_valid().

\see \ref sec_design_parameters

## Output Tutorials {#sec_output_tutor}

### How to add a new output variable {#sec_new_output_variable}
After separating the megafauna model from LPJ-GUESS into its own library only a minimal set of variables are made available for output.
If your variable of interest is already present in \ref Fauna::Output::HabitatData or \ref Fauna::Output::HerbivoreData, then you can skip the first step in the following tutorial.

- Extend the appropriate container: \ref Fauna::Output::HabitatData or \ref Fauna::Output::HerbivoreData by a new member variable and initialize it with zero.
    + Add it in the `reset()` function.
	+ Implement average building in the appropriate `merge()` function.
	+ For herbivore data, you need to add it to \ref Fauna::Output::HerbivoreData::create_datapoint(). If your value is *per individual*, you will need to weight the value by individual density; if it is *per area* or *per habitat*, you can calculate the sum.
	+ Assign a value to the variable somewhere in daily simulation.

- Write the variable in \ref Fauna::Output::TextTableWriter.
    + Add a selector for your new output file as a boolean member variable in \ref Fauna::Output::TextTableWriterOptions. Pay attention to place it in the right Doxygen group.
    + Parse the name of the selector in \ref Fauna::InsfileReader::read_table_output_text_tables() and add it there to the valid options in the error message.
    + Consider adding it in the example output in the file `examples/megafauna.toml`.
    + Add an output file stream (`std::ofstream`) for your variable as a private member variable in \ref Fauna::Output::TextTableWriter.
    + In the constructor \ref Fauna::Output::TextTableWriter::TextTableWriter(), add your new output file to the list of file streams if it is selected in the options.
    + Initialize the column captions of your new file in \ref Fauna::Output::TextTableWriter::write_captions().
    + Write the data to the file in \ref Fauna::Output::TextTableWriter::write_datapoint().

\note If you want to add a variable that is not *per herbivore mass*, you would have to use mass density as weight.

### How to write output to another format {#sec_new_output_writer}
The default output format are very simple tab-separated plaintext tables.
The class \ref Fauna::Output::TextTableWriter is responsible for this format.
However, you can also replace that output format with another one, for instance writing to a NetCDF file or forwarding it to another program or library.

- Derive a new class from \ref Fauna::Output::WriterInterface.
- Add a new enum entry to \ref Fauna::OutputFormat.
- Parse the new option in \ref Fauna::InsfileReader::read_table_output().
- Create a new instance of your writer class in the constructor \ref Fauna::World::World() if your enum entry is selected.

\see \ref sec_design_output design

------------------------------------------------------------

\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
\copyright ...
