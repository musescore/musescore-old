//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: clef.cpp 3672 2010-11-03 19:30:14Z wschweer $
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
#include "m-al/xml.h"
#include "utils.h"
#include "sym.h"
#include "symbol.h"
#include "score.h"
#include "staff.h"
#include "system.h"
#include "style.h"
#include "segment.h"
#include "measure.h"
#include "tablature.h"
#include "part.h"
#include "painter.h"

#define TR(a)  QT_TRANSLATE_NOOP("clefTable", a)

const ClefInfo clefTable[] = {
//   MusicXml           octchng  pitchoffset
// tag  xmlName     line   yoffs     |-lines for sharps--||--lines for flats--|   name
{ "G",    "G",         2,  0,   0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef"),            PITCHED_STAFF },
{ "G8va", "G",         2,  1,   7, 52, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 8va"),        PITCHED_STAFF },
{ "G15ma","G",         2,  2,  14, 59, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 15ma"),       PITCHED_STAFF },
{ "G8vb", "G",         2, -1,  -7, 38, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Treble clef 8vb"),        PITCHED_STAFF },
{ "F",    "F",         4,  0, -12, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef"),              PITCHED_STAFF },
{ "F8vb", "F",         4, -1, -19, 26, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 8vb"),          PITCHED_STAFF },
{ "F15mb","F",         4, -2, -26, 19, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 15mb"),         PITCHED_STAFF },
{ "F3",   "F",         3,  0, -10, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, TR("Baritone clef (F clef)"), PITCHED_STAFF },
{ "F5",   "F",         5,  0, -14, 31, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Subbass clef"),           PITCHED_STAFF },
{ "C1",   "C",         1,  0,  -2, 43, { 5, 1, 4, 0, 3,-1, 2, 2,-1, 3, 0, 4, 1, 5 }, TR("Soprano clef"),           PITCHED_STAFF }, // CLEF_C1
{ "C2",   "C",         2,  0,  -4, 41, { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 }, TR("Mezzo-soprano clef"),     PITCHED_STAFF }, // CLEF_C2
{ "C3",   "C",         3,  0,  -6, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 }, TR("Alto clef"),              PITCHED_STAFF }, // CLEF_C3
{ "C4",   "C",         4,  0,  -8, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 }, TR("Tenor clef"),             PITCHED_STAFF }, // CLEF_C4
{ "TAB",  "TAB",       5,  0,   0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Tablature"),              TAB_STAFF     },
{ "PERC", "percussion",2,  0,   0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Percussion"),             PERCUSSION_STAFF },
{ "C5",   "C",         5,  0, -10, 35, { 4, 0, 3,-1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, TR("Baritone clef (C clef)"), PITCHED_STAFF }, // CLEF_C5
{ "G1",   "G",         1,  0,   2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("French violin clef"),     PITCHED_STAFF }, // CLEF_G4
{ "F8va", "F",         4,  1,  -5, 40, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 8va"),          PITCHED_STAFF }, // CLEF_F_8VA
{ "F15ma","F",         4,  2,   2, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 }, TR("Bass clef 15ma"),         PITCHED_STAFF }, // CLEF_F_15MA
{ "PERC2","percussion",2,  0,   0, 45, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Percussion"),             PERCUSSION_STAFF }, // CLEF_PERC2 placeholder
{ "TAB2", "TAB",       5,  0,   0,  0, { 0, 3,-1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, TR("Tablature2"),             TAB_STAFF     },
      };
#undef TR

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef(Score* s)
  : Element(s)
      {
      _showCourtesyClef = true;
      _small = false;
      }

Clef::Clef(const Clef& c)
   : Element(c)
      {
      _showCourtesyClef = c._showCourtesyClef;
      _small = c._small;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Clef::add(Element* e, qreal x, qreal y)
      {
      e->layout();
      e->setPos(x, y);
      e->setParent(this);
      elements.push_back(e);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Clef::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = segment()->measure()->system();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout()
      {
      qreal smag = _small ? score()->style(ST_smallClefMag).toDouble() : 1.0;
      qreal _spatium = spatium();
      qreal msp  = _spatium * smag;
      qreal yoff = 0.0;
      elements.clear();
      Symbol* symbol = new Symbol(score());

      int st = subtype();
      if (staff() && staff()->useTablature() && clefTable[st].staffGroup != TAB_STAFF)
            st = CLEF_TAB;

      switch (st) {
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
                  add(number, 1.0 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_G2:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  add(number, .6 * msp, -5.0 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  add(number, 1.4 * msp, -5.0 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_G3:
                  {
                  symbol->setSym(trebleclefSym);
                  yoff = 3.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefEightSym);
                  add(number, 1.0 * msp, 4.0 * msp + yoff * _spatium);
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
                  add(number, .0, 4.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F15:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  add(number, .0, 4.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  add(number, .8 * msp, 4.5 * msp + yoff * _spatium);
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
                  {
                  symbol->setSym(tabclefSym);
                  Staff* st = staff();
                  if (st && st->useTablature()) {
                        Tablature* tab = st->part()->instr()->tablature();
                        switch(tab->strings()) {
                              default:
                              case 6: yoff = 2.5 * 1.5; break;
                              case 4: yoff = 1.5 * 1.5; break;
                              }
                        }
                  else
                        yoff = 2.0;
                  }
                  break;
            case CLEF_TAB2:
                  {
                  symbol->setSym(tabclef2Sym);
                  Staff* st = staff();
                  if (st && st->useTablature()) {
                        Tablature* tab = st->part()->instr()->tablature();
                        switch(tab->strings()) {
                              default:
                              case 6: yoff = 2.5 * 1.5; break;
                              case 4: yoff = 1.5 * 1.5; break;
                              }
                        }
                  else
                        yoff = 2.0;
                  }
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
                  add(number, .5 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            case CLEF_F_15MA:
                  {
                  symbol->setSym(bassclefSym);
                  yoff = 1.0;
                  Symbol* number = new Symbol(score());
                  symbol->setMag(smag);
                  number->setSym(clefOneSym);
                  add(number, .0 * msp, -1.5 * msp + yoff * _spatium);
                  number = new Symbol(score());
                  number->setSym(clefFiveSym);
                  add(number, .8 * msp, -1.5 * msp + yoff * _spatium);
                  }
                  break;
            }
      add(symbol, .0, yoff * _spatium);
      symbol->setMag(smag * mag());
      _bbox = QRectF();
      for (iElement i = elements.begin(); i != elements.end(); ++i) {
            Element* e = *i;
            _bbox |= e->bbox().translated(e->pos());
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Clef::draw(Painter* p) const
      {
      foreach(Element* e, elements) {
            p->translate(e->pos());
            e->draw(p);
            p->translate(-e->pos());
            }
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

ClefType ClefList::clef(int tick) const
      {
      if (empty())
            return CLEF_G;
      ciClefEvent i = upper_bound(tick);
      if (i == begin())
            return CLEF_G;
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void ClefList::setClef(int tick, ClefType idx)
      {
      std::pair<int, ClefType> clef(tick, idx);
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

void Clef::read(XmlReader* r)
      {
      while (r->readElement()) {
            QString val;
            if (r->readString("subtype", &val)) {
                  ClefType ct = Clef::clefType(val);
                  setClefType(ct);
                  }
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int Clef::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Clef::subtypeName() const
      {
      return QString(clefTable[subtype()].tag);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Clef::setSubtype(const QString& s)
      {
      ClefType ct = clefType(s);
      if (ct == CLEF_INVALID) {
            printf("Clef::setSubtype: unknown: <%s>\n", qPrintable(s));
            ct = CLEF_G;
            }
      setClefType(ct);
      }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType(const QString& s)
      {
      for (unsigned i = 0; i < sizeof(clefTable)/sizeof(*clefTable); ++i) {
            if (clefTable[i].tag == s)
                  return ClefType(i);
            }
      return CLEF_INVALID;
      }

