//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: clef.cpp,v 1.11 2006/03/28 14:58:58 wschweer Exp $
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

#include "clef.h"
#include "xml.h"
#include "utils.h"

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef(Score* s)
  : Compound(s)
      {
      }

Clef::Clef(Score* s, int i)
  : Compound(s)
      {
      setSubtype(i);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Clef::setSubtype(int st)
      {
      Element::setSubtype(st);
      int val     = st & 0x7ff;
      bool _small = st & clefSmallBit;
      double yoff = 0.0;
      double xoff = 0.0;
      clear();
      Symbol* symbol = new Symbol(score());

      switch (val) {
            case CLEF_G:
                  symbol->setSym(_small ? ctrebleclefSym : trebleclefSym);
                  yoff = 3.0;
                  break;
            case CLEF_G1: 
                  {
                  symbol->setSym(_small ? ctrebleclefSym : trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefEightSym);
                  addElement(number, 1.0, -5.0);
                  }
                  break;
            case CLEF_G2:
                  {
                  symbol->setSym(_small ? ctrebleclefSym : trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefOneSym);
                  addElement(number, .6, -5.0);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, 1.4, -5.0);
                  }
                  break;
            case CLEF_G3:
                  {
                  symbol->setSym(_small ? ctrebleclefSym : trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefEightSym);
                  addElement(number, 1.0, 4.0);
                  }
                  break;
            case CLEF_F:
                  symbol->setSym(_small ? cbassclefSym : bassclefSym);
                  yoff = 1.0;
                  break;
            case 5:
                  {
                  symbol->setSym(_small ? cbassclefSym : bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefEightSym);
                  addElement(number, .0, 3.5);
                  }
                  break;
            case 6:
                  {
                  symbol->setSym(_small ? cbassclefSym : bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefOneSym);
                  addElement(number, .0, 3.5);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, .8, 3.5);
                  }
                  break;
            case 7:
            case 8:
                  symbol->setSym(_small ? cbassclefSym : bassclefSym);
                  yoff = 1.0;
                  break;
            case 9:
                  symbol->setSym(_small ? caltoclefSym : altoclefSym);
                  yoff = 1.0;
                  break;
            case 10:
                  symbol->setSym(_small ? caltoclefSym : altoclefSym);
                  yoff = 2.0;
                  break;
            case 11:
                  symbol->setSym(_small ? caltoclefSym : altoclefSym);
                  yoff = 3.0;
                  break;
            case 12:
                  symbol->setSym(_small ? caltoclefSym : altoclefSym);
                  yoff = 4.0;
                  break;
            case 13:
            default:
                  break;
            }
      addElement(symbol, .0, .0);
      setUserOff(QPointF(xoff, yoff));
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Clef::setSmall(bool val)
      {
      if (val)
            setSubtype(subtype() & ~clefSmallBit);
      else
            setSubtype(subtype() | clefSmallBit);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Clef::space(double& min, double& extra) const
      {
      min   = 0.0;
      extra = width(); // + 0.5 * _spatium;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(int type, int) const
      {
      return (type == CLEF);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void Clef::drop(const QPointF&, int t, int st)
      {
      if (t != CLEF)
            return;
      printf("drop clef %d %d\n", t, st);
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

int ClefList::clef(int tick) const
      {
      if (empty())
            return 0;
      ciClefEvent i = upper_bound(tick);
      if (i == begin())
            return 0;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void ClefList::setClef(int tick, int idx)
      {
      std::pair<int, int> clef(tick, idx);
      std::pair<iClefEvent,bool> p = insert(clef);
      if (!p.second)
            (*this)[tick] = idx;
      iClefEvent i = p.first;
      for (++i; i != end();) {
            if (i->second != idx)
                  break;
            iClefEvent ii = i;
            ++ii;
            erase(i);
            i = ii;
            }
      }

//---------------------------------------------------------
//   ClefList::write
//---------------------------------------------------------

void ClefList::write(Xml& xml, const char* name) const
      {
      xml.stag(name);
      for (ciClefEvent i = begin(); i != end(); ++i)
            xml.tagE("clef tick=\"%d\" idx=\"%d\"", i->first, i->second);
      xml.etag(name);
      }

//---------------------------------------------------------
//   ClefList::read
//---------------------------------------------------------

void ClefList::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());

            if (tag == "clef")
                  readEvent(node);
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   ClefEvent::read
//---------------------------------------------------------

void ClefList::readEvent(QDomNode node)
      {
      QDomElement e = node.toElement();
      if (e.isNull())
            return;
      int tick = e.attribute("tick", "0").toInt();
      int idx = e.attribute("idx", "0").toInt();
      (*this)[tick] = idx;
      }

//---------------------------------------------------------
//   removeTime
//---------------------------------------------------------

void ClefList::removeTime(int s, int e)
      {
      erase(find(s), find(e));
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void ClefList::insertTime(int, int)
      {
      printf("ClefList::insertTime(): not impl.\n");
      }

