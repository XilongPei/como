%.o: %.cc
    $(CXX) -o $@ -c $^ -Ipath/to/pybind11-2.2.3/include $(shell python3-config --includes)

main: main.o
    $(CXX) -o $@ $^ $(shell python3-config --libs)
