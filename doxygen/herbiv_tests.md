Unit Tests of the Herbivore Module {#page_herbiv_tests}
==================================
Introduction
------------

Unit tests have been implemented as far as the design of LPJ-GUESS permits.
New classes have been introduced with unit-testing and flexibility in mind.
This has the following implications:

- All classes do not rely on global variables of LPJ-GUESS
(except for \ref Fauna::Parameters).

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






Herbivory Test Simulations
--------------------------

In order to simulate herbivore dynamics in a controlled environment
a testing framework has been implemented.
The software design allows any vegetation model to be combined
with the herbivore model.

The class \ref TestHabitat implements a very basic vegetation
model that can be parametrized with custom parameters in the
instruction file.
One TestHabitat object corresponds to a \ref Patch in a way.

The class \ref TestHabitatGroup holds a list of \ref TestHabitat
objects and thus corresponds to a \ref Gridcell.

When compiling the project, in addition to `guess` another 
executable binary file is produced: `herbivsim`.
It takes the instruction script as one command line parameter.
(The Windows GUI is not supported.)

An example instruction file is provided in 
`data/ins/herbiv_testsim.ins`.
These are the relevant parameters:

<!-- Alternatively to the snippet command, the dontinclude command -->
<!-- could be used. -->
\snippet herbiv_testsim.ins Simulation Parameters

\note Make sure that `ifherbivory` is true because otherwise
no output will be produced (see \ref sec_herbiv_output).


\brief Details on unit tests and test simulations.
\author Wolfgang Pappa, Senckenberg BiK-F
\date June 2017
\see The page \ref group_herbivory gives implementation details.
\see The page \ref page_herbiv_model gives background on the science.
\see The page \ref page_herbiv_tutor offers tutorials on how to use the model.

