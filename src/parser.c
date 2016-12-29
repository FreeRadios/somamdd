/* SomaMmd - Copyright (C) 2005 - Andrea Marchesini <bakunin@autistici.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
# error Use configure; make; make install
#endif

#include "mdd.h"

char *
parser (char *item)
{
  int start, stop, len;
  char buf[1024];

  if (!strncmp (item, "http://", 7) ||
      !strncmp (item, "https://", 8) ||
      !strncmp (item, "ftp://", 6) || 
      !strncmp (item, "ftps://", 7))
    return strdup (item);

  len = strlen (item);

  /* /a/b/c/song.mp3
   *       ^........ */
  for (start = len - 1; start; start--)
    if (*(item + start) == '/')
      break;

  /* /a/b/c/song.mp3
   *        ^ */
  if (start)
    start++;

  /* /a/b/c/song.mp3
   *            ^... */
  for (stop = len-1; stop >=start; stop--)
    if (*(item + stop) == '.')
      {
	*(item + stop) = 0;
	break;
      }

  /* title_song -> title song */
  for (len = 0; start < stop && len<sizeof(buf); start++)
    {
      switch(*(item + start)) {
        case '.':
  	  buf[len++] = ' ';
	  break;

        case '_':
  	  buf[len++] = ' ';
	  break;

        default:
	  buf[len++] = *(item + start);
	  break;
      }
    }

  buf[len++] = 0;

  return strdup (buf);
}

/* EOF */
