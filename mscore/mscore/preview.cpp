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
      _score->doLayout();
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
      qreal  m    = (mag1 > mag2) ? mag2 : mag1;
      matrix.setMatrix(m, matrix.m12(), matrix.m13(),
         matrix.m21(), m, matrix.m23(), 10, 10, matrix.m33());
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
      QColor _fgColor(Qt::white);
      QColor _bgColor(Qt::gray);
      QRect rr;
      if (_score->needLayout()) {
            _score->doLayout();
            rr = QRect(00, 0, width(), height());
            }
      else {
            int dx = lrint(matrix.m11());
            int dy = lrint(matrix.m22());
            rr = QRect(ev->rect().x()-dx, ev->rect().y()-dy,
               ev->rect().width()+2*dx, ev->rect().height()+2*dy);
            }

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, true);

      p.fillRect(rr, _fgColor);
      if (_score->pages().empty())
            return;

      p.setTransform(matrix);

      QRegion r1(rr);
      Page* page = _score->pages().front();
      QRectF pbbox(page->abbox());
      r1 -= matrix.mapRect(pbbox).toRect();
      p.translate(page->pos());
      page->draw(p);

      QRectF fr = matrix.inverted().mapRect(QRectF(rr));
      QList<const Element*> ell = _score->items(fr);

      for (int i = 0; i < ell.size(); ++i) {
            const Element* e = ell.at(i);
            e->itemDiscovered = 0;

            if (!(e->visible() || _score->showInvisible()))
                  continue;

            QPointF ap(e->canvasPos());
            p.translate(ap);
            p.setPen(QPen(e->color()));
            e->draw(p);
            p.translate(-ap);
            }

      p.setMatrixEnabled(false);
      p.setClipRegion(r1);
      p.fillRect(rr, _bgColor);
      }

