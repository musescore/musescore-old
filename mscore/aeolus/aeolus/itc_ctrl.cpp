// ---------------------------------------------------------------------------------
//
//  Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>
//  Copyright (C) 2008 Hans Fugal <hans@fugal.net> (OSX version)
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


#ifdef __APPLE__
#include <sys/time.h>
#endif
#include "clthreads.h"


ITC_ctrl::ITC_ctrl (void) : _mptr (0)
{
    for (int i = 0; i < N_OP; i++)
    {
	_dest [i] = 0;
	_ipid [i] = 0;
    }
    for (int i = 0; i < N_EC; i++) _ecnt [i] = 0;
    _time.tv_sec  = 0;
    _time.tv_nsec = 0;
}


ITC_ctrl::~ITC_ctrl (void)
{
}


void ITC_ctrl::set_time (const timespec *v)
{
    if (v)
    {
	_time.tv_sec  = v->tv_sec;
	_time.tv_nsec = v->tv_nsec;
    }
    else
    {
#ifdef __APPLE__
        struct timeval t;
        gettimeofday (&t, 0);
	_time.tv_sec  = t.tv_sec;
	_time.tv_nsec = t.tv_usec * 1000;
#else
	timespec t;
	clock_gettime (CLOCK_REALTIME, &t);
	_time.tv_sec  = t.tv_sec;
	_time.tv_nsec = t.tv_nsec;
#endif
    }
}


void ITC_ctrl::inc_time (unsigned long micros)
{
    unsigned long s, m;

    s = micros / 1000000;
    m = micros % 1000000;
    _time.tv_nsec += 1000 * m;
    if (_time.tv_nsec >= 1000000000)
    {
	_time.tv_nsec -= 1000000000;
	s += 1;
    }
    _time.tv_sec += s;
}


unsigned long ITC_ctrl::delay (void)
{
#ifdef __APPLE__
    struct timeval t;

    gettimeofday (&t, 0);
    return  t.tv_usec - _time.tv_nsec / 1000 + 1000000 * (t.tv_sec - _time.tv_sec);
#else
    timespec t;

    clock_gettime (CLOCK_REALTIME, &t);
    return  (t.tv_nsec - _time.tv_nsec) / 1000 + 1000000 * (t.tv_sec - _time.tv_sec);
#endif
}


int ITC_ctrl::get_event (unsigned int emask)
{
    int e;

    lock ();
    _mptr = 0;
    e = find_event (emask);
    if (e < 0) e = eget (emask);
    if      (e >= N_MQ) _ecnt [e - N_MQ] -= 1;
    else if (e >= 0)    _mptr = _list [e].get ();
    unlock ();
    return e;
}


int ITC_ctrl::get_event_timed (unsigned int emask)
{
    int e;

    lock ();
    _mptr = 0;
    e = find_event (emask);
    if (e < 0) e = eget (emask, &_time);
    if      (e >= N_MQ) _ecnt [e - N_MQ] -= 1;
    else if (e >= 0)    _mptr = _list [e].get ();
    unlock ();
    return e;
}


int ITC_ctrl::get_event_nowait (unsigned int emask)
{
    int e;

    if (trylock ()) return EV_TIME;
    _mptr = 0;
    e = find_event (emask);
    if      (e >= N_MQ) _ecnt [e - N_MQ] -= 1;
    else if (e >= 0)    _mptr = _list [e].get ();
    unlock ();
    return e;
}


int ITC_ctrl::send_event (unsigned int opid, ITC_mesg *M)
      {
      if ((opid < N_OP) && _dest [opid])
            return _dest [opid]->put_event (_ipid [opid], M);
    else return Edest::NOT_CONN;
}


int ITC_ctrl::send_event (unsigned int opid, unsigned int incr)
{
    if ((opid < N_OP) && _dest [opid]) return _dest [opid]->put_event (_ipid [opid], incr);
    else return Edest::NOT_CONN;
}


void ITC_ctrl::connect (ITC_ctrl *srce, unsigned int opid,
                        Edest    *dest, unsigned int ipid)
{
    assert (srce);
    assert (opid < N_OP);
    assert (ipid < N_MQ + N_EC);

    srce->_dest [opid] = 0;
    srce->_ipid [opid] = ipid;
    srce->_dest [opid] = dest;
}

