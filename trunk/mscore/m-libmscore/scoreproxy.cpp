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
#include "scoreproxy.h"
#include "painter.h"
#include "score.h"
#include "page.h"

//---------------------------------------------------------
//    expose
//---------------------------------------------------------

void ScoreProxy::expose(void* gc, double x, double y, double w, double h)
      {
      Painter painter(gc);
      painter.scale(2.0, 2.0);

      QRectF fr(x, y, w, h);
      printf("----pages %d %f %f %f %f\n", s->pages().size(), x, y, w, h);

      foreach(Page* page, s->pages()) {
            QRectF pr(page->abbox());
            QList<const Element*> ell = page->items(fr);

            printf("----elements %d\n", ell.size());

            foreach(const Element* e, ell) {
                  e->itemDiscovered = 0;
                  if (!e->visible())
                        continue;
                  painter.save();
                  QPointF p(e->canvasPos());
                  painter.translate(p.x(), p.y());
                  e->draw(&painter);
                  painter.restore();
                  }
            }
      }

