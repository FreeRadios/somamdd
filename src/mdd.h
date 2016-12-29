/* SomaMmd - Copyright (C) 2005 bakunin - Andrea Marchesini 
 *                                        <bakunin@autistici.org>
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

#ifndef __MDD_H__
#define __MDD_H__

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#include <soma/code.h>
#include <soma/commons.h>
#include <soma/controller.h>
#include "../confuse/confuse.h"

#define SMDD_CONFIG_FILE	"/etc/somamdd.cfg"
#define SMDD_LOG_FILE		"/var/log/somamdd.log"
#define AUTHOR			"Andrea Marchesini <bakunin@autistici.org>"

#define SMDD_ERROR		0
#define SMDD_WARN		1
#define SMDD_INFO		2

#define SMDD_ICECAST		0
#define SMDD_ICECAST2		1
#define SMDD_SHOUTCAST		2

struct cast_config {
  int type;

  char *server;
  int port;

  char *password;
  char *mount;

  struct cast_config *next;
};

struct soma_mdd {
  int unixsocket;

  char *unixpath;
  char *server;
  int port;

  char *password;

  int ssl;

  int sleep;

  int background;
  char *logfile;
  int debug;

  struct cast_config *cast;
} soma_mdd;

/* main.c */
void fatal(char *, ...);
void signal_on (void);
void signal_off (void);

/* config.c */
struct soma_mdd *config(char *);

/* log.c */
void log_init(struct soma_mdd *);
void log_quit(void);
void log_write(int, char *,...);
int log_is_opened(void);
void log_restart(int);

/* parser.c */
char *parser(char *);

/* cast.c */
int cast_update(struct cast_config *, char *);

#endif
