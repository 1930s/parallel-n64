/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memory.h                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define AI_STATUS_FIFO_FULL	0x80000000		/* Bit 31: full */
#define AI_STATUS_DMA_BUSY	   0x40000000		/* Bit 30: busy */
#define MI_INTR_AI			   0x04			   /* Bit 2: AI intr */

extern uint32_t VI_REFRESH;

#define read_word_in_memory() readmem[address>>16]()
#define read_byte_in_memory() readmemb[address>>16]()
#define read_hword_in_memory() readmemh[address>>16]()
#define read_dword_in_memory() readmemd[address>>16]()
#define write_word_in_memory() writemem[address>>16]()
#define write_byte_in_memory() writememb[address >>16]()
#define write_hword_in_memory() writememh[address >>16]()
#define write_dword_in_memory() writememd[address >>16]()

enum { PIF_RAM_SIZE = 0x40 };

extern uint8_t g_pif_ram[PIF_RAM_SIZE];

typedef struct _save_memory_data
{
    uint8_t eeprom[0x200];
    uint8_t mempack[4][0x8000];
    uint8_t sram[0x8000];
    uint8_t flashram[0x20000];

    /* Some games use 16Kbit (2048 bytes) eeprom saves, the initial
     * libretro-mupen64plus save file implementation failed to account
     * for these. The missing bytes are stored here to avoid breaking
     * saves of games unaffected by the issue. */
    uint8_t eeprom2[0x600];
} save_memory_data;
extern save_memory_data saved_memory;
void format_saved_memory(void);

extern uint32_t address, word;
extern uint8_t cpu_byte;
extern uint16_t hword;
extern uint64_t dword, *rdword;

extern void (*readmem[0x10000])(void);
extern void (*readmemb[0x10000])(void);
extern void (*readmemh[0x10000])(void);
extern void (*readmemd[0x10000])(void);
extern void (*writemem[0x10000])(void);
extern void (*writememb[0x10000])(void);
extern void (*writememh[0x10000])(void);
extern void (*writememd[0x10000])(void);

enum si_registers
{
   SI_DRAM_ADDR_REG,
   SI_PIF_ADDR_RD64B_REG,
   SI_R2_REG, /* reserved */
   SI_R3_REG, /* reserved */
   SI_PIF_ADDR_WR64B_REG,
   SI_R5_REG, /* reserved */
   SI_STATUS_REG,
   SI_REGS_COUNT
};

extern uint32_t g_si_regs[SI_REGS_COUNT];

enum cic_type
{
   CIC_X101,
   CIC_X102,
   CIC_X103,
   CIC_X105,
   CIC_X106
};

extern enum cic_type g_cic_type;


#ifndef M64P_BIG_ENDIAN
#if defined(__GNUC__) && (__GNUC__ > 4  || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#define sl(x) __builtin_bswap32(x)
#else
#define sl(mot) \
( \
((mot & 0x000000FF) << 24) | \
((mot & 0x0000FF00) <<  8) | \
((mot & 0x00FF0000) >>  8) | \
((mot & 0xFF000000) >> 24) \
)
#endif

#define S8 3
#define S16 2
#define Sh16 1

#else

#define sl(mot) mot
#define S8 0
#define S16 0
#define Sh16 0

#endif

static inline void masked_write(uint32_t* dst, uint32_t value, uint32_t mask)
{
 *dst = (*dst & ~mask) | (value & mask);
}

int init_memory(void);

void map_region(uint16_t region,
      int type,
 void (*read8)(void),
 void (*read16)(void),
 void (*read32)(void),
 void (*read64)(void),
 void (*write8)(void),
 void (*write16)(void),
 void (*write32)(void),
 void (*write64)(void));

/* XXX: cannot make them static because of dynarec + rdp fb */
void read_rdram(void);
void read_rdramb(void);
void read_rdramh(void);
void read_rdramd(void);
void write_rdram(void);
void write_rdramb(void);
void write_rdramh(void);
void write_rdramd(void);

void read_rdramFB(void);
void read_rdramFBb(void);
void read_rdramFBh(void);
void read_rdramFBd(void);
void write_rdramFB(void);
void write_rdramFBb(void);
void write_rdramFBh(void);
void write_rdramFBd(void);

/* Returns a pointer to a block of contiguous memory
 * Can access RDRAM, SP_DMEM, SP_IMEM and ROM, using TLB if necessary
 * Useful for getting fast access to a zone with executable code. */
uint32_t *fast_mem_access(uint32_t address);

#ifdef DBG
void activate_memory_break_read(uint32_t address);
void deactivate_memory_break_read(uint32_t address);
void activate_memory_break_write(uint32_t address);
void deactivate_memory_break_write(uint32_t address);
int get_memory_type(uint32_t address);
#endif

#endif

