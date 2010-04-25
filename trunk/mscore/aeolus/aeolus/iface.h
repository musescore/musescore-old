/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __IFACE_H
#define __IFACE_H


#include <clthreads.h>
#include "messages.h"

//---------------------------------------------------------
//   Iface
//---------------------------------------------------------

class Iface : public A_thread
      {
      virtual void thr_main (void) = 0;

   public:

      Iface (void) : A_thread ("Iface") {}
      virtual ~Iface (void) {}
      virtual void stop (void) = 0;
      void terminate (void) {  put_event (EV_EXIT, 1); }
      };


typedef Iface *iface_cr (int ac, char *av []);


#endif
