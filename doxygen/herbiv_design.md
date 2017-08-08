Software Design of the Herbivory Module {#page_herbiv_design}
=============================================================
<!-- For doxygen, this is the *page* header -->
\brief Notes on the software design of the LPJ-GUESS herbivory module from a technical perspective.

Software Design of the Herbivory Module {#sec_herbiv_design}
============================================================
<!-- For doxygen, this is the *section* header -->
\tableofcontents

Overview {#sec_herbiv_designoverview}
-------------------------------------

@startuml "Component diagram of the basic interactions of the herbivory module"
	!include herbiv_diagrams.iuml!basic_components
@enduml


@startuml "Most important simulation classes in the Herbivory Module and their connection to LPJ-GUESS"
	!include herbiv_diagrams.iuml!important_classes
@enduml

@startuml "Construction of PatchHabitat"
	!include herbiv_diagrams.iuml!patchhabitat_construction
@enduml

Forage Classes {#sec_herbiv_forageclasses}
------------------------------------------

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
[diet composition](\ref Fauna::ComposeDietInterface), or
[foraging limits](\ref herbiv_foraging.h).

@startuml "Forage classes in the herbivory module."
	!include herbiv_diagrams.iuml!forage_classes
@enduml

\see \ref sec_herbiv_new_forage_type

The Herbivore {#sec_herbiv_herbivoredesign}
-------------------------------------------

The simulation framework of the herbivory module can operate with any class that implements \ref Fauna::HerbivoreInterface (\ref sec_liskov_substitution).
Which class to choose is defined by \ref Fauna::Parameters::herbivore_type.

Currently, two classes, \ref Fauna::HerbivoreIndividual and \ref Fauna::HerbivoreCohort, are implemented.
Their common model mechanics are defined in their abstract parent class, \ref Fauna::HerbivoreBase (see below).
The herbivore model performs calculations generally *per area.*
That’s why individual herbivores can only be simulated if an absolute habitat area size is defined.<!--TODO: Where is it defined-->

@startuml "Herbivore classes"
	!include herbiv_diagrams.iuml!herbivore_classes
@enduml

### HerbivoreBase {#sec_herbiv_herbivorebase} ### 
The herbivore class itself can be seen as a mere framework (compare \ref sec_inversion_of_control) that integrates various compartments:
- The herbivore’s own **energy budget**: \ref Fauna::FatmassEnergyBudget.
- Its **energy needs**, defined by \ref Fauna::Hft::expenditure_model.
The herbivore is self-responsible to call the implementation of the given expenditure model.
(A strategy pattern would not work here as different expenditure models need to know different variables.)
- How much the herbivore **is able to forage** can be constrained by various factors which are defined as a set of \ref Fauna::Hft::foraging_limits.
- The **diet composition** (i.e. feeding preferences in a scenario with multiple forage types) is controlled by a [strategy](\ref sec_strategy) object that implements \ref Fauna::ComposeDietInterface.
HerbivoreBase instantiates the object itself according to \ref Fauna::Hft::diet_composer.
- How much **net energy** the herbivore is able to gain from feeding on forage is calculated by an implementation of \ref Fauna::GetNetEnergyContentInterface 
([constructor injection](\ref sec_inversion_of_control)).
- **Death** of herbivores is controlled by a set of \ref Fauna::Hft::mortality_factors. 
For a cohort that means that the density is proportionally reduced, for an individual, death is a stochastic event.
The corresponding population objects will release dead herbivore objects automatically.

@startuml "Model compartments around Fauna::HerbivoreBase"
	!include herbiv_diagrams.iuml!herbivorebase_compartments
@enduml

### Populations {#sec_herbiv_populations} ### 
Each herbivore class needs a specific population class, implementing \ref Fauna::PopulationInterface, which manages a list of class instances of the same HFT.
Each [habitat](\ref Fauna::Habitat) has a \ref Fauna::HftPopulationsMap which manages a number of population instances, one for each HFT.

@startuml "Herbivore population classes"
	!include herbiv_diagrams.iuml!population_classes
@enduml



Error Handling {#sec_herbiv_errorhandling}
------------------------------------------

### Exceptions ### {#sec_herbiv_exceptions}
The herbivory module uses the C|| standard library exceptions defined in `<stdexcept>`.
All exceptions are derived from `std::exception`:
@startuml "Standard library exceptions used in the herbivory module."
	!include herbiv_diagrams.iuml!exception_classes
@enduml

Any function that potentially *creates* an exception declares that in its doxygen description.
Beware that any function—unless documented otherwise—will not catch exceptions from calls to other functions.
Therefore, even if a function does not announce a potential exception throw in its documentation, it will pass on any exceptions from other functions which it calls.

Exceptions are used…:
- …to check if parameters in public methods are valid.
- …to check the validity of variables coming from outside of the herbivory module where there are no contracts defined and ensured.

Each class makes no assumptions about the simulation framework (e.g. that parameters have been checked), but solely relies on the class contracts in the code documentation.

Exceptions are caught with `try{…}catch(…){…}` blocks in:
- framework.cpp: function \ref framework()
- herbiv_testsimulation.h: %main() function and \ref FaunaSim::Framework::run()

\note No part of the herbivory module writes directly to the shell output (stdout/stderr), except for:
- FaunaSim::Framework
- Fauna::ParamReader

### Assertions ### {#sec_herbiv_assertions}
At appropriate places, `assert()` is called (defined in the standard library header `<cassert>`/`assert.h`).
`assert()` calls are only expanded by the compiler if compilation happens for DEBUG mode; in RELEASE, they are completely ignored.

Assertions are used…: 
- …within non-public methods to check within-class functionality.
- …to verify the result of an algorithm within a function.
- …in code regions that might be expanded later: An assert call serves as a reminder for the developer to implement all necessary dependencies.

Herbivory Parameters {#sec_herbiv_parameters}
---------------------------------------------

The herbivory module uses the same instruction files and plib 
(\ref plib.h) functionality as the vegetation model.
In order to separate concerns, all herbivory-related parameters
are declared and checked in the class \ref Fauna::Parameters, but parsed by the class \ref Fauna::ParamReader.
ParamReader is the only one being directly dependent on \ref parameters.h and \ref plib.h (apart from \ref FaunaSim::Framework and \ref GuessOutput::HerbivoryOutput).

The principle that parameter member variables put in one class, which also knows to check their validity, but parsed in ParamReader, is also applied in \ref Fauna::Hft and \ref Fauna::PftParams.

@startuml "Interactions of parameter-related classes in the herbivory module" 
	!include herbiv_diagrams.iuml!parameters_classes
@enduml

\note The implementation can be called a rather dirty fix around the inflexible design of LPJ-GUESS parameter library. 
Some global constants (checkback and block codes) and global pointers from \ref parameters.h and \ref parameters.cpp are used in \ref herbiv_parameters.cpp.

An example instruction file is provided in 
`data/ins/herbivores.ins`:
\snippet herbivores.ins Example Herbivore
\see \ref sec_herbiv_new_hft_parameter
\see \ref sec_herbiv_new_pft_parameter
\see \ref sec_herbiv_new_global_parameter

Following the [Inversion of Control](\ref sec_inversion_of_control) principle, as few classes as possible have direct access to the classes that hold the parameters (\ref Fauna::Hft, \ref Fauna::Parameters, \ref Fauna::PftParams).
These classes play the role of the “framework” by calling any client classes only with the very necessary parameters.
The following diagram gives an overview:

@startuml "Classes of the herbivory simulation which have direct access to parameter-holding classes."
	!include herbiv_diagrams.iuml!parameters_access
@enduml

\bug When printing out the help with \ref plibhelp() 
(by running `guess -help`), the global parameters declared in 
\ref Fauna::ParamReader::declare_parameters() under 
`BLOCK_GLOBAL` appear out of order in the output.



Herbivory Output {#sec_herbiv_output}
-------------------------------------

### Output Module {#sec_herbiv_outputmodule}

The new output module \ref GuessOutput::HerbivoryOutput is 
derived from the abstract class \ref GuessOutput::OutputModule.
The following diagram show 
<!--TODO: diagram-->
@startuml "Class diagram of the connections around class GuessOutput::HerbivoryOutput"
	!include herbiv_diagrams.iuml!outputmodule_class
@enduml


The following sequence diagram shows the creation process of \ref GuessOutput::HerbivoryOutput :

@startuml "Output initialization in LPJ-GUESS. All participating classes have only one instantiation, but only HerbivoryOutput implements formally the Singleton design pattern."
	!include herbiv_diagrams.iuml!outputmodule_initialization
@enduml


The output module \ref GuessOutput::HerbivoryOutput is used both in the standard LPJ-GUESS framework and in the test simulations
(\ref page_herbiv_tests).
If the parameter `ifherbivory` is 0, the whole class is deactivated and won’t produce any output or create files.

This is necessary because some herbivore module parameters, like `digestibility_model`, are not checked when reading the instruction file.

While the class \ref GuessOutput::HerbivoryOutput complies with the output module framework of LPJ-GUESS, a few technical improvements
to \ref GuessOutput::CommonOutput were made:
- Output interval can be chosen freely with one variable instead of different output files. 
The table structure stays always the same (no month columns).
- The functions are smaller and better maintainable.
- The preprocessing of the data (building averages etc.) is done in the data-holding classes. 
This approach honours the \ref sec_single_responsibility.
- The inherited functions delegate to more generic functions, which are also used by \ref FaunaSim::Framework.
- The class \ref GuessOutput::IncludeDate has been introduced in order to observe the \ref sec_dependency_inversion and to avoid global variables.
  See also: \ref sec_herbiv_limit_output.
- The \ref GuessOutput::OutputModuleRegistry instantiates the class. There is only one global instance, but there is no direct way to access that global instance like in the [Singleton design pattern](\ref sec_singleton).
	To circumvent this restriction (instead of working with a lot of `static` members) the function [get_instance()](\ref GuessOutput::HerbivoryOutput::get_instance()) has been introduced.
	To assert that no other instance can be created, the constructor throws an exception on second call. 

------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see \ref page_object_orientation
\see \ref page_herbiv_model
\see \ref page_herbiv_tutor
\see \ref page_herbiv_tests
\see \ref group_herbivory
