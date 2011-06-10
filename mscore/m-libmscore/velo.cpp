//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: velo.cpp 3282 2010-07-14 07:16:14Z wschweer $
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

/**
 \file
 Implementation of class VeloList.
*/

#include "velo.h"

//---------------------------------------------------------
//   velo
//    return velocity at tick position
//---------------------------------------------------------

int VeloList::velo(int tick) const
      {
      if (empty())
            return 80;
      VeloList::const_iterator i = upperBound(tick);
      if (i == constBegin())
            return 80;
      VeloList::const_iterator ii = i - 1;
      if (ii.value().type == VELO_FIX)
            return ii.value().val;
      int tickDelta = i.key() - ii.key();
      int veloDelta = i.value().val - ii.value().val;
      return ii.value().val + ((tick-ii.key()) * veloDelta) / tickDelta;
      }

//---------------------------------------------------------
//   nextVelo
//    return next velocity event after tick position
//---------------------------------------------------------

int VeloList::nextVelo(int tick) const
      {
      if (empty())
            return 80;
      VeloList::const_iterator i = upperBound(tick);
      return i.value().val;
      }

//---------------------------------------------------------
//   setVelo
//---------------------------------------------------------

void VeloList::setVelo(int tick, VeloEvent ve)
      {
      insert(tick, ve);
      }

void VeloList::setVelo(int tick, int velo)
      {
      insert(tick, VeloEvent(VELO_FIX, velo));
      }

