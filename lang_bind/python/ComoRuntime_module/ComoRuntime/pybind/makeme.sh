#该编译命令会生成 como_pybind.cpython-35m-x86_64-linux-gnu.so
g++ -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` como_pybind.cpp -o gemfield`python3-config --extension-suffix`
