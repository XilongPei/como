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
    set_property(TARGET ${target} PROPERTY IMPORTED_SONAME ${target})
endmacro()

macro(IMPORT_ANY_LIBRARY dir target)
    add_library(${target} UNKNOWN IMPORTED)
    set_property(TARGET ${target} PROPERTY IMPORTED_LOCATION ${dir}/${target})
    set_property(TARGET ${target} PROPERTY IMPORTED_SONAME ${target})
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

macro(IMPORT_COMO_COMPONENT depend_target comoComponent dir)
    # ---------- Parameter validation ----------
    if("__${depend_target}" STREQUAL "__")
        message(FATAL_ERROR "IMPORT_COMO_COMPONENT: depend_target is empty")
    endif()

    if(NOT TARGET ${depend_target})
        message(FATAL_ERROR
            "IMPORT_COMO_COMPONENT: target '${depend_target}' does not exist"
        )
    endif()

    if("__${comoComponent}" STREQUAL "__")
        message(FATAL_ERROR "IMPORT_COMO_COMPONENT: comoComponent is empty")
    endif()

    if("__${dir}" STREQUAL "__")
        message(FATAL_ERROR "IMPORT_COMO_COMPONENT: dir is empty")
    endif()

    file(MAKE_DIRECTORY ${dir})

    # ---------- Code generation target (NO OUTPUT) ----------
    set(gen_target import_como_client_${depend_target})

    add_custom_target(${gen_target}
        COMMAND
            "$ENV{CDLC}"
            -gen
            -mode-client
            -d ${dir}
            -metadata-so ${comoComponent}
        BYPRODUCTS
            ${dir}/*.cpp
            ${dir}/*.h
        COMMENT "Generating COMO client for ${depend_target}"
    )

    # ---------- Collect generated sources ----------
    file(GLOB como_client_srcs
        CONFIGURE_DEPENDS
        ${dir}/*.cpp
    )

    # ---------- Attach generated sources ----------
    target_sources(${depend_target}
        PRIVATE
            ${como_client_srcs}
    )

    # ---------- Ensure build order ----------
    add_dependencies(${depend_target} ${gen_target})
endmacro()

macro(COMPILE_COMO_COMPONENT depend_target comoComponent dir)
    # ---------- Parameter validation ----------
    if("__${depend_target}" STREQUAL "__")
        message(FATAL_ERROR "COMPILE_COMO_COMPONENT: depend_target is empty")
    endif()

    if(NOT TARGET ${depend_target})
        message(FATAL_ERROR
            "COMPILE_COMO_COMPONENT: target '${depend_target}' does not exist"
        )
    endif()

    if("__${comoComponent}" STREQUAL "__")
        message(FATAL_ERROR "COMPILE_COMO_COMPONENT: comoComponent is empty")
    endif()

    if("__${dir}" STREQUAL "__")
        message(FATAL_ERROR "COMPILE_COMO_COMPONENT: dir is empty")
    endif()

    # ---------- Source directory of the .cdl file ----------
    get_filename_component(src_dir "${comoComponent}" DIRECTORY)
    if("__${src_dir}" STREQUAL "__")
        set(src_dir "./")
    endif()

    # ---------- Ensure output directory exists ----------
    file(MAKE_DIRECTORY ${dir})

    # ---------- Code generation target ----------
    set(gen_target como_gen_${depend_target})

    add_custom_target(${gen_target}
        COMMAND
            "$ENV{CDLC}"
            -gen
            -mode-component
            -d ${dir}
            -i ${src_dir}
            -c ${comoComponent}
        BYPRODUCTS
            ${dir}/*.cpp
            ${dir}/*.h
        COMMENT "Generating COMO component for ${depend_target}"
    )

    # ---------- Collect generated source files ----------
    file(GLOB component_srcs
        CONFIGURE_DEPENDS
        ${dir}/*.cpp
    )

    # ---------- Attach generated sources to the dependent target ----------
    target_sources(${depend_target}
        PRIVATE
            ${component_srcs}
    )

    # ---------- Ensure code generation runs before the dependent target ----------
    add_dependencies(${depend_target} ${gen_target})
endmacro()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected (options are: Debug Release), default to Release.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type (default Release)" FORCE)
endif()
