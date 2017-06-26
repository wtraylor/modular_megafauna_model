Unit Tests of the Herbivore Module {#page_herbiv_tests}
==================================
Introduction
------------

Unit tests have been implemented as far as the design of LPJ-GUESS permits.
New classes have been introduced with unit-testing and flexibility in mind.
This has the following implications:

\todo Which do use parameters?

- All classes except for TODO do not rely on global variables in LPJ-GUESS.

- The herbivore model is ignorant of the absolute date, but only knows the 
day of the year.

- All simulation parameters are passed into the model with designated objects
instead of reading instruction file parameters directly.











<!-- Alternatively to the snippet command, the dontinclude command -->
<!-- could be used. -->
\snippet herbiv_testsim.ins Simulation Parameters


\brief Details on unit tests and test simulations.
\author Wolfgang Pappa, Senckenberg BiK-F
\date June 2017
\see The page \ref group_herbivory gives implementation details.
\see The page \ref page_herbiv_model gives background on the science.
\see The page \ref page_herbiv_tutor offers tutorials on how to use the model.

