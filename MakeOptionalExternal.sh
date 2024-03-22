#!/bin/bash

# build result:
# ./external/libzmq/build/lib/libzmq.a
# ./external/phxpaxos/export_como/PhxPaxos.so

current_path=$(pwd);

function perror() {

    echo -e "\033[0;31;1m$1\033[0m"
}

function psucc() {
    echo -e "\e[1;32m$1\e[0m"
}

function go_back()
{
    cd $current_path;
}

function check_dir_exist()
{
    dir_path=$current_path"/$1";
    if [ ! -d $dir_path ]; then
        return 1;
    fi

    return 0;
}

function check_file_exist()
{
    if [ ! -f $1 ]; then
        return 1;
    fi
    return 0;
}

function build_libzmq()
{
    cd ${ROOT}/external/libzmq;

    check_dir_exist build
    if [ $? -eq 0 ]; then
        rm -rf CMakeFiles
        rm -rf CMakeCache.txt
        cd build;
    else
        mkdir build && cd build;
    fi

    cmake ..;
    make;
    go_back;

    psucc "make libzmq ok."
}

function build_phxpaxos()
{
    cd ${ROOT}/external/phxpaxos;
    ./MakeMe.sh;
    go_back;

    psucc "make phxpaxos ok."
}

if [ ! -n "${ROOT}" ]; then
    echo "Not in COMO build environment!";
    exit 0;
fi

build_libzmq;
build_phxpaxos;

psucc "COMO build, Make optional external done."
