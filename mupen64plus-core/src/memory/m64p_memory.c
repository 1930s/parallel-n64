/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memory.c                                                *
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "api/m64p_types.h"

#include "memory.h"
#include "dma.h"
#include "pif.h"
#include "flashram.h"

#include "r4300/r4300.h"
#include "r4300/r4300_core.h"
#include "r4300/cached_interp.h"
#include "r4300/cp0.h"
#include "r4300/interupt.h"
#include "r4300/recomph.h"
#include "r4300/ops.h"
#include "r4300/tlb.h"

#include "ai/ai_controller.h"
#include "api/callbacks.h"
#include "main/main.h"
#include "main/rom.h"
#include "plugin/plugin.h"
#include "r4300/new_dynarec/new_dynarec.h"
#include "rdp/rdp_core.h"
#include "ri/ri_controller.h"
#include "rsp/rsp_core.h"
#include "vi/vi_controller.h"

#ifdef DBG
#include "debugger/dbg_types.h"
#include "debugger/dbg_memory.h"
#include "debugger/dbg_breakpoints.h"
#endif


/* some functions prototypes */
static void read_nothing(void);
static void read_nothingb(void);
static void read_nothingh(void);
static void read_nothingd(void);
static void write_nothing(void);
static void write_nothingb(void);
static void write_nothingh(void);
static void write_nothingd(void);
static void read_nomem(void);
static void read_nomemb(void);
static void read_nomemh(void);
static void read_nomemd(void);
static void write_nomem(void);
static void write_nomemb(void);
static void write_nomemh(void);
static void write_nomemd(void);
static void read_rdramreg(void);
static void read_rdramregb(void);
static void read_rdramregh(void);
static void read_rdramregd(void);
static void write_rdramreg(void);
static void write_rdramregb(void);
static void write_rdramregh(void);
static void write_rdramregd(void);
static void read_rspmem(void);
static void read_rspmemb(void);
static void read_rspmemh(void);
static void read_rspmemd(void);
static void write_rspmem(void);
static void write_rspmemb(void);
static void write_rspmemh(void);
static void write_rspmemd(void);
static void read_rspreg(void);
static void read_rspregb(void);
static void read_rspregh(void);
static void read_rspregd(void);
static void write_rspreg(void);
static void write_rspregb(void);
static void write_rspregh(void);
static void write_rspregd(void);
static void read_rspreg2(void);
static void read_rspreg2b(void);
static void read_rspreg2h(void);
static void read_rspreg2d(void);
static void write_rspreg2(void);
static void write_rspreg2b(void);
static void write_rspreg2h(void);
static void write_rspreg2d(void);
static void read_dp(void);
static void read_dpb(void);
static void read_dph(void);
static void read_dpd(void);
static void write_dp(void);
static void write_dpb(void);
static void write_dph(void);
static void write_dpd(void);
static void read_dps(void);
static void read_dpsb(void);
static void read_dpsh(void);
static void read_dpsd(void);
static void write_dps(void);
static void write_dpsb(void);
static void write_dpsh(void);
static void write_dpsd(void);
static void read_mi(void);
static void read_mib(void);
static void read_mih(void);
static void read_mid(void);
static void write_mi(void);
static void write_mib(void);
static void write_mih(void);
static void write_mid(void);
static void read_vi(void);
static void read_vib(void);
static void read_vih(void);
static void read_vid(void);
static void write_vi(void);
static void write_vib(void);
static void write_vih(void);
static void write_vid(void);
static void read_ai(void);
static void read_aib(void);
static void read_aih(void);
static void read_aid(void);
static void write_ai(void);
static void write_aib(void);
static void write_aih(void);
static void write_aid(void);
static void read_pi(void);
static void read_pib(void);
static void read_pih(void);
static void read_pid(void);
static void write_pi(void);
static void write_pib(void);
static void write_pih(void);
static void write_pid(void);
static void read_ri(void);
static void read_rib(void);
static void read_rih(void);
static void read_rid(void);
static void write_ri(void);
static void write_rib(void);
static void write_rih(void);
static void write_rid(void);
static void read_si(void);
static void read_sib(void);
static void read_sih(void);
static void read_sid(void);
static void write_si(void);
static void write_sib(void);
static void write_sih(void);
static void write_sid(void);
static void read_pi_flashram_status(void);
static void read_pi_flashram_statusb(void);
static void read_pi_flashram_statush(void);
static void read_pi_flashram_statusd(void);
static void write_pi_flashram_command(void);
static void write_pi_flashram_commandb(void);
static void write_pi_flashram_commandh(void);
static void write_pi_flashram_commandd(void);
static void read_rom(void);
static void read_romb(void);
static void read_romh(void);
static void read_romd(void);
static void write_rom(void);
static void read_pif(void);
static void read_pifb(void);
static void read_pifh(void);
static void read_pifd(void);
static void write_pif(void);
static void write_pifb(void);
static void write_pifh(void);
static void write_pifd(void);
static void read_dd(void);
static void read_ddb(void);
static void read_ddh(void);
static void read_ddd(void);
static void write_dd(void);
static void write_ddb(void);
static void write_ddh(void);
static void write_ddd(void);

/* definitions of the rcp's structures and memory area */
uint32_t g_pi_regs[PI_REGS_COUNT];
uint32_t g_si_regs[SI_REGS_COUNT];

enum cic_type g_cic_type;

uint8_t g_pif_ram[PIF_RAM_SIZE];

#if NEW_DYNAREC != NEW_DYNAREC_ARM
// address : address of the read/write operation being done
uint32_t address = 0;
#endif

// values that are being written are stored in these variables
#if NEW_DYNAREC != NEW_DYNAREC_ARM
uint32_t word;
uint8_t cpu_byte;
uint16_t hword;
uint64_t dword;
#endif

// address where the read value will be stored
uint64_t* rdword;

// hash tables of read functions
void (*readmem[0x10000])(void);
void (*readmemb[0x10000])(void);
void (*readmemh[0x10000])(void);
void (*readmemd[0x10000])(void);

// hash tables of write functions
void (*writemem[0x10000])(void);
void (*writememb[0x10000])(void);
void (*writememd[0x10000])(void);
void (*writememh[0x10000])(void);

uint32_t VI_REFRESH = 1500;

static enum cic_type detect_cic_type(const void* ipl3)
{
   size_t i;
   unsigned long long crc = 0;

   for (i = 0; i < 0xfc0/4; i++)
      crc += ((uint32_t*)ipl3)[i];

   switch(crc)
   {
      default:
         DebugMessage(M64MSG_WARNING, "Unknown CIC type (%08x)! using CIC 6102.", crc);
      case 0x000000D057C85244LL: return CIC_X102;
      case 0x000000D0027FDF31LL:
      case 0x000000CFFB631223LL: return CIC_X101;
      case 0x000000D6497E414BLL: return CIC_X103;
      case 0x0000011A49F60E96LL: return CIC_X105;
      case 0x000000D6D5BE5580LL: return CIC_X106;
   }

   /* never reached */
   return 0;
}

typedef int (*readfn)(void*,uint32_t,uint32_t*);
typedef int (*writefn)(void*,uint32_t,uint32_t,uint32_t);

static INLINE unsigned int bshift(uint32_t address)
{
   return ((address & 3) ^ 3) << 3;
}

static INLINE unsigned int hshift(uint32_t address)
{
   return ((address & 2) ^ 2) << 3;
}

static int readb(readfn read_word, void* opaque, uint32_t address, unsigned long long int* value)
{
   uint32_t w;
   unsigned shift = bshift(address);
   int result = read_word(opaque, address, &w);
   *value = (w >> shift) & 0xff;
   return result;
}

static int readh(readfn read_word, void* opaque, uint32_t address, unsigned long long int* value)
{
   uint32_t w;
   unsigned shift = hshift(address);
   int result = read_word(opaque, address, &w);
   *value = (w >> shift) & 0xffff;
   return result;
}

static int readw(readfn read_word, void* opaque, uint32_t address, unsigned long long int* value)
{
   uint32_t w;
   int result = read_word(opaque, address, &w);
   *value = w;
   return result;
}

static int readd(readfn read_word, void* opaque, uint32_t address, unsigned long long int* value)
{
   uint32_t w[2];
   int result =
      read_word(opaque, address , &w[0]);
   read_word(opaque, address + 4, &w[1]);
   *value = ((uint64_t)w[0] << 32) | w[1];
   return result;
}

static int writeb(writefn write_word, void *opaque, uint32_t address, uint8_t value)
{
   unsigned int shift = bshift(address);
   uint32_t w = (uint32_t)value << shift;
   uint32_t mask = (uint32_t)0xff << shift;
   return write_word(opaque, address, w, mask);
}

static int writeh(writefn write_word, void *opaque, uint32_t address, uint16_t value)
{
   unsigned int shift = hshift(address);
   uint32_t w = (uint32_t)value << shift;
   uint32_t mask = (uint32_t)0xffff << shift;
   return write_word(opaque, address, w, mask);
}

static int writew(writefn write_word, void *opaque, uint32_t address, uint32_t value)
{
   return write_word(opaque, address, value, ~0U);
}

static int writed(writefn write_word, void *opaque, uint32_t address, uint64_t value)
{
   int result;
   const uint64_t doubleword = (uint64_t)value;
   const uint32_t word_hi = (uint32_t)(doubleword >> 32);
   const uint32_t word_lo = (uint32_t)(doubleword >>  0);

   result =
      write_word(opaque, address + 0, word_hi, ~0U);
   write_word   (opaque, address + 4, word_lo, ~0U);
   return result;
}

#ifdef DBG
static int memtype[0x10000];
static void (*saved_readmemb[0x10000])(void);
static void (*saved_readmemh[0x10000])(void);
static void (*saved_readmem [0x10000])(void);
static void (*saved_readmemd[0x10000])(void);
static void (*saved_writememb[0x10000])(void);
static void (*saved_writememh[0x10000])(void);
static void (*saved_writemem [0x10000])(void);
static void (*saved_writememd[0x10000])(void);

static void readmemb_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 1,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_READ);
   saved_readmemb[address>>16]();
}
static void readmemh_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 2,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_READ);
   saved_readmemh[address>>16]();
}
static void readmem_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 4,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_READ);
   saved_readmem[address>>16]();
}
static void readmemd_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 8,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_READ);
   saved_readmemd[address>>16]();
}
static void writememb_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 1,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_WRITE);
   return saved_writememb[address>>16]();
}
static void writememh_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 2,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_WRITE);
   return saved_writememh[address>>16]();
}
static void writemem_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 4,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_WRITE);
   return saved_writemem[address>>16]();
}
static void writememd_with_bp_checks(void)
{
   check_breakpoints_on_mem_access((PC->addr)-0x4, address, 8,
         M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_WRITE);
   return saved_writememd[address>>16]();
}

void activate_memory_break_read(uint32_t address)
{
   uint16_t region = address >> 16;
   if (saved_readmem[region] == NULL)
   {
      saved_readmemb[region] = readmemb[region];
      saved_readmemh[region] = readmemh[region];
      saved_readmem [region] = readmem [region];
      saved_readmemd[region] = readmemd[region];
      readmemb[region] = readmemb_with_bp_checks;
      readmemh[region] = readmemh_with_bp_checks;
      readmem [region] = readmem_with_bp_checks;
      readmemd[region] = readmemd_with_bp_checks;
   }
}

void deactivate_memory_break_read(uint32_t address)
{
   uint16_t region = address >> 16;
   if (saved_readmem[region] != NULL)
   {
      readmemb[region] = saved_readmemb[region];
      readmemh[region] = saved_readmemh[region];
      readmem [region] = saved_readmem [region];
      readmemd[region] = saved_readmemd[region];
      saved_readmemb[region] = NULL;
      saved_readmemh[region] = NULL;
      saved_readmem [region] = NULL;
      saved_readmemd[region] = NULL;
   }
}
void activate_memory_break_write(uint32_t address)
{
   uint16_t region = address >> 16;
   if (saved_writemem[region] == NULL)
   {
      saved_writememb[region] = writememb[region];
      saved_writememh[region] = writememh[region];
      saved_writemem [region] = writemem [region];
      saved_writememd[region] = writememd[region];
      writememb[region] = writememb_with_bp_checks;
      writememh[region] = writememh_with_bp_checks;
      writemem [region] = writemem_with_bp_checks;
      writememd[region] = writememd_with_bp_checks;
   }
}
void deactivate_memory_break_write(uint32_t address)
{
   uint16_t region = address >> 16;
   if (saved_writemem[region] != NULL)
   {
      writememb[region] = saved_writememb[region];
      writememh[region] = saved_writememh[region];
      writemem [region] = saved_writemem [region];
      writememd[region] = saved_writememd[region];
      saved_writememb[region] = NULL;
      saved_writememh[region] = NULL;
      saved_writemem [region] = NULL;
      saved_writememd[region] = NULL;
   }
}
int get_memory_type(uint32_t address)
{
   return memtype[address >> 16];
}
#endif

#define R(x) read_ ## x ## b, read_ ## x ## h, read_ ## x, read_ ## x ## d
#define W(x) write_ ## x ## b, write_ ## x ## h, write_ ## x, write_ ## x ## d
#define RW(x) R(x), W(x)

int init_memory(void)
{
   int i;

#ifdef DBG
   memset(saved_readmem, 0, 0x10000*sizeof(saved_readmem[0]));
   memset(saved_writemem, 0, 0x10000*sizeof(saved_writemem[0]));
#endif

   /* clear mappings */
   for (i = 0; i < 0x10000; ++i)
   {
      map_region(i, M64P_MEM_NOMEM, RW(nomem));
   }

   /* map RDRAM */
   for (i = 0; i< 0x80; ++i)
   {
      map_region(0x8000+i, M64P_MEM_RDRAM, RW(rdram));
      map_region(0xa000+i, M64P_MEM_RDRAM, RW(rdram));
   }

   for(i = 0x80; i < 0x3f0; ++i)
   {
      map_region(0x8000+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa000+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map RDRAM registers */
   map_region(0x83f0, M64P_MEM_RDRAMREG, RW(rdramreg));
   map_region(0xa3f0, M64P_MEM_RDRAMREG, RW(rdramreg));

   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x83f0+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa3f0+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map RSP memory */
   map_region(0x8400, M64P_MEM_RSPMEM, RW(rspmem));
   map_region(0xa400, M64P_MEM_RSPMEM, RW(rspmem));

   for (i=1; i<0x4; i++)
   {
      map_region(0x8400+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa400+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map RSP registers (1) */
   map_region(0x8404, M64P_MEM_RSPREG, RW(rspreg));
   map_region(0xa404, M64P_MEM_RSPREG, RW(rspreg));

   for (i=5; i<8; i++)
   {
      map_region(0x8400+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa400+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map RSP registers (2) */
   map_region(0x8408, M64P_MEM_RSP, RW(rspreg2));
   map_region(0xa408, M64P_MEM_RSP, RW(rspreg2));

   for(i = 0x9; i < 0x10; ++i)
   {
      map_region(0x8400+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa400+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map DPC registers */
   map_region(0x8410, M64P_MEM_DP, RW(dp));
   map_region(0xa410, M64P_MEM_DP, RW(dp));
   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x8410+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa410+i, M64P_MEM_NOTHING, RW(nothing));
   }

   map_region(0x8420, M64P_MEM_DPS, RW(dps));
   map_region(0xa420, M64P_MEM_DPS, RW(dps));
   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x8420+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa420+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map MI registers */
   map_region(0x8430, M64P_MEM_MI, RW(mi));
   map_region(0xa430, M64P_MEM_MI, RW(mi));

   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x8430+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa430+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map VI registers */
   map_region(0x8440, M64P_MEM_VI, RW(vi));
   map_region(0xa440, M64P_MEM_VI, RW(vi));
   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x8440+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa440+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map AI registers */
   map_region(0x8450, M64P_MEM_AI, RW(ai));
   map_region(0xa450, M64P_MEM_AI, RW(ai));

   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x8450+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa450+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* init PI registers */
   memset(g_pi_regs, 0, PI_REGS_COUNT*sizeof(g_pi_regs[0]));

   /* map PI registers */

   map_region(0x8460, M64P_MEM_PI, RW(pi));
   map_region(0xa460, M64P_MEM_PI, RW(pi));
   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x8460+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa460+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map RI registers */
   map_region(0x8470, M64P_MEM_RI, RW(ri));
   map_region(0xa470, M64P_MEM_RI, RW(ri));

   for(i = 1; i < 0x10; ++i)
   {
      map_region(0x8470+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa470+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* init SI registers */
   memset(g_si_regs, 0, SI_REGS_COUNT*sizeof(g_si_regs[0])); 

    /* map SI registers */
   map_region(0x8480, M64P_MEM_SI, RW(si));
   map_region(0xa480, M64P_MEM_SI, RW(si));
   for(i = 0x481; i < 0x500; ++i)
   {
      map_region(0x8000+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa000+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map DD registers */
   map_region(0x8500, M64P_MEM_NOTHING, RW(dd));
   map_region(0xa500, M64P_MEM_NOTHING, RW(dd));
   for(i = 0x501; i < 0x800; ++i)
   {
      map_region(0x8000+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa000+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map flashram/sram */
   map_region(0x8800, M64P_MEM_FLASHRAMSTAT, R(pi_flashram_status), W(nothing));
   map_region(0xa800, M64P_MEM_FLASHRAMSTAT, R(pi_flashram_status), W(nothing));
   map_region(0x8801, M64P_MEM_NOTHING, R(nothing), W(pi_flashram_command));
   map_region(0xa801, M64P_MEM_NOTHING, R(nothing), W(pi_flashram_command));
   for(i = 0x802; i < 0x1000; ++i)
   {
      map_region(0x8000+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xa000+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* map cart ROM */

   for(i = 0; i < (rom_size >> 16); ++i)
   {
      map_region(0x9000+i, M64P_MEM_ROM, R(rom), W(nothing));
      map_region(0xb000+i, M64P_MEM_ROM, R(rom),
            write_nothingb, write_nothingh, write_rom, write_nothingd);
   }
   for(i = (rom_size >> 16); i < 0xfc0; ++i)
   {
      map_region(0x9000+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xb000+i, M64P_MEM_NOTHING, RW(nothing));
   }

   /* init CIC type */
   g_cic_type = detect_cic_type(rom + 0x40);

   /* init PIF_RAM */
   memset(g_pif_ram, 0, PIF_RAM_SIZE);

   /* map PIF RAM */
   map_region(0x9fc0, M64P_MEM_PIF, RW(pif));
   map_region(0xbfc0, M64P_MEM_PIF, RW(pif));

   for(i = 0xfc1; i < 0x1000; ++i)
   {
      map_region(0x9000+i, M64P_MEM_NOTHING, RW(nothing));
      map_region(0xb000+i, M64P_MEM_NOTHING, RW(nothing));
   }

   flashram_info.use_flashram = 0;
   init_flashram();

   fast_memory = 1;

   init_r4300(&g_r4300);
   init_rdp(&g_dp);
   init_rsp(&g_sp);
   init_ai(&g_ai);
   init_ri(&g_ri);
   init_vi(&g_vi);

   DebugMessage(M64MSG_VERBOSE, "Memory initialized");
   return 0;
}

static void map_region_t(uint16_t region, int type)
{
#ifdef DBG
   memtype[region] = type;
#else
   (void)region;
   (void)type;
#endif
}

static void map_region_r(uint16_t region,
 void (*read8)(void),
 void (*read16)(void),
 void (*read32)(void),
 void (*read64)(void))
{
#ifdef DBG
   if (lookup_breakpoint(((uint32_t)region << 16), 0x10000,
            M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_READ) != -1)
   {
      saved_readmemb[region] = read8;
      saved_readmemh[region] = read16;
      saved_readmem [region] = read32;
      saved_readmemd[region] = read64;
      readmemb[region] = readmemb_with_bp_checks;
      readmemh[region] = readmemh_with_bp_checks;
      readmem [region] = readmem_with_bp_checks;
      readmemd[region] = readmemd_with_bp_checks;
   }
   else
#endif
   {
      readmemb[region] = read8;
      readmemh[region] = read16;
      readmem [region] = read32;
      readmemd[region] = read64;
   }
}

void map_region_w(uint16_t region,
 void (*write8)(void),
 void (*write16)(void),
 void (*write32)(void),
 void (*write64)(void))
{
#ifdef DBG
   if (lookup_breakpoint(((uint32_t)region << 16), 0x10000,
            M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_WRITE) != -1)
   {
      saved_writememb[region] = write8;
      saved_writememh[region] = write16;
      saved_writemem [region] = write32;
      saved_writememd[region] = write64;
      writememb[region] = writememb_with_bp_checks;
      writememh[region] = writememh_with_bp_checks;
      writemem [region] = writemem_with_bp_checks;
      writememd[region] = writememd_with_bp_checks;
   }
   else
#endif
   {
      writememb[region] = write8;
      writememh[region] = write16;
      writemem [region] = write32;
      writememd[region] = write64;
   }
}

void map_region(uint16_t region,
      int type,
 void (*read8)(void),
 void (*read16)(void),
 void (*read32)(void),
 void (*read64)(void),
 void (*write8)(void),
 void (*write16)(void),
 void (*write32)(void),
 void (*write64)(void))
{
   map_region_t(region, type);
   map_region_r(region, read8, read16, read32, read64);
   map_region_w(region, write8, write16, write32, write64);
}

static void invalidate_code(uint32_t address)
{
   if (r4300emu != CORE_PURE_INTERPRETER && !invalid_code[address>>12])
      if (blocks[address>>12]->block[(address&0xFFF)/4].ops !=
            current_instruction_table.NOTCOMPILED)
         invalid_code[address>>12] = 1;
}

static void read_nothing(void)
{
   *rdword = 0;
}

static void read_nothingb(void)
{
   *rdword = 0;
}

static void read_nothingh(void)
{
   *rdword = 0;
}

static void read_nothingd(void)
{
   *rdword = 0;
}

static void write_nothing(void)
{
}

static void write_nothingb(void)
{
}

static void write_nothingh(void)
{
}

static void write_nothingd(void)
{
}

static void read_nomem(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_word_in_memory();
}

static void read_nomemb(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_byte_in_memory();
}

void read_nomemh(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_hword_in_memory();
}

void read_nomemd(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_dword_in_memory();
}

static void write_nomem(void)
{
   invalidate_code(address);
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_word_in_memory();
}

static void write_nomemb(void)
{
   invalidate_code(address);
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_byte_in_memory();
}

static void write_nomemh(void)
{
   invalidate_code(address);
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_hword_in_memory();
}

static void write_nomemd(void)
{
   invalidate_code(address);
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_dword_in_memory();
}

void read_rdram(void)
{
   readw(read_rdram_dram, &g_ri, address, rdword);
}

void read_rdramb(void)
{
   readb(read_rdram_dram, &g_ri, address, rdword);
}

void read_rdramh(void)
{
   readh(read_rdram_dram, &g_ri, address, rdword);
}

void read_rdramd(void)
{
   readd(read_rdram_dram, &g_ri, address, rdword);
}

void write_rdram(void)
{
   writew(write_rdram_dram, &g_ri, address, word);
}

void write_rdramb(void)
{
   writeb(write_rdram_dram, &g_ri, address, cpu_byte);
}

void write_rdramh(void)
{
   writeh(write_rdram_dram, &g_ri, address, hword);
}

void write_rdramd(void)
{
   writed(write_rdram_dram, &g_ri, address, dword);
}

void read_rdramFB(void)
{
   readw(read_rdram_fb, &g_dp, address, rdword);
}
void read_rdramFBb(void)
{
   readb(read_rdram_fb, &g_dp, address, rdword);
}
void read_rdramFBh(void)
{
   readh(read_rdram_fb, &g_dp, address, rdword);
}
void read_rdramFBd(void)
{
   readd(read_rdram_fb, &g_dp, address, rdword);
}
void write_rdramFB(void)
{
   writew(write_rdram_fb, &g_dp, address, word);
}
void write_rdramFBb(void)
{
   writeb(write_rdram_fb, &g_dp, address, cpu_byte);
}
void write_rdramFBh(void)
{
   writeh(write_rdram_fb, &g_dp, address, hword);
}
void write_rdramFBd(void)
{
   writed(write_rdram_fb, &g_dp, address, dword);
}
static void read_rdramreg(void)
{
   readw(read_rdram_regs, &g_ri, address, rdword);
}
static void read_rdramregb(void)
{
   readb(read_rdram_regs, &g_ri, address, rdword);
}
static void read_rdramregh(void)
{
   readh(read_rdram_regs, &g_ri, address, rdword);
}
static void read_rdramregd(void)
{
   readd(read_rdram_regs, &g_ri, address, rdword);
}
static void write_rdramreg(void)
{
   writew(write_rdram_regs, &g_ri, address, word);
}
static void write_rdramregb(void)
{
   writeb(write_rdram_regs, &g_ri, address, cpu_byte);
}
static void write_rdramregh(void)
{
   writeh(write_rdram_regs, &g_ri, address, hword);
}
static void write_rdramregd(void)
{
   writed(write_rdram_regs, &g_ri, address, dword);
}

static void read_rspmem(void)
{
   readw(read_rsp_mem, &g_sp, address, rdword);
}
static void read_rspmemb(void)
{
   readb(read_rsp_mem, &g_sp, address, rdword);
}
static void read_rspmemh(void)
{
   readh(read_rsp_mem, &g_sp, address, rdword);
}
static void read_rspmemd(void)
{
   readd(read_rsp_mem, &g_sp, address, rdword);
}
static void write_rspmem(void)
{
   writew(write_rsp_mem, &g_sp, address, word);
}
static void write_rspmemb(void)
{
   writeb(write_rsp_mem, &g_sp, address, cpu_byte);
}
static void write_rspmemh(void)
{
   writeh(write_rsp_mem, &g_sp, address, hword);
}
static void write_rspmemd(void)
{
   writed(write_rsp_mem, &g_sp, address, dword);
}
static void read_rspreg(void)
{
   readw(read_rsp_regs, &g_sp, address, rdword);
}
static void read_rspregb(void)
{
   readb(read_rsp_regs, &g_sp, address, rdword);
}
static void read_rspregh(void)
{
   readh(read_rsp_regs, &g_sp, address, rdword);
}
static void read_rspregd(void)
{
   readd(read_rsp_regs, &g_sp, address, rdword);
}
static void write_rspreg(void)
{
   writew(write_rsp_regs, &g_sp, address, word);
}
static void write_rspregb(void)
{
   writeb(write_rsp_regs, &g_sp, address, cpu_byte);
}
static void write_rspregh(void)
{
   writeh(write_rsp_regs, &g_sp, address, hword);
}
static void write_rspregd(void)
{
   writed(write_rsp_regs, &g_sp, address, dword);
}
static void read_rspreg2(void)
{
   readw(read_rsp_regs2, &g_sp, address, rdword);
}
static void read_rspreg2b(void)
{
   readb(read_rsp_regs2, &g_sp, address, rdword);
}
static void read_rspreg2h(void)
{
   readh(read_rsp_regs2, &g_sp, address, rdword);
}
static void read_rspreg2d(void)
{
   readd(read_rsp_regs2, &g_sp, address, rdword);
}
static void write_rspreg2(void)
{
   writew(write_rsp_regs2, &g_sp, address, word);
}
static void write_rspreg2b(void)
{
   writeb(write_rsp_regs2, &g_sp, address, cpu_byte);
}
static void write_rspreg2h(void)
{
   writeh(write_rsp_regs2, &g_sp, address, hword);
}
static void write_rspreg2d(void)
{
   writed(write_rsp_regs2, &g_sp, address, dword);
}

static void read_dp(void)
{
   readw(read_dpc_regs, &g_dp, address, rdword);
}

static void read_dpb(void)
{
   readb(read_dpc_regs, &g_dp, address, rdword);
}

static void read_dph(void)
{
   readh(read_dpc_regs, &g_dp, address, rdword);
}

static void read_dpd(void)
{
   readd(read_dpc_regs, &g_dp, address, rdword);
}

static void write_dp(void)
{
   writew(write_dpc_regs, &g_dp, address, word);
}

static void write_dpb(void)
{
   writeb(write_dpc_regs, &g_dp, address, cpu_byte);
}

static void write_dph(void)
{
   writeh(write_dpc_regs, &g_dp, address, hword);
}

static void write_dpd(void)
{
   writed(write_dpc_regs, &g_dp, address, dword);
}

static void read_dps(void)
{
   readw(read_dps_regs, &g_dp, address, rdword);
}

static void read_dpsb(void)
{
   readb(read_dps_regs, &g_dp, address, rdword);
}

static void read_dpsh(void)
{
   readh(read_dps_regs, &g_dp, address, rdword);
}

static void read_dpsd(void)
{
   readd(read_dps_regs, &g_dp, address, rdword);
}

static void write_dps(void)
{
   writew(write_dps_regs, &g_dp, address, word);
}

static void write_dpsb(void)
{
   writeb(write_dps_regs, &g_dp, address, cpu_byte);
}

static void write_dpsh(void)
{
   writeh(write_dps_regs, &g_dp, address, hword);
}

static void write_dpsd(void)
{
   writed(write_dps_regs, &g_dp, address, dword);
}

static void read_mi(void)
{
   readw(read_mi_regs, &g_r4300, address, rdword);
}

static void read_mib(void)
{
   readb(read_mi_regs, &g_r4300, address, rdword);
}

static void read_mih(void)
{
   readh(read_mi_regs, &g_r4300, address, rdword);
}

static void read_mid(void)
{
   readd(read_mi_regs, &g_r4300, address, rdword);
}

static void write_mi(void)
{
   writew(write_mi_regs, &g_r4300, address, word);
}

static void write_mib(void)
{
   writeb(write_mi_regs, &g_r4300, address, cpu_byte);
}

static void write_mih(void)
{
   writeh(write_mi_regs, &g_r4300, address, hword);
}

static void write_mid(void)
{
   writed(write_mi_regs, &g_r4300, address, dword);
}

static void read_vi(void)
{
   readw(read_vi_regs, &g_vi, address, rdword);
}

static void read_vib(void)
{
   readb(read_vi_regs, &g_vi, address, rdword);
}

static void read_vih(void)
{
   readh(read_vi_regs, &g_vi, address, rdword);
}

static void read_vid(void)
{
   readd(read_vi_regs, &g_vi, address, rdword);
}

static void write_vi(void)
{
   writew(write_vi_regs, &g_vi, address, word);
}

static void write_vib(void)
{
   writeb(write_vi_regs, &g_vi, address, cpu_byte);
}

static void write_vih(void)
{
   writeh(write_vi_regs, &g_vi, address, hword);
}

static void write_vid(void)
{
   writed(write_vi_regs, &g_vi, address, dword);
}

static void read_ai(void)
{
   readw(read_ai_regs, &g_ai, address, rdword);
}

static void read_aib(void)
{
   readb(read_ai_regs, &g_ai, address, rdword);
}

static void read_aih(void)
{
   readh(read_ai_regs, &g_ai, address, rdword);
}

static void read_aid(void)
{
   readd(read_ai_regs, &g_ai, address, rdword);
}

static void write_ai(void)
{
   writew(write_ai_regs, &g_ai, address, word);
}

static void write_aib(void)
{
   writeb(write_ai_regs, &g_ai, address, cpu_byte);
}

static void write_aih(void)
{
   writeh(write_ai_regs, &g_ai, address, hword);
}

static void write_aid(void)
{
   writed(write_ai_regs, &g_ai, address, dword);
}

static INLINE uint32_t pi_reg(uint32_t address)
{
   return (address & 0xffff) >> 2;
}

static int read_pi_regs(void *opaque, uint32_t address, uint32_t* value)
{
   uint32_t reg = pi_reg(address);

   *value = g_pi_regs[reg];

   return 0;
}


static int write_pi_regs(void *opaque, uint32_t address, uint32_t value, uint32_t mask)
{
   uint32_t reg = pi_reg(address);

   switch (reg)
   {
      case PI_RD_LEN_REG:
         masked_write(&g_pi_regs[PI_RD_LEN_REG], value, mask);
         dma_pi_read();
         return 0;

      case PI_WR_LEN_REG:
         masked_write(&g_pi_regs[PI_WR_LEN_REG], value, mask);
         dma_pi_write();
         return 0;

      case PI_STATUS_REG:
         if (value & mask & 2) g_r4300.mi.regs[MI_INTR_REG] &= ~0x10;
         check_interupt();
         return 0;

      case PI_BSD_DOM1_LAT_REG:
      case PI_BSD_DOM1_PWD_REG:
      case PI_BSD_DOM1_PGS_REG:
      case PI_BSD_DOM1_RLS_REG:
      case PI_BSD_DOM2_LAT_REG:
      case PI_BSD_DOM2_PWD_REG:
      case PI_BSD_DOM2_PGS_REG:
      case PI_BSD_DOM2_RLS_REG:
         masked_write(&g_pi_regs[reg], value & 0xff, mask);
         return 0;
   }

   masked_write(&g_pi_regs[reg], value, mask);

   return 0;
}

static void read_pi(void)
{
   readw(read_pi_regs, NULL, address, rdword);
}

static void read_pib(void)
{
   readb(read_pi_regs, NULL, address, rdword);
}

static void read_pih(void)
{
   readh(read_pi_regs, NULL, address, rdword);
}

static void read_pid(void)
{
   readd(read_pi_regs, NULL, address, rdword);
}

static void write_pi(void)
{
   writew(write_pi_regs, NULL, address, word);
}

static void write_pib(void)
{
   writeb(write_pi_regs, NULL, address, cpu_byte);
}

static void write_pih(void)
{
   writeh(write_pi_regs, NULL, address, hword);
}

static void write_pid(void)
{
   writed(write_pi_regs, NULL, address, dword);
}

static void read_ri(void)
{
   readw(read_ri_regs, &g_ri, address, rdword);
}

static void read_rib(void)
{
   readb(read_ri_regs, &g_ri, address, rdword);
}

static void read_rih(void)
{
   readh(read_ri_regs, &g_ri, address, rdword);
}

static void read_rid(void)
{
   readd(read_ri_regs, &g_ri, address, rdword);
}

static void write_ri(void)
{
   writew(write_ri_regs, &g_ri, address, word);
}

static void write_rib(void)
{
   writeb(write_ri_regs, &g_ri, address, cpu_byte);
}

static void write_rih(void)
{
   writeh(write_ri_regs, &g_ri, address, hword);
}

static void write_rid(void)
{
   writed(write_ri_regs, &g_ri, address, dword);
}

static INLINE uint32_t si_reg(uint32_t address)
{
   return (address & 0xffff) >> 2;
}

static int read_si_regs(void *opaque, uint32_t address, uint32_t* value)
{
   uint32_t reg = si_reg(address);
   *value = g_si_regs[reg];
   return 0;
}

static int write_si_regs(void *opaque, uint32_t address, uint32_t value, uint32_t mask)
{
   uint32_t reg = si_reg(address);
   switch (reg)
   {
      case SI_DRAM_ADDR_REG:
         masked_write(&g_si_regs[SI_DRAM_ADDR_REG], value, mask);
         break;
      case SI_PIF_ADDR_RD64B_REG:
         masked_write(&g_si_regs[SI_PIF_ADDR_RD64B_REG], value, mask);
         dma_si_read();
         break;
      case SI_PIF_ADDR_WR64B_REG:
         masked_write(&g_si_regs[SI_PIF_ADDR_WR64B_REG], value, mask);
         dma_si_write();
         break;
      case SI_STATUS_REG:
         g_r4300.mi.regs[MI_INTR_REG] &= ~0x2;
         g_si_regs[SI_STATUS_REG] &= ~0x1000;
         check_interupt();
         break;
   }
   return 0;
}

static void read_si(void)
{
   readw(read_si_regs, NULL, address, rdword);
}

static void read_sib(void)
{
   readb(read_si_regs, NULL, address, rdword);
}

static void read_sih(void)
{
   readh(read_si_regs, NULL, address, rdword);
}

static void read_sid(void)
{
   readd(read_si_regs, NULL, address, rdword);
}

static void write_si(void)
{
   writew(write_si_regs, NULL, address, word);
}

static void write_sib(void)
{
   writeb(write_si_regs, NULL, address, cpu_byte);
}

static void write_sih(void)
{
   writeh(write_si_regs, NULL, address, hword);
}

static void write_sid(void)
{
   writed(write_si_regs, NULL, address, dword);
}

static int read_flashram_status(void* opaque, uint32_t address, uint32_t* value)
{
   if ((flashram_info.use_flashram == -1) || ((address & 0xffff) != 0))
   {
      DebugMessage(M64MSG_ERROR, "unknown read in read_flashram_status()");
      return -1;
   }
   flashram_info.use_flashram = 1;
   *value = flashram_status();
   return 0;
}

static int write_flashram_command(void* opaque, uint32_t address, uint32_t value, uint32_t mask)
{
   if ((flashram_info.use_flashram == -1) || ((address & 0xffff) != 0))
   {
      DebugMessage(M64MSG_ERROR, "unknown write in write_flashram_command()");
      return -1;
   }
   flashram_info.use_flashram = 1;
   flashram_command(value & mask);
   return 0;
}


static void read_pi_flashram_status(void)
{
   readw(read_flashram_status, NULL, address, rdword);
}

static void read_pi_flashram_statusb(void)
{
   readb(read_flashram_status, NULL, address, rdword);
}

static void read_pi_flashram_statush(void)
{
   readh(read_flashram_status, NULL, address, rdword);
}

static void read_pi_flashram_statusd(void)
{
   readd(read_flashram_status, NULL, address, rdword);
}

static void write_pi_flashram_command(void)
{
   writew(write_flashram_command, NULL, address, word);
}

static void write_pi_flashram_commandb(void)
{
   writeb(write_flashram_command, NULL, address, cpu_byte);
}

static void write_pi_flashram_commandh(void)
{
   writeh(write_flashram_command, NULL, address, hword);
}

static void write_pi_flashram_commandd(void)
{
   writed(write_flashram_command, NULL, address, dword);
}

static uint32_t lastwrite = 0;

static INLINE uint32_t rom_address(uint32_t address)
{
   return (address & 0x03fffffc);
}

static int read_cart_rom(void *opaque, uint32_t address, uint32_t* value)
{
   uint32_t addr = rom_address(address);
   if (lastwrite)
   {
      *value = lastwrite;
      lastwrite = 0;
   }
   else
   {
      *value = *(uint32_t*)(rom + addr);
   }
   return 0;
}

static int write_cart_rom(void *opaque, uint32_t address, uint32_t value, uint32_t mask)
{
   lastwrite = value & mask;
   return 0;
}

static void read_rom(void)
{
   readw(read_cart_rom, NULL, address, rdword);
}

static void read_romb(void)
{
   readb(read_cart_rom, NULL, address, rdword);
}

static void read_romh(void)
{
   readh(read_cart_rom, NULL, address, rdword);
}

static void read_romd(void)
{
   readd(read_cart_rom, NULL, address, rdword);
}

static void write_rom(void)
{
   writew(write_cart_rom, NULL, address, word);
}

static INLINE uint32_t pif_ram_address(uint32_t address)
{
   return ((address & 0xfffc) - 0x7c0);
}

static int read_pif_ram(void *opaque, uint32_t address, uint32_t* value)
{
   uint32_t addr = pif_ram_address(address);
   if (addr >= PIF_RAM_SIZE)
   {
      DebugMessage(M64MSG_ERROR, "Invalid PIF address: %08x", address);
      *value = 0;
      return -1;
   }
   memcpy(value, g_pif_ram + addr, sizeof(*value));
   *value = sl(*value);
   return 0;
}

static int write_pif_ram(void *opaque, uint32_t address, uint32_t value, uint32_t mask)
{
   uint32_t addr = pif_ram_address(address);
   if (addr >= PIF_RAM_SIZE)
   {
      DebugMessage(M64MSG_ERROR, "Invalid PIF address: %08x", address);
      return -1;
   }
   masked_write((uint32_t*)(&g_pif_ram[addr]), sl(value), sl(mask));
   if ((addr == 0x3c) && (mask & 0xff))
   {
      if (g_pif_ram[0x3f] == 0x08)
      {
         g_pif_ram[0x3f] = 0;
         update_count();
         add_interupt_event(SI_INT, /*0x100*/0x900);
      }
      else
      {
         update_pif_write();
      }
   }
   return 0;
}

static void read_pif(void)
{
   readw(read_pif_ram, NULL, address, rdword);
}

static void read_pifb(void)
{
   readb(read_pif_ram, NULL, address, rdword);
}

static void read_pifh(void)
{
   readh(read_pif_ram, NULL, address, rdword);
}

static void read_pifd(void)
{
   readd(read_pif_ram, NULL, address, rdword);
}

static void write_pif(void)
{
   writew(write_pif_ram, NULL, address, word);
}

static void write_pifb(void)
{
   writeb(write_pif_ram, NULL, address, cpu_byte);
}

static void write_pifh(void)
{
   writeh(write_pif_ram, NULL, address, hword);
}

static void write_pifd(void)
{
   writed(write_pif_ram, NULL, address, dword);
}

/* HACK: just to get F-Zero to boot
 * TODO: implement a real DD module
 */
static int read_dd_regs(void* opaque, uint32_t address, uint32_t* value)
{
   *value = (address == 0xa5000508)
      ? 0xffffffff
      : 0x00000000;

   return 0;
}

static int write_dd_regs(void* opaque, uint32_t address, uint32_t value, uint32_t mask)
{
   return 0;
}

static void read_dd(void)
{
   readw(read_dd_regs, NULL, address, rdword);
}

static void read_ddb(void)
{
   readb(read_dd_regs, NULL, address, rdword);
}

static void read_ddh(void)
{
   readh(read_dd_regs, NULL, address, rdword);
}

static void read_ddd(void)
{
   readd(read_dd_regs, NULL, address, rdword);
}

static void write_dd(void)
{
   writew(write_dd_regs, NULL, address, word);
}

static void write_ddb(void)
{
   writeb(write_dd_regs, NULL, address, cpu_byte);
}

static void write_ddh(void)
{
   writeh(write_dd_regs, NULL, address, hword);
}

static void write_ddd(void)
{
   writed(write_dd_regs, NULL, address, dword);
}

uint32_t *fast_mem_access(uint32_t address)
{
   /* This code is performance critical, specially on pure interpreter mode.
    * Removing error checking saves some time, but the emulator may crash. */
   if ((address & 0xc0000000) != 0x80000000)
      address = virtual_to_physical_address(address, 2);

   address &= 0x1ffffffc;

   if (address < RDRAM_MAX_SIZE)
      return (unsigned int*)((unsigned char*)g_rdram + address);
   else if (address >= 0x10000000)
      return (unsigned int*)((unsigned char*)rom + address - 0x10000000);
   else if ((address & 0xffffe000) == 0x04000000)
      return (unsigned int*)((unsigned char*)g_sp.mem + (address & 0x1ffc));
   return NULL;
}

void format_saved_memory(void)
{
   uint8_t init[] =
   {
      0x81,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,
      0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b, 0x1c,0x1d,0x1e,0x1f,
      0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
      0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
      0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
      0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
      0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
      0x00,0x71,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03
   };
   int i,j;

   memset(saved_memory.sram, 0, sizeof(saved_memory.sram));
   memset(saved_memory.eeprom, 0, sizeof(saved_memory.eeprom));
   memset(saved_memory.eeprom2, 0, sizeof(saved_memory.eeprom2));
   memset(saved_memory.flashram, 0xFF, sizeof(saved_memory.flashram));

   for (i=0; i<4; i++)
   {
      for (j=0; j<0x8000; j+=2)
      {
         saved_memory.mempack[i][j] = 0;
         saved_memory.mempack[i][j+1] = 0x03;
      }
      memcpy(saved_memory.mempack[i], init, 272);
   }
}
