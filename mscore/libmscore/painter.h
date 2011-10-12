//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __PAINTER_H__
#define __PAINTER_H__

#include "config.h"

class QFont;
class QString;
class QPainterPath;
class QColor;

//---------------------------------------------------------
//   class Painter
//    This class defines the interface to the render
//    a score.
//---------------------------------------------------------

class Painter {

   public:
      Painter() {}

      virtual void save() = 0;
      virtual void restore() = 0;

      virtual void translate(const QPointF&) = 0;
      virtual void scale(qreal) = 0;
      virtual void scale(qreal, qreal) = 0;
      virtual void rotate(qreal) = 0;

      virtual void setFont(const QFont& f) = 0;
      virtual void setLineWidth(qreal) = 0;

      virtual void setCapStyle(Qt::PenCapStyle) = 0;
      virtual void setLineStyle(Qt::PenStyle) = 0;
      virtual void setJoinStyle(Qt::PenJoinStyle) = 0;

      virtual void setNoPen(bool) = 0;
      virtual void setNoBrush(bool) = 0;
      virtual void setPenColor(const QColor&) = 0;
      virtual void setBrushColor(const QColor&) = 0;

      virtual QColor penColor() const = 0;
      virtual const QTransform& transform() const = 0;
      virtual void setTransform(const QTransform& t) = 0;

      virtual void drawLine(qreal, qreal, qreal, qreal) = 0;
      virtual void fillPolygon(qreal x1, qreal y1,
         qreal x2, qreal y2,
         qreal x3, qreal y3,
         qreal x4, qreal y4) = 0;
      virtual void fillRect(qreal x, qreal y, qreal w, qreal h) = 0;
      virtual void drawRect(const QRectF&) const = 0;

      virtual void drawText(qreal x, qreal y, const QString&) = 0;
      virtual void drawText(const QPointF&, const QString&) = 0;
      QAbstractTextDocumentLayout::PaintContext c;
      virtual void drawText(const QTextDocument*, const QAbstractTextDocumentLayout::PaintContext&) = 0;
      virtual void drawTextHCentered(qreal x, qreal y, const QString& s) = 0;
      virtual void drawTextVCentered(qreal x, qreal y, const QString& s) = 0;

#ifdef USE_GLYPHS
      virtual void drawGlyphRun(const QPointF&, const QGlyphRun&) const = 0;
#endif
      virtual void drawPixmap(qreal x, qreal y, const QPixmap&) const = 0;

      virtual void drawEllipse(QRectF) const = 0;

      virtual void drawPath(const QPainterPath&) = 0;
      virtual void drawPolyline(const QPointF*, int) const = 0;
      virtual void drawPolygon(const QPolygonF&) const = 0;

      virtual void drawArc(const QRectF&, int, int) const = 0;
      virtual void drawRoundRect(const QRectF&, qreal, qreal) const = 0;

      virtual void drawBackground(const QRectF& r) = 0;
      virtual bool editMode() const { return false; }
      };

#endif

