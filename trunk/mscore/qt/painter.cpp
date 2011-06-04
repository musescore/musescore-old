//=============================================================================
//  MscorePlayer
//  $Id$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#include <stdio.h>
#include <math.h>
#include <QtGui/QPainter>
#include <QtGui/QGlyphs>

#include "al/color.h"
#include "libmscore/painter.h"
#include "libmscore/globals.h"

void Painter::setBrush(const Color&) {}

//---------------------------------------------------------
//   Painter
//---------------------------------------------------------

Painter::Painter(void* c)
      {
      _context = c;
      }

//---------------------------------------------------------
//   drawText
//---------------------------------------------------------

void Painter::drawText(const Font& f, const QPointF& pt, const QString& txt)
      {
      QPainter* p = (QPainter*)_context;
      QFont qf(f.family());
      qf.setPixelSize(lrint(f.size()));
      p->setFont(qf);
      p->drawText(pt, txt);
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Painter::save()
      {
      QPainter* p = (QPainter*)_context;
      p->save();
      }

//---------------------------------------------------------
//   restore
//---------------------------------------------------------

void Painter::restore()
      {
      QPainter* p = (QPainter*)_context;
      p->restore();
      }

//---------------------------------------------------------
//   drawLine
//---------------------------------------------------------

void Painter::drawLine(qreal x1, qreal y1, qreal x2, qreal y2)
      {
      QPainter* p = (QPainter*)_context;
      p->drawLine(QLineF(x1, y1, x2, y2));
      }

//---------------------------------------------------------
//   scale
//---------------------------------------------------------

void Painter::scale(qreal x, qreal y)
      {
      ((QPainter*)_context)->scale(x, y);
      }

void Painter::rotate(qreal)
      {
      }

//---------------------------------------------------------
//   translate
//---------------------------------------------------------

void Painter::translate(const QPointF& pt)
      {
      ((QPainter*)_context)->translate(pt);
      }

//---------------------------------------------------------
//   drawSymbol
//---------------------------------------------------------

void Painter::drawSymbol(qreal x, qreal y, int idx)
      {
      QPainter* p = (QPainter*)_context;
      QGlyphs glyphs;
      QFont f("Mscore");
      f.setPixelSize(lrint(20.0 * DPI/PPI));
      glyphs.setFont(f);
      glyphs.setGlyphIndexes(QVector<quint32>(1, idx));
      glyphs.setPositions(QVector<QPointF>(1, QPointF()));
      p->drawGlyphs(QPointF(x, y), glyphs);
      }

//---------------------------------------------------------
//   fillRect
//---------------------------------------------------------

void Painter::fillRect(qreal x1, qreal y1, qreal x2, qreal y2,
   qreal x3, qreal y3, qreal x4, qreal y4)
      {
      QPainter* p = (QPainter*)_context;

      QPolygonF pg;
      pg << QPointF(x1, y1) << QPointF(x2, y2)
         << QPointF(x3, y3) << QPointF(x4, y4);

      p->setBrush(Qt::black);
      p->drawPolygon(pg, Qt::OddEvenFill);
      }

void Painter::fillRect(qreal x, qreal y, qreal w, qreal h)
      {
      QPainter* p = (QPainter*)_context;
      p->fillRect(QRectF(x, y, w, h), Qt::black);
      }

//---------------------------------------------------------
//   setPenWidth
//---------------------------------------------------------

void Painter::setPenWidth(qreal w)
      {
      QPainter* p = (QPainter*)_context;
      QPen pen(p->pen());
      pen.setWidthF(w);
      p->setPen(pen);
      }

void Painter::setLineCap(int) {}
void Painter::setLineJoin(int) {}

void Painter::setNoPen(bool) {}
void Painter::setNoBrush(bool) {}
void Painter::setLineStyle(int) {}
void Painter::setPenColor(const Color&) {}
Color Painter::penColor() const { return Color(0, 0, 0); }

//---------------------------------------------------------
//   textMetrics
//---------------------------------------------------------

qreal textMetrics(const QString& s, qreal size, qreal* a, qreal* d, qreal* l)
      {
      QFont f("Mscore", size);
      QFontMetricsF fm(f);
      *a = fm.ascent();
      *d = fm.descent();
      *l = fm.leading();
      return fm.width(s);
      }

qreal textMetrics(const QString& family, const QString& s, qreal size,
   qreal* a, qreal* d, qreal* l)
      {
      QFont f(family, size);
      QFontMetricsF fm(f);
      *a = fm.ascent();
      *d = fm.descent();
      *l = fm.leading();
      return fm.width(s);
      }

//---------------------------------------------------------
//   drawPath
//---------------------------------------------------------

void Painter::drawPath(const PainterPath& pp)
      {
      QPainter* p = (QPainter*)_context;

      QPainterPath path;
      foreach(const PainterPathElement& e, pp) {
            switch(e.type) {
                  case MoveToElement:
                        path.moveTo(e.x, e.y);
                        break;
                  case LineToElement:
                        path.lineTo(e.x, e.y);
                        break;
                  case CurveToElement:
                        path.cubicTo(e.x, e.y, e.c, e.d, e.e, e.f);
                        break;
                  case CurveToDataElement:
                        break;
                  }
            }
      p->setBrush(Qt::black);
      p->drawPath(path);
      }

