# Software Design {#page_design}
<!-- For doxygen, this is the *page* header -->
\brief Notes on the software design of the herbivore model from a programmer’s perspective.

# Software Design {#sec_design}
<!-- For doxygen, this is the *section* header -->
\tableofcontents

## Overview {#sec_design_overview}

The megafauna model aims to apply principles of object-oriented programming as much as possible (see the page [Object-oriented Programming](\ref page_object_orientation)).
Its architecture is modular and extensible.
Each part can be tested in unit tests.

The following UML diagram shows through which interfaces the megafauna model interacts with other components:

@startuml "Component diagram of the basic interactions of the megafauna model."
	!include diagrams.iuml!basic_components
@enduml

The basic simulation design is simple:
- Each **herbivore** is of one **Herbivore Functional Type** ([HFT](\ref Fauna::Hft)).
  In LPJ-GUESS this would correspond to each `Individual` being of one `Pft` (Plant Functional Type).
- Each herbivore lives in a **habitat** (\ref Fauna::Habitat), grouped by HFT in **populations** (\ref Fauna::PopulationInterface).
  In LPJ-GUESS this would correspond to a plant `Individual` growing in a `Patch`.
- **Fauna::World** is the framework class running the simulation.

@startuml "Most important classes in the megafauna model."
	!include diagrams.iuml!important_classes
@enduml

All interactions between herbivores and their environment happen through \ref Fauna::Habitat.
The herbivores don’t feed themselves and don’t have any direct connection to the habitat.
Instead, they are assigned their forage.
With each simulated day (\ref Fauna::HerbivoreInterface::simulate_day()), it is calculated, how much forage they would like to consume (\ref Fauna::HerbivoreInterface::get_forage_demands()).
Based on all forage demands, the forage is distributed with a user-specified forage distribution algorithm (\ref Fauna::DistributeForage).
Then they can eat their portion (\ref Fauna::HerbivoreInterface::eat()).
This approach follows the [Inversion of Control Principle](\ref sec_inversion_of_control).

Similarly, the Habitat does not interact with the herbivores either.
It does not even *know* about the herbivore populations, as it is capsuled in \ref Fauna::SimulationUnit.

## Forage Classes {#sec_design_forage_classes}

The model is designed to make implementation of multiple types of forage (like grass, browse, moss, etc.) easy.
Each forage type is listed in \ref Fauna::ForageType.
The global constant \ref Fauna::FORAGE_TYPES holds all entries of this enum so that it’s easy to iterate over them.

The template class \ref Fauna::ForageValues serves as a multi-purpose container for any forage-specific values.
Many arithmetic operators are defined to perform calculations over all forage types at once.
For any specific use of the class, a semantic `typedef` is defined, e.g. \ref Fauna::ForageMass or \ref Fauna::Digestibility.
This helps to directly see in the code what a variable contains.

Forage types need to have specific model properties.
Grass, for instance, has the property *sward density,* which would not make sense for leaves of trees.
Therefore, a second set of forage classes is defined with one class for each forage type.
All these classes inherit from \ref Fauna::ForageBase.

Any forage properties are defined by the habitat implementation in \ref Fauna::Habitat::get_available_forage().
They can be used for example in algorithms of:

- forage distribution (\ref Fauna::DistributeForage),
- diet composition (\ref Fauna::GetForageDemands::get_diet_composition),
- digestion limits (\ref Fauna::GetForageDemands::get_max_digestion), or
- foraging limits (\ref Fauna::GetForageDemands::get_max_foraging).

@startuml "Forage classes in the megafauna model."
	!include diagrams.iuml!forage_classes
@enduml

\see \ref sec_new_forage_type

## The Herbivore {#sec_design_the_herbivore}

The simulation framework can operate with any class that implements \ref Fauna::HerbivoreInterface (compare \ref sec_liskov_substitution).
Which class to choose is defined by the instruction file parameter \ref Fauna::Parameters::herbivore_type.

Currently, two classes, \ref Fauna::HerbivoreIndividual and \ref Fauna::HerbivoreCohort, are implemented.
Their common model mechanics are defined in their abstract parent class, \ref Fauna::HerbivoreBase.

The herbivore model performs calculations generally *per area* and not per individual.
That’s why individual herbivores can only be simulated if an absolute habitat area size is defined.
That is done by the parameter \ref Fauna::Parameters::habitat_area_km2.

@startuml "Class diagram of the two default herbivore classes (for individual and cohort mode), which share the same model mechanics defined in Fauna::HerbivoreBase."
	!include diagrams.iuml!herbivore_classes
@enduml

\see \ref sec_new_herbivore_class

### HerbivoreBase {#sec_herbivorebase}
The herbivore class itself can be seen as a mere framework (compare \ref sec_inversion_of_control) that integrates various components:

- The herbivore’s own **energy budget**: \ref Fauna::FatmassEnergyBudget.
- Its **energy needs**, defined by \ref Fauna::Hft::expenditure_components.
The herbivore object is self-responsible to call the implementation of the given expenditure models.
(A strategy pattern would not work here as different expenditure models need to know different variables.)
- How much the herbivore **is able to digest** is limited by a single algorithm defined in \ref Fauna::Hft::digestion_limit.
- How much the herbivore **is able to forage** can be constrained by various factors which are defined as a set of \ref Fauna::Hft::foraging_limits.
- The **diet composition** (i.e. feeding preferences in a scenario with multiple forage types) is controlled by a the model selected in \ref Fauna::Hft::foraging_diet_composer, whose implementation should be called in \ref Fauna::GetForageDemands::get_diet_composition().
- How much **net energy** the herbivore is able to gain from feeding on forage is calculated by an implementation of \ref Fauna::GetNetEnergyContentInterface
(given by [constructor injection](\ref sec_inversion_of_control)).
- **Death** of herbivores is controlled by a set of \ref Fauna::Hft::mortality_factors.
For a cohort that means that the density is proportionally reduced.
For an individual, death is a stochastic event.
The corresponding population objects will release dead herbivore objects automatically.

@startuml "Model components around Fauna::HerbivoreBase. Each component is selected by an HFT enum parameter by the user through the instruction file. The herbivore class then creates/calls the appropriate classes and functions."
	!include diagrams.iuml!herbivorebase_compartments
@enduml

### Populations {#sec_populations}
Each herbivore class needs a specific population class, implementing \ref Fauna::PopulationInterface, which manages a list of class instances of the same HFT.
Each habitat (\ref Fauna::Habitat) is populated by herbivores.
The class \ref Fauna::SimulationUnit contains a habitat and the herbivore populations (\ref Fauna::PopulationList).

A herbivore population instantiates new herbivore objects in the function \ref Fauna::PopulationInterface::establish().
For cohort and individual herbivores, there are simple helper classes to construct new objects: \ref Fauna::CreateHerbivoreCohort and \ref Fauna::CreateHerbivoreIndividual.
The `establish()` function is called by the simulation framework (\ref Fauna::World).
In this design, the framework is only responsible for triggering the spawning of herbivores.
How the reproduce and die is managed by the herbivore class itself, and the corresponding population and creator class.

@startuml "Herbivore population classes."
	!include diagrams.iuml!population_classes
@enduml

## Error Handling {#sec_design_error_handling}

### Exceptions {#sec_design_exceptions}
The library uses the C++ standard library exceptions defined in `<stdexcept>`.
All exceptions are derived from `std::exception`:

@startuml "Standard library exceptions used in the megafauna library."
	!include diagrams.iuml!exception_classes
@enduml

Any function that potentially *creates* an exception declares that in its doxygen description.

\warning Beware that any function—unless documented otherwise—will not catch exceptions from calls to other functions.
Therefore, even if a function does not announce a potential exception throw in its documentation, exceptions created in other functions can arise.

Exceptions are used…:
- …to check if the TOML instruction file is correct.
- …to check if parameters in public methods are valid.
- …to check the validity of variables coming from outside of the herbivory module where there are no contracts defined and ensured.

You throw an exception (in this case class `std::invalid_argument`) like this:

    ```cpp
    if (/*error occurs/*)
        throw std::invalid_argument("My error message");
    ```

Each class makes no assumptions about the simulation framework (e.g. that parameters have been checked), but solely relies on the class contracts in the code documentation.

The megafauna library will never exit the program or print messages to STDERR or STDOUT.
No exceptions arising in the library will be handled.
The host program is responsible to handle exceptions from the library and to inform the user and halt the program.

Exception messages in the megafauna library should start with the fully qualified function name (including namespace) of the function that is creating the exception.

\remark
If you debug with [`gdb`](https://www.gnu.org/software/gdb) and want to backtrace an exception, use the command `catch throw`.
That forces gdb to stop at an exception, and then you can use the command `backtrace` to see the function stack.

### Assertions {#sec_design_assertions}
At appropriate places, `assert()` is called (defined in the standard library header `<cassert>`/`assert.h`).
`assert()` calls are only expanded by the compiler if compilation happens for DEBUG mode; in RELEASE, they are completely ignored.

Assertions are used…:
- …within non-public methods to check within-class functionality.
- …to verify the result of an algorithm within a function.
- …in code regions that might be expanded later: An assert call serves as a reminder for the developer to implement all necessary dependencies.

## Parameters {#sec_design_parameters}

All user-defined simulation parameters are contained in the two classes \ref Fauna::Hft and \ref Fauna::Parameters.
All parameters must be constant within one simulation run.
Since some classes work with pointers to the classes \ref Fauna::Hft, \ref Fauna::HftList, and \ref Fauna::Parameters, all objects of these classes must not be moved in memory.

The host program only passes the path to the TOML instruction file to the class \ref Fauna::World.
The parameters are parsed by the megafauna library independently, using [cpptoml](https://github.com/skystrife/cpptoml).
This is done by the class \ref Fauna::InsfileReader.

Following the [Inversion of Control](\ref sec_inversion_of_control) principle, as few classes as possible have direct access to the classes that hold the parameters.
These classes play the role of the “framework”: They call any client classes _only_ with the very necessary parameters instead of the complete \ref Fauna::Hft or \ref Fauna::Parameters objects.
The following diagram gives an overview:

@startuml "Classes that have direct access to the parameter-holding classes Fauna::Hft and Fauna::Parameters."
	!include diagrams.iuml!parameters_access
@enduml

## Output {#sec_design_output}

### Output Classes {#sec_design_output_classes}

Output classes within the herbivory module are collected in the namespace \ref Fauna::Output.
- The two structs \ref Fauna::Output::HabitatData and \ref Fauna::Output::HerbivoreData are simple data containers.
- The struct \ref Fauna::Output::CombinedData represents one datapoint (‘tupel’/‘observation’) of all output variables in space and time.

@startuml "Output classes of the herbivory module."
	!include diagrams.iuml!output_classes
@enduml

There are three levels of data aggregation:

1) Each day in \ref Fauna::World::simulate_day(), a new set of output data (\ref Fauna::Output::CombinedData) is created for each simulation unit.
For this, the habitat data is taken as is, but the herbivore data is aggregated per HFT (see \ref Fauna::Output::HerbivoreData::create_datapoint()).
This level of aggregation is **spatial within one habitat**.
Sums and averages are calculated.
Any variables *per habitat* or *per area* are summed, for instance herbivore densities.
Variables *per individual* are averaged, using individual density as weight.

2) The second level of aggregation happens also in \ref Fauna::World::simulate_day().
The datapoint for that day is added to the temporal average in the \ref Fauna::SimulationUnit object.
This level of aggregation is therefore **temporal across days**.

3) The third level of aggregation takes place in \ref Fauna::Output::Aggregator.
Here, the accumulated temporal averages from the simulation units are combined in spatial aggregation units (\ref Fauna::Output::Datapoint::aggregation_unit).
This level of aggregation is therefore **spatial across habitats**.

For the latter two aggregation levels the function \ref Fauna::Output::CombinedData::merge() is used.

\note
All time-dependent variables are always **per day.**
For example, there is no such thing like *forage eaten in one year.*
This way, all variables can be aggregated using the same algorithm, whether they are time-independent (like *individual density*) or represent a time-dependent rate (like *mortality* or *eaten forage*).


#### Pros and Cons of the Output Design {#sec_output_prosandcons}

The pros of this design:

- Simplicity: There are only few classes.
- Separation of concerns: Each class (herbivores and habitats) is self-responsible for managing its own output, and the output data containers are self-responsible for aggregating their data.
- Diversity of data structures: There is no restriction in regards to data type for new member variables in the output containers (as long as they can be merged).

The cons of this design:

- Strong coupling: The output module is highly dependent on the data structure of the output containers.
- Rigidity of data containers: Ideally, the containers should be oblivious to the details of the data they hold.
- Lack of modularity: A submodule of, e.g. \ref Fauna::HerbivoreBase cannot easily deliver its own output variable.
- Cumbersome extensibility: New output variables need to be introduced in various places (see \ref sec_new_output_variable).
That is a violation of the [Open/Closed Principle](\ref sec_open_closed).
- Any variable that is specific to a submodule or interface implementation (e.g. `bodyfat` is specific to \ref Fauna::HerbivoreBase) will produce undefined values if that submodule is not active.
The user is then responsible to interpret them as invalid or disable their output.
So far, there is no check of congruency between [parameters](\ref Fauna::Parameters)/[HFT settings](\ref Fauna::Hft) and the selection of output variables in the output module.

\todo
The output data classes are currently always in a consistent state. With every new datum, the total average is recalculated.
This is a great waste of computing power.
It would be a lot more efficient to _first_ gather a long series of data, and _finally_ calculate the mean or sum.

------------------------------------------------------------

\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
