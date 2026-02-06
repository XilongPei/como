
#include <dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

// =============================================================================
// Symbol Structure
// =============================================================================
struct Symbol {
    std::string name;
    unsigned char info;  // ELF symbol info field (bind + type)
};

// =============================================================================
// Extract symbols from ELF shared library and verify dlsym availability
// =============================================================================
std::vector<Symbol> GetSymbols(const char* path, void* handle) {
    std::vector<Symbol> result;

    // Open file (MISRA: check return value)
    const int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return result;
    }

    // Get file size (MISRA: check return value)
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return result;
    }

    // Memory map the file (MISRA: check for MAP_FAILED)
    void* const map = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        return result;
    }

    // Parse ELF header (MISRA: use reinterpret_cast for pointer arithmetic)
    const Elf64_Ehdr* const eh = static_cast<const Elf64_Ehdr*>(map);
    const Elf64_Shdr* const sh = reinterpret_cast<const Elf64_Shdr*>(
        static_cast<const char*>(map) + eh->e_shoff);
    const char* const shstr = static_cast<const char*>(map) + sh[eh->e_shstrndx].sh_offset;

    // Find .dynsym and .dynstr sections
    const Elf64_Shdr* dynsym = nullptr;
    const Elf64_Shdr* dynstr = nullptr;

    for (int i = 0;  i < eh->e_shnum;  i++) {
        const char* name = shstr + sh[i].sh_name;
        if (strcmp(name, ".dynsym") == 0) {
            dynsym = &sh[i];
        } else if (strcmp(name, ".dynstr") == 0) {
            dynstr = &sh[i];
        }
    }

    // MISRA: Verify pointers before dereferencing
    if ((dynsym == nullptr) || (dynstr == nullptr)) {
        munmap(map, st.st_size);
        close(fd);
        return result;
    }

    // Extract symbols (MISRA: use reinterpret_cast for pointer arithmetic)
    const char* const strtab = static_cast<const char*>(map) + dynstr->sh_offset;
    const Elf64_Sym* const syms = reinterpret_cast<const Elf64_Sym*>(
        static_cast<const char*>(map) + dynsym->sh_offset);
    const int symbol_count = static_cast<int>(dynsym->sh_size / sizeof(Elf64_Sym));

    for (int i = 0;  i < symbol_count;  i++) {
        // Skip symbols that cannot be resolved by dlsym

        // 1. Must have a name
        if (syms[i].st_name == 0) {
            continue;
        }

        // 2. Must be global binding
        if (ELF64_ST_BIND(syms[i].st_info) != STB_GLOBAL) {
            continue;
        }

        // 3. Must be function type
        if (ELF64_ST_TYPE(syms[i].st_info) != STT_FUNC) {
            continue;
        }

        // 4. CRITICAL: Must NOT be GNU_IFUNC (indirect function)
        //    IFUNC functions resolve at runtime based on CPU features
        //    dlsym returns the resolver, not the actual implementation
        if (ELF64_ST_TYPE(syms[i].st_info) == STT_GNU_IFUNC) {
            continue;
        }

        // 5. Must be defined (not undefined reference)
        if (syms[i].st_shndx == SHN_UNDEF) {
            continue;
        }

        // 6. Must have default visibility (not hidden/internal)
        if (ELF64_ST_VISIBILITY(syms[i].st_other) != STV_DEFAULT) {
            continue;
        }

        // Verify symbol is dlsym-resolvable
        void* const sym_addr = dlsym(handle, strtab + syms[i].st_name);
        if (sym_addr == nullptr) {
            continue;
        }

        Symbol s;
        s.name = strtab + syms[i].st_name;
        s.info = syms[i].st_info;
        result.push_back(s);
    }

    // Cleanup (MISRA: close file handle on all exit paths)
    munmap(map, st.st_size);
    close(fd);

    return result;
}

// =============================================================================
// Sandbox call: compare function outputs in isolated process
// =============================================================================
bool SandboxCall(void* f1, void* f2) {
    const pid_t pid = fork();
    if (pid < 0) {
        // Fork failed
        return false;
    }

    if (pid == 0) {
        // Child process: call both functions and compare results
        using FuncType = int (*)();

        FuncType fn1 = reinterpret_cast<FuncType>(f1);
        FuncType fn2 = reinterpret_cast<FuncType>(f2);

        // volatile prevents compiler from eliminating calls
        volatile int r1 = fn1();
        volatile int r2 = fn2();

        // Exit with 0 if results match, 1 otherwise
        _exit((r1 == r2) ? 0 : 1);
    }

    // Parent process: wait for child
    int status = 0;
    const pid_t wait_result = waitpid(pid, &status, 0);
    if (wait_result < 0) {
        return false;
    }

    // Check if child was terminated by signal (MISRA: check wait status)
    if (WIFSIGNALED(status)) {
        return false;
    }

    // Return true if exit status is 0 (results matched)
    return (WEXITSTATUS(status) == 0);
}

// =============================================================================
// Main entry point: Compare symbols in two shared libraries
// =============================================================================
int main(int argc, char** argv) {
    // Validate arguments
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " libA.so libB.so\n";
        return 0;
    }

    // Load both libraries first (dlsym needs handle)
    void* const h1 = dlopen(argv[1], RTLD_NOW);
    void* const h2 = dlopen(argv[2], RTLD_NOW);

    if ((h1 == nullptr) || (h2 == nullptr)) {
        std::cerr << "dlopen failed\n";
        return 1;
    }

    // Extract symbols from first library with dlsym verification
    const std::vector<Symbol> syms = GetSymbols(argv[1], h1);

    // Compare each symbol
    int ok_count = 0;
    int fail_count = 0;

    for (const Symbol& sym : syms) {
        // Resolve symbol addresses
        void* const f1 = dlsym(h1, sym.name.c_str());
        void* const f2 = dlsym(h2, sym.name.c_str());

        if ((f1 == nullptr) || (f2 == nullptr)) {
            // Symbol not found in one or both libraries
            ++fail_count;
            std::cout << "[MISS] " << sym.name << "\n";
            continue;
        }

        // Sandbox and compare outputs
        if (SandboxCall(f1, f2)) {
            ok_count++;
        }
        else {
            fail_count++;
            std::cout << "[DIFF] " << sym.name << "\n";
        }
    }

    // Print summary
    std::cout << "\nOK=" << ok_count << " FAIL=" << fail_count << "\n";

    return 0;
}
