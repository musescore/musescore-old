//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: timesig.cpp,v 1.4 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "painter.h"

//---------------------------------------------------------
//   TimeSig
//---------------------------------------------------------

TimeSig::TimeSig(Score* s)
  : Element(s)
      {
      }

TimeSig::TimeSig(Score* s, int st)
  : Element(s)
      {
      setSubtype(st);
      }

TimeSig::TimeSig(Score* s, int n, int z1, int z2, int z3, int z4)
  : Element(s)
      {
      setSig(n, z1, z2, z3, z4);
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void TimeSig::setSig(int n, int z1, int z2, int z3, int z4)
      {
      setSubtype((z4 << 24) + (z3 << 18) + (z2 << 12) + (z1 << 6) + n);
      layout();
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

bool TimeSig::acceptDrop(const QPointF&, int type, int) const
      {
      return type == TIMESIG;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void TimeSig::drop(const QPointF&, int type, int stype)
      {
      if (type == TIMESIG) {
            int st = subtype();
            if (st == stype)
                  return;
            // change timesig applies to all staves, can't simply set subtype
            // for this one only
            score()->changeTimeSig(tick(), stype);
            }
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
      xml.etag("TimeSig");
      }

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(QDomNode node)
      {
      setSubtype(-1);
      int n=0, z1=0, z2=0, z3=0, z4=0;

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
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
            else if (!Element::readProperties(node))
                  domError(node);
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
      int st = subtype();
      if (st == 0)
            _bbox = QRectF(0, 0,0, 0);
      else if (st ==  TSIG_FOUR_FOUR)
            _bbox = symbols[fourfourmeterSym].bbox().translated(0.0, 2.0 * _spatium);
      else if (st == TSIG_ALLA_BREVE)
            _bbox = symbols[allabreveSym].bbox().translated(0.0, 2.0 * _spatium);
      else {
            int n, z1, z2, z3, z4;
            getSig(&n, &z1, &z2, &z3, &z4);
            QString zs = QString("%1").arg(z1);
            if (z2)
                  zs += QString("+%1").arg(z2);
            if (z3)
                  zs += QString("+%1").arg(z3);
            if (z4)
                  zs += QString("+%1").arg(z4);
            QString ns = QString("%1").arg(n);
            QFontMetricsF fm(symbols[allabreveSym].font());

            qreal  zw = fm.width(zs);
            qreal  nw = fm.width(ns);
            _bbox = QRectF(0.0, 0.0, qMax(zw, nw), 4.0 * _spatium);
            }
      }

//---------------------------------------------------------
//   draw1
//---------------------------------------------------------

void TimeSig::draw1(Painter& p)
      {
      int st = subtype();
      if (st == 0)
            return;
      if (st ==  TSIG_FOUR_FOUR)
            symbols[fourfourmeterSym].draw(p, 0.0, 2.0 * _spatium);
      else if (st == TSIG_ALLA_BREVE)
            symbols[allabreveSym].draw(p, 0.0, 2.0 * _spatium);
      else {
            int n, z1, z2, z3, z4;
            getSig(&n, &z1, &z2, &z3, &z4);
            QString zs = QString("%1").arg(z1);
            if (z2)
                  zs += QString("+%1").arg(z2);
            if (z3)
                  zs += QString("+%1").arg(z3);
            if (z4)
                  zs += QString("+%1").arg(z4);
            QString ns = QString("%1").arg(n);
            p.setFont(symbols[allabreveSym].font());

            QRectF r(0.0, 0.0 * _spatium, 0.0, 0.0);
            QRectF rz = p.boundingRect(r, Qt::AlignLeft | Qt::TextDontClip, zs);
            QRectF rn = p.boundingRect(r, Qt::AlignLeft | Qt::TextDontClip, ns);

            p.drawText(QPointF(0.0, 2.0 * _spatium), zs);
            p.drawText(QPointF((rz.width()-rn.width())*.5, 4.0 * _spatium), ns);
            }
      }

