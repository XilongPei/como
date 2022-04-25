export LD_LIBRARY_PATH=/usr/local/lib/mimalloc-1.7/ 
pwd
cd ..
rm -rf build/
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
sudo make install
cd ../export_como
g++ -o test main.cpp -lmimalloc-debug -I/usr/local/include/mimalloc-1.7/ -I../include -g
