//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: mtime.cpp,v 1.2 2006/03/02 17:08:36 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "mtime.h"

int division = 480;     // 480 midi ticks represent a quarter note

//---------------------------------------------------------
//   MTime
//---------------------------------------------------------

MTime::MTime()
      {
      _tick        = -1;      // set invalid
      _nominator   = 0;
      _denominator = 4;
      tickTime     = false;
      }

MTime::MTime(int z, int n)
      {
      setFrac(z, n);
      }

MTime::MTime(int t)
      {
      setTick(t);
      }

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void MTime::setTick(int t)
      {
      _tick    = t;
      tickTime = true;
      }

//---------------------------------------------------------
//   setFrac
//---------------------------------------------------------

void MTime::setFrac(int z, int n)
      {
      _nominator = z;
      _denominator = n;
      switch(_denominator) {
            case 1:   _tick = division*4;   break;
            case 2:   _tick = division*2;   break;
            case 4:   _tick = division;     break;
            case 8:   _tick = division/2;   break;
            case 16:  _tick = division/4;   break;
            case 32:  _tick = division/8;   break;
            case 64:  _tick = division/16;  break;
            case 128: _tick = division/32;  break;
            case 256: _tick = division/64;  break;
            case 512: _tick = division/128; break;
            }
      _tick *= _nominator;
      tickTime = false;
      }

