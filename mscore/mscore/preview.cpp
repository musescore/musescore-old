//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: preview.cpp,v 1.4 2006/03/02 17:08:41 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "painter.h"
#include "globals.h"
#include "layout.h"

//---------------------------------------------------------
//   PagePreview
//---------------------------------------------------------

PagePreview::PagePreview(QWidget* parent)
   : QWidget(parent)
      {
      setAttribute(Qt::WA_NoBackground);
      _score  = 0;
      _layout = 0;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PagePreview::setScore(Score* s)
      {
      _score = s;
      _layout = new ScoreLayout(*(_score->scoreLayout()));
      _layout->setScore(s);
      setMag();
      _layout->layout();
      }

//---------------------------------------------------------
//   PagePreview
//---------------------------------------------------------

PagePreview::~PagePreview()
      {
      if (_layout)
            delete _layout;
      }

//---------------------------------------------------------
//   setMag
//    make sure page fits in window
//---------------------------------------------------------

void PagePreview::setMag()
      {
      double mag1 = (width()  - 20) / _score->pageFormat()->width();
      double mag2 = (height() - 20) / _score->pageFormat()->height();
      qreal  m    = (mag1 > mag2) ? mag2 : mag1;
      matrix.setMatrix(m, matrix.m12(), matrix.m21(),
         m * qreal(appDpiY)/qreal(appDpiX), 10, 10);
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
      _layout->pages()->update();
      _layout->layout();
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
      if (_layout->needLayout()) {
            _layout->doLayout();
            rr = QRect(00, 0, width(), height());
            }
      else {
            int dx = lrint(matrix.m11());
            int dy = lrint(matrix.m22());
            rr = QRect(ev->rect().x()-dx, ev->rect().y()-dy,
               ev->rect().width()+2*dx, ev->rect().height()+2*dy);
            }

      Painter p(this);
      p.setClipRect(ev->rect());
      p.setRenderHint(QPainter::Antialiasing, true);

      p.fillRect(rr, _fgColor);
      if (_layout->pages()->empty())
            return;

      p.setMatrix(matrix);

      QRegion r1(rr);
      Page* page = _layout->pages()->front();
      QRectF pbbox(page->abbox());
      r1 -= matrix.mapRect(pbbox).toRect();
      p.translate(page->pos());
      page->draw(p);

      p.setMatrixEnabled(false);
      p.setClipRegion(r1);
      p.fillRect(rr, _bgColor);
      }

