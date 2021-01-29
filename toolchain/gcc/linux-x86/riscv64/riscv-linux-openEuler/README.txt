riscv64--glibc--bleeding-edge-2020.08-1


Toolchains are hosted here: http://toolchains.bootlin.com/

All the licenses can be found here: http://toolchains.bootlin.com/downloads/releases/licenses/
All the sources can be found here: http://toolchains.bootlin.com/downloads/releases/sources/

PACKAGE      VERSION              LICENSE
buildroot    2020.08-14-ge5a2a90  GPL-2.0+
gcc-final    10.2.0               unknown
bison        3.6.4                GPL-3.0+
m4           1.4.18               GPL-3.0+
gawk         5.1.0                GPL-3.0+
gcc-initial  10.2.0               unknown
binutils     2.34                 GPL-3.0+, libiberty LGPL-2.1+
gmp          6.1.2                LGPL-3.0+ or GPL-2.0+
mpc          1.1.0                LGPL-3.0+
mpfr         4.0.2                LGPL-3.0+
python3      3.8.5                Python-2.0, others
autoconf     2.69                 GPL-3.0+ with exceptions
libtool      2.4.6                GPL-2.0+ (libtool), LGPL-2.1+ (libltdl)
automake     1.15.1               GPL-2.0+
expat        2.2.9                MIT
pkgconf      1.6.1                pkgconf license
libffi       3.3                  MIT
libzlib      1.2.11               Zlib
ncurses      6.1                  MIT with advertising clause
patchelf     0.9                  GPL-3.0+
glibc          2.31-54-g6fdf971c9dbf7dac9bea552113fe4694015bbc4d  GPL-2.0+ (programs), LGPL-2.1+, BSD-3-Clause, MIT (library)
linux-headers  5.4.61                                             GPL-2.0
gdb            8.2.1                                              GPL-2.0+, LGPL-2.0+, GPL-3.0+, LGPL-3.0+
ncurses        6.1                                                MIT with advertising clause

For those who would like to reproduce the toolchain, you can just follow these steps:

    git clone https://github.com/bootlin/buildroot-toolchains.git buildroot
    cd buildroot
    git checkout toolchains.bootlin.com-2020.08-1

    curl http://toolchains.bootlin.com/downloads/releases/toolchains/riscv64/build_fragments/riscv64--glibc--bleeding-edge-2020.08-1.defconfig > .config
    make olddefconfig
    make

This toolchain has been built, but the infrastructure does not contains enough
informations about testing it.
This doesn't mean that this toolchain doesn't work, just that it hasn't been
fully tested.
FLAG: CAN-NOT-TEST
