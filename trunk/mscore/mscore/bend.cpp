//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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
#include "bendcanvas.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"

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

void Bend::draw(QPainter& p, ScoreView*) const
      {
      if (staff() && !staff()->useTablature())
            return;
      QPen pen = p.pen();
      pen.setWidthF(_lw);
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::RoundJoin);
      p.setPen(pen);
      p.setBrush(Qt::black);

      double _spatium = spatium();
      const TextStyle* st = &score()->textStyle(TEXT_STYLE_BENCH);
      QFont f = st->fontPx(_spatium);
      p.setFont(f);

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
                  p.drawLine(x, y, x2, y2);

                  p.setBrush(Qt::black);
                  p.drawPolygon(arrowUp.translated(x2, y2 + _spatium * .2));

                  int idx = (pitch + 12)/25;
                  const char* l = label[idx];
                  QRectF r;
                  p.drawText(QRectF(x2, y2, 0, 0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip,
                     QString(l),
                     &r
                     );
                  y = y2;
                  }
            if (pitch == _points[pt+1].pitch) {
                  if (pt == (n-2))
                        break;
                  x2 = x + _spatium;
                  y2 = y;
                  p.drawLine(x, y, x2, y2);
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
                  p.setBrush(Qt::NoBrush);
                  p.drawPath(path);

                  p.setBrush(Qt::black);
                  p.drawPolygon(arrowUp.translated(x2, y2 + _spatium * .2));

                  int idx = (_points[pt+1].pitch + 12)/25;
                  const char* l = label[idx];
                  QRectF r;
                  p.drawText(QRectF(x2, y2, 0, 0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip,
                     QString(l),
                     &r
                     );
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
                  p.setBrush(Qt::NoBrush);
                  p.drawPath(path);

                  p.setBrush(Qt::black);
                  p.drawPolygon(arrowDown.translated(x2, y2 - _spatium * .2));
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

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Bend::genPropertyMenu(QMenu* popup) const
      {
      QAction* a;
      a = popup->addAction(tr("Bend Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Bend::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            BendProperties bp(this, 0);
            if (bp.exec()) {
                  printf("bend properties\n");
                  score()->undo()->push(new ChangeBend(this, bp.points()));
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   BendProperties
//---------------------------------------------------------

BendProperties::BendProperties(Bend* b, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      bend = b;
      bendCanvas->setPoints(bend->points());
      bendTypes = new QButtonGroup(this);
      bendTypes->addButton(bend1, 0);
      bendTypes->addButton(bend2, 1);
      bendTypes->addButton(bend3, 2);
      bendTypes->addButton(bend4, 3);
      bendTypes->addButton(bend5, 4);
      bendTypes->setExclusive(true);
      connect(bendTypes, SIGNAL(buttonClicked(int)), SLOT(bendTypeChanged(int)));
      }

//---------------------------------------------------------
//   points
//---------------------------------------------------------

const QList<PitchValue>& BendProperties::points() const
      {
      return bendCanvas->points();
      }

//---------------------------------------------------------
//   bendTypeChanged
//---------------------------------------------------------

void BendProperties::bendTypeChanged(int n)
      {
      QList<PitchValue>& points = bendCanvas->points();

      points.clear();
      switch(n) {
            case 0:
                  points.append(PitchValue(0,0));
                  points.append(PitchValue(15,100));
                  points.append(PitchValue(60,100));
                  break;
            case 1:
                  points.append(PitchValue(0,0));
                  points.append(PitchValue(10,100));
                  points.append(PitchValue(20,100));
                  points.append(PitchValue(30,0));
                  points.append(PitchValue(60,0));
                  break;
            case 2:
                  points.append(PitchValue(0,0));
                  points.append(PitchValue(10,100));
                  points.append(PitchValue(20,100));
                  points.append(PitchValue(30,0));
                  points.append(PitchValue(40,0));
                  points.append(PitchValue(50,100));
                  points.append(PitchValue(60,100));
                  break;
            case 3:
                  points.append(PitchValue(0,100));
                  points.append(PitchValue(60,100));
                  break;
            case 4:
                  points.append(PitchValue(0,100));
                  points.append(PitchValue(15,100));
                  points.append(PitchValue(30,0));
                  points.append(PitchValue(60,0));
                  break;
            }
      update();
      }

//---------------------------------------------------------
//   BendCanvas
//---------------------------------------------------------

BendCanvas::BendCanvas(QWidget* parent)
   : QFrame(parent)
      {
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void BendCanvas::paintEvent(QPaintEvent* ev)
      {
      int w = width();
      int h = height();

      QPainter p(this);
      p.fillRect(rect(), Qt::white);

      static const int ROWS    = 13;
      static const int COLUMNS = 13;

      int xs = w / (COLUMNS);
      int ys = h / (ROWS);
      int lm = xs / 2;
      int tm = ys / 2;
      int tw = (COLUMNS - 1) * xs;
      int th = (ROWS - 1)    * ys;

      QPen pen = p.pen();
      pen.setWidth(1);
      for (int x = 0; x < COLUMNS; ++x) {
            int xx = lm + x * xs;
            pen.setColor(x % 3 ? Qt::gray : Qt::black);
            p.setPen(pen);
            p.drawLine(xx, tm, xx, tm + th);
            }

      for (int y = 0; y < ROWS; ++y) {
            int yy = tm + y * ys;
            pen.setColor(y % 4 ? Qt::gray : Qt::black);
            p.setPen(pen);
            p.drawLine(lm, yy, lm + tw, yy);
            }

      static const int GRIP  = 10;
      static const int GRIP2 = 5;

      int x1 = 0;
      int y1 = 0;
      int idx = 0;
      pen = p.pen();
      pen.setWidth(5);
      pen.setColor(Qt::gray);
      p.setPen(pen);
      foreach(const PitchValue& v, _points) {
            int x = ((tw * v.time) / 60) + lm;
            int y = th - ((th * v.pitch) / 300) + tm;
            if (idx)
                  p.drawLine(x1, y1, x, y);
            x1 = x;
            y1 = y;
            ++idx;
            }

      foreach(const PitchValue& v, _points) {
            int x = ((tw * v.time) / 60) + lm;
            int y = th - ((th * v.pitch) / 300) + tm;
            p.fillRect(x - GRIP2, y - GRIP2, GRIP, GRIP, Qt::blue);
            }

      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void BendCanvas::mousePressEvent(QMouseEvent* ev)
      {
      static const int ROWS = 13;
      static const int COLUMNS = 13;

      int xs = width() / (COLUMNS);
      int ys = height() / (ROWS);
      int lm = xs / 2;
      int tm = ys / 2;
//      int tw = (COLUMNS - 1) * xs;
//      int th = (ROWS - 1)    * ys;

      int x = ev->x() - lm;
      int y = ev->y() - tm;
      x = (x + xs/2) / xs;
      y = (y + ys/2) / ys;
      if (x >= COLUMNS)
            x = COLUMNS - 1;
      if (y >= ROWS)
            y = ROWS - 1;
      y = ROWS - y - 1;

      int time = x * 5;
      int pitch = y * 25;

      int n = _points.size();
      bool found = false;
      for (int i = 0; i < n; ++i) {
            if (_points[i].time > time) {
                  _points.insert(i, PitchValue(time, pitch, false));
                  found = true;
                  break;
                  }
            if (_points[i].time == time) {
                  if (_points[i].pitch == pitch && i > 0 && i < (n-1)) {
                        _points.removeAt(i);
                        }
                  else {
                        _points[i].pitch = pitch;
                        }
                  found = true;
                  break;
                  }
            }
      if (!found)
            _points.append(PitchValue(time, pitch, false));
      update();
      }


