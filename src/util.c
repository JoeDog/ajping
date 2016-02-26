/**
 * Utility Functions
 *
 * Copyright (C) 2000-2014 by
 * Jeffrey Fulmer - <jeff@joedog.org>, et al. 
 * This file is distributed as part of Siege 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *--
 */
#include <setup.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <joedog/boolean.h>
#include <joedog/defs.h>

BOOLEAN
strmatch(char *option, char *param)
{
  if(!strncasecmp(option,param,strlen(param))&&strlen(option)==strlen(param))
    return TRUE;
  else
    return FALSE;
}

BOOLEAN
startswith(const char *pre, const char *str)
{
  size_t lenpre = strlen(pre);
  size_t lenstr = strlen(str);
  return lenstr < lenpre ? FALSE : strncmp(pre, str, lenpre) == 0;
}

BOOLEAN
endswith(const char *suffix, const char *str)
{
  if (!str || !suffix)
    return FALSE;
  size_t lenstr    = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix >  lenstr)
    return FALSE;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

char *
uppercase(char *s, size_t len){
  unsigned char *c, *e;

  c = (unsigned char*)s;
  e = c+len;

  while(c < e){
    *c = TOUPPER((unsigned char)(*c));
    c++;
  }
  return s;
}


char *
lowercase(char *s, size_t len){
  unsigned char *c, *e;

  c = (unsigned char*)s;
  e = c+len;

  while(c < e){
    *c = TOLOWER((unsigned char)(*c));
    c++;
  }
  return s;
}

char * 
stristr(const char* haystack, const char* needle) {
  do {
    const char* h = haystack;
    const char* n = needle;
    while (tolower((unsigned char) *h) == tolower((unsigned char ) *n) && *n) {
      h++;
      n++;
    }
    if (*n == 0) {
      return (char *) haystack;
    }
  } while (*haystack++);
  return NULL;
}
