//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: accidental.cpp 3515 2010-09-27 11:42:13Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "accidental.h"
#include "symbol.h"
#include "sym.h"
#include "score.h"
#include "staff.h"
#include "al/xml.h"

//---------------------------------------------------------
//   Acc
//---------------------------------------------------------

struct Acc {
      const char* tag;        // for use in xml file
      const char* name;       // translated name
      int offset;             // semitone offset
      int centOffset;
      int sym;
      Acc(const char* t, const char* n, int o, int o2, int s)
         : tag(t), name(n), offset(o), centOffset(o2), sym(s) {}
      };

Acc accList[] = {
      Acc("none",                QT_TRANSLATE_NOOP("accidental", "none"),                0, 0, -1),
      Acc("sharp",               QT_TRANSLATE_NOOP("accidental", "sharp"),               1, 0, sharpSym),
      Acc("flat",                QT_TRANSLATE_NOOP("accidental", "flat"),               -1, 0, flatSym),
      Acc("double sharp",        QT_TRANSLATE_NOOP("accidental", "double sharp"),        2, 0, sharpsharpSym),
      Acc("double flat",         QT_TRANSLATE_NOOP("accidental", "double flat"),        -2, 0, flatflatSym),
      Acc("natural",             QT_TRANSLATE_NOOP("accidental", "natural"),             0, 0, naturalSym),

      Acc("flat-slash",          QT_TRANSLATE_NOOP("accidental", "flat-slash"),          0, 0, flatslashSym),
      Acc("flat-slash2",         QT_TRANSLATE_NOOP("accidental", "flat-slash2"),         0, 0, flatslash2Sym),
      Acc("mirrored-flat2",      QT_TRANSLATE_NOOP("accidental", "mirrored-flat2"),      0, 0, mirroredflat2Sym),
      Acc("mirrored-flat",       QT_TRANSLATE_NOOP("accidental", "mirrored-flat"),       0, 0, mirroredflatSym),
      Acc("mirrored-flat-slash", QT_TRANSLATE_NOOP("accidental", "mirrored-flat-slash"), 0, 0, mirroredflatslashSym),
      Acc("flat-flat-slash",     QT_TRANSLATE_NOOP("accidental", "flat-flat-slash"),     0, 0, flatflatslashSym),

      Acc("sharp-slash",         QT_TRANSLATE_NOOP("accidental", "sharp-slash"),         0, 0, sharpslashSym),
      Acc("sharp-slash2",        QT_TRANSLATE_NOOP("accidental", "sharp-slash2"),        0, 0, sharpslash2Sym),
      Acc("sharp-slash3",        QT_TRANSLATE_NOOP("accidental", "sharp-slash3"),        0, 0, sharpslash3Sym),
      Acc("sharp-slash4",        QT_TRANSLATE_NOOP("accidental", "sharp-slash4"),        0, 0, sharpslash4Sym),

      Acc("sharp arrow up",      QT_TRANSLATE_NOOP("accidental", "sharp arrow up"),      0, 0, sharpArrowUpSym),
      Acc("sharp arrow down",    QT_TRANSLATE_NOOP("accidental", "sharp arrow down"),    0, 0, sharpArrowDownSym),
      Acc("sharp arrow both",    QT_TRANSLATE_NOOP("accidental", "sharp arrow both"),    0, 0, sharpArrowBothSym),
      Acc("flat arrow up",       QT_TRANSLATE_NOOP("accidental", "flat arrow up"),       0, 0, flatArrowUpSym),
      Acc("flat arrow down",     QT_TRANSLATE_NOOP("accidental", "flat arrow down"),     0, 0, flatArrowDownSym),
      Acc("flat arrow both",     QT_TRANSLATE_NOOP("accidental", "flat arrow both"),     0, 0, flatArrowBothSym),
      Acc("natural arrow up",    QT_TRANSLATE_NOOP("accidental", "natural arrow up"),    0, 0, naturalArrowUpSym),
      Acc("natural arrow down",  QT_TRANSLATE_NOOP("accidental", "natural arrow down"),  0, 0, naturalArrowDownSym),
      Acc("natural arrow both",  QT_TRANSLATE_NOOP("accidental", "natural arrow both"),  0, 0, naturalArrowBothSym)
      };

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

Accidental::Accidental(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _hasBracket = false;
      _role       = ACC_AUTO;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Accidental::read(XmlReader* r)
      {
      int i;
      while (r->readElement()) {
            if (r->readInt("bracket", &i)) {
                  if (i == 0 || i == 1)
                        _hasBracket = i;
                  }
            else if (r->readInt("role", &i)) {
                  if (i == ACC_AUTO || i == ACC_USER)
                        _role = AccidentalRole(i);
                  }
            else if (Element::readProperties(r))
                  ;
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Accidental::subtypeName() const
      {
      return accList[subtype()].tag;
      }

//---------------------------------------------------------
//   subTypeName
//---------------------------------------------------------

const char* Accidental::subtypeUserName() const
      {
      return accList[subtype()].name;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Accidental::setSubtype(const QString& tag)
      {
      int n = sizeof(accList)/sizeof(*accList);
      for (int i = 0; i < n; ++i) {
            if (accList[i].tag == tag) {
                  Element::setSubtype(i);
                  return;
                  }
            }
      Element::setSubtype(0);
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

int Accidental::symbol()
      {
      return accList[subtype()].sym;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Accidental::layout()
      {
      el.clear();

      qreal m = magS();
      QRectF r;

      QPointF pos;
      if (_hasBracket) {
            SymElement e(leftparenSym, 0.0);
            el.append(e);
            r |= symbols[score()->symIdx()][leftparenSym].bbox(m);
            pos = symbols[score()->symIdx()][leftparenSym].attach(m);
            }

      int s = symbol();
      SymElement e(s, pos.x());
      el.append(e);
      r |= symbols[score()->symIdx()][s].bbox(m);
      pos += symbols[score()->symIdx()][s].attach(m);

      if (_hasBracket) {
            qreal x = pos.x();     // symbols[s].width(m) + symbols[s].bbox(m).x();
            SymElement e(rightparenSym, x);
            el.append(e);
            r |= symbols[score()->symIdx()][rightparenSym].bbox(m).translated(x, 0.0);
            }
      setbbox(r);
      }

//---------------------------------------------------------
//   subtype2value
//    returns the resulting pitch offset
//---------------------------------------------------------

int Accidental::subtype2value(AccidentalType st)
      {
      return accList[st].offset;
      }

//---------------------------------------------------------
//   subtype2name
//---------------------------------------------------------

const char* Accidental::subtype2name(AccidentalType st)
      {
      return accList[st].tag;
      }

//---------------------------------------------------------
//   value2subtype
//---------------------------------------------------------

AccidentalType Accidental::value2subtype(int v)
      {
      switch(v) {
            case 0:  return ACC_NONE;
            case 1:  return ACC_SHARP;
            case 2:  return ACC_SHARP2;
            case -1: return ACC_FLAT;
            case -2: return ACC_FLAT2;
            default:
                  printf("value2subtype: illegal accidental val %d\n", v);
                  abort();
            }
      return ACC_NONE;
      }

//---------------------------------------------------------
//   name2subtype
//---------------------------------------------------------

AccidentalType Accidental::name2subtype(const QString& tag)
      {
      int n = sizeof(accList)/sizeof(*accList);
      for (int i = 0; i < n; ++i) {
            if (accList[i].tag == tag)
                  return AccidentalType(i);
            }
      return ACC_NONE;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Accidental::draw(Painter* painter) const
      {
      qreal m = magS();
      foreach(const SymElement& e, el)
            symbols[score()->symIdx()][e.sym].draw(painter, m, e.x, 0.0);
      }

//---------------------------------------------------------
//   AccidentalBracket
//---------------------------------------------------------

AccidentalBracket::AccidentalBracket(Score* s)
   : Compound(s)
      {
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void AccidentalBracket::setSubtype(int i)
      {
      Element::setSubtype(i);
      clear();

      Symbol* s1 = new Symbol(score());
      Symbol* s2 = new Symbol(score());
      switch(i) {
            case 0:
                  s1->setSym(leftparenSym);
                  s2->setSym(rightparenSym);
                  break;
            default:
            case 1:
                  break;
            }
      addElement(s1, -s1->bbox().x(), 0.0);
      addElement(s2, s2->bbox().width() - s2->bbox().x(), 0.0);
      }
