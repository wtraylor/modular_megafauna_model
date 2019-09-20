# Software Design of the Megafauna Model {#page_design}
<!-- For doxygen, this is the *page* header -->
\brief Notes on the software design of the herbivore model from a programmer’s perspective.

# Software Design of the Herbivory Module {#sec_design}
<!-- For doxygen, this is the *section* header -->
\tableofcontents

## Overview {#sec_design_overview}

The megafauna model aims to apply principles of object-oriented programming as much as possible (see the page [Object-oriented Programming](\ref page_object_orientation)).
Its architecture is modular and extensible.
Each part can be tested in unit tests (see the page [Unit Tests](\ref page_tests)).

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

## Forage Classes {#sec_forageclasses}

The model is designed to make implementation of multiple types of forage (like grass, browse, moss, etc.) easy.
Each forage type is listed in \ref Fauna::ForageType.
The global constant \ref Fauna::FORAGE_TYPES holds all entries of this enum.

The template class \ref Fauna::ForageValues serves as a multi-purpose container for any forage-specific values.
Many arithmetic operators are defined to perform calculations over all forage types at once.
For any specific use of the class, a typedef is defined, e.g. \ref Fauna::ForageMass or \ref Fauna::Digestibility.
This helps to directly see in the code what a variable contains.

Forage types need to have specific model properties.
Grass, for instance, has the property *sward density,* which would not make sense for leaves of trees.
Therefore, a second set of forage classes is defined with one class for each forage type.
All these classes inherit from \ref Fauna::ForageBase.

Any type-specific properties are defined by \ref Fauna::Habitat::get_available_forage().
They can be used for example in algorithms of
[forage distribution](\ref Fauna::DistributeForage),
[diet composition](\ref Fauna::GetForageDemands::get_diet_composition),
[digestion limits](\ref Fauna::GetForageDemands::get_max_digestion), or
[foraging limits](\ref Fauna::GetForageDemands::get_max_foraging).

@startuml "Forage classes in the herbivory module."
	!include diagrams.iuml!forage_classes
@enduml

\see \ref sec_new_forage_type

## The Herbivore {#sec_herbivoredesign}

The simulation framework of the herbivory module can operate with any class that implements \ref Fauna::HerbivoreInterface (\ref sec_liskov_substitution).
Which class to choose is defined by \ref Fauna::Parameters::herbivore_type.

Currently, two classes, \ref Fauna::HerbivoreIndividual and \ref Fauna::HerbivoreCohort, are implemented.
Their common model mechanics are defined in their abstract parent class, \ref Fauna::HerbivoreBase (see below).
The herbivore model performs calculations generally *per area.*
That’s why individual herbivores can only be simulated if an absolute habitat area size is defined.<!--TODO: Where is it defined-->

@startuml "Class diagram of the two default herbivore classes (for individual and cohort mode), which share the same model mechanics defined in Fauna::HerbivoreBase."
	!include diagrams.iuml!herbivore_classes
@enduml

\see \ref sec_new_herbivore_class

### HerbivoreBase {#sec_herbivorebase} ###
The herbivore class itself can be seen as a mere framework (compare \ref sec_inversion_of_control) that integrates various compartments:
- The herbivore’s own **energy budget**: \ref Fauna::FatmassEnergyBudget.
- Its **energy needs**, defined by \ref Fauna::Hft::expenditure_components.
The herbivore is self-responsible to call the implementation of the given expenditure models.
(A strategy pattern would not work here as different expenditure models need to know different variables.)
- How much the herbivore **is able to digest** is limited by a single algorithm defined in \ref Fauna::Hft::digestive_limit.
- How much the herbivore **is able to forage** can be constrained by various factors which are defined as a set of \ref Fauna::Hft::foraging_limits.
- The **diet composition** (i.e. feeding preferences in a scenario with multiple forage types) is controlled by a the model selected in \ref Fauna::Hft::diet_composer, whose implementation may be called in \ref Fauna::GetForageDemands::get_diet_composition().
- How much **net energy** the herbivore is able to gain from feeding on forage is calculated by an implementation of \ref Fauna::GetNetEnergyContentInterface
([constructor injection](\ref sec_inversion_of_control)).
- **Death** of herbivores is controlled by a set of \ref Fauna::Hft::mortality_factors.
For a cohort that means that the density is proportionally reduced, for an individual, death is a stochastic event.
The corresponding population objects will release dead herbivore objects automatically.

@startuml "Model compartments around Fauna::HerbivoreBase."
	!include diagrams.iuml!herbivorebase_compartments
@enduml

### Populations {#sec_populations} ###
Each herbivore class needs a specific population class, implementing \ref Fauna::PopulationInterface, which manages a list of class instances of the same HFT.
Each [habitat](\ref Fauna::Habitat) is populated by herbivores.
The class \ref Fauna::SimulationUnit a habitat and its herbivores (managed by HFT in \ref Fauna::HftPopulationsMap).

@startuml "Herbivore population classes."
	!include diagrams.iuml!population_classes
@enduml

## Error Handling {#sec_errorhandling}

### Exceptions ### {#sec_exceptions}
The herbivory module uses the C++ standard library exceptions defined in `<stdexcept>`.
All exceptions are derived from `std::exception`:
@startuml "Standard library exceptions used in the herbivory module."
	!include diagrams.iuml!exception_classes
@enduml

Any function that potentially *creates* an exception declares that in its doxygen description.
Beware that any function—unless documented otherwise—will not catch exceptions from calls to other functions.
Therefore, even if a function does not announce a potential exception throw in its documentation, it will pass on any exceptions from other functions which it calls.

Exceptions are used…:
- …to check if parameters in public methods are valid.
- …to check the validity of variables coming from outside of the herbivory module where there are no contracts defined and ensured.

You throw an exception (in this case class `std::invalid_argument`) like this:

    ```cpp
    if (/*error occurs/*)
        throw std::invalid_argument("My error message");
    ```

Each class makes no assumptions about the simulation framework (e.g. that parameters have been checked), but solely relies on the class contracts in the code documentation.

Exceptions are caught with `try{…}catch(…){…}` blocks in:
- framework.cpp: function \ref framework()
- testsimulation.h: %main() function and \ref FaunaSim::Framework::run()

\note No part of the herbivory module writes directly to the shell output (stdout/stderr via `std::cout` or `std::cerr`, respectively), except for:
- FaunaSim::Framework
- Fauna::ParamReader

\remark
If you debug with [`gdb`](https://www.gnu.org/software/gdb) and want to backtrace an exception, use the command `catch throw`.
That forces gdb to stop at an exception, and then you can use the command `backtrace` to see the function stack.

### Assertions ### {#sec_assertions}
At appropriate places, `assert()` is called (defined in the standard library header `<cassert>`/`assert.h`).
`assert()` calls are only expanded by the compiler if compilation happens for DEBUG mode; in RELEASE, they are completely ignored.

Assertions are used…:
- …within non-public methods to check within-class functionality.
- …to verify the result of an algorithm within a function.
- …in code regions that might be expanded later: An assert call serves as a reminder for the developer to implement all necessary dependencies.

## Herbivory Parameters {#sec_parameters}

Following the [Inversion of Control](\ref sec_inversion_of_control) principle, as few classes as possible have direct access to the classes that hold the parameters (\ref Fauna::Hft, \ref Fauna::Parameters).
These classes play the role of the “framework” by calling any client classes only with the very necessary parameters.
The following diagram gives an overview:

@startuml "Classes of the herbivory simulation which have direct access to parameter-holding classes."
	!include diagrams.iuml!parameters_access
@enduml

## Herbivory Output {#sec_output}

### Output Classes {#sec_outputclasses}

Output classes within the herbivory module are collected in the namespace \ref Fauna::Output.
- The two structs \ref Fauna::Output::HabitatData and \ref Fauna::Output::HerbivoreData are simple data containers.
- The struct \ref Fauna::Output::CombinedData represents one datapoint (‘tupel’/‘observation’) of all output variables in space and time.

@startuml "Output classes of the herbivory module."
	!include diagrams.iuml!outputclasses
@enduml

There are three levels of data aggregation:

1) Each day in \ref Fauna::Simulator::simulate_day(), a new datapoint (\ref Fauna::Output::CombinedData) is created for each simulation unit.
For this, the habitat data is taken as is, but the herbivore data is aggregated per HFT (see \ref Fauna::Output::HerbivoreData::create_datapoint()).
This level of aggregation is **spatial within one habitat**.
Here, any variables *per habitat* or *per area* are summed, for instance herbivore densities.
Variables *per individual* are averaged, using individual density as weight.

2) The second level of aggregation happens also in \ref Fauna::Simulator::simulate_day().
The datapoint for that day is added to the temporal average in the \ref Fauna::SimulationUnit object.
This level of aggregation is therefore **temporal across days**.

3) The third level of aggregation takes place in \ref GuessOutput::OutputModule.
Here, the accumulated temporal averages from the simulation units are combined in spatial units.
This level of aggregation is therefore **spatial across habitats**.

The latter two aggregation levels are performed by \ref Fauna::Output::CombinedData::merge().

Note that all time-dependent variables are always **per day.**
For example, there is no such thing like *forage eaten in one year.*
This way, all variables can be aggregated using the same algorithm, whether they are time-independent (like *individual density*) or represent a time-dependent rate (like *mortality* or *eaten forage*).


#### Pros and Cons of the Output Design {#sec_output_prosandcons}

The pros of this design:
- Simplicity: Only few, easy-to-understand classes.
- Separation of concerns: Each class (herbivores and habitats) is self-responsible for managing its own output, and the output data containers are self-responsible for aggregating their data.
- Diversity of data structures: There is no restriction in regards to data type for new member variables in the output containers (as long as they can be merged).

The cons of this design:
- Strong coupling: The output module is highly dependent on the data structure of the output containers.
- Rigidity of data containers: Ideally, the containers should be oblivious to the details of the data they hold.
- Lack of modularity: A submodule of, e.g. \ref Fauna::HerbivoreBase cannot easily deliver its own output variable.
- Cumbersome extensibility: New output variables need to be introduced in various places (see \ref sec_new_output).
That is a violation of the [Open/Closed Principle](\ref sec_open_closed).
- Any variable that is specific to a submodule or interface implementation (e.g. `bodyfat` is specific to \ref Fauna::HerbivoreBase) will produce undefined values if that submodule is not active.
The user is then responsible to interpret them as invalid or disable their output.
So far, there is no check of congruency between [parameters](\ref Fauna::Parameters)/[HFT settings](\ref Fauna::Hft) and the selection of output variables in the output module.

------------------------------------------------------------

\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
