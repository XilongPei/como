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
#include "ElfProxyBuilder.h"

namespace cdlc {

// ----------------------------------------------------------------------
// Missing ELF constants (for portability across toolchains)
// ----------------------------------------------------------------------
#ifndef EM_RISCV
#define EM_RISCV 243
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
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
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
    uint8_t pad = 0x80;
    SHA1_Update(ctx, &pad, 1);

    while ((ctx->count % SHA1_BLOCK_SIZE) != 56) {
        uint8_t zero = 0;
        SHA1_Update(ctx, &zero, 1);
    }

    uint8_t bits[8];
    for (int i = 0;  i < 8;  i++) {
        bits[i] = static_cast<uint8_t>((bit_count >> ((7 - i) * 8)) & 0xFF);
    }
    SHA1_Update(ctx, bits, 8);

    for (int i = 0;  i < 5;  i++) {
        md[i * 4 + 0] = static_cast<uint8_t>((ctx->state[i] >> 24) & 0xFF);
        md[i * 4 + 1] = static_cast<uint8_t>((ctx->state[i] >> 16) & 0xFF);
        md[i * 4 + 2] = static_cast<uint8_t>((ctx->state[i] >> 8) & 0xFF);
        md[i * 4 + 3] = static_cast<uint8_t>(ctx->state[i] & 0xFF);
    }
}

// ----------------------------------------------------------------------
// Constants and types
// ----------------------------------------------------------------------
constexpr size_t kMaxSyms = 1024;
constexpr uint64_t kPageSize = 0x1000;

struct Symbol {
    char name[256];
};

Symbol g_symbols[kMaxSyms];
size_t g_symbol_count = 0;
TargetArch g_target_arch = TargetArch::kX86_64;

void Die(
    /* [in] */ const char* msg)
{
    perror(msg);
    _exit(1);
}

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

void ParseOriginDynSym(
    /* [in] */ const std::string& path)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        Die("open origin");
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        Die("fstat origin");
    }

    Elf64_Ehdr eh;
    if (read(fd, &eh, sizeof(eh)) != static_cast<ssize_t>(sizeof(eh))) {
        Die("read ehdr");
    }

    if (memcmp(eh.e_ident, ELFMAG, SELFMAG) != 0) {
        Die("not elf");
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
        fprintf(stderr, "Unsupported arch: 0x%04x (need x86-64=62, aarch64=183, riscv=243)\n", eh.e_machine);
        Die("unsupported arch");
    }

    Elf64_Shdr* shdrs = static_cast<Elf64_Shdr*>(
        malloc(eh.e_shnum * sizeof(Elf64_Shdr)));
    lseek(fd, static_cast<off_t>(eh.e_shoff), SEEK_SET);
    if (read(fd, shdrs, eh.e_shnum * sizeof(Elf64_Shdr)) !=
        static_cast<ssize_t>(eh.e_shnum * sizeof(Elf64_Shdr))) {
        Die("read shdrs");
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
        Die("no dynsym/dynstr");
    }

    if (dynstr_sh->sh_offset + dynstr_sh->sh_size > static_cast<uint64_t>(st.st_size)) {
        Die("dynstr out of bounds");
    }

    char* strtab = static_cast<char*>(malloc(dynstr_sh->sh_size));
    lseek(fd, static_cast<off_t>(dynstr_sh->sh_offset), SEEK_SET);
    if (read(fd, strtab, dynstr_sh->sh_size) !=
        static_cast<ssize_t>(dynstr_sh->sh_size)) {
        Die("read dynstr");
    }

    size_t nsyms = dynsym_sh->sh_size / sizeof(Elf64_Sym);
    lseek(fd, static_cast<off_t>(dynsym_sh->sh_offset), SEEK_SET);

    for (size_t i = 0;  i < nsyms && g_symbol_count < kMaxSyms;  i++) {
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
        if (len >= sizeof(g_symbols[0].name)) {
            continue;
        }

        strncpy(g_symbols[g_symbol_count].name, name, len);
        g_symbols[g_symbol_count].name[len] = '\0';
        ++g_symbol_count;
    }

    free(strtab);
    free(shdrs);
    close(fd);
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

    // Use 1 bucket for simplicity
    constexpr uint32_t kNBuckets = 1;
    constexpr uint32_t kMaskwords = 1;

    GnuHashHeader header = {};
    header.nbuckets = kNBuckets;
    header.symndx = 0;  // Index of first symbol in chain
    header.maskwords = kMaskwords;
    header.shift2 = 0;  // Will be calculated

    // Calculate shift2 based on symbol count
    // bloom filter shift count
    uint32_t shift2 = 0;
    if (g_symbol_count > 0) {
        // Count trailing zeros of symbol count
        shift2 = 0;
        uint32_t n = static_cast<uint32_t>(g_symbol_count);
        while ((n & 1) == 0 && shift2 < 31) {
            n >>= 1;
            shift2++;
        }
    }
    header.shift2 = shift2;

    // Bloom filter
    uint64_t bloom[kMaskwords] = {0};

    // Buckets (all point to 0 initially)
    uint32_t buckets[kNBuckets] = {0};

    // Chain entries: hash with bit 31 set for last symbol
    uint32_t* chains = static_cast<uint32_t*>(malloc((g_symbol_count + 1) * sizeof(uint32_t)));
    if (!chains) {
        return;
    }

    // Compute GNU hash for each symbol
    // GNU hash: h = 5 * h + c * 2^{c * 5 % 32}
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        const char* name = g_symbols[i].name;
        uint32_t h = 5381;
        while (*name) {
            h += (h << 5) + *name++;
        }
        uint32_t hash = h;

        // Update bloom filter
        uint32_t h1 = hash;
        uint32_t h2 = hash >> header.shift2;
        bloom[0] |= (1ULL << (h1 % 64));
        bloom[0] |= (1ULL << (h2 % 64));

        // Store hash with last bit marker
        chains[i] = hash | 0x80000000;
    }

    // Clear last bit for first symbol
    if (g_symbol_count > 0) {
        chains[0] &= ~0x80000000;
    }

    // Write hash table
    lseek(fd, static_cast<off_t>(data_off + hash_off_in_data), SEEK_SET);
    write(fd, &header, sizeof(header));
    write(fd, bloom, sizeof(bloom));
    write(fd, buckets, sizeof(buckets));
    write(fd, chains, g_symbol_count * sizeof(uint32_t));

    free(chains);
}

bool BuildElfProxy(
    /* [in] */ const std::string& origin_so,
    /* [in] */ const std::string& output_so,
    /* [in] */ const void* session_data,
    /* [in] */ size_t session_size)
{
    g_symbol_count = 0;
    g_target_arch = TargetArch::kX86_64;
    ParseOriginDynSym(origin_so);

    int fd = open(output_so.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0755);
    if (fd < 0) {
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
    uint64_t hash_off_in_data = AlignUp(rela_off_in_data + g_symbol_count * sizeof(Elf64_Rela), 8);
    uint64_t dynsym_off_in_data = AlignUp(hash_off_in_data + (4 + 4 + 3*4 + (g_symbol_count+1)*4), 8);
    uint64_t dynstr_off_in_data = AlignUp(dynsym_off_in_data + (g_symbol_count + 1) * sizeof(Elf64_Sym), 1);

    size_t dynstr_sz = 1;
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        dynstr_sz += strlen(g_symbols[i].name) + 1;
    }
    dynstr_sz += origin_so.size() + 1;

    uint64_t note_off_in_data = AlignUp(dynstr_off_in_data + dynstr_sz, 8);
    constexpr size_t kNoteAlign = 4;
    constexpr size_t kBuildIdSize = 20;
    constexpr char kNoteNameGNU[] = "GNU";
    constexpr char kNoteNameComo[] = ".metadata";
    size_t note_desc_off_gnu = AlignUp(sizeof(Elf64_Nhdr) + strlen(kNoteNameGNU) + 1, kNoteAlign);
    size_t note_total_sz_gnu = note_desc_off_gnu + kBuildIdSize;
    uint64_t dynamic_off_in_data = AlignUp(note_off_in_data + note_total_sz_gnu, 8);

    // .note.metadata section
    constexpr size_t kSessionNoteType = 1;
    size_t note_desc_off_session = AlignUp(sizeof(Elf64_Nhdr) + strlen(kNoteNameComo) + 1, kNoteAlign);
    size_t note_total_sz_session = note_desc_off_session + session_size;
    uint64_t session_off_in_data = AlignUp(dynamic_off_in_data + 8 * sizeof(Elf64_Dyn), 8);
    uint64_t data_sz = session_off_in_data + note_total_sz_session;

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
    eh.e_phentsize = sizeof(Elf64_Phdr);
    eh.e_phnum = 2;

    if (write(fd, &eh, sizeof(eh)) != sizeof(eh)) {
        close(fd);
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
        return false;
    }

    // ------------------ Stubs ------------------
    lseek(fd, static_cast<off_t>(text_off), SEEK_SET);
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        uint64_t stub_va = text_va + i * stub_size;
        uint64_t got_va  = data_va + got_off_in_data + i * 8;

        if (g_target_arch == TargetArch::kX86_64) {
            int32_t disp = static_cast<int32_t>(got_va - (stub_va + 6));
            uint8_t code[6] = {0xff, 0x25};
            memcpy(code + 2, &disp, 4);
            write(fd, code, 6);
        }
        else if (g_target_arch == TargetArch::kAARCH64) {
            uint64_t page_base = stub_va & ~0xFFFULL;
            uint64_t got_page = got_va & ~0xFFFULL;
            int64_t immhi = static_cast<int64_t>((got_page - page_base) >> 12);
            uint32_t immlo = static_cast<uint32_t>((got_va & 0xFFF) << 10);

            uint32_t adrp = 0x90000010U | ((immhi & 0x3) << 29) |
                            (((immhi >> 2) & 0x7FFFF) << 5);
            uint32_t ldr  = 0xF9400210U | immlo;
            uint32_t br   = 0xD61F0200U;
            write(fd, &adrp, 4);
            write(fd, &ldr,  4);
            write(fd, &br,   4);
        }
        else if (g_target_arch == TargetArch::kRISCV64) {
            int64_t delta = static_cast<int64_t>(got_va) - static_cast<int64_t>(stub_va);
            int32_t imm_hi = static_cast<int32_t>((delta + 0x800) >> 12);
            int32_t imm_lo = static_cast<int32_t>(delta - (imm_hi << 12));
            if (imm_lo & 0x800) {
                imm_hi++;
                imm_lo -= 0x1000;
            }

            // AUIPC: add upper immediate to pc
            // opcode=0x17, rd=x5, imm[31:12]=imm_hi
            uint32_t auipc = 0x17U | (5U << 7) | ((imm_hi & 0xFFFFFU) << 12);

            // LD: load double word
            // opcode=0x03, funct3=0x3, rd=x5, rs1=x5, imm[11:0]=imm_lo
            uint32_t ld    = 0x03U | (0x3U << 12) | (5U << 15) | (5U << 7) | ((imm_lo & 0xFFFU) << 20);

            // JALR: jump and link register
            // opcode=0x67, funct3=0, rd=x0 (discard link), rs1=x5, imm=0
            uint32_t jalr  = 0x67U | (0U << 7) | (0x0U << 12) | (5U << 15);
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

    // ------------------ RELA ------------------
    lseek(fd, static_cast<off_t>(data_off + rela_off_in_data), SEEK_SET);
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        uint64_t got_entry_va = data_va + got_off_in_data + i * 8;
        Elf64_Rela rela = {};
        rela.r_offset = got_entry_va;
        if (g_target_arch == TargetArch::kX86_64) {
            rela.r_info = ELF64_R_INFO(i + 1, R_X86_64_JUMP_SLOT);
        }
        else if (g_target_arch == TargetArch::kAARCH64) {
            rela.r_info = ELF64_R_INFO(i + 1, R_AARCH64_JUMP_SLOT);
        }
        else { // RISCV
            rela.r_info = ELF64_R_INFO(i + 1, R_RISCV_JUMP_SLOT);
        }
        rela.r_addend = 0;
        write(fd, &rela, sizeof(rela));
    }

    // ------------------ GNU_HASH ------------------
    WriteGnuHashTable(fd, data_off, hash_off_in_data, data_va, dynsym_off_in_data);

    // ------------------ DYNSYM/DYNSTR ------------------
    lseek(fd, static_cast<off_t>(data_off + dynsym_off_in_data), SEEK_SET);
    Elf64_Sym null_sym = {};
    write(fd, &null_sym, sizeof(null_sym));

    lseek(fd, static_cast<off_t>(data_off + dynstr_off_in_data), SEEK_SET);
    write(fd, "", 1);
    size_t str_off = 1;
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        const char* name = g_symbols[i].name;
        size_t len = strlen(name) + 1;

        Elf64_Sym sym = {};
        sym.st_name  = static_cast<uint32_t>(str_off);
        sym.st_info  = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
        sym.st_shndx = SHN_UNDEF;
        sym.st_value = text_va + i * stub_size;
        lseek(fd, static_cast<off_t>(data_off + dynsym_off_in_data + (i + 1) * sizeof(Elf64_Sym)), SEEK_SET);
        write(fd, &sym, sizeof(sym));

        lseek(fd, static_cast<off_t>(data_off + dynstr_off_in_data + str_off), SEEK_SET);
        write(fd, name, len);
        str_off += len;
    }
    size_t origin_name_off = str_off;
    write(fd, origin_so.c_str(), origin_so.size() + 1);

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
    if (padding) {
        write(fd, pad, padding);
    }

    unsigned char build_id[20];
    SHA1_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, origin_so.data(), origin_so.size());
    for (size_t i = 0;  i < g_symbol_count;  i++) {
        SHA1_Update(&ctx, g_symbols[i].name, strlen(g_symbols[i].name));
    }
    SHA1_Final(build_id, &ctx);
    write(fd, build_id, kBuildIdSize);

    // ------------------ DYNAMIC ------------------
    lseek(fd, static_cast<off_t>(data_off + dynamic_off_in_data), SEEK_SET);
    Elf64_Dyn dyn[] = {
        { DT_NEEDED,     static_cast<Elf64_Xword>(origin_name_off) },
        { DT_SYMTAB,     data_va + dynsym_off_in_data },
        { DT_SYMENT,     static_cast<Elf64_Xword>(sizeof(Elf64_Sym)) },
        { DT_STRTAB,     data_va + dynstr_off_in_data },
        { DT_GNU_HASH,   data_va + hash_off_in_data },
        { DT_JMPREL,     data_va + rela_off_in_data },
        { DT_PLTRELSZ,   g_symbol_count * sizeof(Elf64_Rela) },
        { DT_PLTREL,     static_cast<Elf64_Xword>(DT_RELA) },
        { DT_RELAENT,    static_cast<Elf64_Xword>(sizeof(Elf64_Rela)) },
        { DT_NULL,       0 }
    };
    write(fd, dyn, sizeof(dyn));

    // ------------------ .note.metadata ------------------
    lseek(fd, static_cast<off_t>(data_off + session_off_in_data), SEEK_SET);
    Elf64_Nhdr session_nh = {};
    session_nh.n_namesz = static_cast<uint32_t>(strlen(kNoteNameComo) + 1);
    session_nh.n_descsz = session_size;
    session_nh.n_type   = kSessionNoteType;
    write(fd, &session_nh, sizeof(session_nh));
    write(fd, kNoteNameComo, session_nh.n_namesz);
    padding = (4 - (session_nh.n_namesz % 4)) % 4;
    if (padding) {
        write(fd, pad, padding);
    }
    write(fd, session_data, session_size);
    padding = (4 - (session_size % 4)) % 4;
    if (padding) {
        write(fd, pad, padding);
    }

    close(fd);
    return true;
}

} // namespace cdlc
