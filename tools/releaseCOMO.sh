#!/bin/bash

# COMO运行时发布工具

# OUT_PATH=/home/xilong/home/como/out/target/como.linux.x64.rls
# LIB_PATH=/home/xilong/home/como/bin/target/como.linux.x64.rls
# COMORT_PATH=/home/xilong/home/como/bin/target/como.linux.x64.rls/comort.so
# CLASS_PATH=/home/xilong/home/como/bin/target/como.linux.x64.rls/libcore.so

while getopts ":n:h" optname;
do
    case "$optname" in
        "n")
            echo "get option -n, value is $OPTARG"
            ;;
        "h")
cat <<EOF
copy COMO runtime file into target, work at COMO development.
$ releaseCOMO.sh targetPath
EOF
            ;;
        ":")
            echo "This option -$OPTARG requires an argument."
            exit 1
            ;;
        "?")
            # found an invalid option
            echo "-$OPTARG is not an option"
            exit 2
            ;;
        *)
            echo "Unknown error while processing options"
            ;;
    esac
done

# if getopts find a non-option argument
# it will stop parsing and return OPTIND
# as index to the first non-option argument
#
# if getopts doesn't find any non-option argument
# it will set OPTIND=$#+1
if [ $OPTIND -ne $(($#+1)) ]
then
    if [ ! -d $1 ]; then
        mkdir $1
    fi
    cp -avx ${COMORT_PATH} $1
    cp -avx ${CLASS_PATH} $1
fi
