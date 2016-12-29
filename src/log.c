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

static FILE *log_fd = NULL;
static int background = 0;
static int debug = 0;
static char *log_file = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static void puts_time (FILE *);

int
log_is_opened (void)
{
  if (log_fd)
    return 1;
  return 0;
}

void
log_restart (int signo)
{

  pthread_mutex_lock (&log_mutex);

  if (log_fd)
    fclose (log_fd);

  if (!(log_fd = fopen (log_file, "ab")))
    {
      fprintf (stderr, "Open %s log file error.\n", log_file);
      exit (1);
    }

  pthread_mutex_unlock (&log_mutex);
}

void
log_init (struct soma_mdd *conf)
{
  if (!conf->logfile)
    {
      fprintf (stderr,
	       "Error: you must set your log file. Check your config file.\n");
      exit (1);
    }

  pthread_mutex_lock (&log_mutex);

  if (!(log_fd = fopen (conf->logfile, "ab")))
    {
      fprintf (stderr, "Open %s log file error.\n", conf->logfile);
      exit (1);
    }

  pthread_mutex_unlock (&log_mutex);

  log_file = conf->logfile;
  background = conf->background;
  debug = conf->debug;
}

void
log_quit (void)
{
  pthread_mutex_lock (&log_mutex);

  if (log_fd)
    fclose (log_fd);

  log_fd = 0;

  pthread_mutex_unlock (&log_mutex);
}

static void
puts_time (FILE * fd)
{
  time_t j;
  struct tm *k;

  static char *day[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

  j = time (NULL);
  k = localtime (&j);

  fprintf (fd, "[%.2d %s %.2d:%.2d:%.2d] ", k->tm_mday, day[k->tm_wday],
	   k->tm_hour, k->tm_min, k->tm_sec);
}

void
log_write (int d, char *str, ...)
{
  va_list va;
  static char prev[SOMA_MAX_BUFF];
  char s[SOMA_MAX_BUFF];

  if (d > debug)
    return;

  va_start (va, str);
  vsnprintf (s, SOMA_MAX_BUFF, str, va);

  if (strcmp (s, prev))
    {
      if (!background)
	{
	  puts_time (stderr);
	  fprintf (stderr, "%s\n", s);
	}

      pthread_mutex_lock (&log_mutex);

      if (log_fd)
	{
	  puts_time (log_fd);
	  fprintf (log_fd, "%s\n", s);
	  fflush (log_fd);
	  fsync (fileno (log_fd));
	}

      pthread_mutex_unlock (&log_mutex);

      strcpy (prev, s);
    }

  va_end (va);
}

/* EOF */
