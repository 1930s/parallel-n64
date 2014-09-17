/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "resampler.h"
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../../general.h"

static const rarch_resampler_t *resampler_drivers[] = {
   &sinc_resampler,
#ifdef HAVE_CC_RESAMPLER
   &CC_resampler,
#endif
   &nearest_resampler,
   NULL,
};

static int find_resampler_driver_index(const char *driver)
{
   unsigned i;
   for (i = 0; resampler_drivers[i]; i++)
      if (strcasecmp(driver, resampler_drivers[i]->ident) == 0)
         return i;
   return -1;
}

/* Resampler is used by multiple modules so avoid 
 * clobbering g_extern.audio_data.resampler here. */

static const rarch_resampler_t *find_resampler_driver(const char *ident)
{
   int i = find_resampler_driver_index(ident);
   if (i >= 0)
      return resampler_drivers[i];
   else
   {
      unsigned d;
      RARCH_ERR("Couldn't find any resampler driver named \"%s\"\n", ident);
      RARCH_LOG_OUTPUT("Available resampler drivers are:\n");
      for (d = 0; resampler_drivers[d]; d++)
         RARCH_LOG_OUTPUT("\t%s\n", resampler_drivers[d]->ident);

      RARCH_WARN("Going to default to first resampler driver ...\n");

      return resampler_drivers[0];
   }
}

#ifndef RESAMPLER_TEST
void find_prev_resampler_driver(void)
{
   int i = find_resampler_driver_index(g_settings.audio.resampler);
   if (i > 0)
      strlcpy(g_settings.audio.resampler, resampler_drivers[i - 1]->ident,
            sizeof(g_settings.audio.resampler));
   else
      RARCH_WARN("Couldn't find any previous resampler driver (current one: \"%s\").\n",
            g_extern.audio_data.resampler->ident);
}

void find_next_resampler_driver(void)
{
   int i = find_resampler_driver_index(g_settings.audio.resampler);
   if (i >= 0 && resampler_drivers[i + 1])
      strlcpy(g_settings.audio.resampler, resampler_drivers[i + 1]->ident,
            sizeof(g_settings.audio.resampler));
   else
      RARCH_WARN("Couldn't find any next resampler driver (current one: \"%s\").\n",
            g_extern.audio_data.resampler->ident);
}
#endif

bool rarch_resampler_realloc(void **re, const rarch_resampler_t **backend,
      const char *ident, double bw_ratio)
{
   if (*re && *backend)
      (*backend)->free(*re);

   *re      = NULL;
   /* TODO - make optional */
#if 1
   *backend = find_resampler_driver("nearest");
#else
   *backend = find_resampler_driver(ident);
#endif

   if (!*backend)
      return false;

   *re = (*backend)->init(bw_ratio);

   if (!*re)
   {
      *backend = NULL;
      return false;
   }

   return true;
}
