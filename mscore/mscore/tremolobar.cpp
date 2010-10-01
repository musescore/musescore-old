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

#include "tremolobar.h"
#include "tremolobarcanvas.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"

//---------------------------------------------------------
//   TremoloBar
//---------------------------------------------------------

TremoloBar::TremoloBar(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TremoloBar::layout()
      {
      double _spatium = spatium();

      if (staff() && !staff()->useTablature()) {
            setbbox(QRectF());
            if (!parent()) {
                  noteWidth = -_spatium*2;
                  notePos   = QPointF(0.0, _spatium*3);
                  }
            return;
            }

      _lw = _spatium * 0.1;
      Note* note = 0;
      if (note == 0) {
            noteWidth = 0.0;
            notePos = QPointF();
            }
      else {
            noteWidth = note->width();
            notePos = note->pos();
            }
//      int n    = _points.size();
//      int pt   = 0;
//      double x = noteWidth * .5;
//      double y = notePos.y() - _spatium;
//      double x2, y2;

      QRectF bb (0, 0, _spatium*3, -_spatium * 4);
#if 0
      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            x = x2;
            y = y2;
            }
#endif
      bb.adjust(-_lw, -_lw, _lw, _lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TremoloBar::draw(QPainter& p, ScoreView*) const
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
//      double x = noteWidth;
//      double y = -_spatium * .8;
//      double x2, y2;

      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
//            int pitch = _points[pt].pitch;
            }
      //debug:
      p.drawLine(0.0, 0.0, _spatium*1.5, _spatium*3);
      p.drawLine(_spatium*1.5, _spatium*3, _spatium*3, 0.0);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TremoloBar::write(Xml& xml) const
      {
      xml.stag("TremoloBar");
      foreach(const PitchValue& v, _points) {
            xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
               .arg(v.time).arg(v.pitch).arg(v.vibrato));
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TremoloBar::read(QDomElement e)
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

bool TremoloBar::genPropertyMenu(QMenu* popup) const
      {
      QAction* a;
      a = popup->addAction(tr("TremoloBar Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TremoloBar::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            TremoloBarProperties bp(this, 0);
            if (bp.exec()) {
                  score()->undo()->push(new ChangeTremoloBar(this, bp.points()));
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   TremoloBarProperties
//---------------------------------------------------------

TremoloBarProperties::TremoloBarProperties(TremoloBar* b, QWidget* parent)
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

const QList<PitchValue>& TremoloBarProperties::points() const
      {
      return bendCanvas->points();
      }

//---------------------------------------------------------
//   bendTypeChanged
//---------------------------------------------------------

void TremoloBarProperties::bendTypeChanged(int n)
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
//   TremoloBarCanvas
//---------------------------------------------------------

TremoloBarCanvas::TremoloBarCanvas(QWidget* parent)
   : QFrame(parent)
      {
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void TremoloBarCanvas::paintEvent(QPaintEvent* ev)
      {
      int w = width();
      int h = height();

      QPainter p(this);
      p.fillRect(rect(), Qt::white);

      static const int ROWS    = 25;
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
            pen.setColor(y % 2 ? Qt::gray : Qt::black);
            pen.setWidth(y == ROWS/2 ? 3 : 1);
            p.setPen(pen);
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

void TremoloBarCanvas::mousePressEvent(QMouseEvent* ev)
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


