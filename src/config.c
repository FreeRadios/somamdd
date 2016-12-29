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

struct soma_mdd *
config (char *config_file)
{
  struct soma_mdd *conf;
  cfg_t *cfg = NULL;
  int i;
  struct cast_config *cast;

  cfg_opt_t cast_opts[] = {
    CFG_STR ("Type", NULL, CFGF_NONE),
    CFG_STR ("Server", NULL, CFGF_NONE),
    CFG_INT ("Port", 0, CFGF_NONE),
    CFG_STR ("Mount", NULL, CFGF_NONE),
    CFG_STR ("Password", NULL, CFGF_NONE),
    CFG_END ()
  };

  cfg_opt_t opts[] = {
    CFG_BOOL ("UnixSocket", cfg_false, CFGF_NONE),

    CFG_STR ("UnixPath", NULL, CFGF_NONE),
    CFG_STR ("Server", NULL, CFGF_NONE),
    CFG_INT ("Port", SOMA_PORT, CFGF_NONE),

    CFG_STR ("Password", NULL, CFGF_NONE),

    CFG_BOOL ("Ssl", cfg_false, CFGF_NONE),

    CFG_INT ("Sleep", 10, CFGF_NONE),

    CFG_STR ("LogFile", NULL, CFGF_NONE),
    CFG_BOOL ("Background", cfg_false, CFGF_NONE),
    CFG_INT ("Debug", 2, CFGF_NONE),

    CFG_SEC ("Cast", cast_opts, CFGF_MULTI),

    CFG_END ()
  };

  if (!(conf = (struct soma_mdd *) malloc (sizeof (struct soma_mdd))))
    fatal ("Memory!");
  memset (conf, 0, sizeof (struct soma_mdd));

  cfg = cfg_init (opts, CFGF_NOCASE);
  switch (cfg_parse (cfg, config_file))
    {
    case CFG_FILE_ERROR:
      fatal ("Error: open file: %s", config_file);
      break;
    case CFG_PARSE_ERROR:
      fatal ("Error: syntax error: %s", config_file);
      break;
    }

  conf->logfile = cfg_getstr (cfg, "LogFile");
  if (!conf->logfile || !*conf->logfile)
    conf->logfile = strdup (SMDD_LOG_FILE);
  else
    conf->logfile = strdup (conf->logfile);
  if (!conf->logfile)
    fatal ("Memory!");

  conf->background = cfg_getbool (cfg, "Background");

  conf->debug = cfg_getint (cfg, "Debug");

  conf->ssl = cfg_getbool (cfg, "Ssl");

  conf->unixsocket = cfg_getbool (cfg, "UnixSocket");

  conf->unixpath = cfg_getstr (cfg, "UnixPath");
  if (!conf->unixpath || !*conf->unixpath)
    conf->unixpath = strdup (SOMA_UNIX_SOCK);
  else
    conf->unixpath = strdup (conf->unixpath);
  if (!conf->unixpath)
    fatal ("Memory!");

  conf->server = cfg_getstr (cfg, "Server");
  if (!conf->server || !*conf->server)
    conf->server = strdup ("localhost");
  else
    conf->server = strdup (conf->server);
  if (!conf->server)
    fatal ("Memory!");

  conf->password = cfg_getstr (cfg, "Password");
  if (!conf->password || !*conf->password)
    fatal ("Set the password!");

  else if (!(conf->password = strdup (conf->password)))
    fatal ("Memory!");

  conf->port = cfg_getint (cfg, "Port");

  conf->sleep = cfg_getint (cfg, "Sleep");

  i = cfg_size (cfg, "Cast");
  for (i--; i >= 0; i--)
    {
      cfg_t *s_cfg;
      char *c;

      s_cfg = cfg_getnsec (cfg, "Cast", i);

      if (!
	  (cast =
	   (struct cast_config *) malloc (sizeof (struct cast_config))))
	fatal ("Memory!");
      memset (cast, 0, sizeof (struct cast_config));

      c = cfg_getstr (s_cfg, "Type");
      if (!c || !*c)
	fatal ("Set a Type in your cast configuration!");

      if (!strcasecmp (c, "icecast"))
	cast->type = SMDD_ICECAST;

      else if (!strcasecmp (c, "icecast2"))
	cast->type = SMDD_ICECAST2;

      else if (!strcasecmp (c, "shoutcast"))
	cast->type = SMDD_SHOUTCAST;

      else
	fatal ("Unknown type '%s' in your config file!", c);

      cast->server = cfg_getstr (s_cfg, "Server");
      if (!cast->server || !*cast->server)
	fatal ("Set the Server in your cast configuration!");
      else if (!(cast->server = strdup (cast->server)))
	fatal ("Memory!");

      if (!(cast->port = cfg_getint (s_cfg, "Port")))
	fatal ("Set the Port in your cast configuration!");

      cast->mount = cfg_getstr (s_cfg, "Mount");
      if (!cast->mount || !*cast->mount)
	fatal ("Set the Mount in your cast configuration!");
      else if (!(cast->mount = strdup (cast->mount)))
	fatal ("Memory!");

      while(*cast->mount=='/') cast->mount++;

      cast->password = cfg_getstr (s_cfg, "Password");
      if (!cast->password || !*cast->password)
	fatal ("Set the Password in your cast configuration!");
      else if (!(cast->password = strdup (cast->password)))
	fatal ("Memory!");

      cast->next = conf->cast;
      conf->cast = cast;
    }

  cfg_free (cfg);

  return conf;
}

/* EOF */
