//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "preview.h"
#include "score.h"
#include "page.h"
#include "globals.h"

//---------------------------------------------------------
//   PagePreview
//---------------------------------------------------------

PagePreview::PagePreview(QWidget* parent)
   : QWidget(parent)
      {
      setAttribute(Qt::WA_NoBackground);
      _score  = 0;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PagePreview::setScore(Score* s)
      {
      delete _score;
      _score  = s->clone();
      if (_score == 0)
            return;
      layout();
      }

//---------------------------------------------------------
//   PagePreview
//---------------------------------------------------------

PagePreview::~PagePreview()
      {
      delete _score;
      }

//---------------------------------------------------------
//   setMag
//    make sure page fits in window
//---------------------------------------------------------

void PagePreview::setMag()
      {
      double mag1 = (width()  - 20) / (_score->pageFormat()->width() * DPI);
      double mag2 = (height() - 20) / (_score->pageFormat()->height() * DPI);
      qreal  m    = qMin(mag1, mag2);

      matrix.setMatrix(m,      0.0,    0.0,
                       0.0,    m,      0.0,
                       10.0,   10.0,   1.0);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void PagePreview::resizeEvent(QResizeEvent*)
      {
      setMag();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void PagePreview::layout()
      {
      _score->doLayout();
      update();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void PagePreview::paintEvent(QPaintEvent* ev)
      {
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.fillRect(ev->rect(), Qt::gray);

      if (_score->pages().empty())
            return;

      int dx = lrint(matrix.m11());
      int dy = lrint(matrix.m22());
      QRect rr(ev->rect().adjusted(-dx, -dy, dx, dy));

      p.setTransform(matrix);

      Page* page = _score->pages().front();
      QRectF pbbox(page->abbox());
      p.fillRect(pbbox, Qt::white);

      QRectF fr(matrix.inverted().mapRect(QRectF(rr)));
      QList<const Element*> el = page->items(fr);
      foreach(const Element* e, el) {
            e->itemDiscovered = 0;

            if (!(e->visible() || _score->showInvisible()))
                  continue;

            p.save();
            p.translate(e->canvasPos());
            p.setPen(QPen(e->curColor()));
            e->draw(p, 0);
            p.restore();
            }
      }

