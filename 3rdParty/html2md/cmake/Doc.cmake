find_package(Doxygen)

if(DOXYGEN_FOUND)
    add_custom_target(
      doc
      COMMAND echo "PROJECT_NUMBER = ${PROJECT_VERSION}" >> docs/Doxyfile && ${DOXYGEN_EXECUTABLE} docs/Doxyfile
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Generating API documentation using Doxygen"
      VERBATIM
    )
else()
    message(WARNING "Doxygen not found. The documentation will not be created!")
endif()
