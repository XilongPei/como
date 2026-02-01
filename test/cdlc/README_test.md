# ElfProxyBuilder Test

## Build

```bash
# From current directory
g++ -o elf_proxy_builder_test elf_proxy_builder_test.cpp ../../como/tools/cdlc/util/ElfProxyBuilder.cpp -I. -std=c++11
```

## Usage

```bash
# Basic test
./elf_proxy_builder_test /path/to/libexample.so ./output/proxy.so

# Custom session data
./elf_proxy_builder_test /path/to/libexample.so ./output/proxy.so "custom_session_data"
```

## Verify Output

```bash
# Check ELF header
readelf -h ./output/proxy.so

# Check dynamic entries (should contain both DT_HASH and DT_GNU_HASH)
readelf -d ./output/proxy.so

# Check symbol table
readelf -s ./output/proxy.so

# Check relocations
readelf -r ./output/proxy.so

# Check notes (should contain .note.gnu.build-id and .note.metadata)
readelf -n ./output/proxy.so
```
