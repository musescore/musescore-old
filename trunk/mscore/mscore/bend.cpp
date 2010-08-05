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

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

Bend::Bend(Score* s)
   : Element(s)
      {
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
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   BendProperties
//---------------------------------------------------------

BendProperties::BendProperties(Bend*, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
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

      QPen pen = p.pen();
      pen.setWidth(1);
      p.setPen(pen);
      for (int x = 0; x < COLUMNS; ++x) {
            int xx = lm + x * xs;
            p.drawLine(xx, tm, xx, tm + (ROWS-1) * ys);
            }

      for (int y = 0; y < ROWS; ++y) {
            int yy = tm + y * ys;
            p.drawLine(lm, yy, lm + (COLUMNS-1) * xs, yy);
            }

      QFrame::paintEvent(ev);
      }

