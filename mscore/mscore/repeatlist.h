//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __REPEATLIST_H__
#define __REPEATLIST_H__

class Score;

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

class RepeatSegment {
   public:
      int tick;         // start tick
      int len;
      int utick;
      double utime;
      double timeOffset;

      RepeatSegment();
      };

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

class RepeatList: public QList<RepeatSegment*>
      {
      Score* _score;
      mutable unsigned idx1, idx2;   // cached values

   public:
      RepeatList(Score* s);
      void unwind();
      int utick2tick(int tick) const;
      int tick2utick(int tick) const;
      void dump() const;
      int utime2utick(double) const;
      double utick2utime(int) const;
      void update();
      };

#endif

