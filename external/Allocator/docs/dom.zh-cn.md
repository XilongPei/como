**文档来源：**https://github.com/Tencent/rapidjson/tree/master/doc




# Allocator应用示例

教程中使用了 `Value` 和 `Document` 类型。与 `std::string` 相似，这些类型其实是两个模板类的 `typedef`：

~~~~~~~~~~cpp
namespace rapidjson {

template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
class GenericValue {
    // ...
};

template <typename Encoding, typename Allocator = MemoryPoolAllocator<> >
class GenericDocument : public GenericValue<Encoding, Allocator> {
    // ...
};

typedef GenericValue<UTF8<> > Value;
typedef GenericDocument<UTF8<> > Document;

} // namespace rapidjson
~~~~~~~~~~

使用者可以自定义这些模板参数。



## 使用者缓冲区 {#UserBuffer}

许多应用软件可能需要尽量减少内存分配。

`MemoryPoolAllocator` 可以帮助这方面，它容许使用者提供一个缓冲区。该缓冲区可能置于程序堆栈，或是一个静态分配的「草稿缓冲区（scratch buffer）」（一个静态／全局的数组），用于储存临时数据。

`MemoryPoolAllocator` 会先用使用者缓冲区去解决分配请求。当使用者缓冲区用完，就会从基础分配器（缺省为 `CrtAllocator`）分配一块内存。

以下是使用堆栈内存的例子，第一个分配器用于存储值，第二个用于解析时的临时缓冲。

~~~~~~~~~~cpp
typedef GenericDocument<UTF8<>, MemoryPoolAllocator<>, MemoryPoolAllocator<>> DocumentType;
char valueBuffer[4096];
char parseBuffer[1024];
MemoryPoolAllocator<> valueAllocator(valueBuffer, sizeof(valueBuffer));
MemoryPoolAllocator<> parseAllocator(parseBuffer, sizeof(parseBuffer));
DocumentType d(&valueAllocator, sizeof(parseBuffer), &parseAllocator);
d.Parse(json);
~~~~~~~~~~

若解析时分配总量少于 4096+1024 字节时，这段代码不会造成任何堆内存分配（经 `new` 或 `malloc()`）。

使用者可以通过 `MemoryPoolAllocator::Size()` 查询当前已分的内存大小。那么使用者可以拟定使用者缓冲区的合适大小。
