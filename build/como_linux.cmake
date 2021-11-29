
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)

set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LIBRARY_PREFIX_CXX "" CACHE STRING "" FORCE)

if("$ENV{COMO_FUNCTION_SAFETY}" STREQUAL "COMO_FUNCTION_SAFETY_ENABLE")
    set(FUNCTION_SAFETY_C_FLAGS "-DCOMO_FUNCTION_SAFETY=$ENV{COMO_FUNCTION_SAFETY}")
else()
    set(FUNCTION_SAFETY_C_FLAGS "")
endif()

if($ENV{ARCH} MATCHES "(x64)|(aarch64)")
    #! -DARM_FP_SUPPORT, Architecture-related options
    set(COMMON_C_FLAGS
        "-fPIC -ffunction-sections -fdata-sections ${FUNCTION_SAFETY_C_FLAGS} -DARM_FP_SUPPORT")
elseif($ENV{ARCH} MATCHES "(riscv64)")
    set(COMMON_C_FLAGS
        "-fPIC -ffunction-sections -fdata-sections ${FUNCTION_SAFETY_C_FLAGS}")
elseif($ENV{ARCH} MATCHES "(x32)|(aarch32)|(riscv32)")
    set(COMMON_C_FLAGS
        "-m32 -fPIC -ffunction-sections -fdata-sections ${FUNCTION_SAFETY_C_FLAGS}")
elseif($ENV{ARCH} MATCHES "(aarch32)|(riscv32)")
    set(COMMON_C_FLAGS
        "-fPIC -ffunction-sections -fdata-sections ${FUNCTION_SAFETY_C_FLAGS}")
else()
    set(COMMON_C_FLAGS
        "-fPIC -ffunction-sections -fdata-sections ${FUNCTION_SAFETY_C_FLAGS}")
endif()

set(COMMON_CXX_FLAGS
    "${COMMON_C_FLAGS} -fno-exceptions -fno-rtti -std=gnu++11")

set(COMMON_SHARED_LINKER_FLAGS
    "-shared -fPIC ${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,--no-undefined,--no-undefined-version -Wl,--hash-style=gnu")

set(COMMON_EXE_LINKER_FLAGS
    "-Bdynamic -pie ${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,-z,nocopyreloc")

if($ENV{VERSION} STREQUAL "rls")
    set(CMAKE_C_FLAGS
        "${COMMON_C_FLAGS} -fvisibility=hidden -Os" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS
        "${COMMON_CXX_FLAGS} -fvisibility=hidden -Os" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS
        "${COMMON_SHARED_LINKER_FLAGS} -Wl,--strip-all" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS
        "${COMMON_EXE_LINKER_FLAGS} -Wl,--strip-all" CACHE STRING "" FORCE)
else()
    set(CMAKE_C_FLAGS
        "${COMMON_C_FLAGS} -O0 -g" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS
        "${COMMON_CXX_FLAGS} -O0 -g" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS
        "${COMMON_SHARED_LINKER_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS
        "${COMMON_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
endif()
