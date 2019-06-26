Unit Tests of the Herbivore Module {#page_tests}
==================================
<!-- For doxygen, this is the *page* header -->
\brief Details on unit tests and test simulations.

Unit Tests of the Herbivore Module {#sec_page_tests}
==================================
<!-- For doxygen, this is the *section* header -->
\tableofcontents


Unit Tests {#sec_page_unittests}
----------------------------------

New classes have been introduced with unit-testing and flexibility in mind.
This has the following implications:

- The herbivore model is ignorant of the absolute date, but only knows the
day of the year.

- All simulation parameters are passed into the model with designated objects
instead of reading instruction file parameters directly
(see section \ref sec_page_parameters).

However, those classes that form the bridge to the vegetation
model can only be tested to a limited degree because it would
involve instantiating the whole (rather monolithic) vegetation
model.
Those classes are:
\ref Fauna::PatchHabitat,
\ref Fauna::Parameters,

Unit tests use the [Catch](https://github.com/philsquared/Catch)
framework.






Herbivory Test Simulations {#sec_page_testsimulations}
--------------------------------------------------------

### Overview

In order to simulate herbivore dynamics in a controlled environment a testing framework has been implemented.
Since the herbivore model can be attached to any vegetation model that implements the \ref Fauna::Habitat interface, it is easy to perform herbivore simulations independent of a full vegetation model.

All classes that are not connected to the actual herbivory simulations but only serve the independent test simulations, are gathered in the namespace \ref FaunaSim.
The central class running the simulations is \ref FaunaSim::Framework, which in turn employs \ref Fauna::Simulator.

The class \ref FaunaSim::SimpleHabitat implements a very basic vegetation model that can be parametrized with custom parameters in the instruction file.
One SimpleHabitat object corresponds to a \ref Patch in a way.
So far, only this one kind of vegetation model is implemented.
(However, if desirable, also other types of vegetation models could be implemented as classes derived from \ref Fauna::Habitat.)

The class \ref FaunaSim::HabitatGroup holds a list of \ref Fauna::SimulationUnit objects, each of which contains a \ref Fauna::Habitat implementing object and a herbivore population (\ref Fauna::HftPopulationsMap).
The HabitatGroup corresponds in a way to the LPJ-GUESS `Gridcell`.

@startuml "Class interactions of the Herbivory Test Simulations"
	!include diagrams.iuml!testsim_classes
@enduml

\see \ref sec_page_new_testhabitat

### Parameters

An example instruction file is provided in
`examples/testsim.ins`.
It emulates the scenario of the metaphysiological model by Norman Owen-Smith \cite owensmith2002metaphysiological during growing season.
These are the relevant parameters:

<!-- Alternatively to the snippet command, the dontinclude command could be used. -->
\snippet testsim.ins Simulation Parameters

### Simulation Sequence

@startuml "Sequence diagram for test simulations of the herbivory module"
	!include diagrams.iuml!testsim_sequence
@enduml

------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date June 2017
\see \ref page_design
\see \ref page_model
\see \ref page_tutor
