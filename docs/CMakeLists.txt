# Creating the doxygen documentation is an independent compilation project.

option (BUILD_DOC "Build doxygen documentaton" ON)

cmake_minimum_required (VERSION 3.14)

find_package (Doxygen)
if (DOXYGEN_FOUND)

  set (DOXYGEN_IN Doxyfile.in)
  set (DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  # Replace any "@...@" variables in `Doxyfile.in`.
  configure_file (${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

  add_custom_target (doxygen
    ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating doxygen documentation..."
    VERBATIM)

else (DOXYGEN_FOUND)
  message ("Doxygen does not seem to be installed. No documentation will be
  generated.")
endif (DOXYGEN_FOUND)
