SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

macro(COPY target file dest)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND
            "${CMAKE_COMMAND}"
            -E copy_if_different ${file} ${dest})
endmacro()

macro(MKDIR target dir)
    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND
            "${CMAKE_COMMAND}"
            -E make_directory ${dir})
endmacro()

macro(IMPORT_LIBRARY target)
    add_library(${target} UNKNOWN IMPORTED)
    set_property(TARGET ${target} PROPERTY IMPORTED_LOCATION ${BIN_DIR}/${target})
endmacro()

macro(IMPORT_GTEST)
    add_library(libgtest.a UNKNOWN IMPORTED)
    set_property(TARGET libgtest.a PROPERTY IMPORTED_LOCATION $ENV{OUT_PATH}/lib/gtest.a)
    add_library(libgtest_main.a UNKNOWN IMPORTED)
    set_property(TARGET libgtest_main.a PROPERTY IMPORTED_LOCATION $ENV{OUT_PATH}/lib/gtest_main.a)
    include_directories(${EXTERNAL_DIR}/googletest/include)
if("$ENV{PLATFORM}" STREQUAL "linux")
    set(GTEST_LIBS libgtest.a libgtest_main.a pthread)
elseif("$ENV{PLATFORM}" STREQUAL "openEuler")
    set(GTEST_LIBS libgtest.a libgtest_main.a pthread)
elseif("$ENV{PLATFORM}" STREQUAL "android")
    set(GTEST_LIBS libgtest.a libgtest_main.a)
endif()
endmacro()

macro(IMPORT_COMO_COMPONENT comoComponent dir)
    if(DEFINED ARGN)
        set(genSources ARGN)
    else()
        set(genSources "GENERATED_SOURCES")
    endif()

    get_filename_component(src_dir "${comoComponent}" DIRECTORY)
    if("__${src_dir}" STREQUAL "__")
        set(src_dir "./")
    endif()
    message(STATUS "$ENV{CDLC} -gen -mode-client -d ${dir} -metadata-so ${comoComponent}")

    execute_process(
        COMMAND
            "$ENV{CDLC}"
            -gen
            -mode-client
            -d ${dir}
            -metadata-so ${comoComponent})

    file(GLOB ${genSources} ${dir}/*.cpp)
endmacro()

macro(COMPILE_COMO_COMPONENT comoComponent dir)
    if(DEFINED ARGN)
        set(genSources ARGN)
    else()
        set(genSources "GENERATED_SOURCES")
    endif()

    get_filename_component(src_dir "${comoComponent}" DIRECTORY)
    if("__${src_dir}" STREQUAL "__")
        set(src_dir "./")
    endif()
    message(STATUS "$ENV{CDLC} -gen -mode-component -d ${dir} -i ${src_dir} -c ${comoComponent}")

    execute_process(
        COMMAND
            "$ENV{CDLC}"
            -gen
            -mode-component
            -d ${dir}
            -i ${src_dir}
            -c ${comoComponent})

    file(GLOB ${genSources} ${dir}/*.cpp)
endmacro()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected (options are: Debug Release), default to Release.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()
