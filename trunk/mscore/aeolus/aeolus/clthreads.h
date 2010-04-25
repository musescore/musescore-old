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


#ifndef __CLTHREADS_H
#define __CLTHREADS_H


#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#ifdef __linux__
#include <semaphore.h>
#endif


// -------------------------------------------------------------------------------------------


#ifdef __linux__
class P_sema
{
public:

    P_sema (void) { if (sem_init (&_sema, 0, 0)) abort (); }
    ~P_sema (void) { sem_destroy (&_sema); }
    P_sema (const P_sema&);
    P_sema& operator= (const P_sema&);

    void post (void) { if (sem_post (&_sema)) abort (); }
    void wait (void) { if (sem_wait (&_sema)) abort (); }
    int trywait  (void) { return sem_trywait (&_sema); }
    int getvalue (void) { int n; sem_getvalue (&_sema, &n); return n; }

private:

    sem_t  _sema;
};
#endif


// -------------------------------------------------------------------------------------------


class Bmutex
{
public:

    Bmutex (void) { if (pthread_mutex_init (&_mutex, 0)) abort (); }
    ~Bmutex (void) { pthread_mutex_destroy (&_mutex); }
    Bmutex (const Bmutex&);
    Bmutex& operator= (const Bmutex&);

    void lock (void) { if (pthread_mutex_lock (&_mutex)) abort (); }
    void unlock (void){ if (pthread_mutex_unlock (&_mutex)) abort (); }
    int trylock (void) { return pthread_mutex_trylock (&_mutex); }

private:

    friend class Esync;

    pthread_mutex_t  _mutex;
};


// -------------------------------------------------------------------------------------------


class Cmutex
{
public:

    Cmutex () : _owner (0), _count (0) { if (pthread_mutex_init (&_mutex, 0)) abort (); }
    ~Cmutex () { pthread_mutex_destroy (&_mutex); } 
    Cmutex (const Cmutex&);
    Cmutex& operator= (const Cmutex&);

    void lock (void);
    void unlock (void);

private:

    pthread_mutex_t   _mutex;
    pthread_t         _owner;
    int               _count;
};


inline void Cmutex::lock (void)
{
    if (_owner == pthread_self ()) ++_count;
    else
    {
	pthread_mutex_lock (&_mutex);
	_owner = pthread_self ();
	_count = 1;
    }
}

inline void Cmutex::unlock (void)
{
    if (_owner == pthread_self ())
    {
	if (--_count == 0)
	{
	    _owner = 0;
	    pthread_mutex_unlock (&_mutex);
	}
    }
}


// -------------------------------------------------------------------------------------------


class Esync : public Bmutex
{
public:

    enum
    {
        EM_ALL   = ~0,
        EV_TIME  = -1,
        EV_ERROR = -2
    };

    Esync (void) : _event (EV_ERROR), _emask (0) { if (pthread_cond_init (&_cond, 0)) abort (); }
    ~Esync (void) { pthread_cond_destroy (&_cond); }
    Esync (const Esync&);
    Esync& operator= (const Esync&);

    void eput (int e);
    int  eget (unsigned int m = EM_ALL, const timespec *t = 0);

private:
  
    volatile int     _event;
    unsigned int     _emask;
    pthread_cond_t   _cond;
};


inline void Esync::eput (int e)
{
    if ((1 << e) & _emask)
    {
	_event = e;
	if (pthread_cond_signal (&_cond)) abort ();
    }
}


inline int Esync::eget (unsigned int m, const timespec *t)
{
    int r; 
   
    _event = EV_ERROR;
    _emask = m;

    do
    {   
	if (t) r = pthread_cond_timedwait (&_cond, &_mutex, t);
	else   r = pthread_cond_wait (&_cond, &_mutex);
	if (_event >= 0) break;
	if (r == ETIMEDOUT)
	{
	    _event = EV_TIME;
	    break; 
	}
    }
    while (r == EINTR);

    _emask = 0; 
    return _event;
} 


// -------------------------------------------------------------------------------------------


class ITC_mesg
{
public:

    enum 
    {
        ITM_CLLIB_BASE = 0x80000000,
    };

    ITC_mesg (unsigned long type = 0) : _forw (0), _back (0), _type (type) { _counter++; }
    virtual ~ITC_mesg (void) { _counter--; }

    virtual void recover (void) { delete this; }
    ITC_mesg *forw (void) const { return _forw; }
    ITC_mesg *back (void) const { return _back; }
    unsigned long  type (void) const { return _type; }

    static size_t object_counter (void) { return _counter; }

private:
  
    friend class ITC_list;

    ITC_mesg      *_forw;
    ITC_mesg      *_back;
    unsigned long  _type;

    static size_t _counter;
};


// -------------------------------------------------------------------------------------------


class ITC_list
{
public:

    ITC_list (void) : _head (0), _tail (0), _count (0) {}
    ~ITC_list (void) { flush (); }
    ITC_list (const ITC_list&);
    ITC_list& operator= (const ITC_list&);

    void put (ITC_mesg *p);
    ITC_mesg *get (void);
    void rem (ITC_mesg *p);
    void flush (void);
    ITC_mesg *head (void) const { return _head; }
    ITC_mesg *tail (void) const { return _tail; }

    int count (void) const { return _count; }

private:

    ITC_mesg   *_head;
    ITC_mesg   *_tail;
    int         _count;
};


inline void ITC_list::put (ITC_mesg *p)
{
    if (p)
    {
	p->_forw = 0;
	p->_back = _tail;
	if (_tail) _tail->_forw = p;
	else              _head = p;
	_tail = p;
	++_count;
    }
}


inline ITC_mesg *ITC_list::get (void)
{
    ITC_mesg *p = _head;

    if (p)
    {
	_head = p->_forw;
	if (_head) _head->_back = 0;
	else              _tail = 0;
	p->_forw = p->_back = 0;
	--_count; 
    }
    return p;
} 


inline void ITC_list::rem (ITC_mesg *p)
{
    if (p == _head) _head = p->_forw;
    else  p->_back->_forw = p->_forw;
    if (p == _tail) _tail = p->_back;
    else  p->_forw->_back = p->_back;
    p->_forw = p->_back = 0;
    --_count; 
}


inline void ITC_list::flush (void)
{
    ITC_mesg *p = _head;
    while (p)
    {
	_head = p->_forw;
	p->recover ();
	p = _head;
    }
    _tail  = 0;
    _count = 0;
} 


// -------------------------------------------------------------------------------------------


class Edest
{
public:

    enum
    {
        NO_ERROR  = 0,
        NOT_CONN  = 1,
        DST_LOCK  = 2,
        BAD_PORT  = 3
    };

    Edest (void) {}
    virtual ~Edest (void) {}
    Edest (const Edest&);
    Edest& operator= (const Edest&);

    virtual int  put_event (unsigned int evid, ITC_mesg *M = 0) = 0;
    virtual int  put_event (unsigned int evid, unsigned int incr = 1) = 0;
    virtual int  put_event_try (unsigned int evid, unsigned int incr = 1) = 0;
    virtual void ipflush (unsigned evid) = 0;

private:
};


// -------------------------------------------------------------------------------------------


class ITC_ip1q : public Edest, protected Esync
{
public:

    enum {  N_BE = 31,  N_MQ =  1,  EM_ALL = ~0 };

    ITC_ip1q (void) : _bits (0), _mptr (0) {};
    ~ITC_ip1q (void) {};
    ITC_ip1q (const ITC_ip1q&);
    ITC_ip1q& operator= (const ITC_ip1q&);

    virtual int put_event (unsigned int evid, unsigned int incr);
    virtual int put_event (unsigned int evid, ITC_mesg *M);
    virtual int put_event_try (unsigned int evid, unsigned int incr);

    void  ipflush (unsigned int evid);
    int   get_event (unsigned int emask = EM_ALL);
    int   get_event_nowait (unsigned int emask = EM_ALL);

    ITC_mesg *get_message (void) const { return _mptr; }

private:
  
    int find_event (unsigned int mask) const;

    unsigned int  _bits;
    ITC_list      _list;
    ITC_mesg     *_mptr;
};


inline int ITC_ip1q::put_event (unsigned int evid, unsigned int incr)
{
    int r = NO_ERROR;
    assert (incr);
    lock ();
    if ((evid >= N_MQ) && (evid < N_MQ + N_BE))
    {
	_bits |= 1 << evid;
	eput (evid);
    }
    else  r = BAD_PORT;
    unlock ();
    return r;
}


inline int ITC_ip1q::put_event (unsigned int evid, ITC_mesg *M)
{
    int r = NO_ERROR;
    assert (M);
    lock ();
    if (evid < N_MQ)
    {
        _list.put (M);
        eput (evid);
    }
    else r = BAD_PORT;
    unlock ();
    return r;
}


inline int ITC_ip1q::put_event_try (unsigned int evid, unsigned int incr)
{
    int r = NO_ERROR;
    assert (incr);
    if (trylock ()) return DST_LOCK;
    if ((evid >= N_MQ) && (evid < N_MQ + N_BE))
    {
        _bits |= 1 << evid;
        eput (evid);
    }
    else  r = BAD_PORT;
    unlock ();
    return r;
}

inline void ITC_ip1q::ipflush (unsigned int evid)
{
    lock ();
    if (evid) _bits &= ~(1 << evid);
    else      _list.flush ();
    unlock ();
}


inline int ITC_ip1q::find_event (unsigned int mask) const
{
    int          i;
    unsigned int b;

    for (b = mask & _bits, i = 31; b; b <<= 1, i--)
    {
	if (b & 0x80000000) return i;
    }
    if ((mask & 1) && _list.head ()) return 0;

    return EV_TIME;
}


// -------------------------------------------------------------------------------------------


class ITC_ctrl : public Edest, protected Esync
{
public:

    enum
    {
	N_EC = 16,
	N_MQ = 16,
	EM_EC = (int) 0xFFFF0000,
	EM_MQ = (int) 0x0000FFFF,
	EM_ALL = EM_EC | EM_MQ,
	N_OP = 32
    };

    ITC_ctrl (void);
    ~ITC_ctrl (void);
    ITC_ctrl (const ITC_ctrl&);
    ITC_ctrl& operator= (const ITC_ctrl&);

    int send_event (unsigned int opid, ITC_mesg *M);
    int send_event (unsigned int opid, unsigned int incr);

    int get_event (unsigned int emask = EM_ALL);
    int get_event_timed (unsigned int emask = EM_ALL);
    int get_event_nowait (unsigned int emask = EM_ALL);
 
    void set_time (const timespec *t = 0);
    void get_time (timespec *t) { t->tv_sec = _time.tv_sec; t->tv_nsec = _time.tv_nsec; }
    void inc_time (unsigned long micros);
    unsigned long delay (void);

    virtual int  put_event (unsigned int evid, ITC_mesg *M);
    virtual int  put_event (unsigned int evid, unsigned int incr = 1);
    virtual int  put_event_try (unsigned int evid, unsigned int incr = 1);
    virtual void ipflush (unsigned int evid);

    void conn (unsigned int opid, Edest *dest, unsigned int ipid);

    ITC_mesg *get_message (void) const { return _mptr; }

    static void connect (ITC_ctrl *srce, unsigned int opid,
			 Edest    *dest, unsigned int ipid);

private:
  
    int find_event (unsigned int emask) const;

    ITC_list        _list [N_MQ];
    unsigned int    _ecnt [N_EC];
    ITC_mesg       *_mptr; 
    timespec        _time;
    Edest          *_dest [N_OP];
    int             _ipid [N_OP];

};


inline int ITC_ctrl::put_event_try (unsigned int evid, unsigned int incr)
{
    int r = NO_ERROR;
    assert (incr);
    if (trylock ()) return DST_LOCK;
    if ((evid >= N_MQ) && (evid < N_MQ + N_EC))
    {
        _ecnt [evid - N_MQ] += incr;
        eput (evid);
    }
    else  r = BAD_PORT;
    unlock ();
    return r;
}


inline int ITC_ctrl::put_event (unsigned int evid, unsigned int incr)
{
    int r = NO_ERROR;
    assert (incr);
    lock ();
    if ((evid >= N_MQ) && (evid < N_MQ + N_EC))
    {
        _ecnt [evid - N_MQ] += incr;
        eput (evid);
    }
    else  r = BAD_PORT;
    unlock ();
    return r;
}


inline int ITC_ctrl::put_event (unsigned int evid, ITC_mesg *M)
{
    int r = NO_ERROR;
    assert (M);
    lock ();
    if (evid < N_MQ)
    {
        _list [evid].put (M);
        eput (evid);
    }
    else r = BAD_PORT;
    unlock ();
    return r;
}


inline void ITC_ctrl::ipflush (unsigned int evid)
{
    lock ();
    if (evid < N_MQ) _list [evid].flush ();
    else if (evid < N_MQ + N_EC) _ecnt [evid - N_MQ] = 0;
    unlock ();
}


inline int ITC_ctrl::find_event (unsigned int mask) const
{
    int          i;
    unsigned int b;

    for (b = mask & EM_EC, i = N_EC - 1; b; b <<= 1, i--)
    {
	if ((b & 0x80000000) && _ecnt [i]) return i + N_MQ;  
    }
    mask <<= N_EC;
    for (b = mask & (EM_MQ << N_EC), i = N_MQ - 1; b; b <<= 1, i--)
    {
	if ((b & 0x80000000) && _list [i].head ()) return i;  
    }

    return EV_TIME;
}


// -------------------------------------------------------------------------------------------


class P_thread
{
public:

    P_thread (void);
    virtual ~P_thread (void);
    P_thread (const P_thread&);
    P_thread& operator=(const P_thread&);

    void sepuku (void) { pthread_cancel (_ident); }

    virtual void thr_main (void) = 0;
    virtual int  thr_start (int policy, int priority, size_t stacksize = 0);

private:
  
    pthread_t  _ident;
};


// -------------------------------------------------------------------------------------------


class H_thread : public P_thread, public ITC_ip1q
{
public:

    H_thread (Edest *dest, int ipid) : _dest (dest), _ipid (ipid) {}
    virtual ~H_thread (void) {};
    H_thread (const H_thread&);
    H_thread& operator=(const H_thread&);

    void reply (ITC_mesg *M) { _dest->put_event (_ipid, M); }
    void reply (void) { _dest->put_event (_ipid, 1); }

private:
  
    Edest *_dest;
    int    _ipid;
};


// -------------------------------------------------------------------------------------------


class A_thread : public P_thread, public ITC_ctrl
{
public:

    A_thread (const char *name);
    virtual ~A_thread (void) {};
    A_thread (const A_thread&);
    A_thread& operator=(const A_thread&);

    void mprintf (int opid, const char *fmt, ...);
    int inst (void) { return _inst; }
    const char *name (void) { return _name; }

    static unsigned long _trace;

private:

    char    _name [32];
    int     _inst;
};


// -------------------------------------------------------------------------------------------


class Textmsg : public ITC_mesg
{
public:

    enum
    {
        ITM_CLLIB_TEXT = ITM_CLLIB_BASE + 1
    };

    Textmsg (size_t size);
    ~Textmsg (void) { delete _text; _counter--; }
    Textmsg (const Textmsg&);
    Textmsg& operator= (const Textmsg&);

    char *text (void) const { return _text; }
    size_t size (void) const { return _size; }
    size_t strlen (void) const { return _strlen; }
    int count (void) const { return _count; }

    virtual void recover (void) { delete this; }

    int set_count (int k) { return _count = k; }  
    int inc_count (void) { return ++_count; }  
    int dec_count (void) { return --_count; }  
    void reset (void) { _strlen = 0; _count = 0; _lp = 0; _lc = 0; }

    void vprintf (const char *fmt, va_list ap);
    void printf (const char *fmt, ...);

    const char *getword (void);
    const char *gettail (void);
    void restore (void);

    static size_t object_counter (void) { return _counter; }

private:

    char    *_text;
    size_t   _size;
    size_t   _strlen;
    int      _count;
    char    *_lp;
    char     _lc;

    static size_t _counter;
};


// -------------------------------------------------------------------------------------------


#endif
