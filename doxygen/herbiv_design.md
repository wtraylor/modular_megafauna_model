Software Design of the Herbivory Module {#page_herbiv_design}
=============================================================
<!-- For doxygen, this is the *page* header -->
\brief Notes on the software design of the LPJ-GUESS herbivory module from a technical perspective.

Software Design of the Herbivory Module {#sec_herbiv_design}
============================================================
<!-- For doxygen, this is the *section* header -->

Herbivory Parameters {#sec_herbiv_parameters}
---------------------------------------------

The herbivory module uses the same instruction files and plib 
(\ref plib.h) functionality as the vegetation model.
In order to separate concerns, all herbivory-related parameters
are declared and checked in the class \ref Fauna::Parameters.

Note that the implementation is rather a dirty fix around
the inflexible design of LPJ-GUESS parameter library. Some
global constants (checkback and block codes) and global
pointers from \ref parameters.h and \ref parameters.cpp are
used in \ref herbiv_parameters.cpp.

\see \ref sec_herbiv_new_pft_parameter

HFT parameters are declared and parsed by 
\ref Fauna::Parameters, but checked for integrity by \ref Fauna::Hft
itself.

\image html herbiv_parameters.svg "UML class diagram of instruction file parameters in the herbivory module" width=800px

An example instruction file is provided in 
`data/ins/herbivores.ins`:
\snippet herbivores.ins Example Herbivore
\see \ref sec_herbiv_new_hft_parameter



\bug When printing out the help with \ref plibhelp() 
(by running `guess -help`), the global parameters declared in 
\ref Fauna::Parameters::declare_parameters() under 
`BLOCK_GLOBAL` appear out of order in the output.




\todo UML diagram with tiers:
low-level and high-level classes
\todo UML diagram of replacable modules: digestibility model, energy model etc.


\image html herbiv_overview_classes.svg "Overview on herbivory-related classes" width=800px




Object-oriented Design {#sec_object_orientation}
------------------------------------------------

In the Herbivore Module a couple of object-oriented design patterns
were employed that are explained here along with general 
concepts of object-oriented programming.

\todo functors

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

### Design Patterns {#sec_design_patterns}

### Singleton {#sec_singleton}
A class is called *singleton* if it permits only one global
instantiation in the program.
This object-oriented approach has advantages over global variables
because it is more generally flexible and the time of instantiation
is flexible.

It is used in these classes: 
\ref Fauna::DigestibilityModel, 
\ref Fauna::TestSimulator,
\ref Fauna::HftList

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

To get access to the instance or trigger the initial instatiation, use `MySingleton::get_instance();`.

### Strategy {#sec_strategy}
The strategy design pattern defines a set of interchangable algorithms, each of which are encapsulated in a class that is derived from one abstract interface parent class.
Thanks to C++ polymorphism, the used algorithm can change during runtime.
Here is a basic implementation:

    struct Strategy {
			virtual operator()(const int param) const = 0;
		};

		struct StrategyOne {
			virtual operator()(const int param) const{ /*...*/ };
		};

		struct StrategyTwo {
			virtual operator()(const int param) const{ /*...*/ };
		};

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

## Herbivory Output {#sec_herbiv_output}
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
  \ref Fauna::TestSimulator.
- The class \ref GuessOutput::OutputLimiter has been introduced
  in order to observe the \ref sec_dependency_inversion and
  to avoid global variables.
  See also: \ref sec_herbiv_limit_output.


------------------------------------------------------------

\author Wolfgang Pappa, Senckenberg BiK-F
\date May 2017
\see \ref page_herbiv_model
\see \ref page_herbiv_tutor
\see \ref page_herbiv_tests
\see \ref group_herbivory
