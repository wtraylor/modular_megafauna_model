Unit Tests of the Herbivore Module {#page_herbiv_tests}
==================================
<!-- For doxygen, this is the *page* header -->
\brief Details on unit tests and test simulations.

Unit Tests of the Herbivore Module {#sec_herbiv_tests}
==================================
<!-- For doxygen, this is the *section* header -->
\tableofcontents


Unit Tests {#sec_herbiv_unittests}
----------------------------------

Unit tests have been implemented as far as the design of LPJ-GUESS permits.
New classes have been introduced with unit-testing and flexibility in mind.
This has the following implications:

- All classes do not rely on global variables of LPJ-GUESS
(except for \ref Fauna::ParamReader).

- The herbivore model is ignorant of the absolute date, but only knows the 
day of the year.

- All simulation parameters are passed into the model with designated objects
instead of reading instruction file parameters directly
(see section \ref sec_herbiv_parameters).

However, those classes that form the bridge to the vegetation 
model can only be tested to a limited degree because it would
involve instantiating the whole (rather monolithic) vegetation 
model.
Those classes are:
\ref Fauna::PatchHabitat,
\ref Fauna::Parameters,
\ref GuessOutput::HerbivoryOutput.

Unit tests use the [Catch](https://github.com/philsquared/Catch)
framework.






Herbivory Test Simulations {#sec_herbiv_testsimulations}
--------------------------------------------------------

### Overview ###

In order to simulate herbivore dynamics in a controlled environment a testing framework has been implemented. 
Since the herbivore model can be attached to any vegetation model that implements the \ref Fauna::Habitat interface, it is easy to perform herbivore simulations independent of LPJ-GUESS.

When compiling the project, in addition to `guess` another 
executable binary file is produced: `herbivsim`.
It takes the instruction script as one command line parameter.
The Windows GUI is not supported. Help output is also not supported.

All classes that are not connected to the actual herbivory simulations but only serve the independent test simulations, are gathered in the namespace \ref FaunaSim.
The central class running the simulations is \ref FaunaSim::Framework, which in turn employs \ref Fauna::Simulator.

The class \ref FaunaSim::SimpleHabitat implements a very basic vegetation model that can be parametrized with custom parameters in the instruction file. 
One SimpleHabitat object corresponds to a \ref Patch in a way.
So far, only this one kind of vegetation model is implemented.
(However, if desirable, also other types of vegetation models could be implemented as classes derived from \ref Fauna::Habitat.)

The class \ref FaunaSim::HabitatGroup holds a list of objects which implement \ref Fauna::Habitat and thus corresponds to a \ref Gridcell.

@startuml "Class interactions of the Herbivory Test Simulations"
hide members
hide methods 
annotation "parameters.h" as parameters 
namespace FaunaSim {
	class "Framework" <<singleton>>
	class HabitatGroup
	class SimpleHabitat
	HabitatGroup "1" *-- "*" SimpleHabitat
}
namespace Fauna {
	class "ParamReader"   <<singleton>>
	interface Habitat 
	Simulator     ..> Habitat                        : <<call>>
}
FaunaSim.SimpleHabitat --|> Fauna.Habitat
FaunaSim.Framework ..> Fauna.ParamReader              : <<use>>
FaunaSim.Framework ..> .parameters                    : <<use>>
FaunaSim.Framework ..> Fauna.Simulator                : <<create>>
FaunaSim.Framework *-> "1" .GuessOutput.HerbivoryOutput   
FaunaSim.Framework ..> .GuessOutput.FileOutputChannel : <<create>>
FaunaSim.Framework ..> "*" FaunaSim.HabitatGroup      : <<create>>
annotation "main()" as main
main ..> FaunaSim.Framework : <<call>>
@enduml

### Parameters ###

An example instruction file is provided in 
`data/ins/herbiv_testsim.ins`.
These are the relevant parameters:

<!-- Alternatively to the snippet command, the dontinclude command -->
<!-- could be used. -->
\snippet herbiv_testsim.ins Simulation Parameters

\note Make sure that `ifherbivory` is true because otherwise
no output will be produced (see \ref sec_herbiv_output).

The header file \ref parameters.h declares a number of LPJ-GUESS instruction file parameters, which are not relevant to the herbivore test simulations.
In order to prevent error messages that those parameters were missing when running the `herbivsim` executable, the preprocessor flag `NO_GUESS_PARAMETERS` has been introduced.
It is only defined when the target `herbivsim` is being compiled.
This solution promised the least amount of change of main LPJ-GUESS code.
See \ref parameters.cpp for the affected code regions.

### Simulation Sequence ###

@startuml "Sequence diagram for test simulations of the herbivory module"
participant "main()" as main
participant "plib.h" as plib
participant "FaunaSim::Framework" as Framework <<singleton>>
participant "Fauna::ParamReader" as ParamReader <<singleton>>
participant "Fauna::Simulator" as Simulator
participant "GuessOutput::HerbivoryOutput" as HerbivoryOutput <<singleton>>
== initialization ==
main -> Framework : <<create>>
activate Framework
activate HerbivoryOutput
Framework -> plib : declare parameters
main -> plib : read instruction file
plib -> ParamReader : call indirectly
activate ParamReader
plib -> Framework : plib_callback()
main <-- ParamReader : Fauna::Parameters
main <-- ParamReader : Fauna::HftList
main -> Framework : run()
Framework -> HerbivoryOutput : set_hftlist()
Framework -> HerbivoryOutput : init()
Framework -> Simulator : <<create>>
activate Simulator
note over Framework : create habitats
== simulation ==
loop YEARS: nyears
	loop DAYS: 365
		loop HABITAT GROUPS: nhabitat_groups
			loop HABITATS: nhabitats_per_group
	      Framework -> Simulator : simulate_day()
		  end
		end
  end
	loop HABITAT GROUPS: nhabitat_groups
		Framework -> HerbivoryOutput : outannual()
	end
end
== end of simulation ==
deactivate Simulator
Framework --> main
deactivate HerbivoryOutput
deactivate Framework
@enduml
------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date June 2017
\see \ref page_herbiv_design
\see \ref page_herbiv_model
\see \ref page_herbiv_tutor
\see \ref group_herbivory
