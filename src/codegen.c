#include <codegen.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <sys/stat.h>

#define ELF_MAGIC_NUMBER  0x7f454c46
#define ELF_32_BIT_MODE   0x01
#define ELF_LITTLE_ENDIAN 0x01
#define ELF_VERSION       0x01
#define ELF_OSABI_NONE    0x00
#define ELF_TYPE_DYN      0x0003
#define ELF_X86_64        0x0003
#define ELF_SEGMENT_START 0x400000
#define ELF_HEADER_SIZE   0x40
#define ELF_PHENTRY_SIZE  0x38
#define ELF_ALIGNMENT     0x1000

static void write8(Generator_T* g, uint8_t b) 
{
    fwrite(&b, sizeof(uint8_t), 1, g->out);
    g->address++;
}

static void write16(Generator_T* g, uint16_t s)
{
    uint8_t b[2];
    memcpy(&b, &s, sizeof(uint16_t));
    write8(g, b[0]);
    write8(g, b[1]);
}

static void write32(Generator_T* g, uint32_t i)
{
    uint8_t b[4];
    memcpy(&b, &i, sizeof(uint32_t));
    write8(g, b[3]);
    write8(g, b[2]);
    write8(g, b[1]);
    write8(g, b[0]);
}

static void write64(Generator_T* g, uint64_t l)
{
    uint8_t b[8];
    memcpy(&b, &l, sizeof(uint64_t));
    write8(g, b[7]);
    write8(g, b[6]);
    write8(g, b[5]);
    write8(g, b[4]);
    write8(g, b[3]);
    write8(g, b[2]);
    write8(g, b[1]);
    write8(g, b[0]);
}

static void elf_header(Generator_T* g)
{
    write32(g, ELF_MAGIC_NUMBER);
    write8(g, ELF_32_BIT_MODE);
    write8(g, ELF_LITTLE_ENDIAN);
    write8(g, ELF_VERSION);
    write8(g, ELF_OSABI_NONE);
    write64(g, 0);
    write16(g, ELF_TYPE_DYN);
    write16(g, ELF_X86_64);
    write32(g, ELF_VERSION);
    write64(g, ELF_SEGMENT_START + ELF_HEADER_SIZE + ELF_PHENTRY_SIZE);
    write64(g, ELF_HEADER_SIZE);
    write64(g, 0);
    write32(g, 0);
    write16(g, ELF_HEADER_SIZE);
    write16(g, ELF_PHENTRY_SIZE);
    write16(g, 1);
    write16(g, 0);
    write16(g, 0);
    write16(g, 0);
    write32(g, 1);
    write32(g, 5);
    write64(g, 0);
    write64(g, ELF_SEGMENT_START);
    write64(g, ELF_SEGMENT_START);
    g->file_size_addr = g->address;
    write64(g, 0); // placeholder
    write64(g, 0);
    write64(g, ELF_ALIGNMENT);
}

static void elf_footer(Generator_T* g)
{

}

void generate(Generator_T* g, Node_T* ast, const char* destination)
{
    g->out = fopen(destination, "wb");

    elf_header(g);
    g->code_start_pos = g->address;
    elf_footer(g);

    fflush(g->out);
    fclose(g->out);
    chmod(destination, 0775);
}