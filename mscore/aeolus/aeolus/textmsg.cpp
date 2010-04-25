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


#include <ctype.h>
#include <stdarg.h>
#include "clthreads.h"


size_t Textmsg::_counter = 0;


Textmsg::Textmsg (size_t size) :
    ITC_mesg (ITM_CLLIB_TEXT),
    _size (size),
    _strlen (0),
    _count (1),
    _lp (0),
    _lc (0)
{
    _text = new char [size];
    _counter++;
}
 

void Textmsg::vprintf (const char *fmt, va_list ap)
{
    _strlen += vsnprintf (_text + _strlen, _size - _strlen, fmt, ap);
    if (_strlen > _size - 1) _strlen = _size - 1;
}


void Textmsg::printf (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vprintf (fmt, ap);
    va_end (ap);
}


const char *Textmsg::getword (void)
{
    char *p, *q;

    p = _lp ? _lp : _text;
    if (_lc) p++;
    while (*p && isspace (*p)) p++;
    q = p;
    while (*p && ! isspace (*p)) p++;
    _lc = *p;
    _lp = p;
    *p = 0;  
    return *q ? q : 0;
}


const char *Textmsg::gettail (void)
{
    char *p;

    p = _lp ? _lp : _text;
    if (_lc) p++;
    return p;
}


void Textmsg::restore (void)
{
    char *p;
  
    if (_lp)
    {
	p = _lp;
	*p = _lc;
	while (p-- > _text) if (! *p) *p = ' ';
	_lp = 0;
	_lc = 0;
    }
}
