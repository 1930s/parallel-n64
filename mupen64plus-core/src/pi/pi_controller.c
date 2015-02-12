/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - pi_controller.c                                         *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2014 Bobby Smiles                                       *
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

#include "pi_controller.h"

#define M64P_CORE_PROTOTYPES 1
#include "../api/m64p_types.h"
#include "../api/m64p_config.h"
#include "../api/callbacks.h"
#include "../api/config.h"
#include "../main/main.h"
#include "../main/rom.h"
#include "../memory/memory.h"
#include "../r4300/cached_interp.h"
#include "../r4300/cp0.h"
#include "../r4300/interupt.h"
#include "../r4300/r4300.h"
#include "../r4300/r4300_core.h"
#include "../ri/ri_controller.h"
#include "../si/si_controller.h"

#include <string.h>

int32_t enable_expmem = 1;

void dma_enable_expmem(int enable)
{
   enable_expmem = enable;
}

static void dma_pi_read(struct pi_controller *pi)
{
   if (pi->regs[PI_CART_ADDR_REG] >= 0x08000000
         && pi->regs[PI_CART_ADDR_REG] < 0x08010000)
   {
      if (pi->use_flashram != 1)
      {
         dma_write_sram(pi);
         pi->use_flashram = -1;
      }
      else
      {
         dma_write_flashram(pi);
      }
   }
   else
   {
      DebugMessage(M64MSG_WARNING, "Unknown dma read in dma_pi_read()");
   }

   pi->regs[PI_STATUS_REG] |= 1;
   update_count();
   add_interupt_event(PI_INT, 0x1000/* pi->regs[PI_RD_LEN_REG] */);
}

static void dma_pi_write(struct pi_controller *pi)
{
   uint32_t longueur;
   int32_t i;

   if (pi->regs[PI_CART_ADDR_REG] < 0x10000000)
   {
      if (pi->regs[PI_CART_ADDR_REG] >= 0x08000000
            && pi->regs[PI_CART_ADDR_REG] < 0x08010000)
      {
         if (pi->use_flashram != 1)
         {
            dma_read_sram(pi);
            pi->use_flashram = -1;
         }
         else
         {
            dma_read_flashram(pi);
         }
      }
#if 0
      else if (pi->regs[PI_CART_ADDR_REG] >= 0x06000000
            && pi->regs[PI_CART_ADDR_REG] < 0x08000000)
      {
      }
      else
      {
         DebugMessage(M64MSG_WARNING, "Unknown dma write 0x%x in dma_pi_write()", (int32_t)pi->regs[PI_CART_ADDR_REG]);
      }
#endif

      pi->regs[PI_STATUS_REG] |= 1;
      update_count();
      add_interupt_event(PI_INT, /* pi->regs[PI_WR_LEN_REG]*/0x1000);

      return;
   }

   if (pi->regs[PI_CART_ADDR_REG] >= 0x1fc00000) // for paper mario
   {
      pi->regs[PI_STATUS_REG] |= 1;
      update_count();
      add_interupt_event(PI_INT, 0x1000);

      return;
   }

   longueur = (pi->regs[PI_WR_LEN_REG] & 0xFFFFFF)+1;
   i = (pi->regs[PI_CART_ADDR_REG]-0x10000000)&0x3FFFFFF;
   longueur = (i + (int32_t)longueur) > g_rom_size ?
      (g_rom_size - i) : longueur;
   longueur = (pi->regs[PI_DRAM_ADDR_REG] + longueur) > 0x7FFFFF ?
      (0x7FFFFF - pi->regs[PI_DRAM_ADDR_REG]) : longueur;

   if (i>g_rom_size || pi->regs[PI_DRAM_ADDR_REG] > 0x7FFFFF)
   {
      pi->regs[PI_STATUS_REG] |= 3;
      update_count();
      add_interupt_event(PI_INT, longueur/8);

      return;
   }

   if (r4300emu != CORE_PURE_INTERPRETER)
   {
      for (i=0; i<(int32_t)longueur; i++)
      {
         uint32_t rdram_address1 = pi->regs[PI_DRAM_ADDR_REG]+i+0x80000000;
         uint32_t rdram_address2 = pi->regs[PI_DRAM_ADDR_REG]+i+0xa0000000;
         ((uint8_t*)g_rdram)[(pi->regs[PI_DRAM_ADDR_REG]+i)^S8]=
            g_rom[(((pi->regs[PI_CART_ADDR_REG]-0x10000000)&0x3FFFFFF)+i)^S8];

         if (!invalid_code[rdram_address1>>12])
         {
            if (!blocks[rdram_address1>>12] ||
                  blocks[rdram_address1>>12]->block[(rdram_address1&0xFFF)/4].ops !=
                  current_instruction_table.NOTCOMPILED)
               invalid_code[rdram_address1>>12] = 1;
#ifdef NEW_DYNAREC
            invalidate_block(rdram_address1>>12);
#endif
         }

         if (!invalid_code[rdram_address2>>12])
         {
            if (!blocks[rdram_address1>>12] ||
                  blocks[rdram_address2>>12]->block[(rdram_address2&0xFFF)/4].ops !=
                  current_instruction_table.NOTCOMPILED)
               invalid_code[rdram_address2>>12] = 1;
         }
      }
   }
   else
   {
      for (i=0; i<(int32_t)longueur; i++)
      {
         ((uint8_t*)g_rdram)[(pi->regs[PI_DRAM_ADDR_REG]+i)^S8] =
            g_rom[(((pi->regs[PI_CART_ADDR_REG]-0x10000000)&0x3FFFFFF)+i)^S8];
      }
   }

   // Set the RDRAM memory size when copying main ROM code
   // (This is just a convenient way to run this code once at the beginning)
   if (pi->regs[PI_CART_ADDR_REG] == 0x10001000)
   {
      switch (g_si.pif.cic.version)
      {
         case CIC_X101:
         case CIC_X102:
         case CIC_X103:
         case CIC_X106:
            if (enable_expmem)
               g_rdram[0x318/4] = 0x800000;
            else
               g_rdram[0x318/4] = 0x400000;
            break;
         case CIC_X105:
            if (enable_expmem)
               g_rdram[0x3F0/4] = 0x800000;
            else
               g_rdram[0x3F0/4] = 0x400000;
            break;
      }
   }

   pi->regs[PI_STATUS_REG] |= 3;
   update_count();
   add_interupt_event(PI_INT, longueur/8);
}

void connect_pi(struct pi_controller* pi,
                struct r4300_core* r4300,
                struct ri_controller *ri,
                uint8_t *rom, size_t rom_size)
{
   connect_cart_rom(&pi->cart_rom, rom, rom_size);
   pi->r4300 = r4300;
   pi->ri = ri;
}

void init_pi(struct pi_controller* pi)
{
    memset(pi->regs, 0, PI_REGS_COUNT*sizeof(uint32_t));

    init_flashram(&pi->flashram);

    pi->use_flashram = 0;
}


int read_pi_regs(void* opaque, uint32_t address, uint32_t* value)
{
    struct pi_controller* pi = (struct pi_controller*)opaque;
    uint32_t reg = pi_reg(address);

    *value = pi->regs[reg];

    return 0;
}

int write_pi_regs(void* opaque, uint32_t address,
      uint32_t value, uint32_t mask)
{
   struct pi_controller* pi = (struct pi_controller*)opaque;
   uint32_t reg = pi_reg(address);

   switch (reg)
   {
      case PI_RD_LEN_REG:
         masked_write(&pi->regs[PI_RD_LEN_REG], value, mask);
         dma_pi_read(pi);
         return 0;

      case PI_WR_LEN_REG:
         masked_write(&pi->regs[PI_WR_LEN_REG], value, mask);
         dma_pi_write(pi);
         return 0;

      case PI_STATUS_REG:
         if (value & mask & 2)
            clear_rcp_interrupt(pi->r4300, MI_INTR_PI);
         return 0;

      case PI_BSD_DOM1_LAT_REG:
      case PI_BSD_DOM1_PWD_REG:
      case PI_BSD_DOM1_PGS_REG:
      case PI_BSD_DOM1_RLS_REG:
      case PI_BSD_DOM2_LAT_REG:
      case PI_BSD_DOM2_PWD_REG:
      case PI_BSD_DOM2_PGS_REG:
      case PI_BSD_DOM2_RLS_REG:
         masked_write(&pi->regs[reg], value & 0xff, mask);
         return 0;
   }

   masked_write(&pi->regs[reg], value, mask);

   return 0;
}

void pi_end_of_dma_event(struct pi_controller* pi)
{
   pi->regs[PI_STATUS_REG] &= ~3;
   raise_rcp_interrupt(pi->r4300, MI_INTR_PI);
}
