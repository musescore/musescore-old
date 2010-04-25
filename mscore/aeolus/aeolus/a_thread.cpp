// ---------------------------------------------------------------------------------
//
//  Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// ---------------------------------------------------------------------------------


#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "clthreads.h"


unsigned long A_thread::_trace = 0;


A_thread::A_thread (const char *name) :  _inst (0)
{
    const char *p;

    strncpy (_name, name, 32);
    _name [31] = 0;
    for (p = name; *p && !isdigit (*p); p++);
    if (*p) _inst = atoi (p);
}


void A_thread::mprintf (int opid, const char *fmt, ...)
{
    Textmsg *M = new Textmsg (1024);

    va_list ap;
    va_start (ap, fmt);
    M->vprintf (fmt, ap);
    va_end (ap);
    send_event (opid, M);
}

