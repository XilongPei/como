
# Auto SO Equivalence Test (ELF symbol-table rewrite verification)

## Build
mkdir build
cd build
cmake ..
make

## Run example (compare libm to itself)
./auto_so_equiv /usr/lib/x86_64-linux-gnu/libm.so.6 /usr/lib/x86_64-linux-gnu/libm.so.6

## Or compare your generated proxy
./auto_so_equiv liborig.so libgen.so
