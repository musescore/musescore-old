//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "cursor.h"
#include "preferences.h"
#include "painter.h"

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

void Cursor::draw(Painter* p) const
      {
      if (!visible())
            return;
      int v = track() == -1 ? 0 : voice();
      Color c(preferences.selectColor[v]);
      c.setAlpha(50);
      p->setBrush(c);
      QRectF r(abbox());
      p->fillRect(r.x(), r.y(), r.width(), r.height());
      }

