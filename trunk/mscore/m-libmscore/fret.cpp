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

#include <math.h>

#include "fret.h"
#include "measure.h"
#include "system.h"
#include "score.h"
#include "preferences.h"
#include "tablature.h"
#include "chord.h"
#include "note.h"
#include "segment.h"
#include "m-al/xml.h"
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
      font.setSize(4.0 * DPI * mag()/ PPI);
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
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      qreal yp = y();
      if (system)
            yp += system->staffY(staffIdx());
      return QPointF(xp, yp);
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

void FretDiagram::draw(Painter* /*p*/) const
      {
#if 0
      qreal _spatium = spatium();
      p->setPenWidth(lw2);
      p->setLineCap(Qt::FlatCap);
      p->setBrush(pen.color());
      qreal x2 = (_strings-1) * stringDist;
      p->drawLine(QLineF(-lw1*.5, 0.0, x2+lw1*.5, 0.0));

      p->setPenWidth(lw1);
      qreal y2 = (_frets+1) * fretDist - fretDist*.5;
      for (int i = 0; i < _strings; ++i) {
            qreal x = stringDist * i;
            p->drawLine(QLineF(x, _fretOffset ? -_spatium*.2 : 0.0, x, y2));
            }
      for (int i = 1; i <= _frets; ++i) {
            qreal y = fretDist * i;
            p->drawLine(QLineF(0.0, y, x2, y));
            }
      for (int i = 0; i < _strings; ++i) {
            if (_dots && _dots[i]) {
                  qreal dotd = stringDist * .6;
                  int fret = _dots[i] - 1;
                  qreal x = stringDist * i - dotd * .5;
                  qreal y = fretDist * fret + fretDist * .5 - dotd * .5;
                  p->drawEllipse(QRectF(x, y, dotd, dotd));
                  }
            if (_marker && _marker[i]) {
                  p.setFont(font);
                  qreal x = stringDist * i;
                  qreal y = -fretDist * .1;
                  p->drawText(QRectF(x, y, 0.0, 0.0),
                     Qt::AlignHCenter | Qt::AlignBottom | Qt::TextDontClip, QChar(_marker[i]));
                  }
            }
      if (_fretOffset > 0) {
            p->drawText(QRectF(-stringDist * .4, 0.0, 0.0, fretDist),
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
      qreal _spatium = spatium();
      lw1             = _spatium * 0.08;
      lw2             = _fretOffset ? lw1 : _spatium * 0.2;
      stringDist      = _spatium * .7;
      fretDist        = _spatium * .8;

      qreal w = stringDist * (_strings-1);
      qreal h = _frets * fretDist + fretDist * .5;
      qreal y = 0.0;
      qreal dotd = stringDist * .6;
      qreal x = -((dotd+lw1) * .5);
      w += dotd + lw1;
      if (_marker) {
            FontMetricsF fm(font);
            y = -(fretDist * .1 + fm.height());
            h -= y;
            }
      setbbox(QRectF(x, y, w, h));
//      Element::layout();      // alignment & offset
//      setPos(ipos() + QPointF(-w * .5, - (h + _spatium * 1.5)));
      setPos(0.0, 0.0);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FretDiagram::read(XmlReader* r)
      {
      delete _dots;
      delete _marker;
      delete _fingering;
      _dots       = 0;
      _marker     = 0;
      _fingering  = 0;
      _fretOffset = 0;

      while (r->readElement()) {
            if (r->readInt("strings", &_strings))
                  ;
            else if (r->readInt("frets", &_frets))
                  ;
            else if (r->readInt("fretOffset", &_fretOffset))
                  ;
            else if (r->tag() == "string") {
                  int no = 0;
                  while (r->readAttribute()) {
                        if (r->tag() == "no")
                              no = r->intValue();
                        }
                  while (r->readElement()) {
                        int i;
                        if (r->readInt("dot", &i))
                              setDot(no, i);
                        else if (r->readInt("marker", &i))
                              setMarker(no, i);
                        else if (r->readInt("fingering", &i))
                              setFingering(no, i);
                        else
                              r->unknown();
                        }
                  }
            else if (!Element::readProperties(r))
                  r->unknown();
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

