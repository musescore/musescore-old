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

#ifndef __TUPLETMAP_H__
#define __TUPLETMAP_H__

class Tuplet;

//---------------------------------------------------------
//   Tuplet2
//---------------------------------------------------------

struct Tuplet2 {
      Tuplet* o;
      Tuplet* n;
      Tuplet2(Tuplet* _o, Tuplet* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   TupletMap
//---------------------------------------------------------

class TupletMap {
      QList<Tuplet2> map;

   public:
      TupletMap() {}
      Tuplet* findNew(Tuplet* o);
      void add(Tuplet* _o, Tuplet* _n) { map.append(Tuplet2(_o, _n)); }
      };

#endif

