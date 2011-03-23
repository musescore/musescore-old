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

#ifndef __SLURMAP_H__
#define __SLURMAP_H__

//---------------------------------------------------------
//   Slur2
//---------------------------------------------------------

struct Slur2 {
      Slur* o;
      Slur* n;
      Slur2(Slur* _o, Slur* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   SlurMap
//---------------------------------------------------------

class SlurMap {
      QList<Slur2> map;

   public:
      SlurMap() {}
      Slur* findNew(Slur* o);
      void add(Slur* _o, Slur* _n) { map.append(Slur2(_o, _n)); }
      void check();
      };

#endif

