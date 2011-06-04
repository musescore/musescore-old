//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
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

#ifndef __PAINTER_H__
#define __PAINTER_H__

#include <QtCore/qglobal.h>
#include <QtCore/QList>
#include <QtCore/QString>
#include "font.h"

class QPainter;
class Font;
class QPointF;
class QString;
class Color;

enum CAP_STYLE {
      CAP_BUTT, CAP_ROUND, CAP_SQUARE     // apple ios
      };

enum JOIN_STYLE {
      JOIN_MITER, JOIN_ROUND, JOIN_BEVEL
      };

//---------------------------------------------------------
//   MRect
//---------------------------------------------------------

struct MRect {
      qreal x, y, w, h;

      MRect() { x = y = w = h = 0.0; }
      };

//---------------------------------------------------------
//   PainterPathElement
//---------------------------------------------------------

enum PainterPathElementType {
      MoveToElement,
      LineToElement,
      CurveToElement,
      CurveToDataElement
      };

class PainterPathElement {
   public:
      PainterPathElementType type;
      qreal x, y, c, d, e, f;

      PainterPathElement(PainterPathElementType t, qreal a, qreal b)
        : type(t), x(a), y(b) {}
      PainterPathElement(PainterPathElementType t, qreal a, qreal b,
         qreal _c, qreal _d, qreal _e, qreal _f)
        : type(t), x(a), y(b), c(_c), d(_d), e(_e), f(_f) {}
      };

//---------------------------------------------------------
//   PainterPath
//---------------------------------------------------------

class PainterPath : public QList<PainterPathElement> {
   public:
      PainterPath() {}
      void moveTo(qreal x, qreal y) {
            append(PainterPathElement(MoveToElement, x, y));
            }
      void lineTo(qreal x, qreal y) {
            append(PainterPathElement(LineToElement, x, y));
            }
      void cubicTo(qreal x, qreal y, qreal c, qreal d, qreal e, qreal f) {
            append(PainterPathElement(CurveToElement, x, y, c, d, e, f));
            }
      };

//---------------------------------------------------------
//   Painter
//---------------------------------------------------------

class Painter {
      void* _context;
      Font _font;
      bool _hasPen;
      bool _hasBrush;

   public:
      Painter();
      Painter(void* c);
      void drawText(const Font& f, const QPointF&, const QString& txt);
      void drawLine(qreal, qreal, qreal, qreal);
      void save();
      void restore();
      void translate(const QPointF&);
      void setBrush(const Color&);
      void scale(qreal, qreal);

      void rotate(qreal);
      void drawPolyline(const QPointF*, int);
      void* context() { return _context; }
      void drawSymbol(qreal x, qreal y, int code);
      void fillRect(qreal x1, qreal y1,
         qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4);
      void fillRect(qreal x1, qreal y1, qreal w, qreal h);
      void drawPath(const PainterPath&);

      void setPenColor(const Color&);
      void setNoPen(bool);
      void setNoBrush(bool);
      Color penColor() const;
      void setPenWidth(qreal);
      void setLineCap(int);
      void setLineJoin(int);
      void setLineStyle(int);
      bool hasPen() const   { return _hasPen; }
      bool hasBrush() const { return _hasBrush; }
      };

#endif

