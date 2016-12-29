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

static char *base64 (char *str);
static char *html_encode (char *data);

int
cast_update (struct cast_config *cast, char *item)
{
  char *metadata;
  char buf[1024];
  int len = 0;
  struct sockaddr_in sock;
  struct hostent *hp;
  int fd;

  if (!(metadata = html_encode (item)))
    return 1;

  switch (cast->type)
    {
    case SMDD_ICECAST:
      len =
	snprintf (buf, sizeof (buf),
		  "GET /admin.cgi?mode=updinfo&pass=%s&mount=/%s&song=%s HTTP/1.0\r\n"
		  "User-Agent: %s/%s\r\n\r\n", cast->password, cast->mount,
		  metadata, PACKAGE, VERSION);

      break;

    case SMDD_ICECAST2:
      {
	char t[1024];

	snprintf (t, sizeof (t), "source:%s", cast->password);
	len =
	  snprintf (buf, sizeof (buf),
		    "GET /admin/metadata?mode=updinfo&mount=/%s&song=%s HTTP/1.0\r\n"
		    "User-Agent: %s/%s\r\n" "Authorization: Basic %s\r\n\r\n",
		    cast->mount, metadata, PACKAGE, VERSION, base64 (t));
      }

      break;

    case SMDD_SHOUTCAST:
      len =
	snprintf (buf, sizeof (buf),
		  "GET /admin.cgi?mode=updinfo&pass=%s&song=%s HTTP/1.0\r\n"
		  "User-Agent: %s/%s\r\n\r\n", cast->password, metadata,
		  PACKAGE, VERSION);
      break;
    }

  free (metadata);

  if (!len)
    return 1;

  if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    fatal ("Socket error!");

  if (!(hp = gethostbyname (cast->server)))
    {
      log_write (SMDD_ERROR, "Error: unknow host '%s'", cast->server);
      return 1;
    }

  memset ((void *) &sock, 0, sizeof (sock));
  sock.sin_family = AF_INET;
  sock.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
  sock.sin_port = htons (cast->port);

  if (connect (fd, (struct sockaddr *) &sock, sizeof (sock)) < 0)
    {
      log_write (SMDD_ERROR, "Connect error to '%s' port '%d'", cast->server,
		 cast->port);
      return 1;
    }

  write (fd, buf, len);
  close (fd);

  return 0;
}

/* Base64 decoding */
static char base64table[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
  'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
  't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', '+', '/'
};

static char *
base64 (char *str)
{
  const char *data = str;
  size_t len = strlen (data);
  char *output;
  char *result;
  unsigned chunk;

  if (!(output = (char *) malloc ((len * 4 / 3 + 4) * sizeof (char))))
    fatal ("Error of memory.");
  result = output;

  while (len > 0)
    {
      chunk = (len > 3) ? 3 : len;
      *output++ = base64table[(*data & 0xfc) >> 2];
      *output++ =
	base64table[((*data & 0x03) << 4) | ((*(data + 1) & 0xf0) >> 4)];
      switch (chunk)
	{
	case 3:
	  *output++ = base64table[((*(data + 1) & 0x0f) << 2) |
				  ((*(data + 2) & 0xc0) >> 6)];
	  *output++ = base64table[(*(data + 2)) & 0x3f];
	  break;
	case 2:
	  *output++ = base64table[((*(data + 1) & 0x0f) << 2)];
	  *output++ = '=';
	  break;
	case 1:
	  *output++ = '=';
	  *output++ = '=';
	  break;
	}
      data += chunk;
      len -= chunk;
    }
  *output = 0;

  return result;
}

static char urltable[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e',
  'f'
};

static char safechars[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static char *
html_encode (char *data)
{
  const char *p;
  char *q, *dest;
  int digit;
  size_t n;

  if (!data)
    return NULL;

  for (p = data, n = 0; *p; p++)
    {
      n++;
      if (!safechars[(unsigned char) (*p)])
	n += 2;
    }
  if (!(dest = malloc (n + 1)))
    fatal ("Memory!");

  for (p = data, q = dest; *p; p++, q++)
    {
      if (safechars[(unsigned char) (*p)])
	{
	  *q = *p;
	}
      else
	{
	  *q++ = '%';
	  digit = (*p >> 4) & 0xF;
	  *q++ = urltable[digit];
	  digit = *p & 0xf;
	  *q = urltable[digit];
	  n += 2;
	}
    }
  *q = '\0';

  return dest;
}

/* EOF */
