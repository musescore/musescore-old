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

