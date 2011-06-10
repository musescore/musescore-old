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

#ifndef __VELO_H__
#define __VELO_H__

/**
 \file
 Definition of classes VeloList.
*/

#include <QtCore/QMap>

//---------------------------------------------------------
///   VeloEvent
///   item in VeloList
//---------------------------------------------------------

enum VeloType { VELO_FIX, VELO_RAMP };

struct VeloEvent {
      VeloType type;
      char val;

      VeloEvent() {}
      VeloEvent(VeloType t, char v) : type(t), val(v) {}
      };

//---------------------------------------------------------
///  VeloList
///  List of note velocity changes
//---------------------------------------------------------

class VeloList : public QMap<int, VeloEvent> {
   public:
      VeloList() {}
      int velo(int tick) const;
      int nextVelo(int tick) const;
      void setVelo(int tick, VeloEvent velo);
      void setVelo(int tick, int velocity);
      };

#endif

