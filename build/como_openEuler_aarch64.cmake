
set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 1)

set(GCC_DIR $ENV{ROOT}/prebuilt/aarch64-openeuler-linux/x86_64-openeulersdk-linux/usr)
set(PREBUILT_DIR $ENV{ROOT}/prebuilt/android-80-pixel/sysroot)
set(PREBUILT_INC ${PREBUILT_DIR}/usr/include)
set(PREBUILT_LIB ${PREBUILT_DIR}/usr/lib64)

set(CMAKE_C_COMPILER ${GCC_DIR}/bin/aarch64-openeuler-linux-gcc)
set(CMAKE_CXX_COMPILER ${GCC_DIR}/bin/aarch64-openeuler-linux-g++)

set(CMAKE_SHARED_LIBRARY_PREFIX_CXX "" CACHE STRING "" FORCE)
set(CMAKE_STATIC_LIBRARY_PREFIX_CXX "" CACHE STRING "" FORCE)

set(CMAKE_SYSROOT ${PREBUILT_DIR})

include_directories(
    ${PREBUILT_INC}
    ${PREBUILT_INC}/libcxx
    ${PREBUILT_INC}/libcxxabi
    ${PREBUILT_INC}/asm-arm64
    ${PREBUILT_INC}/arch-arm64)

set(COMMON_C_FLAGS "\
    -D__openEuler__ \
    -fPIC -ffunction-sections -fdata-sections -fstack-protector -fno-short-enums -fmessage-length=0 \
    -no-canonical-prefixes -Wno-nullability-completeness -Wno-extern-c-compat")

set(COMMON_CXX_FLAGS
    "${COMMON_C_FLAGS} -std=c++14 -fno-exceptions -fno-rtti")

set(DYNAMIC_LINKER
    "-Wl,-dynamic-linker,/system/bin/linker64")
set(SO_CRT "\
    ${PREBUILT_DIR}/usr/lib64/crtbegin_so.o \
    ${PREBUILT_DIR}/usr/lib64/crtend_so.o")
set(EXE_CRT "\
    ${PREBUILT_DIR}/usr/lib64/crtbegin_dynamic.o \
    ${PREBUILT_DIR}/usr/lib64/crtend_android.o")
set(LIBC
    "-lc -lc++ -lcompiler_rt")

set(COMMON_LINKER_FLAGS "\
    -B${GCC_DIR}/bin -fuse-ld=gold -nostdlib \
    ${DYNAMIC_LINKER} \
    -Wl,-X -Wl,--gc-sections \
    ${LIBC}")

set(COMMON_SHARED_LINKER_FLAGS "\
    -shared -fPIC ${COMMON_LINKER_FLAGS} \
    -Wl,--no-undefined,--no-undefined-version -Wl,--hash-style=gnu \
    ${SO_CRT} ${PREBUILT_DIR}/usr/lib64/libgcc.a")

set(COMMON_EXE_LINKER_FLAGS "\
    -Bdynamic -pie ${COMMON_LINKER_FLAGS} \
    -Wl,--entry,_start -Wl,-z,nocopyreloc \
    ${EXE_CRT}")

if($ENV{VERSION} STREQUAL "rls")
    set(CMAKE_C_FLAGS
        "${COMMON_C_FLAGS} -Os -fno-strict-aliasing" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS
        "${COMMON_CXX_FLAGS} -fvisibility=hidden -Os" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS
        "${COMMON_SHARED_LINKER_FLAGS} -Wl,--strip-all" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS
        "${COMMON_EXE_LINKER_FLAGS} -Wl,--strip-all" CACHE STRING "" FORCE)
else()
    set(CMAKE_C_FLAGS
        "${COMMON_C_FLAGS} -O0" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS
        "${COMMON_CXX_FLAGS} -O0 -g" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS
        "${COMMON_SHARED_LINKER_FLAGS}" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS
        "${COMMON_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)
endif()
