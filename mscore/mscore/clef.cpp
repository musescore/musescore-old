//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: clef.cpp,v 1.11 2006/03/28 14:58:58 wschweer Exp $
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

/**
 \file
 Implementation of classes Clef (partial) and ClefList (complete).
*/

#include "clef.h"
#include "xml.h"
#include "utils.h"
#include "sym.h"
#include "symbol.h"
#include "score.h"
#include "staff.h"
#include "viewer.h"
#include "system.h"

// FIXME!
// only values for CLEF_G..CLEF_G3 CLEF_F and CLEF_C3 are
// checked

const ClefInfo clefTable[] = {
      ClefInfo("G",   2,  0,   0,  45, "Treble Clef (G Clef)"),
      ClefInfo("G",   2,  1,   7,  52, "Treble Clef 8va"),
      ClefInfo("G",   2,  2,  14,  59, "Treble Clef 15ma"),
      ClefInfo("G",   2, -1,  -7,  38, "Treble Clef 8va bassa"),
      ClefInfo("F",   4,  0, -12,  33, "Bass Clef (F Clef)"),

      ClefInfo("F",   4, -1, -19,  26, "Bass Clef 8va bassa"),
      ClefInfo("F",   4, -2, -26,  19, "Bass Clef 15mb bassa"),

/*NC*/      ClefInfo("F",   4,  0, -10,  35, "Bass Clef"),
/*NC*/      ClefInfo("F",   4,  0, -14,  31, "Bass Clef"),

/*NC*/      ClefInfo("C",   1,  0, -9,   35, "Soprano Clef"),        // CLEF_C1
/*NC*/      ClefInfo("C",   2,  0,  -8,  37, "Mezzo-soprano Clef"),  // CLEF_C2
            ClefInfo("C",   3,  0,  -6,  39, "Alto Clef"),           // CLEF_C3
/*NC*/      ClefInfo("C",   4,  0,  -8,  41, "Tenor Clef"),          // CLEF_C4

/*NC*/      ClefInfo("TAB", 5,  0,   0,   0, "Tablature"),
      ClefInfo("PERC", 2,   0,  0,  45, "Percussion"),

      ClefInfo("C",   3,  0,  -7,   38, "C Clef on third space"),           // CLEF_C5
      };

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
//   canvasPos
//---------------------------------------------------------

QPointF Clef::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = segment()->measure()->system();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout(ScoreLayout*)
      {
      int val     = subtype();
      double yoff = 0.0;
      double xoff = 0.0;
      clear();
      Symbol* symbol = new Symbol(score());

      switch (val) {
            case CLEF_G:
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  break;
            case CLEF_G1:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefEightSym);
                  addElement(number, 1.0 * _spatium, -5.0 * _spatium);
                  }
                  break;
            case CLEF_G2:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefOneSym);
                  addElement(number, .6 * _spatium, -5.0 * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, 1.4 * _spatium, -5.0 * _spatium);
                  }
                  break;
            case CLEF_G3:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefEightSym);
                  addElement(number, 1.0*_spatium, 4.0 * _spatium);
                  }
                  break;
            case CLEF_F:
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  break;
            case CLEF_F8:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefEightSym);
                  addElement(number, .0, 4.5 * _spatium);
                  }
                  break;
            case CLEF_F15:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  number->setSym(clefOneSym);
                  addElement(number, .0, 4.5 * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, .8 * _spatium, 4.5 * _spatium);
                  }
                  break;
            case CLEF_F_B:
            case CLEF_F_C:
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  break;
            case CLEF_C1:
                  symbol->setSym(altoclefSym);
                  yoff = 4.0;
                  break;
            case CLEF_C2:
                  symbol->setSym(altoclefSym);
                  yoff = 3.0;
                  break;
            case CLEF_C3:
                  symbol->setSym(altoclefSym);
                  yoff = 2.0;
                  break;
            case CLEF_C4:
                  symbol->setSym(altoclefSym);
                  yoff = 1.0;
                  break;
            case CLEF_C5:
                  symbol->setSym(altoclefSym);
                  yoff = 2.5;
                  break;
            case CLEF_TAB:
                  symbol->setSym(tabclefSym);
                  yoff = 2.0; //(staff()->lines() - 1) * 0.5;
                  break;
            case CLEF_PERC:
                  symbol->setSym(percussionclefSym);
                  yoff = 2.0;   //(staff()->lines() - 1) * 0.5;
                  break;
            }
      addElement(symbol, .0, .0);
      setUserOff(QPointF(xoff, yoff));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(Viewer* viewer, const QPointF&, int type, int) const
      {
      if (type == CLEF) {
            viewer->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Clef::drop(const QPointF&, const QPointF&, Element* e)
      {
      Element* clef = 0;
      if (e->type() == CLEF) {
            int stype  = e->subtype();
            if (subtype() != stype) {
                  staff()->changeClef(tick(), stype);
                  clef = this;
                  }
            }
      delete e;
      return clef;
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
      xml.etag();
      }

//---------------------------------------------------------
//   ClefList::read
//---------------------------------------------------------

void ClefList::read(QDomElement e, Score* cs)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "clef") {
                  int tick = e.attribute("tick", "0").toInt();
                  int idx = e.attribute("idx", "0").toInt();
                  (*this)[cs->fileDivision(tick)] = idx;
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   removeTime
//---------------------------------------------------------

void ClefList::removeTime(int tick, int len)
      {
      ClefList tmp;
      for (ciClefEvent i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0)) {
                  if (i->first >= tick + len)
                        tmp[i->first - len] = i->second;
                  else
                        printf("remove clef event\n");
                  }
            else
                  tmp[i->first] = i->second;
            }
      clear();
      insert(tmp.begin(), tmp.end());
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void ClefList::insertTime(int tick, int len)
      {
      ClefList tmp;
      for (ciClefEvent i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0))
                  tmp[i->first + len] = i->second;
            else
                  tmp[i->first] = i->second;
            }
      clear();
      insert(tmp.begin(), tmp.end());
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Clef::space(double& min, double& extra) const
      {
      min   = 0.0;
      extra = width();
      }

