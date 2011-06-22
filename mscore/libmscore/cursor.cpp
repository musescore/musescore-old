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

#include "cursor.h"
#include "painter.h"
#include "mscore.h"

//---------------------------------------------------------
//   Cursor
//---------------------------------------------------------

Cursor::Cursor(Score* s)
   : Element(s)
      {
      setVisible(false);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Cursor::draw(Painter* painter) const
      {
      if (!visible())
            return;
      int v = track() == -1 ? 0 : voice();
      QColor c(MScore::selectColor[v]);
      c.setAlpha(50);
      painter->setBrushColor(c);
      QRectF r(abbox());
      painter->fillRect(r.x(), r.y(), r.width(), r.height());
      }

