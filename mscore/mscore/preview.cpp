//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: preview.cpp,v 1.4 2006/03/02 17:08:41 wschweer Exp $
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
      _layout = new ScoreLayout(_score);
      ScoreLayout* ol = _score->mainLayout();
      _layout->setSpatium(ol->spatium());
      _layout->setPageFormat(*(ol->pageFormat()));

      for (Measure* m = ol->first(); m; m = m->next()) {
            //
            // HACK:
            // create deep copy of Measure
            //
            static const char* mimeType = "application/mscore/measure";
            QMimeData* mimeData = new QMimeData;
            mimeData->setData(mimeType, m->mimeData(QPointF()));

            Measure* nm = new Measure(_score);
            QByteArray data(mimeData->data(mimeType));

// printf("DATA\n%s\n", data.data());

            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  printf("error reading internal data\n");
                  return;
                  }
            QDomElement e = doc.documentElement();
            e = e.firstChildElement();
            nm->read(e);
            _layout->push_back(nm);
            }
      _layout->setScore(s);
      setMag();
      _layout->doLayout();
      }

//---------------------------------------------------------
//   PagePreview
//---------------------------------------------------------

PagePreview::~PagePreview()
      {
      if (_layout) {
            for (Measure* m = _layout->first(); m; m = m->next())
                  delete m;
            delete _layout;
            }
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
      _layout->doLayout();
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

      QPainter p(this);
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

      QRectF fr = matrix.inverted().mapRect(QRectF(rr));
      QList<Element*> ell = _layout->items(fr);

      for (int i = 0; i < ell.size(); ++i) {
            Element* e = ell.at(i);
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

