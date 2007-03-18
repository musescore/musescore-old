//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: palette.cpp,v 1.23 2006/03/06 21:08:55 wschweer Exp $
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

#include "palette.h"
#include "element.h"
#include "style.h"
#include "spatium.h"
#include "globals.h"
#include "sym.h"
#include "symbol.h"
#include "layout.h"

//---------------------------------------------------------
//   SymbolPalette
//---------------------------------------------------------

/**
 Create Symbol palette with \a r rows and \a c columns
*/

SymbolPalette::SymbolPalette(int r, int c, qreal mag)
   : QWidget(0)
      {
      extraMag      = mag;
      staff         = false;
      rows          = r;
      columns       = c;
      currentSymbol = 0;;
      symbols       = new Element*[rows*columns];
      names         = new QString[rows*columns];
      for (int i = 0; i < rows*columns; ++i)
            symbols[i] = 0;
      setGrid(50, 60);
      }

SymbolPalette::~SymbolPalette()
      {
      delete symbols;
      delete names;
      }

//---------------------------------------------------------
//   setGrid
//---------------------------------------------------------

/**
 Set size of one element in palette
*/

void SymbolPalette::setGrid(int hh, int vv)
      {
      hgrid = hh;
      vgrid = vv;
      setFixedSize(columns * hgrid, rows * vgrid);
      }

//---------------------------------------------------------
//   setRowsColumns
//---------------------------------------------------------

void SymbolPalette::setRowsColumns(int r, int c)
      {
      int n = rows * columns;
      rows = r;
      columns = c;
      setFixedSize(columns * hgrid, rows * vgrid);

      Element** nsymbols = new Element*[rows*columns];
      QString* nnames    = new QString[rows*columns];
      for (int i = 0; i < r*c; ++i)
            nsymbols[i] = 0;

      for (int i = 0; i < n; ++i) {
            nsymbols[i] = symbols[i];
            nnames[i] = names[i];
            }
      // TODO: delete symbols
      delete[] symbols;
      delete[] names;
      symbols = nsymbols;
      names = nnames;
      }

//---------------------------------------------------------
//   showStaff
//---------------------------------------------------------

void SymbolPalette::showStaff(bool flag)
      {
      staff = flag;
      }

//---------------------------------------------------------
//   contentsMousePressEvent
//---------------------------------------------------------

void SymbolPalette::mousePressEvent(QMouseEvent* ev)
      {
      if (ev->button() == Qt::LeftButton)
            dragStartPosition = ev->pos();
      int x = ev->pos().x();
      int y = ev->pos().y();

      int row = y / vgrid;
      int col = x / hgrid;

      if (row < 0 || row >= rows)
            return;
      if (col < 0 || col >= columns)
            return;
      int idx = row * columns + col;
      if (symbols[idx] == 0)
            return;

      int cc = currentSymbol % columns;
      int cr = currentSymbol / columns;
      QRect r(cc * hgrid, cr * vgrid, hgrid, vgrid);
      symbols[currentSymbol]->setSelected(false);
      currentSymbol = idx;
      symbols[currentSymbol]->setSelected(true);

      cc = currentSymbol % columns;
      cr = currentSymbol / columns;
      r |= QRect(cc * hgrid, cr * vgrid, hgrid, vgrid);
      update(r);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void SymbolPalette::mouseMoveEvent(QMouseEvent* ev)
      {
      if (!(ev->buttons() & Qt::LeftButton))
            return;
      if ((ev->pos() - dragStartPosition).manhattanLength()
         > QApplication::startDragDistance())
            return;
      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;
      Element* el = symbols[currentSymbol];

      // qreal mag = PALETTE_SPATIUM * extraMag / _spatium;
      qreal mag = extraMag;
      QPointF sm(dragStartPosition);
      QPointF rpos(QPointF(sm.x() * mag, sm.y() * mag) - el->pos());

      mimeData->setData("application/mscore/symbol", el->mimeData(rpos));
      drag->setMimeData(mimeData);

      drag->start(Qt::CopyAction);
      }

//---------------------------------------------------------
//   addObject
//---------------------------------------------------------

void SymbolPalette::addObject(int idx, Element* s, const QString& name)
      {
      s->setSelected(false);
      symbols[idx] = s;
      names[idx]   = name;
      update();
      }

//---------------------------------------------------------
//   addObject
//---------------------------------------------------------

void SymbolPalette::addObject(int idx, int symIdx)
      {
      Symbol* s = new Symbol(0);
      s->setSym(symIdx);
      addObject(idx, s, ::symbols[symIdx].name());
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void SymbolPalette::paintEvent(QPaintEvent* e)
      {
      qreal mag = PALETTE_SPATIUM * extraMag / _spatium;
      ScoreLayout layout;
      layout.setSpatium(_spatium);
      layout.setPaintDevice(this);

      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.fillRect(e->rect(), Qt::white);
      p.setClipRect(e->rect());

      //
      // draw grid
      //

      p.setPen(Qt::gray);
      for (int row = 1; row < rows; ++row)
            p.drawLine(0, row*vgrid, columns*hgrid, row*vgrid);
      for (int column = 1; column < columns; ++column)
            p.drawLine(hgrid*column, 0, hgrid*column, rows*vgrid);

      qreal dy = lrint(2 * PALETTE_SPATIUM);

      //
      // draw symbols
      //

      QPen pen(QColor(Qt::black));
      pen.setWidthF(::style->staffLineWidth.val() * PALETTE_SPATIUM);

      for (int row = 0; row < rows; ++row) {
            for (int column = 0; column < columns; ++column) {
                  int idx = row * columns + column;
                  Element* el = symbols[idx];
                  if (el == 0)
                        continue;
                  el->layout(&layout);

                  QRect r(column * hgrid, row * vgrid, hgrid, vgrid);

                  p.setPen(pen);
                  if (el->selected())
                        p.fillRect(r, Qt::yellow);
                  if (staff) {
                        qreal y = r.y() + vgrid / 2 - dy;
                        qreal x = r.x() + 7;
                        qreal w = hgrid - 14;
                        for (int i = 0; i < 5; ++i) {
                              qreal yy = y + PALETTE_SPATIUM * i;
                              p.drawLine(QLineF(x, yy, x + w, yy));
                              }
                        }
                  p.save();
                  p.scale(mag, mag);

                  double gw = hgrid / mag;
                  double gh = vgrid / mag;
                  double gx = column * gw;
                  double gy = row    * gh;

                  double sw = el->width();
                  double sh = el->height();
                  double sy;

                  if (staff)
                        sy = gy + gh * .5 - 2.0 * _spatium;
                  else
                        sy  = gy + (gh - sh) * .5 - el->bbox().y();
                  double sx  = gx + (gw - sw) * .5 - el->bbox().x();

                  el->setPos(sx, sy);
                  QPointF pt(el->pos());
                  p.translate(pt);
                  el->draw(p);
                  p.restore();
                  }
            }
      }

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool SymbolPalette::event(QEvent* ev)
      {
      if (ev->type() == QEvent::ToolTip) {
            QHelpEvent* he = (QHelpEvent*)ev;
            int x = he->pos().x();
            int y = he->pos().y();

            int row = y / vgrid;
            int col = x / hgrid;

            if (row < 0 || row >= rows)
                  return false;
            if (col < 0 || col >= columns)
                  return false;
            int idx = row * columns + col;
            if (symbols[idx] == 0)
                  return false;
            QToolTip::showText(he->globalPos(), names[idx], this);
            return false;
            }
      return QWidget::event(ev);
      }

