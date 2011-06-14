//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer and others
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

#include "bend.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "painter.h"

//---------------------------------------------------------
//   label
//---------------------------------------------------------

static const char* label[] = {
      "", "1/4", "1/2", "3/4", "full",
      "1 1/4", "1 1/2", "1 3/4", "2",
      "2 1/4", "2 1/2", "2 3/4", "3"
      };

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

Bend::Bend(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bend::layout()
      {
      double _spatium = spatium();

      if (staff() && !staff()->useTablature()) {
            setbbox(QRectF());
            if (!parent()) {
                  noteWidth = -_spatium*2;
                  notePos   = QPointF(0.0, _spatium*3);
                  }
            }

      _lw        = _spatium * 0.1;
      Note* note = static_cast<Note*>(parent());
      if (note == 0) {
            noteWidth = 0.0;
            notePos = QPointF();
            }
      else {
            notePos = note->pos();
            noteWidth = note->width();
            }
      QRectF bb;

      const TextStyle* st = &score()->textStyle(TEXT_STYLE_BENCH);
      QFont f = st->fontPx(_spatium);
      QFontMetricsF fm(f);

      int n    = _points.size();
//      int pt   = 0;
      double x = noteWidth;
      double y = -_spatium * .8;
      double x2, y2;

      double aw = _spatium * .5;
      QPolygonF arrowUp;
      arrowUp << QPointF(0, 0) << QPointF(aw*.5, aw) << QPointF(-aw*.5, aw);
      QPolygonF arrowDown;
      arrowDown << QPointF(0, 0) << QPointF(aw*.5, -aw) << QPointF(-aw*.5, -aw);

      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            int pitch = _points[pt].pitch;
            if (pt == 0 && pitch) {
                  y2 = -notePos.y() -_spatium * 2;
                  x2 = x;
                  bb |= QRectF(x, y, x2-x, y2-y);

                  bb |= arrowUp.translated(x2, y2 + _spatium * .2).boundingRect();

                  int idx = (pitch + 12)/25;
                  const char* l = label[idx];
                  bb |= fm.boundingRect(QRectF(x2, y2, 0, 0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l));
                  y = y2;
                  }
            if (pitch == _points[pt+1].pitch) {
                  if (pt == (n-2))
                        break;
                  x2 = x + _spatium;
                  y2 = y;
                  bb |= QRectF(x, y, x2-x, y2-y);
                  }
            else if (pitch < _points[pt+1].pitch) {
                  // up
                  x2 = x + _spatium*.5;
                  y2 = -notePos.y() -_spatium * 2;
                  double dx = x2 - x;
                  double dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  bb |= path.boundingRect();

                  bb |= arrowUp.translated(x2, y2 + _spatium * .2).boundingRect();

                  int idx = (_points[pt+1].pitch + 12)/25;
                  const char* l = label[idx];
                  QRectF r;
                  bb |= fm.boundingRect(QRectF(x2, y2, 0, 0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QString(l));
                  }
            else {
                  // down
                  x2 = x + _spatium*.5;
                  y2 = y + _spatium * 3;
                  double dx = x2 - x;
                  double dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  bb |= path.boundingRect();

                  bb |= arrowDown.translated(x2, y2 - _spatium * .2).boundingRect();
                  }
            x = x2;
            y = y2;
            }

      bb.adjust(-_lw, -_lw, _lw, _lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bend::draw(Painter* painter) const
      {
      if (staff() && !staff()->useTablature())
            return;
      painter->setLineWidth(_lw);
      painter->setCapStyle(Qt::RoundCap);
      painter->setJoinStyle(Qt::RoundJoin);
      painter->setBrushColor(Qt::black);

      double _spatium = spatium();
      const TextStyle* st = &score()->textStyle(TEXT_STYLE_BENCH);
      QFont f = st->fontPx(_spatium);
      painter->setFont(f);

      int n    = _points.size();
//      int pt   = 0;
      double x = noteWidth;
      double y = -_spatium * .8;
      double x2, y2;

      double aw = _spatium * .5;
      QPolygonF arrowUp;
      arrowUp << QPointF(0, 0) << QPointF(aw*.5, aw) << QPointF(-aw*.5, aw);
      QPolygonF arrowDown;
      arrowDown << QPointF(0, 0) << QPointF(aw*.5, -aw) << QPointF(-aw*.5, -aw);

      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            int pitch = _points[pt].pitch;
            if (pt == 0 && pitch) {
                  y2 = -notePos.y() -_spatium * 2;
                  x2 = x;
                  painter->drawLine(x, y, x2, y2);

                  painter->setBrushColor(Qt::black);
                  painter->drawPolygon(arrowUp.translated(x2, y2 + _spatium * .2));
                  int idx = (pitch + 12)/25;
                  const char* l = label[idx];
                  painter->drawTextHCentered(x2, y2, QString(l));
                  y = y2;
                  }
            if (pitch == _points[pt+1].pitch) {
                  if (pt == (n-2))
                        break;
                  x2 = x + _spatium;
                  y2 = y;
                  painter->drawLine(x, y, x2, y2);
                  }
            else if (pitch < _points[pt+1].pitch) {
                  // up
                  x2 = x + _spatium*.5;
                  y2 = -notePos.y() -_spatium * 2;
                  double dx = x2 - x;
                  double dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  painter->setNoBrush(true);
                  painter->drawPath(path);

                  painter->setBrushColor(Qt::black);
                  painter->drawPolygon(arrowUp.translated(x2, y2 + _spatium * .2));

                  int idx = (_points[pt+1].pitch + 12)/25;
                  const char* l = label[idx];
                  painter->drawTextHCentered(x2, y2, QString(l));
                  }
            else {
                  // down
                  x2 = x + _spatium*.5;
                  y2 = y + _spatium * 3;
                  double dx = x2 - x;
                  double dy = y2 - y;

                  QPainterPath path;
                  path.moveTo(x, y);
                  path.cubicTo(x+dx/2, y, x2, y+dy/4, x2, y2);
                  painter->setNoBrush(true);
                  painter->drawPath(path);

                  painter->setBrushColor(Qt::black);
                  painter->drawPolygon(arrowDown.translated(x2, y2 - _spatium * .2));
                  }
            x = x2;
            y = y2;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Bend::write(Xml& xml) const
      {
      xml.stag("Bend");
      foreach(const PitchValue& v, _points) {
            xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
               .arg(v.time).arg(v.pitch).arg(v.vibrato));
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Bend::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "point") {
                  PitchValue pv;
                  pv.time    = e.attribute("time").toInt();
                  pv.pitch   = e.attribute("pitch").toInt();
                  pv.vibrato = e.attribute("vibrato").toInt();
                  _points.append(pv);
                  }
            else
                  domError(e);
            }
      }
