pwd
cd ..
rm -rf build/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. # debug mode
make
sudo make install
cd ../export_como
export LD_LIBRARY_PATH=/usr/local/lib/mimalloc-1.7/ 
g++ -o test main.cpp -lmimalloc-debug -I/usr/local/include/mimalloc-1.7/ -I../include -g
