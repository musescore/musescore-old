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

#include "painterqt.h"
#include "scoreview.h"

//---------------------------------------------------------
//   drawText
//---------------------------------------------------------

void PainterQt::drawText(const QTextDocument* doc, const QAbstractTextDocumentLayout::PaintContext& c)
      {
      doc->documentLayout()->draw(_painter, c);
      }

//---------------------------------------------------------
//   drawTextHCentered
//---------------------------------------------------------

void PainterQt::drawTextHCentered(qreal x, qreal y, const QString& s)
      {
      _painter->drawText(QRectF(x, y, 0.0, 0.0), Qt::AlignHCenter|Qt::TextDontClip, s);
      }

//---------------------------------------------------------
//   drawTextVCentered
//---------------------------------------------------------

void PainterQt::drawTextVCentered(qreal x, qreal y, const QString& s)
      {
      _painter->drawText(QRectF(x, y, 0.0, 0.0), Qt::AlignVCenter|Qt::TextDontClip, s);
      }

//---------------------------------------------------------
//   setLineWidth
//---------------------------------------------------------

void PainterQt::setLineWidth(qreal v)
      {
      QPen p(_painter->pen());
      p.setWidthF(v);
      _painter->setPen(p);
      }

//---------------------------------------------------------
//   setCapStyle
//---------------------------------------------------------

void PainterQt::setCapStyle(Qt::PenCapStyle style)
      {
      QPen p(_painter->pen());
      p.setCapStyle(style);
      _painter->setPen(p);
      }

//---------------------------------------------------------
//   setLineStyle
//---------------------------------------------------------

void PainterQt::setLineStyle(Qt::PenStyle style)
      {
      QPen p(_painter->pen());
      p.setStyle(style);
      _painter->setPen(p);
      }

//---------------------------------------------------------
//   setJoinStyle
//---------------------------------------------------------

void PainterQt::setJoinStyle(Qt::PenJoinStyle style)
      {
      QPen p(_painter->pen());
      p.setJoinStyle(style);
      _painter->setPen(p);
      }

//---------------------------------------------------------
//   setNoPen
//---------------------------------------------------------

void PainterQt::setNoPen(bool)
      {
      _painter->setPen(Qt::NoPen);
      }

//---------------------------------------------------------
//   setNoBrush
//---------------------------------------------------------

void PainterQt::setNoBrush(bool)
      {
      _painter->setBrush(Qt::NoBrush);
      }

//---------------------------------------------------------
//   setPenColor
//---------------------------------------------------------

void PainterQt::setPenColor(const QColor& c)
      {
      QPen p(_painter->pen());
      p.setColor(c);
      _painter->setPen(p);
      }

//---------------------------------------------------------
//   setBrushColor
//---------------------------------------------------------

void PainterQt::setBrushColor(const QColor& c)
      {
      _painter->setBrush(QBrush(c));
      }

//---------------------------------------------------------
//   fillPolygon
//---------------------------------------------------------

void PainterQt::fillPolygon(qreal x1, qreal y1,
   qreal x2, qreal y2,
   qreal x3, qreal y3,
   qreal x4, qreal y4)
      {
      QPolygonF pg;
      pg << QPointF(x1, y1) << QPointF(x2, y2)
         << QPointF(x3, y3) << QPointF(x4, y4);

      _painter->drawPolygon(pg, Qt::OddEvenFill);
      }

//---------------------------------------------------------
//   editMode
//---------------------------------------------------------

bool PainterQt::editMode() const
      {
      return _view && _view->editMode();
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void PainterQt::drawBackground(const QRectF& r)
      {
      if (_view)
            _view->drawBackground(*_painter, r);
      else
            _painter->eraseRect(r);
      }

//---------------------------------------------------------
//   drawRoundRect
//---------------------------------------------------------

void PainterQt::drawRoundRect(const QRectF& r, qreal a, qreal b) const
      {
      _painter->drawRoundRect(r, a, b);
      }

