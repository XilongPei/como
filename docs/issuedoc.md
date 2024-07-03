### 1. git clone复制子仓库

git clone时需要同时clone其子仓库：

```sh
git clone --recursive https://gitee.com/tjopenlab/como
git pull --recurse-submodules
git submodule update --remote --recursive
git submodule update --recursive
```

### 2. No rule to make libphxpaxos.a

问题：

```sh
error:
make[2]: *** No rule to make target '../../../external/phxpaxos/.lib/libphxpaxos.a', needed by 'servicemanager/exe/linux/servicemanager'.  Stop.
make[1]: *** [CMakeFiles/Makefile2:3076: servicemanager/exe/linux/CMakeFiles/servicemanager.dir/all] Error 2
make: *** [Makefile:136: all] Error 2
```

解决方法：

[phxpaxos子仓库](https://gitee.com/tjopenlab/phxpaxos)也需要build。按照子仓库的说明build即可。

python安装（MakeMe.sh需要）：
```sh
sudo apt install python3 python-is-python3
```

在external/phxpaxos根目录下执行 ./MakeMe.sh。

