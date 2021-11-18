
# 清空gtest-runner中原有内容
# gtest-runner -r

# 在工作下的test目录找所有可执行文件，这些是测试用例的可执行程序，把这些加入到gtest-runner中
# find ./test -executable -type f

testCases=" \
./test/entrypoint/testEntryPoint \
./test/compiler/testCompiler \
./test/runtime/comostring/testString \
./test/runtime/localstaticvariable/testLocalStaticVariable \
./test/runtime/char/testChar \
./test/runtime/macro/testMacro \
./test/runtime/autoptr/testAutoPtr \
./test/runtime/rpc/component/RPCTestUnit.so \
./test/runtime/rpc/client/testRPCCli \
./test/runtime/rpc/service/testRPCSrv \
./test/runtime/array/testArray \
./test/runtime/outreferencetype/component/ReferenceTypeTest.so \
./test/runtime/outreferencetype/client/testReferenceType \
./test/runtime/nestedinterface/testNestedInterface \
./test/runtime/reflection/component/ReflectionTestUnit.so \
./test/runtime/reflection/client/testReflection \
./test/libcore/como/io/fileinputstream/testFileInputStream \
./test/libcore/como/io/charset/charset/testCharset \
./test/libcore/como/io/file/testFile \
./test/libcore/como/core/stringutils/testStringUtils \
./test/libcore/como/core/thread/client/testThreadCli \
./test/libcore/como/math/bigdecimal/testBigDecimal \
./test/libcore/como/math/biginteger/testBigInteger \
./test/libcore/como/util/hashmap/testHashMap \
./test/libcore/como/util/concurrent/concurrenthashmap/testConcurrentHashMap \
./test/libcore/como/util/concurrent/atomic/atomicinteger/testAtomicInteger \
./test/libcore/como/util/concurrent/concurrentlinkedqueue/testConcurrentLinkedQueue \
./test/libcore/como/util/stringtokenizer/testStringTokenizer \
./test/libcore/como/util/vector/testVector \
./test/libcore/como/util/hashtable/testHashtable \
./test/libcore/como/util/random/testRandom \
./test/libcore/como/util/treemap/testTreeMap \
./test/libcore/como/util/arraylist/testArrayList \
./test/libcore/como/util/locale/testLocale \
./test/libcore/como/util/calendar/testCalendar \
./test/libcore/como/util/date/testDate \
./test/libcore/como/util/formatter/testFormatter \
./test/libcore/como/text/attributedstring/testAttributedString \
./test/libcore/libcore/util/zoneinfo/testZoneInfo"

CMD="gtest-runner"

for testcase in $testCases
do
    echo "$CMD $testcase"
done
