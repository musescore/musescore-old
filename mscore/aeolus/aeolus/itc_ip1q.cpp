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


#include "clthreads.h"


int ITC_ip1q::get_event (unsigned int emask)
{
    int e;

    lock ();
    _mptr = 0;
    e = find_event (emask);
    if (e < 0) e = eget (emask);
    if (e >= N_MQ) _bits &= ~(1 << e);
    else if (e == 0) _mptr = _list.get ();
    unlock ();

    return e;
}


int ITC_ip1q::get_event_nowait (unsigned int emask)
{
    int e; 

    if (trylock ()) return EV_TIME;
    _mptr = 0;
    e = find_event (emask);
    if (e >= N_MQ) _bits &= ~(1 << e);
    else if (e == 0) _mptr = _list.get ();
    unlock ();

    return e;
}


