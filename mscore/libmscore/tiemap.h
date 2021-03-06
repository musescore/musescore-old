//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TIEMAP_H__
#define __TIEMAP_H__

#include "elementmap.h"

class Tie;

//---------------------------------------------------------
//   TieMap
//---------------------------------------------------------

class TieMap : public ElementMap {
   public:
      TieMap() {}
      Tie* findNew(Tie* o) const { return (Tie*)(ElementMap::findNew((Element*)o)); }
      void add(Tie* _o, Tie* _n) { ElementMap::add((Element*)_o, (Element*)_n); }
      };

#endif

