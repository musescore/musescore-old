//=============================================================================
//  MuseScore
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

/**
 \file
 Implementation of class VeloList.
*/

#include "velo.h"

//---------------------------------------------------------
//   velo
//---------------------------------------------------------

int VeloList::velo(int tick) const
      {
      if (empty())
            return 80;
      ciVeloEvent i = upper_bound(tick);
      if (i == begin())
            return 80;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   setVelo
//---------------------------------------------------------

void VeloList::setVelo(int tick, int v)
      {
      std::pair<int, int> velo(tick, v);
      std::pair<iVeloEvent,bool> p = insert(velo);
      if (!p.second)
            (*this)[tick] = v;
      iVeloEvent i = p.first;
      for (++i; i != end();) {
            if (i->second != v)
                  break;
            iVeloEvent ii = i;
            ++ii;
            erase(i);
            i = ii;
            }
      }

