project(tests LANGUAGES C CXX)

if (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/md4c/src/)
    include(FindGit)

    if(NOT GIT_FOUND)
        message(WARNING "git not found. Please download md4c manually or disable tests.")
        return()
    endif()

    get_directory_property(dir PARENT_DIRECTORY)

    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --depth=1
                    WORKING_DIRECTORY ${dir})
endif()

set(MD4C_FILES
    md4c/src/entity.c
    md4c/src/entity.h
    md4c/src/md4c-html.c
    md4c/src/md4c-html.h
    md4c/src/md4c.c
    md4c/src/md4c.h
)

add_library(md4c-html STATIC ${MD4C_FILES})
target_include_directories(md4c-html PUBLIC md4c/src)

add_executable(test-exe main.cpp)
target_link_libraries(test-exe md4c-html html2md-static)
target_compile_definitions(test-exe PUBLIC DIR="${CMAKE_CURRENT_LIST_DIR}")
set_target_properties(test-exe PROPERTIES OUTPUT_NAME "tests")
target_compile_features(test-exe PUBLIC cxx_std_17) # Require at least c++17


if (CMAKE_VERSION VERSION_LESS 3.11.0)
    return()
endif()

add_custom_target(test
    COMMAND $<TARGET_FILE:test-exe>
    COMMENT Runing tests..
    DEPENDS test-exe
)
