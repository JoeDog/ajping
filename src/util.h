/**
 * Utility Functions
 *
 * Copyright (C) 2001-2014 
 * by Jeffrey Fulmer <jeff@joedog.org>, et al.
 * This file is part of Siege
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
 *
 */
#ifndef  UTIL_H
#define  UTIL_H

#include <joedog/boolean.h>

BOOLEAN strmatch(char *option, char *param);
BOOLEAN startswith(const char *pre, const char *str);
BOOLEAN endswith(const char *suffix, const char *str);
char *  uppercase(char *s, size_t len);
char *  lowercase(char *s, size_t len);
char *  stristr(const char* haystack, const char* needle);

#endif /*UTIL_H*/

