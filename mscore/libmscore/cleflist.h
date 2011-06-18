//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#ifndef __CLEFLIST_H__
#define __CLEFLIST_H__

#include "mscore.h"
#include "clef.h"

class Score;

//---------------------------------------------------------
//   ClefList
//---------------------------------------------------------

class ClefList : public QMultiMap<int, ClefTypeList> {
   public:
      ClefList() {}
      ClefTypeList clef(int tick) const;
      void setClef(int tick, ClefTypeList);
      void read(QDomElement, Score*);
      };

typedef QMultiMap<int, ClefTypeList>::iterator iClefEvent;
typedef QMultiMap<int, ClefTypeList>::const_iterator ciClefEvent;

#endif

