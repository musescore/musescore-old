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

#ifndef __PAINTERQT_H__
#define __PAINTERQT_H__

#include "libmscore/painter.h"

class ScoreView;

//---------------------------------------------------------
//   class PainterQt
//---------------------------------------------------------

class PainterQt : public Painter {
      QPainter*  _painter;
      ScoreView* _view;

   public:
      PainterQt(QPainter* p, ScoreView* v) : Painter(), _painter(p), _view(v) {}

      virtual void save()                       { _painter->save();    }
      virtual void restore()                    { _painter->restore(); }

      virtual void translate(const QPointF& pt) { _painter->translate(pt); }
      virtual void scale(qreal v)               { _painter->scale(v, v);   }
      virtual void scale(qreal x, qreal y)      { _painter->scale(x, y);   }
      virtual void rotate(qreal v)              { _painter->rotate(v);     }

      virtual void setFont(const QFont& f)      { _painter->setFont(f);    }
      virtual void setLineWidth(qreal);
      virtual void setCapStyle(Qt::PenCapStyle);
      virtual void setLineStyle(Qt::PenStyle);
      virtual void setJoinStyle(Qt::PenJoinStyle);
      virtual void setNoPen(bool);
      virtual void setNoBrush(bool);
      virtual void setPenColor(const QColor&);
      virtual void setBrushColor(const QColor&);

      virtual QColor penColor() const                { return _painter->pen().color(); }
      virtual const QTransform& transform() const    { return _painter->transform();   }
      virtual void setTransform(const QTransform& t) { _painter->setWorldTransform(t); }

      virtual void drawLine(qreal a, qreal b, qreal c, qreal d) {
            _painter->drawLine(QLineF(a, b, c, d));
            }
      virtual void fillPolygon(qreal x1, qreal y1,
         qreal x2, qreal y2,
         qreal x3, qreal y3,
         qreal x4, qreal y4);

      virtual void fillRect(qreal x, qreal y, qreal w, qreal h) {
            _painter->fillRect(QRectF(x, y, w, h), _painter->brush());
            }
      virtual void drawRect(const QRectF& r) const {
            _painter->drawRect(r);
            }

      virtual void drawText(const QPointF& p, const QString& s) {
            _painter->drawText(p, s);
            }
      virtual void drawText(qreal x, qreal y, const QString& s) {
            _painter->drawText(QPointF(x, y), s);
            }

      virtual void drawText(const QTextDocument*, const QAbstractTextDocumentLayout::PaintContext&);
      virtual void drawTextHCentered(qreal x, qreal y, const QString& s);
      virtual void drawTextVCentered(qreal x, qreal y, const QString& s);
#ifdef USE_GLYPHS
      virtual void drawGlyphRun(const QPointF& pt, const QGlyphRun& gr) const {
            _painter->drawGlyphRun(pt, gr);
            }
#endif
      virtual void drawPixmap(qreal x, qreal y, const QPixmap& pm) const {
            _painter->drawPixmap(QPointF(x, y), pm);
            }
      virtual void drawEllipse(QRectF v) const { _painter->drawEllipse(v); }

      virtual void drawPath(const QPainterPath& v) { _painter->drawPath(v); }
      virtual void drawPolyline(const QPointF* v, int n) const {
            _painter->drawPolyline(v, n);
            }
      virtual void drawPolygon(const QPolygonF& pg) const { _painter->drawPolygon(pg); }

      virtual void drawArc(const QRectF& r, int a, int b) const {
            _painter->drawArc(r, a, b);
            }
      virtual void drawRoundRect(const QRectF& r, qreal a, qreal b) const {
            _painter->drawRoundRect(r, a, b);
            }
      virtual void drawBackground(const QRectF& r);
      virtual bool editMode() const;

      QPainter* painter() const     { return _painter; }
      ScoreView* view() const       { return _view;    }
      };

#endif

