//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "timesig.h"
#include "undo.h"
#include "xml.h"
#include "score.h"
#include "style.h"
#include "sym.h"
#include "symbol.h"
#include "system.h"
#include "segment.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "painter.h"

//---------------------------------------------------------
//   TimeSig
//---------------------------------------------------------

TimeSig::TimeSig(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _showCourtesySig = true;
      customText = false;
      _stretch.set(1, 1);
      }

TimeSig::TimeSig(Score* s, int st)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _showCourtesySig = true;
      customText = false;
      _stretch.set(1, 1);
      setSubtype(st);
      }

TimeSig::TimeSig(Score* s, int z, int n)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _showCourtesySig = true;
      customText = false;
      _stretch.set(1, 1);
      setSig(Fraction(z, n));
      setSubtype(TSIG_NORMAL);
      }

TimeSig::TimeSig(Score* s, const Fraction& f)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _showCourtesySig = true;
      customText = false;
      _stretch.set(1, 1);
      setSig(f);
      setSubtype(TSIG_NORMAL);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF TimeSig::pagePos() const
      {
      if (parent() == 0)
            return pos();
      System* system = segment()->measure()->system();
      qreal yp = system ? y() + system->staff(staffIdx())->y() + system->y() : 0.0;
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void TimeSig::setSubtype(int st)
      {
      switch(st) {
            case TSIG_NORMAL:
                  break;
            case TSIG_FOUR_FOUR:
                  setSig(Fraction(4, 4));
                  customText = false;
                  break;
            case TSIG_ALLA_BREVE:
                  setSig(Fraction(2, 2));
                  customText = false;
                  break;
            default:
                  printf("illegal TimeSig subtype 0x%x\n", st);
                  break;
            }
      Element::setSubtype(st);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TimeSig::acceptDrop(MuseScoreView*, const QPointF&, int type, int) const
      {
      return type == TIMESIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* TimeSig::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() == TIMESIG) {
            // change timesig applies to all staves, can't simply set subtype
            // for this one only
            score()->cmdAddTimeSig(measure(), staffIdx(), static_cast<TimeSig*>(e));
            }
      else
            delete e;
      return 0;
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TimeSig::setText(const QString& a, const QString& b)
      {
      sz = a;
      sn = b;
      customText = true;
      }

//---------------------------------------------------------
//   setActualSig
//---------------------------------------------------------

void TimeSig::setActualSig(const Fraction& actual)
      {
      _stretch = (_nominal / actual).reduced();
      printf("setActual %d/%d  stretch %d/%d\n",
        actual.numerator(), actual.denominator(),
        _stretch.numerator(), _stretch.denominator());
      }

//---------------------------------------------------------
//   write TimeSig
//---------------------------------------------------------

void TimeSig::write(Xml& xml) const
      {
      xml.stag("TimeSig");
      Element::writeProperties(xml);

      xml.tag("sigN",   _nominal.numerator());
      xml.tag("sigD",  _nominal.denominator());
      if (_stretch != Fraction(1,1)) {
            xml.tag("stretchN", _stretch.numerator());
            xml.tag("stretchD", _stretch.denominator());
            }
      if (customText) {
            xml.tag("textN", sz);
            xml.tag("textD", sn);
            }
      xml.tag("showCourtesySig", _showCourtesySig);
      xml.etag();
      }

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(QDomElement e)
      {
      int n=0, z1=0, z2=0, z3=0, z4=0;
      bool old = false;

      customText = false;
      _stretch.set(1, 1);

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "den") {
                  old = true;
                  n = val;
                  }
            else if (tag == "nom1") {
                  old = true;
                  z1 = val;
                  }
            else if (tag == "nom2") {
                  old = true;
                  z2 = val;
                  }
            else if (tag == "nom3") {
                  old = true;
                  z3 = val;
                  }
            else if (tag == "nom4") {
                  old = true;
                  z4 = val;
                  }
            else if (tag == "subtype") {
                  if (score()->mscVersion() < 122) {
                        setSig(Fraction(
                           ((val>>24)& 0x3f)
                           + ((val>>18)& 0x3f)
                           + ((val>>12)& 0x3f)
                           + ((val>>6) & 0x3f), val & 0x3f));
                        val = TSIG_NORMAL;
                        }
                  setSubtype(val);
                  }
            else if (tag == "showCourtesySig")
                  _showCourtesySig = e.text().toInt();
            else if (tag == "sigN")
                  _nominal.setNumerator(val);
            else if (tag == "sigD")
                  _nominal.setDenominator(val);
            else if (tag == "stretchN")
                  _stretch.setNumerator(val);
            else if (tag == "stretchD")
                  _stretch.setDenominator(val);
            else if (tag == "textN") {
                  customText = true;
                  sz = e.text();
                  }
            else if (tag == "textD") {
                  customText = true;
                  sn = e.text();
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (old) {
            _nominal.set(z1+z2+z3+z4, n);
            customText = false;
            if (subtype() == 0x40000104)
                  setSubtype(TSIG_FOUR_FOUR);
            else if (subtype() == 0x40002084)
                  setSubtype(TSIG_ALLA_BREVE);
            else
                  setSubtype(TSIG_NORMAL);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TimeSig::layout()
      {
      qreal _spatium = spatium();

      setbbox(QRectF());                          // prepare for an empty time signature
      pz = QPointF(0.0, 0.0);
      pn = QPointF(0.0, 0.0);

      int st            = subtype();
      qreal lineDist   = 1.0;            // assume dimensions a standard staff
      int    numOfLines = 5;
      if (staff()) {
            StaffType* staffType = staff()->staffType();
            numOfLines  = staff()->staffType()->lines();
            lineDist    = staff()->staffType()->lineDistance().val();

            // if tablature, but without time sig, set empty symbol
            if ((staffType->group() == TAB_STAFF) &&
               !(static_cast<StaffTypeTablature*>(staffType)->genTimesig())) {
                  st = 0;
                  }
            }

      // if some symbol
      // compute vert. displacement to center in the staff height
      // determine middle staff position:
      qreal yoff = _spatium * (numOfLines-1) / 2.0 * lineDist;
      qreal mag  = magS();
      // C and Ccut are placed at the middle of the staff: use yoff directly
      if (st ==  TSIG_FOUR_FOUR) {
            pz = QPointF(0.0, yoff);
            Sym& sym = symbols[score()->symIdx()][fourfourmeterSym];
            setbbox(sym.bbox(mag).translated(pz));
            sz = sym.toString();
            sn.clear();
            }
      else if (st == TSIG_ALLA_BREVE) {
            pz = QPointF(0.0, yoff);
            Sym& sym = symbols[score()->symIdx()][allabreveSym];
            setbbox(sym.bbox(mag).translated(pz));
            sz = sym.toString();
            sn.clear();
            }
      else {
            if (!customText) {
                  Fraction f(actualSig());
                  sz = QString("%1").arg(f.numerator());   // build numerator string
                  sn = QString("%1").arg(f.denominator()); // build denominator string
                  }
            QFontMetricsF fm(fontId2font(0));
            QRectF rz = fm.tightBoundingRect(sz);     // get 'tight' bounding boxes for strings
            QRectF rn = fm.tightBoundingRect(sn);

            // scale bounding boxes to mag
            rz = QRectF(rz.x() * mag, rz.y() * mag, rz.width() * mag, rz.height() * mag);
            rn = QRectF(rn.x() * mag, rn.y() * mag, rn.width() * mag, rn.height() * mag);

            // position numerator and denominator; vertical displacement:
            // number of lines is odd: 0.0 (strings are directly above and below the middle line)
            // number of lines even:   0.5 (strings are moved up/down to leave 1 line dist. between them)
            qreal displ = numOfLines & 1 ? 0.0 : (0.5 * lineDist * _spatium);
            pz = QPointF(0.0, yoff - displ);
            // denom. horiz. posit.: centred around centre of numerator
            // vert. position:       base line is lowered by displ and by the whole height of a digit
            pn = QPointF((rz.width() - rn.width())*.5, yoff + displ + _spatium*2.0);
            setbbox(rz.translated(pz));   // translate bounding boxes to actual string positions
            addbbox(rn.translated(pn));
            }

      qreal im = (DPI * SPATIUM20) / _spatium;

      pz *= im;                           // convert positions to raster units
      pn *= im;
      // adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(Painter* painter) const
      {
      if (staff() && !staff()->staffType()->genTimesig())
            return;
      painter->setFont(symbols[score()->symIdx()][allabreveSym].font());
      qreal m  = spatium() / (DPI * SPATIUM20);
      painter->scale(m);
      painter->drawText(pz, sz);    // use positions and strings computed in layout()
      painter->drawText(pn, sn);
      painter->scale(1.0 / m);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space TimeSig::space() const
      {
      return Space(point(score()->styleS(ST_timesigLeftMargin)), width());
      }

//---------------------------------------------------------
//   setFrom
//---------------------------------------------------------

void TimeSig::setFrom(const TimeSig* ts)
      {
      Element::setSubtype(ts->subtype());
      sz         = ts->sz;
      sn         = ts->sn;
      _nominal   = ts->_nominal;
      _stretch   = ts->_stretch;
      customText = ts->customText;
      }
