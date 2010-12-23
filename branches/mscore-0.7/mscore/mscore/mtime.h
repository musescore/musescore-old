//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: mtime.h,v 1.2 2006/03/02 17:08:36 wschweer Exp $
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

#ifndef __MTIME_H__
#define __MTIME_H__

extern int division;    // midi ticks per quarter note

//---------------------------------------------------------
//   MTime
//    "Musical Time"
//    Type 0:  expressed as fraction nominator/denominator
//    Type 1:  midi ticks with resolution of 'division'
//             ticks per quarter note
//---------------------------------------------------------

class MTime {
      int _nominator;
      int _denominator;
      int _tick;
      bool tickTime;

   public:
      MTime();
      MTime(int z, int n);
      MTime(int t);
      int tick() const     { return _tick; }
      bool isValid() const { return _tick != -1; }
      void setTick(int t);
      void setFrac(int z, int n);
      };

#endif
