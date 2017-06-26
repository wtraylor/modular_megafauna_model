/**
 * \defgroup group_herbivory Code elements in the Herbivore Module
 * \brief New classes and changes in main LPJ-GUESS code are documented here.
 * \todo Document the object-oriented design patterns that were employed:
 * singleton, interface
 *
 * \see The page \ref page_herbiv_model gives background on the science.
 * \see The page \ref page_herbiv_tests explains the testing %framework.
 * \see The page \ref page_herbiv_tutor offers tutorials on how to use the model.
 * \author Wolfgang Pappa, Senckenberg BiK-F
 * \date May 2017
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
 * \code
class MySingleton{
	public:
		static MySingleton& get_instance(){ 
			static MySingleton global_instance; // creates the instance on first call
			return global_instance;
		}
	private:
		MySingleton(){} // Constructor hidden from the outside
		MySingleton(MySingleton const&); // deleted copy constructor
		void operator=(MySingleton const&); // deleted assignment constructor
}
 * \endcode
 * To get access to the instance or trigger the initial instatiation, use:
 * \code
 * MySingleton::get_instance();
 * \endcode
 */


/**
 * \ingroup group_herbivory
 * \namespace Fauna
 * \brief Namespace for all classes in the herbivory module.
 * \author Wolfgang Pappa, Senckenberg BiK-F
 * \date May 2017
 */

