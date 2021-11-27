/*
  for assembly program in CProxy.cpp
 */

#include <iostream>
using namespace std;

extern "C" void __entry();

__asm__(
    ".text;"
    ".align 8;"
    ".global __entry;"
    "__entry:"
    "sub    sp, sp, #32;"
    "stp    lr, x9, [sp, #16];"
    "mov    x9, #0x0;"
    "stp    x9, x0, [sp];"
    "ldr    x9, [x0, #8];"
    "mov    x0, sp;"
    "adr    lr, return_from_func;"
    "br     x9;"
    "return_from_func:"
    "ldp    lr, x9, [sp, #16];"
    "add    sp, sp, #32;"
    "ret;"
    "nop;"
    "nop;"
    "nop;"
    "nop;"
    "nop;"
);

class Class2
{
    public:
        int var2;

    public:
    void method2(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8, int i9, int i10, int i11, int i12)
    {
        cout << "call class2->method2, var2: " << var2 << endl;
    };
};

class Class1 : public Class2
{
    public:
        int var1;

    public:
    void method1(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8, int i9, int i10, int i11, int i12)
    {
        cout << "call class1->method1, var1: " << var1 << endl;
    };
    /*
    void method2(int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8, int i9, int i10, int i11, int i12)
    {
        cout << "call class1->method2, var2: " << var2 << endl;
    };*/
};

int main()
{
    Class1 *class1 = new Class1();

    class1->method1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    class1->method2(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
    return 0;
}

/*
g++ -g cpp_test.cpp
objdump -d -S -C a.out

*/
