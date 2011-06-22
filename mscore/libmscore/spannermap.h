//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer et al.
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

#ifndef __SPANNERMAP_H__
#define __SPANNERMAP_H__

#include "elementmap.h"

//---------------------------------------------------------
//   SpannerMap
//---------------------------------------------------------

class SpannerMap : public ElementMap {
   public:
      SpannerMap() {}
      Spanner* findNew(Spanner* o) const { return static_cast<Spanner*>(ElementMap::findNew(o)); }
      void add(Spanner* _o, Spanner* _n) { ElementMap::add(_o, _n); }
      };

#endif

