#!/bin/bash

function help()
{
cat <<EOF
- help:                     this screen.
- comotools:                Switch to build como tools.
- como_linux_x64:           Switch to build como for linux x64.
- como_linux_aarch64:       Switch to build como for linux aarch64.
- como_linux_riscv64:       Switch to build como for linux riscv64.
- como_linux_x32:           Switch to build como for linux x32.
- como_linux_aarch32:       Switch to build como for linux aarch32.
- como_linux_riscv32:       Switch to build como for linux riscv32.
- como_android_aarch64:     Switch to build como for android aarch64.
- como_openEuler_aarch64:   Switch to build como for openEuler aarch64.
- como_openEuler_riscv:     Switch to build como for openEuler RISC-V.
- debug:                    Switch to build debug version.
- release:                  Switch to build release version.
- build:                    Build source codes.
- build install:            Build source codes and install the building results.
- rebuild:                  Rebuild source codes.
- rebuild install:          Rebuild source codes and install the building results.
- clobber:                  Clean the building results and generated files of the current project.
- clobber all:              Clean all of the building results and generated files.
EOF
}

function check()
{
    if [ -z "PRODUCT" ]; then
        echo "PRODUCT not defined"
        return 0
    fi

    if [ -z "PLATFORM" ]; then
        echo "PLATFORM not defined"
        return 0
    fi

    return 1
}

function root()
{
    cd $ROOT
}

function out()
{
    if [ ! check ]; then
        return
    fi

    if [ "$PRODUCT" == "comotools" ]; then
        cd $HOST_OUT_PATH
    elif [ "$PRODUCT" == "como" ]; then
        cd $OUT_PATH
    fi
}

function bin()
{
    if [ ! check ]; then
        return
    fi

    if [ "$PRODUCT" == "comotools" ]; then
        cd $HOST_BIN_PATH
    elif [ "$PRODUCT" == "como" ]; then
        cd $BIN_PATH
    fi
}

function comotools()
{
    export PRODUCT=comotools
    export PLATFORM=linux
    export BUILD=$PRODUCT.$PLATFORM.$ARCH.$VERSION
    export HOST_OUT_PATH=$ROOT/out/host
    export HOST_BIN_PATH=$ROOT/bin/host
    if grep -qi "openEuler" /etc/os-release; then
        export ARCH=openEuler
    else
        export ARCH=x64
    fi

    if [ ! -d "$HOST_OUT_PATH" ]; then
        mkdir $HOST_OUT_PATH
    fi

    if [ ! -d "$HOST_BIN_PATH" ]; then
        mkdir $HOST_BIN_PATH
    fi

    cd $HOST_OUT_PATH

    if [ ! -f "$HOST_OUT_PATH/CMakeCache.txt" ]; then
        if [ "$VERSION" == "rls" ]; then
            BUILD_TYPE=Release
        else
            BUILD_TYPE=Debug
        fi
        cmake -DCMAKE_TOOLCHAIN_FILE="$ROOT/build/comotools_linux_x64.cmake" -DCMAKE_BUILD_TYPE=$BUILD_TYPE $ROOT
    fi
}

function como_linux_common()
{
    export PRODUCT=como
    export PLATFORM=linux
    export BUILD=$PRODUCT.$PLATFORM.$ARCH.$VERSION
    export OUT_PATH=$ROOT/out/target/$BUILD
    export BIN_PATH=$ROOT/bin/target/$BUILD
    export DATA_PATH=$ROOT/data
    export LIB_PATH=$BIN_PATH
    export COMORT_PATH=$BIN_PATH/comort.so
    export CLASS_PATH=$BIN_PATH/libcore.so
    export COMO_ROOT=
    export CDLC=$ROOT/tools/"$ARCH"/cdlc

    # to disable some config
    # export xxx=no_......
    export COMO_WITH_ICU=icu
    export COMO_WITH_RPC=rpc
    export COMO_WITH_MATH_BIG_NUMBER=bn

    if [ ! -d "$ROOT/out/target" ]; then
        mkdir $ROOT/out/target
    fi

    if [ ! -d "$OUT_PATH" ]; then
        mkdir $OUT_PATH
    fi

    if [ ! -d "$ROOT/bin/target" ]; then
        mkdir $ROOT/bin/target
    fi

    if [ ! -d "$BIN_PATH" ]; then
        mkdir $BIN_PATH
    fi

    cd $OUT_PATH

    if [ ! -f "$OUT_PATH/CMakeCache.txt" ]; then
        TOOLCHAIN_FILE="$ROOT/build/$PRODUCT"_"$PLATFORM".cmake
        if [ "$VERSION" == "rls" ]; then
            BUILD_TYPE=Release
        else
            BUILD_TYPE=Debug
        fi
        cmake -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE -DCMAKE_BUILD_TYPE=$BUILD_TYPE $ROOT
    fi
}

function como_linux_x64()
{
    export ARCH=x64
    como_linux_common
    export COMO_WITH_RPC=rpc
}

function como_linux_aarch64()
{
    export ARCH=aarch64
    como_linux_common
}

function como_linux_riscv64()
{
    export ARCH=riscv64
    como_linux_common
}

function como_linux_x32()
{
    export ARCH=x32
    como_linux_common
    export COMO_WITH_RPC=rpc
}

function como_linux_aarch32()
{
    export ARCH=aarch32
    como_linux_common
}

function como_linux_riscv32()
{
    export ARCH=riscv32
    como_linux_common
}

function como_android_aarch64()
{
    export PRODUCT=como
    export PLATFORM=android
    export ARCH=aarch64
    export BUILD=$PRODUCT.$PLATFORM.$ARCH.$VERSION
    export OUT_PATH=$ROOT/out/target/$BUILD
    export BIN_PATH=$ROOT/bin/target/$BUILD
    export DATA_PATH=
    export COMORT_PATH=$BIN_PATH/comort.so
    export CLASS_PATH=
    export COMO_ROOT=

    # The host environment supports only x64
    export CDLC=$ROOT/tools/x64/cdlc

    export PATH=$PATH:$ROOT/toolchain/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin

    if [ ! -d "$ROOT/out/target" ]; then
        mkdir $ROOT/out/target
    fi

    if [ ! -d "$OUT_PATH" ]; then
        mkdir $OUT_PATH
    fi

    if [ ! -d "$ROOT/bin/target" ]; then
        mkdir $ROOT/bin/target
    fi

    if [ ! -d "$BIN_PATH" ]; then
        mkdir $BIN_PATH
    fi

    cd $OUT_PATH

    if [ ! -f "$OUT_PATH/CMakeCache.txt" ]; then
        TOOLCHAIN_FILE="$ROOT/build/$PRODUCT"_"$PLATFORM"_"$ARCH.cmake"
        if [ "$VERSION" == "rls" ]; then
            BUILD_TYPE=Release
        else
            BUILD_TYPE=Debug
        fi
        cmake -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE -DCMAKE_BUILD_TYPE=$BUILD_TYPE $ROOT
    fi
}

function como_openEuler_aarch64()
{
    export PRODUCT=como
    export PLATFORM=openEuler
    export ARCH=aarch64
    export BUILD=$PRODUCT.$PLATFORM.$ARCH.$VERSION
    export OUT_PATH=$ROOT/out/target/$BUILD
    export BIN_PATH=$ROOT/bin/target/$BUILD
    export DATA_PATH=
    export COMORT_PATH=$BIN_PATH/comort.so
    export CLASS_PATH=$BIN_PATH/libcore.so
    export CLASS_PATH=
    export COMO_ROOT=
    export LIB_PATH=$BIN_PATH

    # to disable some config
    # export xxx=no_......
    export COMO_WITH_ICU=icu
    export COMO_WITH_RPC=rpc
    export COMO_WITH_MATH_BIG_NUMBER=bn

    # The host environment supports only x64
    export CDLC=$ROOT/tools/x64/cdlc

    export PATH=$PATH:$ROOT/prebuilt/aarch64-openeuler-linux/x86_64-openeulersdk-linux/usr/bin

    if [ ! -d "$ROOT/prebuilt/aarch64-openeuler-linux/x86_64-openeulersdk-linux/usr/bin" ]; then
        echo -n -e "The cross-compilation toolchain "
        echo -e "$ROOT/prebuilt/aarch64-openeuler-linux does not exist, Run GetPrebuilts.sh first.\n"
        # defined in /usr/include/sysexits.h
        # #define EX_CONFIG       78      /* configuration error */
        return 78
    fi

    if [ ! -d "$ROOT/out/target" ]; then
        mkdir $ROOT/out/target
    fi

    if [ ! -d "$OUT_PATH" ]; then
        mkdir $OUT_PATH
    fi

    if [ ! -d "$ROOT/bin/target" ]; then
        mkdir $ROOT/bin/target
    fi

    if [ ! -d "$BIN_PATH" ]; then
        mkdir $BIN_PATH
    fi

    cd $OUT_PATH

    if [ ! -f "$OUT_PATH/CMakeCache.txt" ]; then
        TOOLCHAIN_FILE="$ROOT/build/$PRODUCT"_"$PLATFORM"_"$ARCH.cmake"
        if [ "$VERSION" == "rls" ]; then
            BUILD_TYPE=Release
        else
            BUILD_TYPE=Debug
        fi
        cmake -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE -DCMAKE_BUILD_TYPE=$BUILD_TYPE $ROOT
    fi
}

function como_openEuler_riscv()
{
    export PRODUCT=como
    export PLATFORM=openEuler
    export ARCH=riscv64
    export BUILD=$PRODUCT.$PLATFORM.$ARCH.$VERSION
    export OUT_PATH=$ROOT/out/target/$BUILD
    export BIN_PATH=$ROOT/bin/target/$BUILD
    export DATA_PATH=
    export COMORT_PATH=$BIN_PATH/comort.so
    export CLASS_PATH=
    export COMO_ROOT=

    # The host environment supports only x64
    export CDLC=$ROOT/tools/x64/cdlc

    export PATH=$PATH:$ROOT/toolchain/gcc/linux-x86/riscv64/riscv-linux-openEuler/bin

    if [ ! -d "$ROOT/toolchain/gcc/linux-x86/riscv64/riscv-linux-openEuler/bin" ]; then
        echo -n -e "The cross-compilation toolchain "
        echo -e "$ROOT/toolchain/gcc/linux-x86/riscv64/riscv-linux-openEuler/bin does not exist.\n"
        # defined in /usr/include/sysexits.h
        # #define EX_CONFIG       78      /* configuration error */
        return 78
    fi

    if [ ! -d "$ROOT/out/target" ]; then
        mkdir $ROOT/out/target
    fi

    if [ ! -d "$OUT_PATH" ]; then
        mkdir $OUT_PATH
    fi

    if [ ! -d "$ROOT/bin/target" ]; then
        mkdir $ROOT/bin/target
    fi

    if [ ! -d "$BIN_PATH" ]; then
        mkdir $BIN_PATH
    fi

    cd $OUT_PATH

    if [ ! -f "$OUT_PATH/CMakeCache.txt" ]; then
        TOOLCHAIN_FILE="$ROOT/build/$PRODUCT"_"$PLATFORM"_"$ARCH.cmake"
        if [ "$VERSION" == "rls" ]; then
            BUILD_TYPE=Release
        else
            BUILD_TYPE=Debug
        fi
        cmake -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE -DCMAKE_BUILD_TYPE=$BUILD_TYPE $ROOT
    fi
}

function debug()
{
    if [ ! check ]; then
        return
    fi

    if [ "$VERSION" == "dbg" ]; then
        return
    fi

    echo "Change to debug version"
    export VERSION=dbg

    if [ "$PRODUCT" == "comotools" ]; then
        rm -fr $HOST_OUT_PATH
        rm -fr $HOST_BIN_PATH
        comotools
    elif [ "$PRODUCT" == "como" ]; then
        if [ "$PLATFORM" == "linux" ] && [ "$ARCH" == "x64" ]; then
            rm -fr $OUT_PATH
            rm -fr $BIN_PATH
            como_linux_x64
        elif [ "$PLATFORM" == "android" ] && [ "$ARCH" == "aarch64" ]; then
            rm -fr $OUT_PATH
            rm -fr $BIN_PATH
            como_android_aarch64
        fi
    fi
}

function release()
{
    if [ ! check ]; then
        return
    fi

    if [ "$VERSION" == "rls" ]; then
        return
    fi

    echo "Change to release version"
    export VERSION=rls

    if [ "$PRODUCT" == "comotools" ]; then
        rm -fr $HOST_OUT_PATH
        rm -fr $HOST_BIN_PATH
        comotools
    elif [ "$PRODUCT" == "como" ]; then
        if [ "$PLATFORM" == "linux" ] && [ "$ARCH" == "x64" ]; then
            rm -fr $OUT_PATH
            rm -fr $BIN_PATH
            como_linux_x64
        elif [ "$PLATFORM" == "android" ] && [ "$ARCH" == "aarch64" ]; then
            rm -fr $OUT_PATH
            rm -fr $BIN_PATH
            como_android_aarch64
        fi
    fi
}

function build()
{
    if [ ! check ]; then
        return
    fi

    echo "Building $BUILD"
    make $@
}

function clobber()
{
    if [ "$1" == "all" ]; then
        if [ -d "$ROOT/out" ]; then
            cd $ROOT
            rm -fr $ROOT/out
            mkdir $ROOT/out
        fi

        if [ -d "$ROOT/bin" ]; then
            cd $ROOT
            rm -fr $ROOT/bin
            mkdir $ROOT/bin
        fi
    else
        if [ "$PRODUCT" == "comotools" ]; then
            rm -fr $HOST_OUT_PATH
            rm -fr $HOST_BIN_PATH
        elif [ "$PRODUCT" == "como" ]; then
            if [ "$PLATFORM" == "linux" ] && [ "$ARCH" == "x64" ]; then
                rm -fr $OUT_PATH
                rm -fr $BIN_PATH
            elif [ "$PLATFORM" == "android" ] && [ "$ARCH" == "aarch64" ]; then
                rm -fr $OUT_PATH
                rm -fr $BIN_PATH
            fi
        fi
    fi

    cd $ROOT/out
}

function rebuild()
{
    if [ ! check ]; then
        return
    fi

    clobber

    if [ "$PRODUCT" == "comotools" ]; then
        comotools
    elif [ "$PRODUCT" == "como" ]; then
        if [ "$PLATFORM" == "linux" ] && [ "$ARCH" == "x64" ]; then
            como_linux_x64
        elif [ "$PLATFORM" == "android" ] && [ "$ARCH" == "aarch64" ]; then
            como_android_aarch64
        fi
    fi

    build $@
}

function drop()
{
    if [ "$PLATFORM" == "android" ]; then
        $ROOT/build/$PLATFORM/drop.py $ROOT/build/$PLATFORM/mlist.json $@
    fi
}

function clean_drop()
{
    if [ "$PLATFORM" == "android" ]; then
        $ROOT/build/$PLATFORM/drop.py $ROOT/build/$PLATFORM/mlist.json clean
    fi
}

export ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"

if [ ! -d "$ROOT/out" ]; then
    mkdir $ROOT/out
fi

if [ ! -d "$ROOT/bin" ]; then
    mkdir $ROOT/bin
fi

if [ -z "${VERSION:-}" ]; then
    export VERSION=rls
fi

help
