//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: note.cpp 3935 2011-01-20 21:18:03Z miwarre $
//
//  Copyright (C) 2011 Werner Schweer
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

      virtual QColor penColor() const             { return _painter->pen().color(); }
      virtual const QTransform& transform() const { return _painter->transform(); }
      virtual void setTransform(const QTransform& t) { _painter->setWorldTransform(t); }

      virtual void drawLine(qreal a, qreal b, qreal c, qreal d) {
            _painter->drawLine(a, b, c, d);
            }
      virtual void fillRect(qreal x1, qreal y1,
         qreal x2, qreal y2,
         qreal x3, qreal y3,
         qreal x4, qreal y4);

      virtual void fillRect(qreal x, qreal y, qreal w, qreal h) {
            _painter->fillRect(x, y, w, h, _painter->brush());
            }
      virtual void drawRect(const QRectF& r) const {
            _painter->drawRect(r);
            }

      virtual void drawText(const QPointF& p, const QString& s) {
            drawText(p.x(), p.y(), s);
            }
      virtual void drawText(qreal x, qreal y, const QString& s) {
            _painter->drawText(x, y, s);
            }

      virtual void drawText(const QTextDocument*, const QColor&, int cursor);
      virtual void drawTextHCentered(qreal x, qreal y, const QString& s);

      virtual void drawPixmap(qreal x, qreal y, const QPixmap& pm) const {
            _painter->drawPixmap(x, y, pm);
            }
      virtual void drawEllipse(QRectF v) const { _painter->drawEllipse(v); }

      virtual void drawPath(const QPainterPath& v) { _painter->drawPath(v); }
      virtual void drawPolyline(const QPointF* v, int n) const {
            _painter->drawPolyline(v, n);
            }

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

