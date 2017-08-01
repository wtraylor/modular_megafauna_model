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

\startuml "Basic interactions of the herbivory module"
[Herbivory Module] as herbivory
() "Fauna::ParamReader" as paramreader
() "Fauna::PatchHabitat" as patchhabitat
() "GuessOutput::HerbivoryOutput" as herbivoutput
node "Herbivory Output Files" as outputfiles
[LPJ-GUESS] as guess
node "Instruction Files" as insfiles
guess        <..  patchhabitat
patchhabitat  --  herbivory
herbivory     --  herbivoutput 
herbivoutput  ..> outputfiles : write
herbivory     --  paramreader 
paramreader   ..> insfiles  : read
\enduml


@startuml "Basic simulation classes in the Herbivory Module and their connection to LPJ-GUESS"

hide members
hide methods

annotation "framework()" as framework
Gridcell "1" *--> "*" Stand
Stand    "1" *--> "*" Patch
Patch    "1" *--> "*" Individual
class Individual{
	Pft* pft
}
show Individual members

namespace Fauna{
	Simulator          ..> Habitat            : <<call>>
	.framework          ..> Simulator          : <<create>>
	PatchHabitat      <-right-* .Patch
	PatchHabitat       ..> .Patch              : plant–animal interactions
	Habitat           *..> HerbivoreInterface : contains>
	interface HerbivoreInterface {
		Hft* hft
	}
	show HerbivoreInterface members
	abstract Habitat
	Habitat            <|-left- PatchHabitat
}
@enduml

@startuml "Construction of PatchHabitat"
participant "framework()" as framework
framework    -> "Fauna::Simulator" : <<create>>
activate "Fauna::Simulator"
framework    -> Gridcell : <<create>>
activate Gridcell
Gridcell     -> Patch : <<create>>
activate Patch
framework    -> "Fauna::Simulator" : create_populations()
framework  <--  "Fauna::Simulator" : populations
framework    -> "Fauna::Simulator" : create_digestibility_model()
framework  <--  "Fauna::Simulator" : digestibility_model
framework    -> "Fauna::PatchHabitat" : <<create>> (populations, digestibility_model)
note right : constructor injection
activate "Fauna::PatchHabitat"
framework    -> Patch : set_habitat()
note right : setter injection
@enduml

Forage Classes {#sec_herbiv_forageclasses}
------------------------------------------

\todo Explain why different forage classes
\see \ref sec_herbiv_new_forage_type

The Herbivore {#sec_herbiv_herbivoredesign}
-------------------------------------------

The simulation framework of the herbivory module can operate with any class that implements \ref Fauna::HerbivoreInterface (\ref sec_liskov_substitution).
Which class to choose is defined by \ref Fauna::Parameters::herbivore_type.

Currently, two classes, \ref Fauna::HerbivoreIndividual and \ref Fauna::HerbivoreCohort, are implemented.
Their common model mechanics are defined in their abstract parent class, \ref Fauna::HerbivoreBase (see below).
The herbivore model performs calculations generally *per area.*
That’s why individual herbivores can only be simulated if an absolute habitat area size is defined.<!--TODO: Where is it defined-->

@startuml "Herbivore class relationships"
hide members
hide methods
interface HerbivoreInterface
interface PopulationInterface
class HerbivoreBase
HerbivoreInterface  <|-- HerbivoreBase
HerbivoreBase       <|-- HerbivoreIndividual
HerbivoreBase       <|-- HerbivoreCohort
PopulationInterface <|-up- IndividualPopulation
PopulationInterface <|-up- CohortPopulation
HerbivoreIndividual "*" <--* "1" IndividualPopulation
HerbivoreCohort     "*" <--* "1" CohortPopulation
HerbivoreIndividual  <..  IndividualPopulation : <<create>>
(HerbivoreIndividual, IndividualPopulation) .. CreateHerbivoreIndividual
HerbivoreCohort      <..  CohortPopulation     : <<create>>
(HerbivoreCohort, CohortPopulation) .. CreateHerbivoreCohort
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
hide members
hide methods
namespace Fauna{
class HerbivoreBase{
	-Hft hft
	-int age_days
}
show HerbivoreBase members
class FatmassEnergyBudget{
	+catabolize_fat()
	+metabolize_energy()
	-double energy_needs
	-double fatmass
	-double max_fatmass
}
show FatmassEnergyBudget members
show FatmassEnergyBudget methods
HerbivoreBase *-up-> "1" FatmassEnergyBudget
package "Forage Energy" <<rectangle>> {
	interface GetNetEnergyContentInterface <<strategy>>
	GetNetEnergyContentInterface <|-- GetNetEnergyContentDefault
}
HerbivoreBase *-up-> "1" "Forage Energy"
package "Reproduction" <<rectangle>> {
	class ReproductionIllius2000
}
HerbivoreBase .up.> "1" "Reproduction"
package "Diet" <<rectangle>> {
	interface ComposeDietInterface <<strategy>>
	ComposeDietInterface <|-- PureGrazerDiet
}
HerbivoreBase  .up.> "1" "Diet"
package "Energy Expenditure" <<rectangle>>  {
	annotation "get_expenditure_taylor_1981()"
}
HerbivoreBase  ..> "1" "Energy Expenditure"
package "Foraging Limits" <<rectangle>>  {
	class GetDigestiveLimitIllius1992 <<functor>>
	class GetHalfMaxForagingLimit     <<functor>>
} 
HerbivoreBase  ..> "*" "Foraging Limits"
package "Mortality" <<rectangle>> {
	class GetBackgroundMortality           <<functor>>
	class GetStarvationMortalityIllius2000 <<functor>>
	class GetStarvationMortalityThreshold  <<functor>>
	class GetSimpleLifespanMortality       <<functor>>
}
HerbivoreBase ..> "*" "Mortality"
}
@enduml

### Populations {#sec_herbiv_populations} ### 
Each herbivore class needs a specific population class, implementing \ref Fauna::PopulationInterface, which manages a list of class instances of the same HFT.
Each [habitat](\ref Fauna::Habitat) has a \ref Fauna::HftPopulationsMap which manages a number of population instances, one for each HFT.

@startuml "Herbivore population classes"
hide members
hide methods
namespace Fauna{
	interface PopulationInterface
	PopulationInterface <|-- IndividualPopulation
	PopulationInterface <|-- CohortPopulation
	IndividualPopulation *-- "*" HerbivoreIndividual
	CohortPopulation     *-- "*" HerbivoreCohort
	abstract Habitat
	Habitat *-- "*" PopulationInterface
	(Habitat, PopulationInterface) .. HftPopulationsMap
}
@enduml



Error Handling {#sec_herbiv_errorhandling}
------------------------------------------

### Exceptions ### {#sec_herbiv_exceptions}
The herbivory module uses the C++ standard library exceptions defined in `<stdexcept>`.
All exceptions are derived from `std::exception`:
@startuml "Standard library exceptions used in the herbivory module."
namespace std{
	hide members
	hide methods
	exception <|-- logic_error
	exception <|-- runtime_error
	logic_error <|-- invalid_argument
	logic_error <|-- out_of_range 
}
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
hide members
hide methods
annotation "framework()"       as framework
annotation "Instruction Files" as insfiles
annotation "plib.h"            as plib
annotation "parameters.h"      as parameters
class "FaunaSim.Framework" <<singleton>>
class "Fauna.ParamReader"   <<singleton>>
Pft                "1" *-- "1" Fauna.PftParams
framework           ..> Fauna.ParamReader : <<use>>
parameters          ..> Pft               : <<use>>
GuessOutput.HerbivoryOutput     ..> parameters        : <<use>>
parameters          ..> plib              : <<use>>
plib                ..> insfiles          : <<read>>
framework           ..> parameters        : <<use>>
namespace Fauna {
	ParamReader  <..> .parameters : <<call>>
	ParamReader   ..> .plib       : <<call>>
	ParamReader   ..> Hft         : <<create>>
	ParamReader   ..> PftParams   : <<call>>
	ParamReader   ..> Parameters  : <<create>>
	class Hft {
		+is_valid()
	}
	show Hft methods
	class Parameters {
		+is_valid()
	}
	show Parameters methods
	class PftParams {
		+is_valid()
	}
	show PftParams methods
}
FaunaSim.Framework ..> Fauna.ParamReader : <<use>>
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
hide members
hide methods
namespace Fauna{
	CreateHerbivoreIndividual ..> Parameters
	CreateHerbivoreIndividual ..> Hft
	CreateHerbivoreCohort     ..> Parameters
	CreateHerbivoreCohort     ..> Hft
	Simulator                 ..> Parameters
	Simulator                 ..> Hft
	PatchHabitat              ..> PftParams
	CohortPopulation          .up.> Hft
	IndividualPopulation      .up.> Hft
	HerbivoreBase             .up.> Hft
}
namespace FaunaSim{
	Framework                 ..> Fauna.Parameters
	Framework                 ..> Fauna.Hft
} 
@enduml

\bug When printing out the help with \ref plibhelp() 
(by running `guess -help`), the global parameters declared in 
\ref Fauna::ParamReader::declare_parameters() under 
`BLOCK_GLOBAL` appear out of order in the output.



Object-oriented Design {#sec_object_orientation}
------------------------------------------------

In the Herbivore Module a couple of object-oriented design patterns
were employed that are explained here along with general 
concepts of object-oriented programming.

### Good Programming Practice ### {#sec_good_practice}

#### Rule of Three #### {#sec_rule_of_three}
If a class explicitely defines at least one of the following methods, it should most likely also define the other ones:

- Destructor
- Copy Constructor
- Copy Assignment Operator

\note If moving to C++11, the **Rule of Five** becomes relevant

### S-O-L-I-D Design principles ### {#sec_design_solid}

#### Single Responsibility Principle {#sec_single_responsibility} 
A class should have only a single responsibility:
A class should have only one reason to change.

#### Open/Closed Principle {#sec_open_closed} 
A class/module/function should be open for extension, but
closed for modification.

#### Liskov’s Substitution Principle {#sec_liskov_substitution}
Objects in a program should be replaceable with instances of
their subtypes without altering the correctness of that program.

#### Interface Segregation Principle {#sec_interface_segregation}
Many client-specific interfaces are better than one 
general-purpose interface.

#### Dependency Inversion Principle {#sec_dependency_inversion}
1. High-level modules should not depend on low-level modules.
Both should depend on abstractions.
2. Abstractions should not depend on details. 
Details should depend on abstractions.


### Inversion of Control {#sec_inversion_of_control}
The design principle of inversion of control is also called
*Hollywood Principle:* “Don’t call us, we’ll call you.”
An architecture following that principle is built around a generic framework which directs the flow of control by delegating tasks to various, interchangeable submodules.
This approach makes the system more modular and extensible.

@startuml
hide members
hide methods
Framework ..> Client1 : <<create & call>>
Framework ..> Client2 : <<create & call>
@enduml

Inversion of control is related to the
[Dependency Inversion Principle](\ref sec_dependency_inversion), which different in that it is concerned about the relationship between high-level and low-level modules rather than the framework design.

#### Dependency Injection ####
Dependency Injection is a major technique to implement
inversion of control:
One object (the framework) supplies the dependencies for another object (the client), which then become a part of the client’s state.
This is a direct alternative to global objects/variables.

Two kinds of dependency injection are used
1. **Setter Injection:** A client receives its dependency *after* being constructed, via a setter method. This is dangerous, because until initialization through the setter method, the client might be in an invalid state. (Example: \ref Patch::set_habitat())
2. **Constructor Injection:** Any object of the client class receives its dependency in the constructor. (Example: \ref Fauna::PatchHabitat::PatchHabitat())

For example: The \ref Fauna::HftList object is not a global variable, but is instead being passed down from the \ref framework() function to \ref Fauna::Simulator.

In the case of \ref Fauna::Habitat, the \ref Fauna::Simulator is oblivious to the concrete realization of the interface, which makes it possible to substitute parts of the model without changing the framework. 


### Design Patterns

#### Singleton {#sec_singleton}
A class is called *singleton* if it permits only one global
instantiation in the program.
This object-oriented approach has advantages over global variables
because it is more generally flexible and the time of instantiation
is flexible.

The basic implementation is as follows:

    class MySingleton{
    public:
     static MySingleton& get_instance(){ 
 	     static MySingleton global_instance; // creates the instance on first call
 	     return global_instance;
     }
    private:
     MySingleton(){}                     // Constructor hidden from the outside
     MySingleton(MySingleton const&);    // deleted copy constructor
     void operator=(MySingleton const&); // deleted assignment constructor
    }

To get access to the instance or trigger the initial instantiation, use `MySingleton::get_instance();`.

Wrapping global variables and free functions into a singleton class is good, but it is better to *avoid singletons all together* and instead follow the principle of [Inversion of Control](sec_inversion_of_control).
A few classes in the herbivory module employ are built singleton in order to interact in a save manner with the existing LPJ-GUESS framework, namely the declaration of instruction file parameters.

The following classes of the herbivory module are singleton:
\ref FaunaSim::Framework,
\ref Fauna::ParamReader.
The principle of a global single class shows also up in 
\ref GuessOutput::HerbivoryOutput.


#### Strategy {#sec_strategy}
The strategy design pattern defines a set of interchangable algorithms, each of which are encapsulated in a class that is derived from one abstract interface parent class.
Thanks to C++ polymorphism, the used algorithm can change during runtime.
Here is a basic implementation:

    struct Strategy {
    	virtual operator()(const int param) const = 0;
    };
    
    struct StrategyOne: public Strategy {
    	virtual operator()(const int param) const{ /*...*/ };
    };
    
    struct StrategyTwo: public Strategy {
    	virtual operator()(const int param) const{ /*...*/ };
    };

@startuml
hide members
hide methods
interface Strategy
Strategy <|-- StrategyOne
Strategy <|-- StrategyTwo
Client --> Strategy : <<use>>
@enduml

\anchor sec_functors
In this example, the classes are implemented as *function objects* (**functors**) because they overload the `operator()`.
In contrast to simple functions, they could also hold state variables and make use of class inheritance.
(They should still be light-weight, though.)
Their implemented algorithm can be called by using the object like a function:

    Strategy* do_algorithm = new StrategyOne; // just substitute the instantiated class here
    
    int i = 1;
    do_algorithm(i); // this line does not need to change when the strategy is replaced.

**Naming Conventions:**
Obviously, the class name (and the names of their instances and pointers) should be verbs.
The class name should of course be capitalized.

Strictly speaking, the strategy pattern aims to offer the possibility to substitute an algorithm during the lifetime of a client object.
In the herbivory model that is usually not the case, but rather it is used as a means of [dependency injection](sec_dependency_inversion).


Herbivory Output {#sec_herbiv_output}
-------------------------------------

The new output module \ref GuessOutput::HerbivoryOutput is 
derived from the abstract class \ref GuessOutput::OutputModule.
The following sequence diagram shows the creation process:

@startuml "Output initialization in LPJ-GUESS"
participant "framework()" as framework
activate OutputModuleModuleRegistry
[--> OutputModuleModuleRegistry : register_output_module("herbivory")
[--> framework : start guess
== initialization ==
framework --> OutputModuleContainer : create
activate OutputModuleContainer
framework --> OutputModuleModuleRegistry : create_all_modules()
OutputModuleModuleRegistry --> HerbivoryOutput : create
activate HerbivoryOutput
note over HerbivoryOutput : declare parameters
note over framework : read parameters
framework --> OutputModuleContainer : init()
OutputModuleContainer      --> FileOutputChannel : create
activate FileOutputChannel
OutputModuleContainer --> HerbivoryOutput : init()
HerbivoryOutput --> FileOutputChannel : create tables
framework --> HerbivoryOutput : set_include_date()
== simulation ==
framework --> OutputModuleContainer : outannual(gridcell)
OutputModuleContainer --> HerbivoryOutput : outannual(gridcell)
HerbivoryOutput --> FileOutputChannel : add values
@enduml

The output module \ref GuessOutput::HerbivoryOutput is used both in the 
standard LPJ-GUESS framework and in the test simulations
(\ref page_herbiv_tests).
If the parameter `ifherbivory` is 0, the whole class is
deactivated and won’t produce any output or create files.

This is necessary because some herbivore module parameters,
like `digestibility_model`, are not checked when reading
the instruction file.

While the class \ref GuessOutput::HerbivoryOutput complies with the output
module framework of LPJ-GUESS, a few technical improvements
to \ref GuessOutput::CommonOutput were made:
- Output interval can be chosen freely with one variable
  instead of different output files. The table structure
  stays always the same (no month columns).
- The functions are smaller and better maintainable.
- The preprocessing of the data (building averages etc.) is
  done in \ref Fauna::HabitatOutputData and other data-holding
  classes (e.g. \ref Fauna::HabitatForage). This approach
  honours the \ref sec_single_responsibility.
- The inherited functions outannual() and outdaily() delegate
  to more generic functions, which are also used by
  \ref FaunaSim::Framework.
- The class \ref GuessOutput::IncludeDate has been introduced
  in order to observe the \ref sec_dependency_inversion and
  to avoid global variables.
  See also: \ref sec_herbiv_limit_output.
- The \ref GuessOutput::OutputModuleRegistry instantiates the
  class. There is only one global instance, but there is no direct
	way to access that global instance like in the
	[Singleton design pattern](\ref sec_singleton).
	To circumvent this restriction (instead of working with a lot
	of `static` members) the function
	[get_instance()](\ref GuessOutput::HerbivoryOutput::get_instance())
	has been introduced.
	To assert that no other instance can be created, the constructor
	throws an exception on second call. 

@startuml "Output classes of the herbivory module"
hide members 
annotation "Output Directory" as outputdirectory 
namespace GuessOutput{
	HerbivoryOutput     --> OutputChannel : <<use>>
	OutputModule      <|--  HerbivoryOutput
	OutputChannel     <|--  FileOutputChannel
	FileOutputChannel   --> .outputdirectory : write
	enum interval {
		DAILY
		MONTHLY
		ANNUAL
	}
	show interval members
	HerbivoryOutput *-- interval
	note on link : nested
}
namespace Fauna{
	abstract Habitat
	Habitat o-- "365" HabitatOutputData : daily
	GuessOutput.HerbivoryOutput ..> Fauna.HabitatOutputData : <<use>>
	HabitatOutputData --> HabitatForage : available forage
	HabitatOutputData --> ForageMass : eaten forage
	class HabitatOutputData
	note right : various other values
}
@enduml


------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see \ref page_herbiv_model
\see \ref page_herbiv_tutor
\see \ref page_herbiv_tests
\see \ref group_herbivory
