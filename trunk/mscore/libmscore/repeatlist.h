//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2009-2011 Werner Schweer
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

#include <QtCore/QList>

class Score;

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

class RepeatSegment {
   public:
      int tick;         // start tick
      int len;
      int utick;
      qreal utime;
      qreal timeOffset;

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
      int utime2utick(qreal) const;
      qreal utick2utime(int) const;
      void update();
      };

#endif

