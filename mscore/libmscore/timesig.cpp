//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: timesig.cpp 3627 2010-10-26 08:32:55Z wschweer $
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
#include "al/xml.h"
#include "score.h"
#include "style.h"
#include "sym.h"
#include "symbol.h"
#include "system.h"
#include "segment.h"
#include "measure.h"
#include "staff.h"
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
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = segment()->measure()->system();
      qreal yp = system ? y() + system->staff(staffIdx())->y() + system->y() : 0.0;
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
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(XmlReader* r)
      {
      setSubtype(-1);
      int n=0, z1=0, z2=0, z3=0, z4=0;

      while (r->readElement()) {
            if (r->readInt("den", &n))
                  ;
            else if (r->readInt("nom1", &z1))
                  ;
            else if (r->readInt("nom2", &z2))
                  ;
            else if (r->readInt("nom3", &z3))
                  ;
            else if (r->readInt("nom4", &z4))
                  ;
            else if (r->readBool("showCourtesySig", &_showCourtesySig))
                  ;
            else if (!Element::readProperties(r))
                  r->unknown();
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
      qreal _spatium = spatium();
      QRectF bb;

      int st = subtype();
      if (st == 0)
            bb = QRectF(0, 0,0, 0);
      else if (st ==  TSIG_FOUR_FOUR)
            bb = symbols[score()->symIdx()][fourfourmeterSym].bbox(magS()).translated(0.0, 2.0 * _spatium);
      else if (st == TSIG_ALLA_BREVE)
            bb = symbols[score()->symIdx()][allabreveSym].bbox(magS()).translated(0.0, 2.0 * _spatium);
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

            FontMetricsF fm(fontId2font(0));
            qreal m  = magS();
            qreal im = (DPI * SPATIUM20) / _spatium;

            qreal wz = fm.width(sz) * m;
            qreal wn = fm.width(sn) * m;

            pz = QPointF(0.0,            2.0 * _spatium);
            qreal x2 = (wz - wn) * .5;
            pn = QPointF(x2, 4.0 * _spatium);

            qreal x1 = qMin(0.0, x2);
            x2       = qMax(wz, x2+wn);
            bb       = QRectF(x1, 0, x2-x1, _spatium * 4);

            pz *= im;
            pn *= im;
            }
      setbbox(bb);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(Painter* p) const
      {
      int st = subtype();
      if (st == 0 || (staff() && staff()->useTablature()))
            return;
      qreal _spatium = spatium();
      if (st ==  TSIG_FOUR_FOUR)
            symbols[score()->symIdx()][fourfourmeterSym].draw(p, magS(), 0.0, 2.0 * _spatium);
      else if (st == TSIG_ALLA_BREVE)
            symbols[score()->symIdx()][allabreveSym].draw(p, magS(), 0.0, 2.0 * _spatium);
      else {
            qreal m  = _spatium / (DPI * SPATIUM20);
            qreal im = 1.0 / m;
            p->scale(m, m);
            p->drawText(symbols[score()->symIdx()][allabreveSym].font(), pz, sz);
            p->drawText(symbols[score()->symIdx()][allabreveSym].font(), pn, sn);
            p->scale(im, im);
            }
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space TimeSig::space() const
      {
      return Space(point(score()->styleS(ST_timesigLeftMargin)), width());
      }


