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

#ifndef __ELEMENTMAP_H__
#define __ELEMENTMAP_H__

//---------------------------------------------------------
//   Element2
//---------------------------------------------------------

struct Element2 {
      Element* o;
      Element* n;
      Element2(Element* _o, Element* _n) : o(_o), n(_n) {}
      };

//---------------------------------------------------------
//   ElementMap
//---------------------------------------------------------

class ElementMap {
      QList<Element2> map;

   public:
      ElementMap() {}
      Element* findNew(Element* o) const;
      void add(Element* _o, Element* _n) { map.append(Element2(_o, _n)); }
      };

#endif

