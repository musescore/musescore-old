//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

Element* TimeSig::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      if (e->type() == TIMESIG) {
            // change timesig applies to all staves, can't simply set subtype
            // for this one only
            score()->cmdAddTimeSig(measure(), static_cast<TimeSig*>(e));
            return 0;
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
               ? QT_TRANSLATE_NOOP("TimeSig", "Hide courtesy sig.")
               : QT_TRANSLATE_NOOP("TimeSig", "Show courtesy sig.") );
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

      int st = subtype();
      if (st == 0)
            bb = QRectF(0, 0,0, 0);
      else if (st ==  TSIG_FOUR_FOUR)
            bb = symbols[fourfourmeterSym].bbox(magS()).translated(0.0, 2.0 * _spatium);
      else if (st == TSIG_ALLA_BREVE)
            bb = symbols[allabreveSym].bbox(magS()).translated(0.0, 2.0 * _spatium);
      else {
            int n, z1, z2, z3, z4;
            getSig(&n, &z1, &z2, &z3, &z4);
            sz = QString("%1").arg(z1);
            if (z2)
                  sz += QString("+%1").arg(z2);
            if (z3)
                  sz += QString("+%1").arg(z3);
            if (z4)
                  sz += QString("+%1").arg(z4);
            sn = QString("%1").arg(n);

            // QFontMetricsF fm(fontId2font(0), pdev);
            QFontMetricsF fm(fontId2font(0));
            QRectF rz = fm.tightBoundingRect(sz);
            QRectF rn = fm.tightBoundingRect(sn);

            double m = _spatium / (DPI * SPATIUM20);
            double im = 1.0 / m;
            pz = QPointF(0.0, 2.0 * _spatium) * im;
            pn = QPointF((rz.width() - rn.width())*.5, 4.0 * _spatium) * im;

            bb |= rz.translated(pz);
            bb |= rn.translated(pn);
            }
      setbbox(bb);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(QPainter& p, ScoreView*) const
      {
      int st = subtype();
      if (st == 0)
            return;
      double _spatium = spatium();
      if (st ==  TSIG_FOUR_FOUR)
            symbols[fourfourmeterSym].draw(p, magS(), 0.0, 2.0 * _spatium);
      else if (st == TSIG_ALLA_BREVE)
            symbols[allabreveSym].draw(p, magS(), 0.0, 2.0 * _spatium);
      else {
            p.setFont(symbols[allabreveSym].font());
            double m  = _spatium / (DPI * SPATIUM20);
            double im = 1.0 / m;
            p.scale(m, m);
            p.drawText(pz, sz);
            p.drawText(pn, sn);
            p.scale(im, im);
            }
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space TimeSig::space() const
      {
      return Space(point(score()->styleS(ST_timesigLeftMargin)), width());
      }


