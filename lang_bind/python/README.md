# COMO反射机制的python绑定ComoRuntime



[ComoRuntime](https://gitee.com/tjopenlab/como) 是一个COMO反射机制的python绑定，本文档将介绍python程序如何通过ComoRuntime操作COMO构件。




## 安装环境需求

[ComoRuntime](https://ComoRuntime.cc/) builds on modern Mac OS and Linux distributions.
Since it uses C\++11 features, it requires a compiler with good C++11 support. You will need [Python](https://www.python.org/) (version 2.7 or ≥ 3.4), [NumPy](http://www.numpy.org/) & [SciPy](https://www.scipy.org/) and [pybind11](https://github.com/pybind/pybind11).



## 安装

安装最新发行包 :
```bash
$ pip install ComoRuntime
```

或者从源码仓库拉取最新代码并从源码开始安装：
```bash
$ git clone https://gitee.com/tjopenlab/como
$ cd como/lang_bind/python
$ sudo pip install .
$ # or :
$ sudo python setup.py install
```



## 开发

python程序中如何通过ComoRuntime操作COMO构件。

#### class ComoComponent

通过COMO的构件加载机制加载构件，并得到这个构件的反射信息（对应COMO的接口IMetaComponent）。

##### 构造函数：

```python
ComoComponent(构件名字字符串)
```

##### COMO的构件加载机制：

- CoGetComponentMetadata，通过构件唯一标识ComponentID加载构件
- CoGetComponentMetadataWithPath，通过构件文件名加载构件

| 函数名            | 功能                                     |
| ----------------- | ---------------------------------------- |
| GetName           | - 功能：获取构件名称<br>- 输出：构件名称 |
| GetComponentID    | - 功能：获取构件唯一标识ComponentID<br>- 输出：构件唯一标识ComponentID |
| **常量Constant** |  |
| GetConstantNumber | - 功能：获取常量数量<br>- 输出：常量数量，整型值 |
| GetAllConstants   | - 功能：获取所有常量<br>- 输出：所有常量，数组 |
| GetConstant       | - 功能：获取常量值<br>- 输入：常量名称<br>- 输出：常量值|
| **枚举值Enumeration** |  |
| GetEnumerationNumber | - 功能：获取枚举值数量<br>- 输出：枚举值数量，整型值 |
| GetAllEnumerations   | - 功能：获取所有枚举值<br>- 输出：所有枚举值，数组 |
| GetEnumeration       | - 功能：获取枚举值值<br>- 输入：枚举值名称<br>- 输出：枚举值值|
| **接口Interface** |  |
| GetInterfaceNumber | - 功能：获取接口数量<br>- 输出：接口数量，整型值 |
| GetAllInterfaces   | - 功能：获取所有接口<br>- 输出：所有接口对象，数组 |
| GetInterface       | - 功能：获取接口对象<br>- 输入：接口唯一标识InterfaceID<br>- 输出：接口对象 |
| GetInterface       | - 功能：获取接口对象<br>- 输入：接口名称<br>- 输出：接口对象 |
| **构件类Coclass** |  |
| GetCoclassNumber | - 功能：获取构件类数量<br>- 输出：构件类数量，整型值 |
| GetCoclasses   | - 功能：获取所有构件类<br>- 输出：所有构件类对象，数组 |
| GetCoclass       | - 功能：获取构件类<br>- 输入：构件类唯一标识InterfaceID<br>- 输出：构件类对象 |
| GetCoclass       | - 功能：获取构件类<br>- 输入：构件类名称<br>- 输出：构件类对象 |
