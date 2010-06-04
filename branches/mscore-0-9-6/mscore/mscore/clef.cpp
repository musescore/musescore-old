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
// #include "scoreview.h"
#include "system.h"
#include "style.h"
#include "segment.h"
#include "measure.h"

// FIXME!
// only values for CLEF_G..CLEF_G3 CLEF_F and CLEF_C3 are
// checked

const ClefInfo clefTable[] = {
//               MusicXml
//      name line octave  yoffset pitchoffset  name
      { "G",   2,  0,   0, 45,
            { 0, 3, -1, 2, 5, 1, 4 , 4, 1,  5, 2, 6, 3, 7 },
            QT_TRANSLATE_NOOP("clefTable", "Treble clef")
            },
      { "G",   2,  1,   7, 52,
            { 0, 3, -1, 2, 5, 1, 4, 4, 1,  5, 2, 6, 3, 7 },
            QT_TRANSLATE_NOOP("clefTable", "Treble clef 8va") },
      { "G",   2,  2,  14, 59,
            { 0, 3, -1, 2, 5, 1, 4, 4, 1,  5, 2, 6, 3, 7 },
            QT_TRANSLATE_NOOP("clefTable", "Treble clef 15ma") },
      { "G",   2, -1,  -7, 38,
            { 0, 3, -1, 2, 5, 1, 4, 4, 1,  5, 2, 6, 3, 7 },
            QT_TRANSLATE_NOOP("clefTable", "Treble clef 8vb") },
      { "F",   4,  0, -12, 33,
            { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },
            QT_TRANSLATE_NOOP("clefTable", "Bass clef") },
      { "F",   4, -1, -19, 26,
            { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },
            QT_TRANSLATE_NOOP("clefTable", "Bass clef 8vb") },
      { "F",   4, -2, -26, 19,
            { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },
            QT_TRANSLATE_NOOP("clefTable", "Bass clef 15mb") },

      { "F",   3,  0, -10, 35,
            { 4, 0, 3, -1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 },
            QT_TRANSLATE_NOOP("clefTable", "Baritone clef (F clef)") },
      { "F",   5,  0, -14, 31,
            { 1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
            QT_TRANSLATE_NOOP("clefTable", "Subbass clef") },

      { "C",   1,  0,  -2, 43,
            { 5, 1, 4, 0, 3, -1, 2, 2, -1, 3, 0, 4, 1, 5 },
            QT_TRANSLATE_NOOP("clefTable", "Soprano clef") },        // CLEF_C1
      { "C",   2,  0,  -4, 41,
            { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 },
            QT_TRANSLATE_NOOP("clefTable", "Mezzo-soprano clef") },  // CLEF_C2
      { "C",   3,  0,  -6, 39,
            { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 },
            QT_TRANSLATE_NOOP("clefTable", "Alto clef") },           // CLEF_C3
      { "C",   4,  0,  -8, 37,
            { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 },
            QT_TRANSLATE_NOOP("clefTable", "Tenor clef")  },          // CLEF_C4

      { "TAB", 5,  0,   0,  0,
            { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 },
            QT_TRANSLATE_NOOP("clefTable", "Tablature") },

      { "percussion", 2,  0,   0, 45,
            { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 },
            QT_TRANSLATE_NOOP("clefTable", "Percussion") },

      { "C",   5,  0, -10, 35,
            { 4, 0, 3, -1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 },
            QT_TRANSLATE_NOOP("clefTable", "Baritone clef (C clef)") },            // CLEF_C5

      { "G",   1,  0,   2, 47,
            { 2, 5, 1, 4, 0, 3, -1, 6, 3, 7, 4, 1, 5, 2 },
            QT_TRANSLATE_NOOP("clefTable", "French violin clef") },       // CLEF_G4

      { "F",   4,  1, -5, 40,                                          // CLEF_F_8VA
            { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },
            QT_TRANSLATE_NOOP("clefTable", "Bass clef 8va") },

      { "F",   4,  2,  2, 47,                                          // CLEF_F_15MA
            { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },
            QT_TRANSLATE_NOOP("clefTable", "Bass clef 15ma") },

      { "percussion", 2,  0,   0, 45,                                   // CLEF_PERC2 placeholder
            { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 },
            QT_TRANSLATE_NOOP("clefTable", "Percussion") },

      };

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef(Score* s)
  : Compound(s)
      {
      _small = false;
      }

Clef::Clef(const Clef& c)
   : Compound(c)
      {
      _small = c._small;
      }

Clef::Clef(Score* s, int i)
  : Compound(s)
      {
      _small = false;
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
      if (!system)
            return pos();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout()
      {
      double smag = _small ? score()->style(ST_smallClefMag).toDouble() : 1.0;
      double _spatium = spatium();
      double msp  = _spatium * smag;
      double yoff = 0.0;
      clear();
      Symbol* symbol = new Symbol(score());

      switch (subtype()) {
            case CLEF_G:
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  break;
            case CLEF_G1:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, 1.0 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_G2:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .6 * msp, -5.0 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, 1.4 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_G3:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, 1.0 * msp, 4.0 * msp + yoff * _spatium);
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
                  symbol->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, .0, 4.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F15:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .0, 4.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, .8 * msp, 4.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F_B:                            // baritone clef
                  symbol->setSym(bassclefSym);
                  yoff = 2.0;
                  break;
            case CLEF_F_C:                            // subbass clef
                  symbol->setSym(bassclefSym);
                  yoff = 0.0;
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
                  yoff = 0.0;
                  break;
            case CLEF_TAB:
                  symbol->setSym(tabclefSym);
                  yoff = 2.0; //(staff()->lines() - 1) * 0.5;
                  break;
            case CLEF_PERC:
            case CLEF_PERC2:
                  symbol->setSym(percussionclefSym);
                  yoff = 2.0;   //(staff()->lines() - 1) * 0.5;
                  break;
            case CLEF_G4:
                  symbol->setSym(trebleclefSym);
                  yoff = 4.0;
                  break;
            case CLEF_F_8VA:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  number->setMag(smag);
                  number->setSym(clefEightSym);
                  addElement(number, .5 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F_15MA:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  addElement(number, .0 * msp, -1.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  addElement(number, .8 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            }
      addElement(symbol, .0, yoff * _spatium);
      symbol->setMag(smag * mag());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(ScoreView*, const QPointF&, int type, int) const
      {
      return type == CLEF;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Clef::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
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

Space Clef::space() const
      {
      return Space(point(score()->styleS(ST_clefLeftMargin)), width());
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Clef::setSmall(bool val)
      {
      if (val != _small)
            _small = val;
      layout();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Clef::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      if (subtype() == 0)      // make sure setSubtype() is called at least once
            setSubtype(0);
      if (score()->mscVersion() < 113)
            setUserOff(QPointF());
      }


