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

#include "fret.h"
#include "measure.h"
#include "system.h"

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::FretDiagram(Score* score)
   : Element(score)
      {
      _strings   = 6;
      _frets     = 4;
      _dots      = 0;
      _marker    = 0;
      _fingering = 0;
      }

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::~FretDiagram()
      {
      delete _dots;
      delete _marker;
      delete _fingering;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(QPainter& p, ScoreView*) const
      {
      QPen pen(p.pen());
      pen.setWidthF(lw2);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);
      p.setBrush(pen.color());
      double x2 = (_strings-1) * stringDist;
      p.drawLine(QLineF(-lw1*.5, 0.0, x2+lw1*.5, 0.0));

      pen.setWidthF(lw1);
      p.setPen(pen);
      double y2 = (_frets+1) * fretDist - fretDist*.5;
      for (int i = 0; i < _strings; ++i) {
            double x = stringDist * i;
            p.drawLine(QLineF(x, 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            double y = fretDist * i;
            p.drawLine(QLineF(0.0, y, x2, y));
            }
      for (int i = 0; i < _strings; ++i) {
            if (_dots && _dots[i]) {
                  double dotd = stringDist * .6;
                  int fret = _dots[i] - 1;
                  double x = stringDist * i - dotd * .5;
                  double y = fretDist * fret + fretDist * .5 - dotd * .5;
                  p.drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            if (_marker && _marker[i]) {
                  QFont f("DejaVuSans");
                  int size = lrint(4.0 * DPI * mag()/ PPI);
                  f.setPixelSize(size);
                  p.setFont(f);
                  double x = stringDist * i;
                  double y = -fretDist * .1;
                  p.drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QChar(_marker[i]));
                  }
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
      {
      double _spatium = spatium();
      lw1 = _spatium * 0.08;
      lw2 = _spatium * 0.2;
      stringDist = _spatium * .7;
      fretDist   = _spatium * .8;

      setbbox(QRectF(0.0, 0.0, stringDist * (_strings-1), (_frets+1) * fretDist));
      Element::layout();      // alignment & offset
      Measure* m = static_cast<Measure*>(parent());
      double yy = track() < 0 ? 0.0 : m->system()->staff(track() / VOICES)->y();
      yy -= _bbox.height() + _spatium * 1.5;
      double xx = (tick() < 0) ? 0.0 : m->tick2pos(tick());

      setPos(ipos() + QPointF(xx, yy));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FretDiagram::write(Xml& xml) const
      {
      xml.stag("FretDiagram");
      if (_strings != 6)
            xml.tag("strings", _strings);
      if (_frets != 4)
            xml.tag("frets", _frets);
      for (int i = 0; i < _strings; ++i) {
            if ((_dots && _dots[i]) || (_marker && _marker[i]) || (_fingering && _fingering[i])) {
                  xml.stag(QString("string no=\"%1\"").arg(i));
                  if (_dots && _dots[i])
                        xml.tag("dot", _dots[i]);
                  if (_marker && _marker[i])
                        xml.tag("marker", _marker[i]);
                  if (_fingering && _fingering[i])
                        xml.tag("fingering", _fingering[i]);
                  xml.etag();
                  }
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FretDiagram::read(QDomElement e)
      {
      delete _dots;
      delete _marker;
      delete _fingering;
      _dots      = 0;
      _marker    = 0;
      _fingering = 0;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "strings")
                  _strings = val;
            else if (tag == "frets")
                  _frets = val;
            else if (tag == "string") {
                  int no = e.attribute("no").toInt();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        int val = ee.text().toInt();
                        if (tag == "dot")
                              setDot(no, val);
                        else if (tag == "marker")
                              setMarker(no, val);
                        else if (tag == "fingering")
                              setFingering(no, val);
                        else
                              domError(ee);
                        }
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setDot
//---------------------------------------------------------

void FretDiagram::setDot(int string, int fret)
      {
      if (_dots == 0) {
            _dots = new char[_strings];
            memset(_dots, 0, _strings);
            }
      _dots[string] = fret;
      }

//---------------------------------------------------------
//   setMarker
//---------------------------------------------------------

void FretDiagram::setMarker(int string, int marker)
      {
      if (_marker == 0) {
            _marker = new char[_strings];
            memset(_marker, 0, _strings);
            }
      _marker[string] = marker;
      }

//---------------------------------------------------------
//   setFingering
//---------------------------------------------------------

void FretDiagram::setFingering(int string, int finger)
      {
      if (_fingering == 0) {
            _fingering = new char[_strings];
            memset(_fingering, 0, _strings);
            }
      _fingering[string] = finger;
      }

