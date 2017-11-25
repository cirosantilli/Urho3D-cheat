cmake_minimum_required (VERSION 3.2.3)
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
    cmake_policy (SET CMP0022 NEW)
    cmake_policy (SET CMP0026 OLD)
    cmake_policy (SET CMP0042 NEW)
    cmake_policy (SET CMP0063 OLD)
endif ()
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)
include (UrhoCommon)

file(GLOB APP_SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "*.cpp")
foreach(testsourcefile ${APP_SOURCES})
    string(REPLACE ".cpp" "" testname ${testsourcefile})

    set (TARGET_NAME ${testname})
    define_source_files (GLOB_CPP_PATTERNS ${testsourcefile})
    setup_main_executable ()

    #add_executable(${testname} ${testsourcefile})

endforeach(testsourcefile ${APP_SOURCES})
