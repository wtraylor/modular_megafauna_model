/**
 * \defgroup group_herbivory Code elements in the Herbivore Module
 * \brief New classes, design decisions, and changes in main LPJ-GUESS code.
 *
 * \section sec_herbiv_parameters Parameters
 * The herbivory module uses the same instruction files and plib 
 * (\ref plib.h) functionality as the vegetation model.
 * In order to separate concerns, all herbivory-related parameters
 * are declared and checked in the class \ref Fauna::Parameters.
 *
 * Note that the implementation is rather a dirty fix around
 * the inflexible design of LPJ-GUESS parameter library.
 * \see \ref sec_herbiv_new_pft_parameter
 * \see \ref sec_herbiv_new_hft_parameter
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * \section sec_object_orientation Object-oriented Design
 * In the Herbivore Module a couple of object-oriented design patterns
 * were employed that are explained here along with general 
 * concepts of object-oriented programming.
 *
 * \subsection sec_design_solid SOLID
 * \subsubsection sec_single_responsibility Single Responsibility Principle
 * A class should have only a single responsibility:
 * A class should have only one reason to change.
 * \subsubsection sec_open_closed Open/Closed Principle
 * A class/module/function should be open for extension, but
 * closed for modification.
 * \subsubsection sec_liskov_substitution Liskov’s Substitution Principle
 * Objects in a program should be replaceable with instances of
 * their subtypes without altering the correctness of that program.
 * \subsubsection sec_interface_segregation Interface Segregation Principle
 * Many client-specific interfaces are better than one 
 * general-purpose interface.
 * \subsubsection sec_dependency_inversion Dependency Inversion Principle
 * a. High-level modules should not depend on low-level modules.
 * Both should depend on abstractions.
 * b. Abstractions should not depend on details. 
 * Details should depend on abstractions.

 *
 * \subsection sec_singleton Singleton
 * A class is called *singleton* if it permits only one global
 * instantiation in the program.
 * This object-oriented approach has advantages over global variables
 * because it is more generally flexible and the time of instantiation
 * is flexible.
 *
 * It is used in these classes: 
 * \ref Fauna::DigestibilityModel, 
 * \ref Fauna::TestSimulator
 * 
 * The basic implementation is as follows:
 * \code
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
 * \endcode
 * To get access to the instance or trigger the initial instatiation, use:
 * \code
 * MySingleton::get_instance();
 * \endcode
 *
 *
 * \section sec_herbiv_output Herbivory Output
 * The output module \ref GuessOutput::HerbivoryOutput is used both in the 
 * standard LPJ-GUESS framework and in the test simulations
 * (\ref page_herbiv_tests).
 * If the parameter `ifherbivory` is 0, the whole class is
 * deactivated and won’t produce any output or create files.
 * 
 * This is necessary because some herbivore module parameters,
 * like `digestibility_model`, are not checked when reading
 * the instruction file.
 *
 * While the class \ref GuessOutput::HerbivoryOutput complies with the output
 * module framework of LPJ-GUESS, a few technical improvements
 * to \ref GuessOutput::CommonOutput were made:
 * - Output interval can be chosen freely with one variable
 *   instead of different output files. The table structure
 *   stays always the same (no month columns).
 * - The functions are smaller and better maintainable.
 * - The preprocessing of the data (building averages etc.) is
 *   done in \ref Fauna::HabitatOutputData and other data-holding
 *   classes (e.g. \ref Fauna::HabitatForage). This approach
 *   honours the \ref sec_single_responsibility.
 * - The inherited functions outannual() and outdaily() delegate
 *   to more generic functions, which are also used by
 *   \ref Fauna::TestSimulator.
 *
 *
 *
 *
 * \see The page \ref page_herbiv_model gives background on the science.
 * \see The page \ref page_herbiv_tests explains the testing %framework.
 * \see The page \ref page_herbiv_tutor offers tutorials on how to use the model.
 * \author Wolfgang Pappa, Senckenberg BiK-F
 * \date May 2017
 *
 */


/**
 * \ingroup group_herbivory
 * \namespace Fauna
 * \brief Namespace for all classes in the herbivory module.
 * \author Wolfgang Pappa, Senckenberg BiK-F
 * \date May 2017
 */

