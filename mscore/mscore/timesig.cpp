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

//---------------------------------------------------------
//   TimeSig
//---------------------------------------------------------

TimeSig::TimeSig(Score* s)
  : Compound(s)
      {
      }

TimeSig::TimeSig(Score* s, int i)
  : Compound(s)
      {
      setSubtype(i);
      }

//---------------------------------------------------------
//   setIdx
//---------------------------------------------------------

void TimeSig::setSubtype(int v)
      {
      clear();
      Element::setSubtype(v);
      Symbol* s;
      switch (subtype()) {
            case TSIG_2_2: setSig(2,2); break;
            case TSIG_2_4: setSig(2,4); break;
            case TSIG_3_4: setSig(3,4); break;
            case TSIG_4_4: setSig(4,4); break;
            case TSIG_5_4: setSig(5,4); break;
            case TSIG_6_4: setSig(6,4); break;
            case TSIG_3_8: setSig(3,8); break;
            case TSIG_6_8: setSig(6,8); break;
            case TSIG_9_8: setSig(9,8); break;
            case TSIG_44:
                  s = new Symbol(score());
                  s->setSym(fourfourmeterSym);
                  addElement(s, 0, 2);
                  _z = 4;
                  _n = 4;
                  break;
            case TSIG_34:
                  s = new Symbol(score());
                  s->setSym(allabreveSym);
                  addElement(s, 0, 2);
                  _z = 3;
                  _n = 4;
                  break;
            case TSIG_12_8:
                  setSig(12,8); break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   zn
//---------------------------------------------------------

void TimeSig::zn(int& z, int& n) const
      {
      z = _z;
      n = _n;
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void TimeSig::setSig(int z, int n)
      {
      _z = z;
      _n = n;
      Symbol* s  = new Symbol(score());
      if (z >= 10) {
            s->setSym(zeroSym + z/10);
            addElement(s, 0, 2.0);
            s  = new Symbol(score());
            s->setSym(zeroSym + (z%10));
            addElement(s, 1.5, 2.0);
            }
      else {
            s->setSym(zeroSym + z);
            addElement(s, 0, 2.0);
            }
      s  = new Symbol(score());
      s->setSym(zeroSym + n);
      addElement(s, z >= 10 ? 0.75 : 0, 4.0);
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
      xml.etag("TimeSig");
      }

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            if (!Element::readProperties(node))
                  domError(node);
            }
      }


