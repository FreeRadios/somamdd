/* SomaMdd - Copyright (C) 2005 - Andrea Marchesini <bakunin@autistici.org>
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

static void usage (void);

void
fatal (char *text, ...)
{
  va_list va;
  char s[1024];

  va_start (va, text);

  vsnprintf (s, sizeof (s), text, va);

  if (log_is_opened ())
    log_write (SMDD_ERROR, s);
  else
    fprintf (stderr, "%s\n", s);

  exit (1);
}

void
signal_off (void)
{
  signal (SIGPIPE, SIG_IGN);
  signal (SIGHUP, log_restart);
}

void
signal_on (void)
{
  signal (SIGPIPE, SIG_DFL);
  signal (SIGHUP, SIG_DFL);
}

int
main (int argc, char **argv)
{
  struct soma_mdd *mdd;
  struct cast_config *cast;
  char *config_file = NULL;
  int debug = 0;
  int i;
  soma_controller *controller;
  char *item;
  char *real;

  fprintf (stdout, "%s %s - %s\n\n", PACKAGE, VERSION, AUTHOR);

  for (i = 1; i < argc; i++)
    {
      if (!strcmp (argv[i], "-f") || !strcmp (argv[i], "--file"))
	{
	  if (!(config_file = argv[i + 1]))
	    {
	      usage ();
	      return 1;
	    }

	  i++;
	}

      else if (!strcmp (argv[i], "-d") || !strcmp (argv[i], "--debug"))
	{
	  if (!argv[i + 1])
	    {
	      usage ();
	      return 1;
	    }

	  debug = atoi (argv[i + 1]);
	  i++;
	}

      else if (!strcmp (argv[i], "-h") || !strcmp (argv[i], "--help"))
	{
	  usage ();
	  return 0;
	}

      else
	{
	  usage ();
	  return 1;
	}
    }

  if (!config_file)
    config_file = SMDD_CONFIG_FILE;

  if (!(mdd = config (config_file)))
    return 1;

  if (!mdd->cast)
    fatal ("Set a cast in your config file!");

  if (debug)
    mdd->debug = debug;

  log_init (mdd);

  if (mdd->background)
    if (fork ())
      return 0;

  log_write (SMDD_ERROR, "%s started", PACKAGE);
  log_write (SMDD_INFO, "Opening socket...");

  signal_off ();

  while (1)
    {
      log_write (SMDD_INFO, "Polling...");

      if (mdd->unixsocket)
	controller = soma_open_unix (mdd->unixpath, mdd->password, mdd->ssl);
      else
	controller =
	  soma_open_tcp (mdd->server, mdd->port, mdd->password, mdd->ssl);

      if (!controller)
	fatal ("Memory!");

      switch (soma_error (controller))
	{
	case SOMA_ERR_SSL_REQUEST:
	  fatal
	    ("Somad need a ssl connection. Set Ssl=true in your config file.");
	  break;

	case SOMA_ERR_NO_SSL_REQUEST:
	  fatal
	    ("Somad need a clear connection. Set Ssl=false in your config file.");
	  break;

	case SOMA_ERR_SSL:
	  fatal ("Ssl error!");
	  break;

	case SOMA_ERR_CONNECT:
	  fatal ("Connect error!");
	  break;

	case SOMA_ERR_HOST:
	  fatal ("Host unknown!");
	  break;

	case SOMA_ERR_PROTOCOL:
	  fatal ("Protocol error!");
	  break;

	case SOMA_ERR_PASSWORD:
	  fatal ("Password error!");
	  break;

	case SOMA_ERR_POSIX:
	  fatal ("Error: %s", strerror (errno));
	  break;

	case SOMA_ERR_OK:
	  break;

	default:
	  fatal ("Internal error.");
	  break;
	}

      item = soma_get_item (controller);

      if (!item)
	log_write (SMDD_WARN, "Failed to get item!");

      else
	{
	  log_write (SMDD_INFO, "Item: '%s'", item);

	  if (!(real = parser (item)))
	    fatal ("Memory!");

	  if (item)
	    free (item);

	  log_write (SMDD_INFO, "Parted to: '%s'", real);

	  cast = mdd->cast;
	  while (cast)
	    {
	      log_write (SMDD_INFO, "Updating the server '%s' with mount '%s'", cast->server, cast->mount);
	      if (!cast_update (cast, real))
		log_write (SMDD_INFO, "Server '%s' updated", cast->server);
	      cast = cast->next;
	    }

	  if (real)
	    free (real);
	}

      log_write (SMDD_INFO, "Sleeping %d seconds...", mdd->sleep);
      soma_free (controller);

      sleep (mdd->sleep);
    }

  log_quit ();

  signal_on ();

  return 0;
}

static void
usage (void)
{
  fprintf (stdout, "\t-f or --file <file>         Set your config file\n");
  fprintf (stdout, "\t-d or --debug <int>         Set your debug level:\n"
	   "\t                            - 0 only error\n"
	   "\t                            - 1 error and warning\n"
	   "\t                            - 2 all messages\n");

  fprintf (stdout, "\n");
}

/* EOF */
