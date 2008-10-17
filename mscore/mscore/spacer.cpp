//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layoutbreak.cpp,v 1.1 2006/03/27 14:16:24 wschweer Exp $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "spacer.h"
#include "preferences.h"
#include "score.h"
#include "canvas.h"
#include "layout.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Score* score)
   : Element(score)
      {
      height = Spatium(0);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Spacer::draw(QPainter& p) const
      {
      if (score()->printing())
            return;
      QPen pen;
      if (selected())
            pen.setColor(preferences.selectColor[0]);
      else
            pen.setColor(preferences.layoutBreakColor);

      pen.setWidthF(_spatium * 0.4);
      p.setPen(pen);
      p.setBrush(Qt::NoBrush);
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Spacer::layout(ScoreLayout* layout)
      {
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Spacer::acceptDrop(Viewer*, const QPointF&, int type, int st) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Spacer::drop(const QPointF& p1, const QPointF& p2, Element* e)
      {
      return e;
      }


