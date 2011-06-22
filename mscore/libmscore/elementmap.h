//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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

