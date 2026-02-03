//=========================================================================
// Copyright (C) 2026 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <functional>
#include "util/Logger.h"
#include "ElfProxyBuilder.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

namespace cdlc {

const char* TAG = "ElfProxyBuilder";

// ----------------------------------------------------------------------
// ELF constants (for portability across toolchains)
// ----------------------------------------------------------------------
#ifndef EM_RISCV
#define EM_RISCV 243
#endif
#ifndef R_X86_64_GLOB_DAT
#define R_X86_64_GLOB_DAT 10
#endif
#ifndef R_AARCH64_GLOB_DAT
#define R_AARCH64_GLOB_DAT 1025
#endif
#ifndef R_RISCV_64
#define R_RISCV_64 3
#endif
#ifndef R_RISCV_JUMP_SLOT
#define R_RISCV_JUMP_SLOT 21
#endif
#ifndef R_AARCH64_JUMP_SLOT
#define R_AARCH64_JUMP_SLOT 1027
#endif
#ifndef NT_GNU_BUILD_ID
#define NT_GNU_BUILD_ID 3
#endif
#ifndef DT_VERSYM
#define DT_VERSYM 0x6ffffff0
#endif
#ifndef DT_VERDEF
#define DT_VERDEF 0x6ffffffc
#endif
#ifndef DT_VERNEED
#define DT_VERNEED 0x6ffffffe
#endif
#ifndef DT_RELA
#define DT_RELA 7
#endif
#ifndef DT_RELASZ
#define DT_RELASZ 8
#endif

// ----------------------------------------------------------------------
// Pure C SHA-1 implementation (Public Domain)
// Adapted from https://github.com/clibs/sha1
// ----------------------------------------------------------------------
#define SHA1_BLOCK_SIZE 64
#define SHA1_DIGEST_SIZE 20

typedef struct {
    uint32_t state[5];
    uint64_t count;
    uint8_t buffer[SHA1_BLOCK_SIZE];
} SHA1_CTX;

void SHA1_Init(
    /* [in] */ SHA1_CTX* ctx)
{
    ctx->state[0] = 0x67452301U;
    ctx->state[1] = 0xEFCDAB89U;
    ctx->state[2] = 0x98BADCFEU;
    ctx->state[3] = 0x10325476U;
    ctx->state[4] = 0xC3D2E1F0U;
    ctx->count = 0;
}

uint32_t sha1_rol(
    /* [in] */ uint32_t value,
    /* [in] */ size_t bits)
{
    return (value << bits) | (value >> (32 - bits));
}

void sha1_transform(
    /* [in] */ uint32_t state[5],
    /* [in] */ const uint8_t buffer[SHA1_BLOCK_SIZE])
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];
    uint32_t w[80];

    for (int i = 0;  i < 16;  i++) {
        w[i] = (static_cast<uint32_t>(buffer[i * 4 + 0]) << 24) |
               (static_cast<uint32_t>(buffer[i * 4 + 1]) << 16) |
               (static_cast<uint32_t>(buffer[i * 4 + 2]) << 8)  |
               (static_cast<uint32_t>(buffer[i * 4 + 3]));
    }
    for (int i = 16;  i < 80;  i++) {
        w[i] = sha1_rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    for (int i = 0;  i < 80;  i++) {
        uint32_t f, k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999U;
        }
        else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1U;
        }
        else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDCU;
        }
        else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6U;
        }
        uint32_t temp = sha1_rol(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = sha1_rol(b, 30);
        b = a;
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
}

void SHA1_Update(
    /* [in] */ SHA1_CTX* ctx,
    /* [in] */ const void* data,
    /* [in] */ size_t len)
{
    const uint8_t* input = static_cast<const uint8_t*>(data);
    size_t free = SHA1_BLOCK_SIZE - (ctx->count % SHA1_BLOCK_SIZE);

    while (len >= free) {
        memcpy(ctx->buffer + (ctx->count % SHA1_BLOCK_SIZE), input, free);
        ctx->count += free;
        input += free;
        len -= free;
        sha1_transform(ctx->state, ctx->buffer);
        free = SHA1_BLOCK_SIZE;
    }

    if (len > 0) {
        memcpy(ctx->buffer + (ctx->count % SHA1_BLOCK_SIZE), input, len);
        ctx->count += len;
    }
}

void SHA1_Final(
    /* [out] */ unsigned char md[SHA1_DIGEST_SIZE],
    /* [in] */ SHA1_CTX* ctx)
{
    uint64_t bit_count = ctx->count * 8;
    size_t index = ctx->count % SHA1_BLOCK_SIZE;

    // Check if we need to process current block before appending 0x80
    // Need space for: 0x80 (1 byte) + zeros + bit length (8 bytes) = 9 bytes minimum
    // If index >= 56, current block has <= 8 bytes remaining, insufficient for padding
    if (index >= SHA1_BLOCK_SIZE - 8) {
        // Not enough space for padding (0x80 + zeros + 8-byte length)
        // Fill current block with zeros and process it
        while (index < SHA1_BLOCK_SIZE) {
            ctx->buffer[index++] = 0;
        }
        sha1_transform(ctx->state, ctx->buffer);
        // Start fresh with new block (index = 0)
        index = 0;
        // NOTE: After this if block, we will append 0x80 to the NEW block below
    }

    // Append 0x80 byte (marks end of message)
    // This is either appended to current block (if space available) or new block (if we just processed one)
    ctx->buffer[index++] = 0x80;

    // Pad with zeros until offset 56 (leaving 8 bytes for bit length)
    while (index < 56) {
        ctx->buffer[index++] = 0;
    }

    // Append 8-byte big-endian bit length
    for (int i = 0;  i < 8;  i++) {
        ctx->buffer[index + i] = static_cast<uint8_t>((bit_count >> ((7 - i) * 8)) & 0xFFu);
    }

    // Process the final block (this block now contains: 0x80 + zeros + bit_length)
    sha1_transform(ctx->state, ctx->buffer);

    // Output digest (big-endian)
    for (int i = 0;  i < 5;  i++) {
        md[i * 4 + 0] = static_cast<uint8_t>((ctx->state[i] >> 24) & 0xFFU);
        md[i * 4 + 1] = static_cast<uint8_t>((ctx->state[i] >> 16) & 0xFFU);
        md[i * 4 + 2] = static_cast<uint8_t>((ctx->state[i] >> 8) & 0xFFU);
        md[i * 4 + 3] = static_cast<uint8_t>(ctx->state[i] & 0xFFU);
    }
}

// ----------------------------------------------------------------------
// Constants and types
// ----------------------------------------------------------------------
constexpr size_t kMaxSyms = 1024;
constexpr uint64_t kPageSize = 0x1000U;

struct Symbol {
    char name[256];
};

// Global state (must be reset/protected for thread safety)
Symbol g_symbols[kMaxSyms];
size_t g_symbol_count = 0;
TargetArch g_target_arch = TargetArch::kX86_64;

uint64_t AlignUp(
    /* [in] */ uint64_t value,
    /* [in] */ uint64_t align)
{
    return (value + align - 1) & ~(align - 1);
}

// ELF hash function (glibc style)
uint32_t ElfHash(
    /* [in] */ const char* name)
{
    uint32_t h = 0, g;
    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000U;
        if (0 != g) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h;
}

uint64_t GetStubSize()
{
    switch (g_target_arch) {
        case TargetArch::kX86_64:
            return 6;
        case TargetArch::kAARCH64:
            return 12;
        case TargetArch::kRISCV64:
            return 12;
        default:
            return 0;
    }
}

int ParseOriginDynSym(
    /* [in] */ const std::string& path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        //Die("open origin");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        //Die("fstat origin");
        return -2;
    }

    Elf64_Ehdr eh;
    if (read(fd, &eh, sizeof(eh)) != static_cast<ssize_t>(sizeof(eh))) {
        //Die("read ehdr");
        return -3;
    }

    if (memcmp(eh.e_ident, ELFMAG, SELFMAG) != 0) {
        //Die("not elf");
        return -4;
    }

    if (eh.e_machine == EM_X86_64) {
        g_target_arch = TargetArch::kX86_64;
    }
    else if (eh.e_machine == EM_AARCH64) {
        g_target_arch = TargetArch::kAARCH64;
    }
    else if (eh.e_machine == EM_RISCV) {
        g_target_arch = TargetArch::kRISCV64;
    }
    else {
        Logger::E(TAG, "Unsupported arch: 0x%04x (need x86-64=62, "
                       "aarch64=183, riscv=243)\n", eh.e_machine);
        //Die("unsupported arch");
        return -5;
    }

    Elf64_Shdr* shdrs = static_cast<Elf64_Shdr*>(
                                        malloc(eh.e_shnum * sizeof(Elf64_Shdr)));
    if (nullptr == shdrs) {
        return -6;
    }

    lseek(fd, static_cast<off_t>(eh.e_shoff), SEEK_SET);
    if (read(fd, shdrs, eh.e_shnum * sizeof(Elf64_Shdr)) !=
                        static_cast<ssize_t>(eh.e_shnum * sizeof(Elf64_Shdr))) {
        //Die("read shdrs");
        return -7;
    }

    Elf64_Shdr* dynsym_sh = nullptr;
    Elf64_Shdr* dynstr_sh = nullptr;
    for (int i = 0;  i < eh.e_shnum;  i++) {
        if (shdrs[i].sh_type == SHT_DYNSYM) {
            dynsym_sh = &shdrs[i];
            if (dynsym_sh->sh_link < static_cast<unsigned>(eh.e_shnum)) {
                dynstr_sh = &shdrs[dynsym_sh->sh_link];
            }
            break;
        }
    }

    if (!dynsym_sh || !dynstr_sh) {
        //Die("no dynsym/dynstr");
        return -8;
    }

    if (dynstr_sh->sh_offset + dynstr_sh->sh_size > static_cast<uint64_t>(st.st_size)) {
        //Die("dynstr out of bounds");
        return -9;
    }

    char* strtab = static_cast<char*>(malloc(dynstr_sh->sh_size));
    if (nullptr == strtab) {
        return -10;
    }

    lseek(fd, static_cast<off_t>(dynstr_sh->sh_offset), SEEK_SET);
    if (read(fd, strtab, dynstr_sh->sh_size) !=
                                    static_cast<ssize_t>(dynstr_sh->sh_size)) {
        //Die("read dynstr");
        return -11;
    }

    size_t nsyms = dynsym_sh->sh_size / sizeof(Elf64_Sym);
    lseek(fd, static_cast<off_t>(dynsym_sh->sh_offset), SEEK_SET);

    for (size_t i = 0;  i < nsyms;  i++) {
        Elf64_Sym sym;
        if (read(fd, &sym, sizeof(sym)) != static_cast<ssize_t>(sizeof(sym))) {
            break;
        }

        if (ELF64_ST_BIND(sym.st_info) != STB_GLOBAL) {
            continue;
        }
        if (ELF64_ST_TYPE(sym.st_info) != STT_FUNC) {
            continue;
        }
        if (sym.st_shndx == SHN_UNDEF) {
            continue;
        }
        if (sym.st_name >= dynstr_sh->sh_size) {
            continue;
        }

        const char* name = strtab + sym.st_name;
        size_t len = strlen(name);
        size_t max_len = sizeof(g_symbols[0].name) - 1;  // Leave space for null terminator

        if (len > max_len) {
            continue;  // Skip symbol names too long for our buffer
        }

        if (g_symbol_count >= kMaxSyms) {
            Logger::E(TAG, "Too many symbols (>%zu), increase kMaxSyms or "
                           "split into multiple libraries", kMaxSyms);
            free(strtab);
            free(shdrs);
            close(fd);
            return -1;
        }

        strncpy(g_symbols[g_symbol_count].name, name, len);
        g_symbols[g_symbol_count].name[len] = '\0';  // Ensure null-terminated
        g_symbol_count++;
    }

    free(strtab);
    free(shdrs);
    close(fd);

    return 0;
}

void WriteGnuHashTable(
    /* [in] */ int fd,
    /* [in] */ uint64_t data_off,
    /* [in] */ uint64_t hash_off_in_data,
    /* [in] */ uint64_t data_va,
    /* [in] */ uint64_t dynsym_off_in_data)
{
    // GNU hash table format
    struct GnuHashHeader {
        uint32_t nbuckets;
        uint32_t symndx;
        uint32_t maskwords;
        uint32_t shift2;
    };

    // Use multiple buckets based on symbol count for better distribution
    // Rule: nbuckets should be power of 2 and >= number of symbols / 4 (roughly)
    // For simplicity, use 1 bucket if symbol_count < 4, else 2 buckets
    constexpr uint32_t kNBuckets = 2;
    constexpr uint32_t kMaskwords = 1;

    GnuHashHeader header = {};
    header.nbuckets = kNBuckets;
    header.symndx = 1;  // Index of first symbol in chain (dynsym[0] is STN_UNDEF)
    header.maskwords = kMaskwords;
    // shift2 is the bloom filter second hash shift constant (glibc uses 5 or 6)
    header.shift2 = 5;

    // Bloom filter
    uint64_t bloom[kMaskwords] = {0};

    // Buckets: each bucket points to the first symbol index that hashes to it
    // Initialize all buckets to 0 (empty)
    uint32_t buckets[kNBuckets] = {0};

    // Chain entries: hash with bit 31 set for last symbol in each bucket's chain
    // chains[i] corresponds to dynsym[i + symndx] = dynsym[i + 1]
    uint32_t* chains = static_cast<uint32_t*>(malloc(g_symbol_count * sizeof(uint32_t)));
    if (nullptr == chains) {
        return;
    }

    // First pass: compute GNU hash for each symbol and store in chains
    // GNU hash: h = h * 33 + c = (h << 5) + h + c
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        const char* name = g_symbols[i].name;
        uint32_t h = 5381;
        while (*name) {
            h = (h << 5) + h + *name++;  // h = h * 33 + c
        }
        uint32_t hash = h;
        chains[i] = hash;
    }

    // Second pass: build bucket chains
    // Chains are stored in symbol order (dynsym order), and each bucket points
    // to the first symbol that hashes to it. The chain is linked via the chain array.
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        uint32_t hash = chains[i];
        uint32_t bucket_id = hash % kNBuckets;

        // Update bloom filter
        uint32_t h1 = hash;
        uint32_t h2 = hash >> header.shift2;
        size_t word1 = (h1 / 64) % kMaskwords;
        size_t word2 = (h2 / 64) % kMaskwords;
        bloom[word1] |= (1ULL << (h1 % 64));
        bloom[word2] |= (1ULL << (h2 % 64));

        // Set bucket pointer if not set (first symbol in this bucket)
        if (buckets[bucket_id] == 0) {
            buckets[bucket_id] = static_cast<uint32_t>(i) + header.symndx;  // 1-based index
        }
    }

    // Third pass: set last bit (bit31 = 1) for the last symbol in each bucket's chain
    // Scan symbols in reverse order and mark the last symbol for each bucket
    uint32_t last_bucket_seen[kNBuckets] = {0};
    for (size_t i = g_symbol_count;  i > 0;  i--) {
        uint32_t idx = i - 1;
        uint32_t hash = chains[idx];
        uint32_t bucket_id = hash % kNBuckets;

        // Mark as last in its chain if we haven't marked any for this bucket yet
        if (last_bucket_seen[bucket_id] == 0) {
            chains[idx] |= 0x80000000U;
            last_bucket_seen[bucket_id] = 1;
        }
    }

    // Write hash table
    lseek(fd, static_cast<off_t>(data_off + hash_off_in_data), SEEK_SET);
    write(fd, &header, sizeof(header));
    write(fd, bloom, sizeof(bloom));
    write(fd, buckets, sizeof(buckets));
    write(fd, chains, g_symbol_count * sizeof(uint32_t));

    free(chains);
}

void WriteSysvHashTable(
    /* [in] */ int fd,
    /* [in] */ uint64_t data_off,
    /* [in] */ uint64_t hash_off_in_data,
    /* [in] */ uint64_t data_va,
    /* [in] */ uint64_t dynsym_off_in_data)
{
    // SysV hash table format
    // nbuckets (4 bytes)
    // nchain   (4 bytes)
    // buckets[nbuckets]
    // chain[nchain]

    uint32_t nbuckets = 1;
    uint32_t nchain = static_cast<uint32_t>(g_symbol_count) + 1;

    // Buckets array (all symbols go to bucket 0)
    // bucket[b] = index of first symbol in this bucket
    uint32_t* buckets = static_cast<uint32_t*>(malloc(nbuckets * sizeof(uint32_t)));
    if (nullptr == buckets) {
        return;
    }
    buckets[0] = 1;  // First symbol is at dynsym[1] (dynsym[0] is STN_UNDEF)

    // Chain array: chain[i] = index of next symbol with same hash, or STN_UNDEF (0)
    // dynsym[0] = STN_UNDEF, dynsym[1..g_symbol_count] = actual symbols
    uint32_t* chain = static_cast<uint32_t*>(malloc(nchain * sizeof(uint32_t)));
    if (nullptr == chain) {
        free(buckets);
        return;
    }

    // Build chain: link all symbols in bucket 0 (all symbols hash to bucket 0)
    // chain[0] is for dynsym[0]=STN_UNDEF (unused in hash lookup)
    // chain[1..g_symbol_count] corresponds to dynsym[1..g_symbol_count]
    chain[0] = 0;  // STN_UNUSED

    // Chain links: 1 -> 2 -> ... -> g_symbol_count -> 0
    // Loop for i = 1 to g_symbol_count-1, each points to i+1
    // Then set chain[g_symbol_count] = 0 to terminate
    if (g_symbol_count > 0) {
        for (size_t i = 1;  i < static_cast<size_t>(g_symbol_count);  i++) {
            chain[i] = i + 1;  // chain[i] points to next symbol index
        }
        chain[g_symbol_count] = 0;  // Terminate the chain
    }
    else {
        // No symbols, bucket should point to STN_UNDEF
        buckets[0] = 0;
    }

    // Write hash table
    lseek(fd, static_cast<off_t>(data_off + hash_off_in_data), SEEK_SET);
    write(fd, &nbuckets, sizeof(nbuckets));
    write(fd, &nchain, sizeof(nchain));
    write(fd, buckets, nbuckets * sizeof(uint32_t));
    write(fd, chain, nchain * sizeof(uint32_t));

    free(chain);
    free(buckets);
}

bool BuildElfProxy(
    /* [in] */ const std::string& origin_so,
    /* [in] */ const std::string& output_so,
    /* [in] */ const void* session_data,
    /* [in] */ size_t session_size)
{
    // Save previous state for thread safety
    Symbol prev_symbols[kMaxSyms];
    size_t prev_symbol_count = g_symbol_count;
    TargetArch prev_arch = g_target_arch;
    memcpy(prev_symbols, g_symbols, sizeof(g_symbols));

    // RAII-style cleanup lambda to restore state
    std::function<void()> cleanup = [&]() {
        memcpy(g_symbols, prev_symbols, sizeof(g_symbols));
        g_symbol_count = prev_symbol_count;
        g_target_arch = prev_arch;
    };

    // Initialize for this build
    g_symbol_count = 0;
    g_target_arch = TargetArch::kX86_64;
    if (ParseOriginDynSym(origin_so) != 0) {
        Logger::E(TAG, "Parse original dynamic symbol error. %s", origin_so.c_str());
        cleanup();
        return false;
    }

    // Check if we have any symbols to export
    if (g_symbol_count == 0) {
        Logger::E(TAG, "No valid symbols found in %s (need GLOBAL FUNC symbols with STB_GLOBAL and STT_FUNC)", origin_so.c_str());
        cleanup();
        return false;
    }

    int fd = open(output_so.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0755);
    if (fd < 0) {
        Logger::E(TAG, "Open file error. %s", output_so.c_str());
        cleanup();
        return false;
    }

    const uint64_t stub_size = GetStubSize();
    const uint64_t ehdr_phdr_sz = sizeof(Elf64_Ehdr) + 2 * sizeof(Elf64_Phdr);
    const uint64_t text_off = AlignUp(ehdr_phdr_sz, kPageSize);
    const uint64_t text_sz = g_symbol_count * stub_size;
    const uint64_t data_off = AlignUp(text_off + text_sz, kPageSize);

    // Data segment internal offsets
    uint64_t got_off_in_data = 0;
    uint64_t rela_off_in_data = AlignUp(got_off_in_data + g_symbol_count * 8, 8);

    // SysV hash table: nbuckets(4) + nchain(4) + buckets(4) + chain(nchain*4)
    uint64_t sysv_hash_off_in_data = AlignUp(rela_off_in_data +
                                             g_symbol_count * sizeof(Elf64_Rela), 8);
    uint32_t sysv_hash_sz = 4 + 4 + 4 + (g_symbol_count + 1) * 4;

    // GNU hash table: header(16) + bloom(8) + buckets(4) + chains((sym_count+1)*4)
    uint64_t gnu_hash_off_in_data = AlignUp(sysv_hash_off_in_data + sysv_hash_sz, 8);
    uint32_t gnu_hash_sz = 16 + 8 + 4 + (g_symbol_count + 1) * 4;

    uint64_t dynsym_off_in_data = AlignUp(gnu_hash_off_in_data + gnu_hash_sz, 8);
    // DYNAMIC after .dynsym
    uint64_t dynamic_off_in_data = AlignUp(dynsym_off_in_data +
                                        (g_symbol_count + 1) * sizeof(Elf64_Sym), 8);
    // DYNSTR after DYNAMIC
    uint64_t dynstr_off_in_data = AlignUp(dynamic_off_in_data + 12 * sizeof(Elf64_Dyn), 1);

    size_t dynstr_sz = 1;
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        dynstr_sz += strlen(g_symbols[i].name) + 1;
    }
    dynstr_sz += origin_so.size() + 1;
    // Reserve space for soname (max filename)
    dynstr_sz += 256;

    uint64_t note_off_in_data = AlignUp(dynstr_off_in_data + dynstr_sz, 8);
    constexpr size_t kNoteAlign = 4;
    constexpr size_t kBuildIdSize = 20;
    constexpr char kNoteNameGNU[] = "GNU";
    size_t note_desc_off_gnu = AlignUp(sizeof(Elf64_Nhdr) + strlen(kNoteNameGNU) + 1, kNoteAlign);
    size_t gnu_padding_after_desc = (4 - (kBuildIdSize % 4)) % 4;
    size_t note_total_sz_gnu = note_desc_off_gnu + kBuildIdSize + gnu_padding_after_desc;

    // .metadata as regular PROGBITS section (not NOTE)
    uint64_t metadata_off_in_data = note_off_in_data + note_total_sz_gnu;
    uint64_t metadata_aligned_sz = AlignUp(session_size, 8);
    uint64_t data_sz = metadata_off_in_data + metadata_aligned_sz;

    // Section headers: will be written at the end of the file
    // Sections: .text, .data, .dynsym, .dynstr, .hash, .gnu.hash,
    //           .dynamic, .rela.dyn, .note.gnu.build-id, .metadata, .shstrtab
    // Section indices (starting from 1, since 0 is SHN_UNDEF):
    constexpr size_t kSectionText = 1;
    constexpr size_t kSectionData = 2;
    constexpr size_t kSectionDynsym = 3;
    constexpr size_t kSectionDynstr = 4;
    constexpr size_t kSectionHash = 5;
    constexpr size_t kSectionGnuHash = 6;
    constexpr size_t kSectionDynamic = 7;
    constexpr size_t kSectionRelaDyn = 8;
    constexpr size_t kSectionNoteBuildId = 9;
    constexpr size_t kSectionMetadata = 10;
    constexpr size_t kSectionShstrtab = 11;
    constexpr size_t kNumSections = 11;
    uint64_t shoff = AlignUp(data_off + data_sz, kPageSize);
    uint64_t shstrtab_off = shoff + (kNumSections + 1) * sizeof(Elf64_Shdr);  // +1 for SHN_UNDEF

    // Calculate .shstrtab size
    // Section names: .text, .data, .dynsym, .dynstr, .hash, .gnu.hash,
    //                .dynamic, .rela.dyn, .note.gnu.build-id, .metadata, .shstrtab
    size_t shstrtab_sz = 1;  // Start with null byte
    const char* section_names[] = {".text", ".data", ".dynsym", ".dynstr",
                                   ".hash", ".gnu.hash",
                                   ".dynamic", ".rela.dyn", ".note.gnu.build-id",
                                   ".metadata", ".shstrtab"};
    for (size_t i = 0;  i < kNumSections;  i++) {
        shstrtab_sz += strlen(section_names[i]) + 1;
    }

    const uint64_t text_va = text_off;
    const uint64_t data_va = data_off;

    // ------------------ ELF Header ------------------
    Elf64_Ehdr eh = {};
    memcpy(eh.e_ident, ELFMAG, SELFMAG);
    eh.e_ident[EI_CLASS] = ELFCLASS64;
    eh.e_ident[EI_DATA] = ELFDATA2LSB;
    eh.e_ident[EI_VERSION] = EV_CURRENT;
    eh.e_type = ET_DYN;
    if (g_target_arch == TargetArch::kX86_64) {
        eh.e_machine = EM_X86_64;
    }
    else if (g_target_arch == TargetArch::kAARCH64) {
        eh.e_machine = EM_AARCH64;
    }
    else {
        eh.e_machine = EM_RISCV;
    }
    eh.e_version = EV_CURRENT;
    eh.e_phoff = sizeof(Elf64_Ehdr);
    eh.e_shoff = shoff;
    eh.e_ehsize = sizeof(Elf64_Ehdr);
    eh.e_phentsize = sizeof(Elf64_Phdr);
    eh.e_phnum = 2;
    eh.e_shentsize = sizeof(Elf64_Shdr);
    eh.e_shnum = kNumSections + 1;  // Include SHN_UNDEF
    eh.e_shstrndx = kSectionShstrtab;  // .shstrtab index

    if (write(fd, &eh, sizeof(eh)) != sizeof(eh)) {
        close(fd);
        cleanup();
        return false;
    }

    // ------------------ Program Headers ------------------
    Elf64_Phdr ph_text = {};
    ph_text.p_type   = PT_LOAD;
    ph_text.p_flags  = PF_R | PF_X;
    ph_text.p_offset = text_off;
    ph_text.p_vaddr  = text_va;
    ph_text.p_paddr  = text_va;
    ph_text.p_filesz = text_sz;
    ph_text.p_memsz  = text_sz;
    ph_text.p_align  = kPageSize;

    Elf64_Phdr ph_data = {};
    ph_data.p_type   = PT_LOAD;
    ph_data.p_flags  = PF_R | PF_W;
    ph_data.p_offset = data_off;
    ph_data.p_vaddr  = data_va;
    ph_data.p_paddr  = data_va;
    ph_data.p_filesz = data_sz;
    ph_data.p_memsz  = data_sz;
    ph_data.p_align  = kPageSize;

    if (write(fd, &ph_text, sizeof(ph_text)) != sizeof(ph_text) ||
        write(fd, &ph_data, sizeof(ph_data)) != sizeof(ph_data)) {
        close(fd);
        cleanup();
        return false;
    }

    // ------------------ Stubs ------------------
    lseek(fd, static_cast<off_t>(text_off), SEEK_SET);
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        uint64_t stub_va = text_va + i * stub_size;
        uint64_t got_va  = data_va + got_off_in_data + i * 8;

        if (g_target_arch == TargetArch::kX86_64) {
            int64_t disp64 = static_cast<int64_t>(got_va) - static_cast<int64_t>(stub_va + 6);

            // Check RIP-relative displacement range: 32-bit signed (±2GB)
            if (disp64 < INT32_MIN || disp64 > INT32_MAX) {
                Logger::E(TAG, "GOT entry too far from stub: 0x%lx - 0x%lx = %ld "
                               "(exceeds ±2GB RIP-relative range)", got_va, stub_va + 6, disp64);
                close(fd);
                cleanup();
                return false;
            }

            int32_t disp = static_cast<int32_t>(disp64);
            uint8_t code[6] = {0xff, 0x25};
            memcpy(code + 2, &disp, 4);
            write(fd, code, 6);
        }
        else if (g_target_arch == TargetArch::kAARCH64) {
            uint64_t page_base = stub_va & ~0xFFFULL;
            uint64_t got_page = got_va & ~0xFFFULL;
            int64_t immhi = static_cast<int64_t>((got_page - page_base) >> 12);

            // Check ADRP immediate range: 21-bit signed (±1MB pages = ±4GB)
            if ((immhi < -(1 << 20)) || (immhi >= (1 << 20))) {
                close(fd);
                cleanup();
                return false;
            }

            // ADRP: adrp x16, #immhi (page-aligned address)
            // immlo: bits 30:29, immhi: bits 23:22 + 20:5
            uint32_t adrp = 0x90000010U | ((immhi & 0x3) << 29) |
                                                (((immhi >> 2) & 0x7FFFFU) << 5);

            // LDR: ldr x16, [x16, #offset] (64-bit unsigned immediate)
            // offset = got_va & 0xFFF (12-bit page offset)
            uint32_t immlo = (got_va & 0xFFF) << 10;  // imm12 at bits 21:10
            uint32_t ldr  = 0xF9400110U | immlo;    // Rn=x16, Rt=x16, opc=01, size=11

            uint32_t br   = 0xD61F0200U;             // br x16
            write(fd, &adrp, 4);
            write(fd, &ldr,  4);
            write(fd, &br,   4);
        }
        else if (g_target_arch == TargetArch::kRISCV64) {
            int64_t delta = static_cast<int64_t>(got_va) - static_cast<int64_t>(stub_va);

            // Check AUIPC effective address range first
            // AUIPC imm[31:12] is 20-bit signed: [-524288, 524287]
            // Effective range: imm_hi * 4096 = [-2GB+4KB, 2GB-4KB]
            // Check delta before computing imm_hi to avoid overflow issues
            constexpr int64_t kAUIPCMin = -static_cast<int64_t>(1LL << 31);  // -2GB
            constexpr int64_t kAUIPCMax = (static_cast<int64_t>(1LL) << 31) - 4096;  // 2GB-4KB
            if ((delta < kAUIPCMin) || (delta > kAUIPCMax)) {
                Logger::E(TAG, "GOT entry too far from stub: delta=%ld "
                               "(exceeds ±2GB AUIPC effective range)", delta);
                close(fd);
                cleanup();
                return false;
            }

            // RISC-V AUIPC + LD immediate decomposition
            // AUIPC uses imm[31:12] (20-bit signed), LD uses imm[11:0] (12-bit signed)
            // Round to nearest page: (delta + 0x800) >> 12
            int32_t imm_hi = static_cast<int32_t>((delta + 0x800) >> 12);
            int32_t imm_lo = static_cast<int32_t>(delta & 0xFFF);
            // Sign extend imm_lo from 12 bits: [0x000-0x7FF] -> [0-2047], [0x800-0xFFF] -> [-2048-(-1)]
            if (imm_lo & 0x800) imm_lo |= 0xFFFFF000;

            // Verify imm_hi is within 20-bit signed range (redundant but safe)
            if ((imm_hi < -(1 << 19)) || (imm_hi >= (1 << 19))) {
                Logger::E(TAG, "GOT entry too far from stub: delta=%ld imm_hi=%d "
                               "(exceeds AUIPC 20-bit signed range)", delta, imm_hi);
                close(fd);
                cleanup();
                return false;
            }

            // Check LD immediate range: 12-bit signed (-2048 to 2047)
            // This should always pass due to & 0xFFF and sign extension, but verify for safety
            if ((imm_lo < -2048) || (imm_lo > 2047)) {
                Logger::E(TAG, "GOT page offset out of range: imm_lo=%d "
                               "(exceeds 12-bit signed range [-2048, 2047])", imm_lo);
                close(fd);
                cleanup();
                return false;
            }

            // AUIPC: add upper immediate to pc
            // opcode=0x17, rd=x5, imm[31:12]=imm_hi
            uint32_t auipc = 0x17U | (5U << 7) | ((imm_hi & 0xFFFFFU) << 12);

            // LD: load double word
            // opcode=0x03, funct3=0x3, rd=x5, rs1=x5, imm[11:0]=imm_lo
            uint32_t ld = 0x03U | (0x3U << 12) | (5U << 15) | (5U << 7) | ((imm_lo & 0xFFFU) << 20);

            // JALR: jump and link register
            // opcode=0x67, funct3=0, rd=x0 (discard link), rs1=x5, imm=0
            uint32_t jalr = 0x67U | (0U << 7) | (0x0U << 12) | (5U << 15);
            write(fd, &auipc, 4);
            write(fd, &ld,    4);
            write(fd, &jalr,  4);
        }
    }

    // ------------------ GOT ------------------
    lseek(fd, static_cast<off_t>(data_off + got_off_in_data), SEEK_SET);
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        uint64_t zero = 0;
        write(fd, &zero, 8);
    }

    // ------------------ RELA (using GLOB_DAT-style for eager binding) ------------------
    lseek(fd, static_cast<off_t>(data_off + rela_off_in_data), SEEK_SET);
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        uint64_t got_entry_va = data_va + got_off_in_data + i * 8;
        Elf64_Rela rela = {};
        rela.r_offset = got_entry_va;
        if (g_target_arch == TargetArch::kX86_64) {
            rela.r_info = ELF64_R_INFO(i + 1, R_X86_64_GLOB_DAT);
        }
        else if (g_target_arch == TargetArch::kAARCH64) {
            rela.r_info = ELF64_R_INFO(i + 1, R_AARCH64_GLOB_DAT);
        }
        else { // RISCV: use R_RISCV_64 for GOT entry initialization (eager binding)
            rela.r_info = ELF64_R_INFO(i + 1, R_RISCV_64);
        }
        rela.r_addend = 0;
        write(fd, &rela, sizeof(rela));
    }

    // ------------------ HASH (SysV) ------------------
    WriteSysvHashTable(fd, data_off, sysv_hash_off_in_data, data_va, dynsym_off_in_data);

    // ------------------ GNU_HASH ------------------
    WriteGnuHashTable(fd, data_off, gnu_hash_off_in_data, data_va, dynsym_off_in_data);

    // ------------------ DYNSYM ------------------
    lseek(fd, static_cast<off_t>(data_off + dynsym_off_in_data), SEEK_SET);
    Elf64_Sym null_sym = {};
    write(fd, &null_sym, sizeof(null_sym));
    size_t str_off = 1;
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        const char* name = g_symbols[i].name;
        size_t len = strlen(name) + 1;

        Elf64_Sym sym = {};
        sym.st_name  = static_cast<uint32_t>(str_off);
        sym.st_info  = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
        // Design choice: Mark symbols as SHN_UNDEF to enable eager binding via DT_RELA.
        // This makes the proxy a pure forwarding stub - symbols are resolved by loader
        // from the original library (via DT_NEEDED), with GOT entries filled at load time.
        // Alternative: Could point st_shndx to .text and set st_value to stub address,
        // which would help debug tools but bypass loader's symbol resolution.
        sym.st_shndx = SHN_UNDEF;
        sym.st_value = 0;  // SHN_UNDEF symbols must have st_value = 0
        write(fd, &sym, sizeof(sym));
        str_off += len;
    }

    // ------------------ DYNSTR ------------------
    lseek(fd, static_cast<off_t>(data_off + dynstr_off_in_data), SEEK_SET);
    write(fd, "", 1);
    str_off = 1;
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        const char* name = g_symbols[i].name;
        size_t len = strlen(name) + 1;
        write(fd, name, len);
        str_off += len;
    }
    size_t origin_name_off = str_off;
    write(fd, origin_so.c_str(), origin_so.size() + 1);
    str_off += origin_so.size() + 1;

    // Extract basename for SONAME
    size_t soname_off = str_off;
    size_t last_slash = origin_so.find_last_of("/\\");
    std::string soname = (last_slash != std::string::npos) ? origin_so.substr(last_slash + 1) : origin_so;
    write(fd, soname.c_str(), soname.size() + 1);

    // ------------------ DYNAMIC (eager binding via GLOB_DAT, no PLT) ------------------
    lseek(fd, static_cast<off_t>(data_off + dynamic_off_in_data), SEEK_SET);
    Elf64_Dyn dyn[] = {
        { DT_SONAME,     static_cast<Elf64_Xword>(data_va + dynstr_off_in_data + soname_off) },
        { DT_NEEDED,     static_cast<Elf64_Xword>(data_va + dynstr_off_in_data + origin_name_off) },
        { DT_SYMTAB,     data_va + dynsym_off_in_data },
        { DT_SYMENT,     static_cast<Elf64_Xword>(sizeof(Elf64_Sym)) },
        { DT_STRTAB,     data_va + dynstr_off_in_data },
        { DT_STRSZ,      static_cast<Elf64_Xword>(dynstr_sz) },
        { DT_HASH,       data_va + sysv_hash_off_in_data },
        { DT_GNU_HASH,   data_va + gnu_hash_off_in_data },
        { DT_RELA,       data_va + rela_off_in_data },
        { DT_RELASZ,     g_symbol_count * sizeof(Elf64_Rela) },
        { DT_RELAENT,    static_cast<Elf64_Xword>(sizeof(Elf64_Rela)) },
        { DT_NULL,       0 }
    };
    write(fd, dyn, sizeof(dyn));

    // ------------------ .note.gnu.build-id ------------------
    lseek(fd, static_cast<off_t>(data_off + note_off_in_data), SEEK_SET);
    Elf64_Nhdr nh = {};
    nh.n_namesz = static_cast<uint32_t>(strlen(kNoteNameGNU) + 1);
    nh.n_descsz = kBuildIdSize;
    nh.n_type   = NT_GNU_BUILD_ID;
    write(fd, &nh, sizeof(nh));
    write(fd, kNoteNameGNU, nh.n_namesz);
    char pad[4] = {0};
    size_t padding = (4 - (nh.n_namesz % 4)) % 4;
    if (0 != padding) {
        write(fd, pad, padding);
    }

    unsigned char build_id[20];
    SHA1_CTX ctx;
    SHA1_Init(&ctx);

    // Hash the original .so path (deterministic)
    SHA1_Update(&ctx, origin_so.data(), origin_so.size());

    // Hash symbol names in the order they appear in the original .dynsym section.
    // This order is deterministic because ParseOriginDynSym reads symbols sequentially
    // from the ELF file without any sorting or reordering.
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        SHA1_Update(&ctx, g_symbols[i].name, strlen(g_symbols[i].name));
    }

    // Hash session data to distinguish different builds with same symbols
    // This ensures uniqueness for different use-cases
    if (session_data && session_size > 0) {
        SHA1_Update(&ctx, session_data, session_size);
    }

    SHA1_Final(build_id, &ctx);
    write(fd, build_id, kBuildIdSize);
    padding = gnu_padding_after_desc;
    if (0 != padding) {
        write(fd, pad, padding);
    }

    // ------------------ .metadata ------------------
    lseek(fd, static_cast<off_t>(data_off + metadata_off_in_data), SEEK_SET);
    write(fd, session_data, session_size);
    padding = metadata_aligned_sz - session_size;
    if (0 != padding) {
        write(fd, pad, padding);
    }

    // ------------------ Section Headers ------------------
    // First write .shstrtab
    lseek(fd, static_cast<off_t>(shstrtab_off), SEEK_SET);
    size_t name_off = 0;
    write(fd, "", 1);  // First entry is empty (SHN_UNDEF)
    name_off++;

    // Build section name offset table
    // Sections: .text(1), .data(2), .dynsym(3), .dynstr(4), .hash(5),
    // .gnu.hash(6), .gnu.version(7), .dynamic(8), .rela.dyn(9),
    // .note.gnu.build-id(10), .note.metadata(11), .shstrtab(12)
    size_t name_offsets[kNumSections + 1];
    for (size_t i = 0;  i < kNumSections;  i++) {
        name_offsets[i + 1] = name_off;  // +1 because index 0 is for SHN_UNDEF (no name)
        write(fd, section_names[i], strlen(section_names[i]) + 1);
        name_off += strlen(section_names[i]) + 1;
    }

    // Write section headers
    lseek(fd, static_cast<off_t>(shoff), SEEK_SET);

    // SHN_UNDEF (always present)
    Elf64_Shdr null_sh = {};
    write(fd, &null_sh, sizeof(null_sh));

    // .text (index 1)
    Elf64_Shdr sh_text = {};
    sh_text.sh_name = name_offsets[1];
    sh_text.sh_type = SHT_PROGBITS;
    sh_text.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
    sh_text.sh_offset = text_off;
    sh_text.sh_addr = text_va;
    sh_text.sh_size = text_sz;
    sh_text.sh_entsize = 0;
    write(fd, &sh_text, sizeof(sh_text));

    // .data (index 2)
    Elf64_Shdr sh_data = {};
    sh_data.sh_name = name_offsets[2];
    sh_data.sh_type = SHT_PROGBITS;
    sh_data.sh_flags = SHF_ALLOC | SHF_WRITE;
    sh_data.sh_offset = data_off;
    sh_data.sh_addr = data_va;
    sh_data.sh_size = data_sz;
    sh_data.sh_entsize = 0;
    write(fd, &sh_data, sizeof(sh_data));

    // .dynsym (index kSectionDynsym) - links to .dynstr (kSectionDynstr)
    Elf64_Shdr sh_dynsym = {};
    sh_dynsym.sh_name = name_offsets[kSectionDynsym];
    sh_dynsym.sh_type = SHT_DYNSYM;
    sh_dynsym.sh_flags = SHF_ALLOC;
    sh_dynsym.sh_offset = data_off + dynsym_off_in_data;
    sh_dynsym.sh_addr = data_va + dynsym_off_in_data;
    sh_dynsym.sh_size = (g_symbol_count + 1) * sizeof(Elf64_Sym);
    sh_dynsym.sh_link = kSectionDynstr;  // .dynstr
    sh_dynsym.sh_info = 1;  // First global symbol index
    sh_dynsym.sh_entsize = sizeof(Elf64_Sym);
    write(fd, &sh_dynsym, sizeof(sh_dynsym));

    // .dynstr (index kSectionDynstr)
    Elf64_Shdr sh_dynstr = {};
    sh_dynstr.sh_name = name_offsets[kSectionDynstr];
    sh_dynstr.sh_type = SHT_STRTAB;
    sh_dynstr.sh_flags = SHF_ALLOC;
    sh_dynstr.sh_offset = data_off + dynstr_off_in_data;
    sh_dynstr.sh_addr = data_va + dynstr_off_in_data;
    sh_dynstr.sh_size = dynstr_sz;
    sh_dynstr.sh_entsize = 0;
    write(fd, &sh_dynstr, sizeof(sh_dynstr));

    // .hash (index kSectionHash) - links to .dynsym (kSectionDynsym)
    Elf64_Shdr sh_hash = {};
    sh_hash.sh_name = name_offsets[kSectionHash];
    sh_hash.sh_type = SHT_HASH;
    sh_hash.sh_flags = SHF_ALLOC;
    sh_hash.sh_offset = data_off + sysv_hash_off_in_data;
    sh_hash.sh_addr = data_va + sysv_hash_off_in_data;
    sh_hash.sh_size = sysv_hash_sz;
    sh_hash.sh_link = kSectionDynsym;  // .dynsym
    sh_hash.sh_entsize = 4;
    write(fd, &sh_hash, sizeof(sh_hash));

    // .gnu.hash (index kSectionGnuHash) - links to .dynsym (kSectionDynsym)
    Elf64_Shdr sh_gnu_hash = {};
    sh_gnu_hash.sh_name = name_offsets[kSectionGnuHash];
    sh_gnu_hash.sh_type = SHT_GNU_HASH;
    sh_gnu_hash.sh_flags = SHF_ALLOC;
    sh_gnu_hash.sh_offset = data_off + gnu_hash_off_in_data;
    sh_gnu_hash.sh_addr = data_va + gnu_hash_off_in_data;
    sh_gnu_hash.sh_size = gnu_hash_sz;
    sh_gnu_hash.sh_link = kSectionDynsym;  // .dynsym
    sh_gnu_hash.sh_entsize = 0;
    write(fd, &sh_gnu_hash, sizeof(sh_gnu_hash));

    // .dynamic (index kSectionDynamic) - links to .dynstr (kSectionDynstr)
    Elf64_Shdr sh_dynamic = {};
    sh_dynamic.sh_name = name_offsets[kSectionDynamic];
    sh_dynamic.sh_type = SHT_DYNAMIC;
    sh_dynamic.sh_flags = SHF_ALLOC | SHF_WRITE;
    sh_dynamic.sh_offset = data_off + dynamic_off_in_data;
    sh_dynamic.sh_addr = data_va + dynamic_off_in_data;
    sh_dynamic.sh_size = 12 * sizeof(Elf64_Dyn);
    sh_dynamic.sh_link = kSectionDynstr;  // .dynstr
    sh_dynamic.sh_info = 0;
    sh_dynamic.sh_entsize = sizeof(Elf64_Dyn);
    write(fd, &sh_dynamic, sizeof(sh_dynamic));

    // .rela.dyn (index kSectionRelaDyn) - links to .dynsym (kSectionDynsym)
    Elf64_Shdr sh_rela = {};
    sh_rela.sh_name = name_offsets[kSectionRelaDyn];
    sh_rela.sh_type = SHT_RELA;
    sh_rela.sh_flags = SHF_ALLOC;
    sh_rela.sh_offset = data_off + rela_off_in_data;
    sh_rela.sh_addr = data_va + rela_off_in_data;
    sh_rela.sh_size = g_symbol_count * sizeof(Elf64_Rela);
    sh_rela.sh_link = kSectionDynsym;  // .dynsym
    sh_rela.sh_info = kSectionData;  // Section to which relocs apply (.data)
    sh_rela.sh_entsize = sizeof(Elf64_Rela);
    write(fd, &sh_rela, sizeof(sh_rela));

    // .note.gnu.build-id (index kSectionNoteBuildId)
    Elf64_Shdr sh_note_buildid = {};
    sh_note_buildid.sh_name = name_offsets[kSectionNoteBuildId];
    sh_note_buildid.sh_type = SHT_NOTE;
    sh_note_buildid.sh_flags = SHF_ALLOC;
    sh_note_buildid.sh_offset = data_off + note_off_in_data;
    sh_note_buildid.sh_addr = data_va + note_off_in_data;
    sh_note_buildid.sh_size = note_total_sz_gnu;
    sh_note_buildid.sh_entsize = 0;
    write(fd, &sh_note_buildid, sizeof(sh_note_buildid));

    // .metadata (index kSectionMetadata) - regular PROGBITS section
    Elf64_Shdr sh_metadata = {};
    sh_metadata.sh_name = name_offsets[kSectionMetadata];
    sh_metadata.sh_type = SHT_PROGBITS;
    sh_metadata.sh_flags = SHF_ALLOC;
    sh_metadata.sh_offset = data_off + metadata_off_in_data;
    sh_metadata.sh_addr = data_va + metadata_off_in_data;
    sh_metadata.sh_size = metadata_aligned_sz;
    sh_metadata.sh_entsize = 0;
    write(fd, &sh_metadata, sizeof(sh_metadata));

    // .shstrtab (index kSectionShstrtab)
    Elf64_Shdr sh_shstrtab = {};
    sh_shstrtab.sh_name = name_offsets[kSectionShstrtab];
    sh_shstrtab.sh_type = SHT_STRTAB;
    sh_shstrtab.sh_flags = 0;
    sh_shstrtab.sh_offset = shstrtab_off;
    sh_shstrtab.sh_addr = 0;
    sh_shstrtab.sh_size = shstrtab_sz;
    sh_shstrtab.sh_entsize = 0;
    write(fd, &sh_shstrtab, sizeof(sh_shstrtab));

    close(fd);

    cleanup();
    return true;
}

} // namespace cdlc

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
