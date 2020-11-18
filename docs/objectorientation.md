<!--
SPDX-FileCopyrightText: 2020 Wolfgang Traylor <wolfgang.traylor@senckenberg.de>

SPDX-License-Identifier: CC-BY-4.0
-->

# Object-oriented Design Concepts {#page_object_orientation}
\brief Some programming principles and paradigms in object-oriented software engineering.

\tableofcontents

In the megafauna model a couple of object-oriented design patterns were employed that are explained here along with general concepts of object-oriented programming.

\todo Add explanation of the PIMPL idiom (compilation firewall). Compare [How to implement the pimpl idiom by using unique_ptr - Fluent C++](https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/).

## Good Programming Practice {#sec_good_practice}

### Information Hiding {#sec_information_hiding}
Information or data hiding means that any parts that may be subject to later changes in the software design must not be accessible from other modules or from clients.
Only the very necessary access may be granted in well-defined, minimal interfaces.

Always assume the worst: If access is given to any other part of the software, that part may change it in unpredictable ways!

Declaring class members `private` (**encapsulation**) is one way to data hiding.

### Rule of Three {#sec_rule_of_three}
If a class explicitely defines at least one of the following methods, it should most likely also define the other ones:

- Destructor
- Copy Constructor
- Copy Assignment Operator

\todo In C++11 the **Rule of Five** becomes relevant!

## S-O-L-I-D Design principles ## {#sec_design_solid}

### Single Responsibility Principle {#sec_single_responsibility}
A class should have only a single responsibility:
A class should have only one reason to change.

### Open/Closed Principle {#sec_open_closed}
A class/module/function should be open for extension, but closed for modification.

### Liskov’s Substitution Principle {#sec_liskov_substitution}
Objects in a program should be replaceable with instances of their subtypes without altering the correctness of that program.

### Interface Segregation Principle {#sec_interface_segregation}
Many client-specific interfaces are better than one general-purpose interface.

### Dependency Inversion Principle {#sec_dependency_inversion}
1. High-level modules should not depend on low-level modules. Both should depend on abstractions.
2. Abstractions should not depend on details. Details should depend on abstractions.

## Inversion of Control {#sec_inversion_of_control}
The design principle of inversion of control is also called *Hollywood Principle:* “Don’t call us, we’ll call you.”
An architecture following that principle is built around a generic framework which directs the flow of control by delegating tasks to various, interchangeable submodules.
This approach makes the system more modular and extensible.

@startuml
	hide members
	hide methods
	Framework ..> Client1 : <<create & call>>
	Framework ..> Client2 : <<create & call>>
@enduml

Inversion of control is related to the [Dependency Inversion Principle](\ref sec_dependency_inversion), which differs in that it is concerned about the relationship between high-level and low-level modules rather than the framework design.

### Dependency Injection
Dependency Injection is a major technique to implement inversion of control:
One object (the framework) supplies the dependencies for another object (the client), which then become a part of the client’s state.
This is a direct alternative to global objects/variables.

Two kinds of dependency injection are used
1. **Setter Injection:** A client receives its dependency *after* being constructed, via a setter method. This is dangerous, because until initialization through the setter method, the client might be in an invalid state.
2. **Constructor Injection:** Any object of the client class receives its dependency in the constructor.

For example: The \ref Fauna::HftList object is not a global variable, but is instead being passed down from \ref Fauna::World to other classes that need it.

In the case of \ref Fauna::Habitat, the \ref Fauna::World is oblivious to the concrete realization of the interface, which makes it possible to substitute parts of the model without changing the framework.

\attention Never use global variables. They make unit tests impossible and violate the principles of modularity and compartmentalization.

## Design Patterns

### Singleton {#sec_singleton}
A class is called *singleton* if it permits only one global instantiation in the program.
This object-oriented approach has advantages over global variables because it is more generally flexible and the time of instantiation is flexible.
However, it is only justified for a class that is at the very top of the management hierarchy in the software architecture.

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

Wrapping global variables and free functions into a singleton class is good, but it is better to *avoid singletons all together* and instead follow the principle of [Inversion of Control](\ref sec_inversion_of_control).

\note The only instance where the Singleton pattern is used is in the demo simulator: \ref Fauna::Demo::Framework.

### Strategy {#sec_strategy}
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
In the megafauna model that is usually not the case, but rather it is used as a means of [dependency injection](sec_dependency_inversion).

### Facade {#sec_facade}
A facade class presents a simple interface to interact with another module or subsystem of the software.
The complexity of the subsystem is hidden behind the public functions of the facade class.
The subsystem doesn’t know about the facade.

@startuml "Structure of the facade design pattern."
	hide members
	hide methods
	client ..> facade : <<use>>
	facade ..> subsystem.class1 : <<call>>
	facade ..> subsystem.class2 : <<call>>
	facade ..> subsystem.class3 : <<call>>
@enduml

-------------------------------------------------

\copyright <a rel="license" href="http://creativecommons.org/licenses/by/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by/4.0/80x15.png" /></a> This software documentation is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/4.0/">Creative Commons Attribution 4.0 International License</a>.
\author Wolfgang Traylor, Senckenberg BiK-F
\date 2019
