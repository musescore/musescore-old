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

#ifndef __PAINTER_H__
#define __PAINTER_H__

//---------------------------------------------------------
//   class Painter
//---------------------------------------------------------

class Painter {
      QPainter*  _painter;
      ScoreView* _view;

   public:
      Painter(QPainter* p, ScoreView* v) : _painter(p), _view(v) {}
      QPainter* painter() const { return _painter; }
      ScoreView* view() const   { return _view;    }

      void translate(const QPointF& pt)                 { _painter->translate(pt); }
      void scale(qreal v)                               { _painter->scale(v, v);   }
      void drawText(const QPointF& p, const QString& s) { _painter->drawText(p, s); }
      void drawText(qreal x, qreal y, const QString& s) { _painter->drawText(x, y, s); }
      void setFont(const QFont& f)                      { _painter->setFont(f);     }
      };

#endif

