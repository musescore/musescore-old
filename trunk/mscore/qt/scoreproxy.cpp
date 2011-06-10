//=============================================================================
//  MscorePlayer
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include <QtCore/QRectF>
#include "m-libmscore/scoreproxy.h"
#include "m-libmscore/painter.h"
#include "m-libmscore/score.h"
#include "m-libmscore/page.h"

//---------------------------------------------------------
//   expose
//---------------------------------------------------------

void ScoreProxy::expose(void* gc, int pageNo, double x, double y, double w, double h)
      {
      Painter painter(gc);

      Page* page = s->page(pageNo);
      QPointF p(page->canvasPos());
      painter.translate(-p);
      x += p.x();
      QRectF fr(x, y, w, h);

      qreal pw = s->pageWidth();
      qreal ph = s->pageHeight();
      painter.setPenColor(Color(0, 255, 0));

      painter.drawLine(0.0, 0.0, pw, 0.0);
      painter.drawLine(0.0, 0.0, 0.0, ph);
      painter.drawLine(pw, ph, 0.0, ph);
      painter.drawLine(pw, ph, pw, 0.0);

      foreach(const Element* e, page->items()) {
            if (e->abbox().intersects(fr)) {
                  painter.save();
                  painter.translate(e->canvasPos());
                  e->draw(&painter);
                  painter.restore();
                  }
            }
      }

