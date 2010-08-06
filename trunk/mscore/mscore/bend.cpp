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

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

Bend::Bend(Score* s)
   : Element(s)
      {
      setYoff(-1.0);
      setOffsetType(OFFSET_SPATIUM);
      setAlign(ALIGN_HCENTER | ALIGN_BOTTOM);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bend::layout()
      {
      double _spatium = spatium();
      path      = QPainterPath();

      // dummy
      _lw       = _spatium * 0.3;
      double h  = _spatium * 4;
      double w  = _spatium * 2.5;
      double w1 = w * .6;

      path.lineTo(w, 0.0);
      path.lineTo(w, h-w1);
      path.lineTo(w1, h-w1);
      path.lineTo(w1, h);
      path.lineTo(0.0, h);
      path.lineTo(0.0, 0.0);
      path.moveTo(w, h-w1);
      path.lineTo(w1, h);

      QRectF bb = path.boundingRect();
      bb.adjust(-_lw, -_lw, _lw, _lw);
      setbbox(bb);
      Element::layout();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bend::draw(QPainter& p, ScoreView*) const
      {
      QPen pen = p.pen();
      pen.setWidthF(_lw);
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::RoundJoin);
      p.setPen(pen);
      // p.setBrush(Qt::black);
      p.drawPath(path);
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

      static const int ROWS = 13;
      static const int COLUMNS = 13;

      int xs = w / (COLUMNS);
      int ys = h / (ROWS);
      int lm = xs / 2;
      int tm = ys / 2;
      int tw = (COLUMNS - 1) * xs;
      int th = (ROWS - 1)    * ys;

      QPen pen = p.pen();
      pen.setWidth(1);
      p.setPen(pen);
      for (int x = 0; x < COLUMNS; ++x) {
            int xx = lm + x * xs;
            p.drawLine(xx, tm, xx, tm + th);
            }

      for (int y = 0; y < ROWS; ++y) {
            int yy = tm + y * ys;
            p.drawLine(lm, yy, lm + tw, yy);
            }

      static const int GRIP  = 10;
      static const int GRIP2 = 5;

      int x1;
      int y1;
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
      int tw = (COLUMNS - 1) * xs;
      int th = (ROWS - 1)    * ys;

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


