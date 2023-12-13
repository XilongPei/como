# COMO

COMO is a C++ Component Model. It can:
1. support interface-oriented programming;
2. support C++ runtime reflection;

**cdlc** is the .cdl compiler.

**comort** is the como component runtime.

**libcore** is the core library.

### Install build-essential
###### openEuler
```shell
sudo yum groupinstall 'Development Tools'
```
###### Ubuntu
```shell
sudo apt install build-essential
```

### Install cmake
###### openEuler
```shell
sudo yum install cmake
```
###### Ubuntu
```shell
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:george-edison55/cmake-3.x
sudo apt-get update
sudo apt-get install cmake
```

### Install DBus
###### openEuler
```shell
安装dbus
sudo yum install dbus-devel
查找dbus.h位置
$ sudo locate dbus.h
如果提示错误，执行 $ sudo updatedb
/usr/local/include/dbus-1.0/dbus/dbus.h
/usr/include/dbus-1.0/dbus/dbus.h
dbus默认安装到了dbus-1.0位置，需要调整dbus头文件位置，调整方法如下：
$cd /usr/include
$ ln -sf dbus-1.0/dbus

出现错误dbus/dbus-arch-deps.h：No such file or directory
查找该文件
$ locate dbus-arch-deps.h
/usr/local/lib/dbus-1.0/include/dbus/dbus-arch-deps.h
/usr/lib/dbus-1.0/include/dbus/dbus-arch-deps.h
将/usr/lib/dbus-1.0/include/dbus/dbus-arch-deps.h文件复制到/usr/include/dbus目录下
```
###### Ubuntu
```shell
sudo apt-get update
sudo apt-get install libdbus-1-dev
```

### Install Unwind
###### openEuler
```shell
wget http://mirror.yongbok.net/nongnu/libunwind/libunwind-1.5.0.tar.gz
tar zxvf libunwind-1.5.0.tar.gz
cd libunwind-1.5.0/
CFLAGS=-fPIC ./configure
make CFLAGS=-fPIC
make CFLAGS=-fPIC install
```
###### Ubuntu
```shell
sudo apt-get install libunwind8-dev
```

### Install ICU
###### openEuler
```shell
sudo yum install libicu-devel

安装 icu4c 最新版
wget https://github.com/unicode-org/icu/releases/download/release-68-2/icu4c-68_2-src.tgz
tar -xf icu4c-68_2-src.tgz
cd icu/source
./configure --prefix=/usr
gmake
gmake install
```
###### Ubuntu
```shell
sudo apt-get install libicu-dev
```

### Install autoreconf libtool
###### Ubuntu
```shell
sudo apt-get install autoreconf
sudo apt-get install libtool
```

### How to build
1. <code>cd como</code>

2. <code>source build/envsetup.sh</code>

   **You will get these commands:**

   ```shell
   $ help
   - comotools:            Switch to build como tools.
   - como_linux_x64:       Switch to build como for linux x64.
   - como_android_aarch64: Switch to build como for android aarch64.
   - debug:                Switch to build debug version.
   - release:              Switch to build release version.
   - build:                Build source codes.
   - build install:        Build source codes and install the building results.
   - rebuild:              Rebuild source codes.
   - rebuild install:      Rebuild source codes and install the building results.
   - clobber:              Clean the building results and generated files of the current project.
   - clobber all:          Clean all of the building results and generated files.
   ```

3. choose build target:
   + <code>comotools</code> to build tools
   + <code>como_linux_x64</code> to build como for linux x64
   + <code>como_android_aarch64</code> to build como for android aarch64

4. <code>build</code> or <code>build -j*n*</code> for release build

### Change build type
1. in any build target:
   + <code>debug</code> for debug build
   + <code>release</code> for release build

### Rebuild
1. <code>rebuild</code> or <code>rebuild -j*n*</code>

### Clobber
1. <code>clobber</code>

### Copy building results
if the build target is android aarch64, you need copy building results to the device.
+ <code>drop</code> or <code>drop all</code> to copy all modules
+ <code>drop core</code> to copy core modules
+ <code>drop test</code> to copy testcase modules

### Other commands
+ <code>root</code> to change to como root directory
+ <code>out</code> to change to out directory
+ <code>bin</code> to change to bin directory

-------------------------------------

# build external/ directory

build external with program:

```shell
./MakeOptionalExternal.sh
```

It needs python interpreator in your PATH environment.
