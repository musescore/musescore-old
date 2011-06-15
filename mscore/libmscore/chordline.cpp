//=============================================================================
//  MuseScore
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

#include "chordline.h"
#include "xml.h"
#include "chord.h"
#include "measure.h"
#include "system.h"
#include "note.h"
#include "painter.h"

//---------------------------------------------------------
//   ChordLine
//---------------------------------------------------------

ChordLine::ChordLine(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      modified = false;
      }

ChordLine::ChordLine(const ChordLine& cl)
   : Element(cl)
      {
      path     = cl.path;
      modified = cl.modified;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void ChordLine::setSubtype(int st)
      {
      double x2, y2;
      switch(st) {
            case 0:
                  break;
            case 1:                 // fall
                  x2 = 2;
                  y2 = 2;
                  break;
            default:
            case 2:                 // doit
                  x2 = 2;
                  y2 = -2;
                  break;
            }
      if (st) {
            path = QPainterPath();
            path.cubicTo(x2/2, 0.0, x2, y2/2, x2, y2);
            }
      Element::setSubtype(st);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void ChordLine::layout()
      {
      double _spatium = spatium();
      if (parent()) {
            Note* note = chord()->upNote();
            QPointF p(note->pos());
            setPos(p.x() + note->headWidth() + _spatium * .2, p.y());
            }
      else
            setPos(0.0, 0.0);
      QRectF r(path.boundingRect());
      setbbox(QRectF(r.x() * _spatium, r.y() * _spatium, r.width() * _spatium, r.height() * _spatium));
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordLine::read(QDomElement e)
      {
      path = QPainterPath();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "Path") {
                  path = QPainterPath();
                  QPointF curveTo;
                  QPointF p1;
                  int state;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        if (tag == "Element") {
                              int type = ee.attribute("type").toInt();
                              double x = ee.attribute("x").toDouble();
                              double y = ee.attribute("y").toDouble();
                              switch(QPainterPath::ElementType(type)) {
                                    case QPainterPath::MoveToElement:
                                          path.moveTo(x, y);
                                          break;
                                    case QPainterPath::LineToElement:
                                          path.lineTo(x, y);
                                          break;
                                    case QPainterPath::CurveToElement:
                                          curveTo.rx() = x;
                                          curveTo.ry() = y;
                                          state = 1;
                                          break;
                                    case QPainterPath::CurveToDataElement:
                                          if (state == 1) {
                                                p1.rx() = x;
                                                p1.ry() = y;
                                                state = 2;
                                                }
                                          else if (state == 2) {
                                                path.cubicTo(curveTo, p1, QPointF(x, y));
                                                }
                                          break;
                                    }
                              }
                        else
                              domError(ee);
                        }
                  modified = true;
                  setSubtype(0);
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordLine::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      if (modified) {
            int n = path.elementCount();
            xml.stag("Path");
            for (int i = 0; i < n; ++i) {
                  const QPainterPath::Element& e = path.elementAt(i);
                  xml.tagE(QString("Element type=\"%1\" x=\"%2\" y=\"%3\"")
                     .arg(int(e.type)).arg(e.x).arg(e.y));
                  }
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void ChordLine::draw(Painter* painter) const
      {
      double _spatium = spatium();
      painter->scale(_spatium);
      double lw = 0.15;
      painter->setLineWidth(lw);
      painter->setCapStyle(Qt::RoundCap);
      painter->setJoinStyle(Qt::RoundJoin);
      painter->setNoBrush(true);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void ChordLine::editDrag(const EditData& ed)
      {
      int n = path.elementCount();
      QPainterPath p;
      double sp = spatium();
      double dx = ed.delta.x() / sp;
      double dy = ed.delta.y() / sp;
      for (int i = 0; i < n; ++i) {
            const QPainterPath::Element& e = path.elementAt(i);
            double x = e.x;
            double y = e.y;
            if (ed.curGrip == i) {
                  x += dx;
                  y += dy;
                  }
            switch(e.type) {
                  case QPainterPath::CurveToDataElement:
                        break;
                  case QPainterPath::MoveToElement:
                        p.moveTo(x, y);
                        break;
                  case QPainterPath::LineToElement:
                        p.lineTo(x, y);
                        break;
                  case QPainterPath::CurveToElement:
                        {
                        double x2 = path.elementAt(i+1).x;
                        double y2 = path.elementAt(i+1).y;
                        double x3 = path.elementAt(i+2).x;
                        double y3 = path.elementAt(i+2).y;
                        if (i + 1 == ed.curGrip) {
                              x2 += dx;
                              y2 += dy;
                              }
                        else if (i + 2 == ed.curGrip) {
                              x3 += dx;
                              y3 += dy;
                              }
                        p.cubicTo(x, y, x2, y2, x3, y3);
                        i += 2;
                        }
                        break;
                  }
            }
      path = p;
      modified = true;
      setSubtype(0);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void ChordLine::updateGrips(int* grips, QRectF* grip) const
      {
      int n = path.elementCount();
      *grips = n;
      QPointF cp(canvasPos());
      double sp = spatium();
      for (int i = 0; i < n; ++i)
            grip[i].translate(cp + QPointF(path.elementAt(i).x * sp, path.elementAt(i).y * sp));
      }

