cmake_minimum_required(VERSION 3.10)

project(CondaEnv)

function(load_dotenv dotenv_file)
    if (EXISTS "${dotenv_file}")
        file(READ "${dotenv_file}" dotenv_content)

        # Split content into lines
        string(REPLACE "\n" ";" dotenv_lines "${dotenv_content}")

        # Process each line
        foreach (line IN LISTS dotenv_lines)

            # Skip comments and empty lines
            if (line MATCHES "^[ \t]*$" OR line MATCHES "^[ \t]*#.*$")
                continue()
            endif()

            # Match lines with "key=value"
            if (line MATCHES "^[ \t]*([^= \t]+)[ \t]*=[ \t]*(.*)$")
                string(REGEX REPLACE "^[ \t]*([^= \t]+)[ \t]*=[ \t]*(.*)$" "\\1" key "${line}")
                string(REGEX REPLACE "^[ \t]*([^= \t]+)[ \t]*=[ \t]*(.*)$" "\\2" value "${line}")

                # Set as environment variable
                set(ENV{${key}} "${value}")
            endif()
        endforeach()
    else()
        message(WARNING "The file ${dotenv_file} does not exist!")
    endif()
endfunction()

set(PKG_NAME $ENV{PKG_NAME})
set(PKG_VERSION $ENV{PKG_VERSION})

set(RECIPE_DIR $ENV{RECIPE_DIR})
set(SRC_DIR $ENV{SRC_DIR})
set(PREFIX $ENV{PREFIX})

load_dotenv(${SRC_DIR}/.env)
set(PROJECT_NAME $ENV{PROJECT_NAME})

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${PREFIX}/etc/conda/activate.d")
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${PREFIX}/etc/conda/deactivate.d")

configure_file(${RECIPE_DIR}/scripts/activate.sh.in
               "${PREFIX}/etc/conda/activate.d/${PKG_NAME}_activate.sh" @ONLY)

configure_file(${RECIPE_DIR}/scripts/deactivate.sh.in
               "${PREFIX}/etc/conda/deactivate.d/${PKG_NAME}_deactivate.sh" @ONLY)
message("${PREFIX}/etc/conda/activate.d/${PKG_NAME}_activate.sh")
