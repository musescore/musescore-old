//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CURSOR_H__
#define __CURSOR_H__

#include "element.h"

class Painter;

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

class Cursor : public Element {

   public:
      Cursor(Score*);
      virtual Cursor* clone() const    { return new Cursor(*this); }
      virtual ElementType type() const { return CURSOR; }
      virtual void draw(Painter*) const;
      };

#endif

