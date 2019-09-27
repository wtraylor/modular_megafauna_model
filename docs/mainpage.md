Doxygen Documentation of the Modular Megafauna Model
====================================================

This source code documentation is a guide for users and developers of the megafauna library.
Before you continue, make sure you have read the `README.md` in the root of this repository in order to gain a general overview.

This documentation contains auto-generated source code documentation and in addition a few pages that generally describe the model and the source code architecture.
You can find all of these manually written pages under “Related Pages” in the upper navigation bar.

A good first read for users and developers are the \ref page_quickstart and the \ref page_model.

## Contributing

If you are considering to contribute to the source code, make yourself familiar with the \ref page_design as well as the \ref page_object_orientation.
Please observe the [Open-closed Principle](\ref sec_open_closed) wherever possible: Only change a class *if necessary.* Try to extend it instead or—even better—write a new class with your functionality.
In \ref page_tutor you might find already a step-by-step instruction for your use case.
In \ref CONTRIBUTING.md you will find all the details about what your code should look like and how you can commit your changes to the repository.

Please keep the documentation pages \ref page_design and \ref page_model updated.
The UML diagrams are rendered by PlantUML and collected all in one file: `docs/diagrams.iuml`.
You will find references for the PlantUML syntax on <http://www.plantuml.com>.

Come into the practice of **test-driven development**.
Write a test for each one of your new classes!
Check all of its public methods and see what example test scenarios it has to fulfill.
Also check if exceptions are thrown correctly.

## Integration into a Vegetation Model

If you want to introduce the megafauna library to a vegetation model, you should study the code of the demo simulator (`tools/demo_simulator`) and its documentation on the page \ref page_demo_simulator.
<!--TODO: Add advice on dynamic vs. static library to comply with library license.-->

-------------------------------------------------

\copyright ...
\date 2019
