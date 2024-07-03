### 1. git clone sub-repository

`git clone` with its sub-repository by adding `--recursive`:

```sh
git clone --recursive https://gitee.com/tjopenlab/como
```

If the repository is already cloned, add sub-repository with commands below:

```sh
git pull --recurse-submodules
git submodule update --remote --recursive
git submodule update --recursive
```

### 2. No rule to make libphxpaxos.a

Issue:

```sh
error:
make[2]: *** No rule to make target '../../../external/phxpaxos/.lib/libphxpaxos.a', needed by 'servicemanager/exe/linux/servicemanager'.  Stop.
make[1]: *** [CMakeFiles/Makefile2:3076: servicemanager/exe/linux/CMakeFiles/servicemanager.dir/all] Error 2
make: *** [Makefile:136: all] Error 2
```

Solution:

Build [phxpaxos - Gitee](https://gitee.com/tjopenlab/phxpaxos) with README instructions.

Python install (Required by MakeMe.sh):
```sh
sudo apt install python3 python-is-python3
```

Then execute `MakeMe.sh` in the directory `external/phxpaxos`.

