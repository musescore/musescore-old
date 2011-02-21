//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "timesig.h"
#include "undo.h"
#include "xml.h"
#include "score.h"
#include "style.h"
#include "sym.h"
#include "symbol.h"
#include "scoreview.h"
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
      _showCourtesySig = true;
      }

TimeSig::TimeSig(Score* s, int st)
  : Element(s)
      {
      _showCourtesySig = true;
      setSubtype(st);
      }

TimeSig::TimeSig(Score* s, int n, int z1, int z2, int z3, int z4)
  : Element(s)
      {
      _showCourtesySig = true;
      setSig(n, z1, z2, z3, z4);
      }

TimeSig::TimeSig(Score* s, const Fraction& f)
   : Element(s)
      {
      _showCourtesySig = true;
      setSig(f.denominator(), f.numerator(), 0, 0, 0);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF TimeSig::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = segment()->measure()->system();
      double yp = system ? y() + system->staff(staffIdx())->y() + system->y() : 0.0;
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void TimeSig::setSubtype(int val)
      {
      Element::setSubtype(val);
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void TimeSig::setSig(int n, int z1, int z2, int z3, int z4)
      {
      setSubtype(sigtype(n, z1, z2, z3, z4));
      }

//---------------------------------------------------------
//   getSig
//---------------------------------------------------------

void TimeSig::getSig(int* n, int* z1, int* z2, int* z3, int* z4) const
      {
      int st = subtype();
      *n = st & 0x3f;
      if (z4)
            *z4 = (st>>24)& 0x3f;
      if (z3)
            *z3 = (st>>18)& 0x3f;
      if (z2)
            *z2 = (st>>12)& 0x3f;
      *z1 = (st>>6) & 0x3f;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TimeSig::acceptDrop(ScoreView*, const QPointF&, int type, int) const
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
            score()->cmdAddTimeSig(measure(), e->subtype());
            }
      delete e;
      return 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool TimeSig::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      int _track = track();
      // if the time sig. is not generated (= not courtesy) and is in track 0
      // add the specific menu item
      if (!generated() && !_track) {
            QAction* a = popup->addAction(_showCourtesySig
               ? QT_TRANSLATE_NOOP("TimeSig", "Hide Courtesy Time Signature")
               : QT_TRANSLATE_NOOP("TimeSig", "Show Courtesy Time Signature") );
            a->setData("courtesy");
            }
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TimeSig::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "courtesy")
            score()->undo()->push(new ChangeTimesig(this, !_showCourtesySig));
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   write TimeSig
//---------------------------------------------------------

void TimeSig::write(Xml& xml) const
      {
      xml.stag("TimeSig");
      Element::writeProperties(xml);
      int n, z1, z2, z3, z4;
      getSig(&n, &z1, &z2, &z3, &z4);

      xml.tag("den", n);
      xml.tag("nom1", z1);
      if (z2) {
            xml.tag("nom2", z2);
            if (z3) {
                  xml.tag("nom3", z3);
                  if (z4)
                        xml.tag("nom4", z4);
                  }
            }
      xml.tag("showCourtesySig", _showCourtesySig);
      xml.etag();
      }

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(QDomElement e)
      {
      setSubtype(-1);
      int n=0, z1=0, z2=0, z3=0, z4=0;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "den")
                  n = val;
            else if (tag == "nom1")
                  z1 = val;
            else if (tag == "nom2")
                  z2 = val;
            else if (tag == "nom3")
                  z3 = val;
            else if (tag == "nom4")
                  z4 = val;
            else if (tag == "showCourtesySig")
                  _showCourtesySig = e.text().toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      //
      // silently accept old values:
      //
      switch(subtype()) {
            case 0:   setSig(2, 2); break;
            case 1:   setSig(4, 2); break;
            case 2:   setSig(4, 3); break;
            case 3:   setSig(4, 4); break;
            case 4:   setSig(4, 5); break;
            case 5:   setSig(4, 6); break;
            case 6:   setSig(8, 3); break;
            case 7:   setSig(8, 6); break;
            case 8:   setSig(8, 12); break;
            case 9:   setSubtype(TSIG_FOUR_FOUR); break;
            case 10:  setSubtype(TSIG_ALLA_BREVE); break;
            case 11:  setSig(8, 9); break;

            case TSIG_FOUR_FOUR:    // special cases
            case TSIG_ALLA_BREVE:
                  break;
            default:
                  setSig(n, z1, z2, z3, z4);
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TimeSig::layout()
      {
      double _spatium = spatium();
      QRectF bb;

      bb = QRectF(0, 0, 0, 0);                  // prepare for an empty time signature
      sz = QString();
      sn = QString();
      pz = QPointF(0.0, 0.0);
      pn = QPointF(0.0, 0.0);

      int st = subtype();
      // if tablature, but without time sig, set empty symbol
      if (staff() && staff()->useTablature() &&
                  !((StaffTypeTablature*)staff()->staffType())->genTimesig())
            st = 0;
      if (st == 0)                              // no symbol: nothing to do
            return;
      else {                                    // if some symbol
            // compute vert. displacement to center in the staff height
            double yoff;
            double lineDist   = 1.0;            // assume dimensions a standard staff
            int    numOfLines = 5;
            if (staff()) {                      // if some staff, adjust to actual staff dimensions
                  numOfLines  = staff()->staffType()->lines();
                  lineDist    = staff()->staffType()->lineDistance().val();
                  }
            yoff = (numOfLines-1) / 2.0 * lineDist;         // determine middle staff position
            yoff *= _spatium;

            // C and Ccut are placed at the middle of the staff: use yoff directly
            if (st ==  TSIG_FOUR_FOUR) {
                  pz = QPointF(0.0, yoff);
                  bb = symbols[score()->symIdx()][fourfourmeterSym].bbox(magS()).translated(pz);
                  sz = symbols[score()->symIdx()][fourfourmeterSym].toString();
                   }
            else if (st == TSIG_ALLA_BREVE) {
                  pz = QPointF(0.0, yoff);
                  bb = symbols[score()->symIdx()][allabreveSym].bbox(magS()).translated(pz);
                  sz = symbols[score()->symIdx()][allabreveSym].toString();
                  }
            else {
                  // other time signatures are made of a numerator (z1...z4) and a denominator (n)
                  int n, z1, z2, z3, z4;
                  getSig(&n, &z1, &z2, &z3, &z4);
                  sz = QString("%1").arg(z1);               // build numerator string
                  if (z2)
                        sz += QString("+%1").arg(z2);
                  if (z3)
                        sz += QString("+%1").arg(z3);
                  if (z4)
                        sz += QString("+%1").arg(z4);
                  sn = QString("%1").arg(n);                // build denominator string

                  QFontMetricsF fm(fontId2font(0));
                  QRectF rz = fm.tightBoundingRect(sz);     // get 'tight' bounding boxes for strings
                  QRectF rn = fm.tightBoundingRect(sn);

                  double m  = magS();

                  // scale bounding boxes to mag
                  rz = QRectF(rz.x() * m, rz.y() * m, rz.width() * m, rz.height() * m);
                  rn = QRectF(rn.x() * m, rn.y() * m, rn.width() * m, rn.height() * m);

                  // position numerator and denominator; vertical displacement:
                  // number of lines is odd: 0.0 (strings are directly above and below the middle line)
                  // number of lines even:   0.5 (strings are moved up/down to leave 1 line dist. between them)
                  double displ = numOfLines & 1 ? 0.0 : (0.5 * lineDist * _spatium);
                  pz = QPointF(0.0, yoff - displ);
                  // denom. horiz. posit.: centred around centre of numerator
                  // vert. position:       base line is lowered by displ and by the whole height of a digit
                  pn = QPointF((rz.width() - rn.width())*.5, yoff + displ + _spatium*2.0);
                  bb |= rz.translated(pz);                  // translate bounding boxes to actual string positions
                  bb |= rn.translated(pn);
                  }

            double im = (DPI * SPATIUM20) / _spatium;

            pz *= im;                           // convert positions to raster units
            pn *= im;
            }
      setbbox(bb);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(Painter* painter) const
      {
      int st = subtype();
      if (st == 0)                              // if no symbol, do nothing
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


