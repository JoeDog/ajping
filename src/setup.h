/**
 * Package header
 *
 * Copyright (C) 2016 by
 * Jeffrey Fulmer - <jeff@joedog.org>, et al. 
 * This file is distributed as part of ajping
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
#ifndef __SETUP_H
#define __SETUP_H

#ifdef  HAVE_CONFIG_H
# include <config.h>
#endif/*HAVE_CONFIG_H*/

#include <joedog/defs.h>
#include <joedog/boolean.h>

/** 
 * configuration struct;
 * NOTE: this data is writeable ONLY during 
 * the configuration step before any threads
 * are spawned.
 */
struct CONFIG
{
  BOOLEAN verbose;      /* boolean, verbose output to screen       */
  BOOLEAN quiet;        /* boolean, turn off all output to screen  */
  BOOLEAN color;        /* boolean, true for color, false for not  */
  int     timeout;      /* socket connection timeout value, def:10 */
  int     reps;         /* reps to run the test, default infinite  */ 
  BOOLEAN ipv6;         /* true for IPv6 false for IPv4 default: 4 */
  BOOLEAN debug;        /* boolean, undocumented debug command     */
  BOOLEAN loop;         /* boolean for ping loop signal falsifies  */
};


#if INTERN
# define EXTERN /* */
#else
# define EXTERN extern
#endif
 
EXTERN struct CONFIG my;

#endif/*__SETUP_H*/
