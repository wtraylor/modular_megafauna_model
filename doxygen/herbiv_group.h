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
 * \section sec_design_patterns Design Patterns
 * In the Herbivore Module a couple of object-oriented design patterns
 * were employed that are explained here.
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

