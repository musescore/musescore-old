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

#include "fret.h"
#include "measure.h"
#include "system.h"
#include "score.h"
#include "preferences.h"
#include "tablature.h"
#include "chord.h"
#include "note.h"
#include "segment.h"
#include "painter.h"

static const int DEFAULT_STRINGS = 6;
static const int DEFAULT_FRETS = 5;

//---------------------------------------------------------
//   FretDiagram
//---------------------------------------------------------

FretDiagram::FretDiagram(Score* score)
   : Element(score)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _strings    = DEFAULT_STRINGS;
      _frets      = DEFAULT_FRETS;
      _maxFrets   = 24;
      maxStrings  = 0;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;
      font.setFamily("FreeSans");
      int size = lrint(4.0 * DPI * mag()/ PPI);
      font.setPixelSize(size);
      }

FretDiagram::FretDiagram(const FretDiagram& f)
   : Element(f)
      {
      _strings    = f._strings;
      _frets      = f._frets;
      _fretOffset = f._fretOffset;
      _maxFrets   = f._maxFrets;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      font        = f.font;

      if (f._dots) {
            _dots = new char[_strings];
            memcpy(_dots, f._dots, _strings);
            }
      if (f._marker) {
            _marker = new char[_strings];
            memcpy(_marker, f._marker, _strings);
            }
      if (f._fingering) {
            _fingering = new char[_strings];
            memcpy(_fingering, f._fingering, _strings);
            }
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
//   canvasPos
//---------------------------------------------------------

QPointF FretDiagram::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      double yp = y();
      if (system)
            yp += system->staffY(staffIdx());
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF FretDiagram::dragAnchor() const
      {
      Measure* m     = measure();
      System* system = m->system();
      double yp      = system->staff(staffIdx())->y() + system->y();
      double xp      = m->tick2pos(segment()->tick()) + m->canvasPos().x();
      QPointF p1(xp, yp);

      double x  = 0.0;
      double y  = 0.0;
#if 0 // TODOxx
      double tw = width();
      double th = height();
      if (_align & ALIGN_BOTTOM)
            y = th;
      else if (_align & ALIGN_VCENTER)
            y = (th * .5);
      else if (_align & ALIGN_BASELINE)
            y = baseLine();
      if (_align & ALIGN_RIGHT)
            x = tw;
      else if (_align & ALIGN_HCENTER)
            x = (tw * .5);
#endif
      return QLineF(p1, abbox().topLeft() + QPointF(x, y));
      }

//---------------------------------------------------------
//   setStrings
//---------------------------------------------------------

void FretDiagram::setStrings(int n)
      {
      if (n <= maxStrings) {
            _strings = n;
            return;
            }
      maxStrings = n;
      if (_dots) {
            char* ndots = new char[n];
            memcpy(ndots, _dots, _strings);
            for (int i = _strings; i < n; ++i)
                  ndots[i] = 0;
            delete _dots;
            _dots = ndots;
            }
      if (_marker) {
            char* nmarker = new char[n];
            memcpy(nmarker, _marker, _strings);
            for (int i = _strings; i < n; ++i)
                  nmarker[i] = 0;
            delete _marker;
            _marker = nmarker;
            }
      if (_fingering) {
            char* nfingering = new char[n];
            memcpy(nfingering, _fingering, _strings);
            for (int i = _strings; i < n; ++i)
                  nfingering[i] = 0;
            delete _fingering;
            _fingering = nfingering;
            }
      _strings = n;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void FretDiagram::init(Tablature* tab, Chord* chord)
      {
      if (tab == 0)
            setStrings(6);
      else
            setStrings(tab->strings());
      if (tab) {
            for (int string = 0; string < _strings; ++string)
                  _marker[string] = 'X';
            foreach(const Note* note, chord->notes()) {
                  int string;
                  int fret;
                  if (tab->convertPitch(note->ppitch(), &string, &fret))
                        setDot(string, fret);
                  }
            _maxFrets = tab->frets();
            }
      else
            _maxFrets = 6;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FretDiagram::draw(Painter* painter) const
      {
      double _spatium = spatium();
      painter->setLineWidth(lw2);
      painter->setCapStyle(Qt::FlatCap);
      painter->setBrushColor(painter->penColor());
      double x2 = (_strings-1) * stringDist;
      painter->drawLine(-lw1*.5, 0.0, x2+lw1*.5, 0.0);

      painter->setLineWidth(lw1);
      double y2 = (_frets+1) * fretDist - fretDist*.5;
      for (int i = 0; i < _strings; ++i) {
            double x = stringDist * i;
            painter->drawLine(x, _fretOffset ? -_spatium*.2 : 0.0, x, y2);
            }
      for (int i = 1; i <= _frets; ++i) {
            double y = fretDist * i;
            painter->drawLine(0.0, y, x2, y);
            }
      for (int i = 0; i < _strings; ++i) {
            if (_dots && _dots[i]) {
                  double dotd = stringDist * .6;
                  int fret = _dots[i] - 1;
                  double x = stringDist * i - dotd * .5;
                  double y = fretDist * fret + fretDist * .5 - dotd * .5;
                  painter->drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            if (_marker && _marker[i]) {
                  painter->setFont(font);
#if 0 // TODO-LIB
                  double x = stringDist * i;
                  double y = -fretDist * .1;
                  painter->drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QChar(_marker[i]));
#endif
                  }
            }
#if 0 // TODO-LIB
      if (_fretOffset > 0) {
            p.drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
               Qt::AlignVCenter|Qt::AlignRight|Qt::TextDontClip,
               QString("%1").arg(_fretOffset+1));
            }
#endif
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FretDiagram::layout()
      {
      double _spatium = spatium();
      lw1             = _spatium * 0.08;
      lw2             = _fretOffset ? lw1 : _spatium * 0.2;
      stringDist      = _spatium * .7;
      fretDist        = _spatium * .8;

      double w = stringDist * (_strings-1);
      double h = _frets * fretDist + fretDist * .5;
      double y = 0.0;
      double dotd = stringDist * .6;
      double x = -((dotd+lw1) * .5);
      w += dotd + lw1;
      if (_marker) {
            QFontMetricsF fm(font);
            y = -(fretDist * .1 + fm.height());
            h -= y;
            }
      setbbox(QRectF(x, y, w, h));
//      Element::layout();      // alignment & offset
//      setPos(ipos() + QPointF(-w * .5, - (h + _spatium * 1.5)));
      setPos(0.0, 0.0);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FretDiagram::write(Xml& xml) const
      {
      xml.stag("FretDiagram");
      Element::writeProperties(xml, 0);

      if (_strings != DEFAULT_STRINGS)
            xml.tag("strings", _strings);
      if (_frets != DEFAULT_FRETS)
            xml.tag("frets", _frets);
      if (_fretOffset)
            xml.tag("fretOffset", _fretOffset);
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
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "strings")
                  _strings = val;
            else if (tag == "frets")
                  _frets = val;
            else if (tag == "fretOffset")
                  _fretOffset = val;
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
      setMarker(string, 0);
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

