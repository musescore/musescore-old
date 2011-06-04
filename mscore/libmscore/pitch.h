//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#ifndef __PITCH_H__
#define __PITCH_H__

#include <QtCore/QMap>

//---------------------------------------------------------
///  PitchList
///  List of note pitch offsets
//---------------------------------------------------------

class PitchList : public QMap<int, int> {
   public:
      PitchList() {}
      int pitchOffset(int tick) const;
      void setPitchOffset(int tick, int offset) { insert(tick, offset); }
      };

#endif

