//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
// #include "note.h"

//---------------------------------------------------------
//   Acc
//---------------------------------------------------------

struct Acc {
      const char* tag;
      int offset;
      Acc(const char* t, int o) : tag(t), offset(o) {}
      };

Acc accList[] = {
      Acc(QT_TRANSLATE_NOOP("accidental", "none"),                0),
      Acc(QT_TRANSLATE_NOOP("accidental", "sharp"),               1),
      Acc(QT_TRANSLATE_NOOP("accidental", "flat"),               -1),
      Acc(QT_TRANSLATE_NOOP("accidental", "double sharp"),        2),
      Acc(QT_TRANSLATE_NOOP("accidental", "double flat"),        -2),
      Acc(QT_TRANSLATE_NOOP("accidental", "natural"),             0),

      Acc(QT_TRANSLATE_NOOP("accidental", "(sharp)"),             1),
      Acc(QT_TRANSLATE_NOOP("accidental", "(flat)"),             -1),
      Acc(QT_TRANSLATE_NOOP("accidental", "(double sharp)"),      2),
      Acc(QT_TRANSLATE_NOOP("accidental", "(double flat)"),      -2),
      Acc(QT_TRANSLATE_NOOP("accidental", "(natural)"),           0),

      Acc(QT_TRANSLATE_NOOP("accidental", "[sharp]"),             1),
      Acc(QT_TRANSLATE_NOOP("accidental", "[flat]"),             -1),
      Acc(QT_TRANSLATE_NOOP("accidental", "[double sharp]"),      2),
      Acc(QT_TRANSLATE_NOOP("accidental", "[double flat]"),      -2),
      Acc(QT_TRANSLATE_NOOP("accidental", "[natural]"),           0),

/*16*/Acc(QT_TRANSLATE_NOOP("accidental", "flat-slash"),          0),
      Acc(QT_TRANSLATE_NOOP("accidental", "flat-slash2"),         0),
      Acc(QT_TRANSLATE_NOOP("accidental", "mirrored-flat2"),      0),
      Acc(QT_TRANSLATE_NOOP("accidental", "mirrored-flat"),       0),
      Acc(QT_TRANSLATE_NOOP("accidental", "mirrored-flat-slash"), 0),
      Acc(QT_TRANSLATE_NOOP("accidental", "flat-flat-slash"),     0),

/*22*/Acc(QT_TRANSLATE_NOOP("accidental", "sharp-slash"),         0),
      Acc(QT_TRANSLATE_NOOP("accidental", "sharp-slash2"),        0),
      Acc(QT_TRANSLATE_NOOP("accidental", "sharp-slash3"),        0),
      Acc(QT_TRANSLATE_NOOP("accidental", "sharp-slash4"),        0),

/*26*/Acc(QT_TRANSLATE_NOOP("accidental", "sharp arrow up"),      0),
      Acc(QT_TRANSLATE_NOOP("accidental", "sharp arrow down"),    0),
      Acc(QT_TRANSLATE_NOOP("accidental", "sharp arrow both"),    0),
      Acc(QT_TRANSLATE_NOOP("accidental", "flat arrow up"),       0),
      Acc(QT_TRANSLATE_NOOP("accidental", "flat arrow down"),     0),
      Acc(QT_TRANSLATE_NOOP("accidental", "flat arrow both"),     0),
      Acc(QT_TRANSLATE_NOOP("accidental", "natural arrow up"),    0),
      Acc(QT_TRANSLATE_NOOP("accidental", "natural arrow down"),  0),
      Acc(QT_TRANSLATE_NOOP("accidental", "natural arrow both"),  0)
      };

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

const char* Accidental::subTypeName() const
      {
      return accList[subtype()].tag;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Accidental::setSubtype(const QString& tag)
      {
      int n = sizeof(accList)/sizeof(*accList);
      for (int i = 0; i < n; ++i) {
            if (accList[i].tag == tag) {
                  setSubtype(i);
                  return;
                  }
            }
      Element::setSubtype(0);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Accidental::setSubtype(int i)
      {
      // change old coding
      switch(i) {
            case 6:  i = 1 + 0x8000; break;
            case 7:  i = 2 + 0x8000; break;
            case 8:  i = 3 + 0x8000; break;
            case 9:  i = 4 + 0x8000; break;
            case 10: i = 5 + 0x8000; break;
            }
      Element::setSubtype(i);
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

int Accidental::symbol()
      {
      int s;
      switch (subtype() & 0x7fff) {
            default: printf("illegal accidental %d\n", subtype() & 0x7fff); abort();
            case  ACC_NONE:    return -1;
            case  ACC_SHARP:   s = sharpSym;             break;
            case  ACC_FLAT:    s = flatSym;              break;
            case  ACC_SHARP2:  s = sharpsharpSym;        break;
            case  ACC_FLAT2:   s = flatflatSym;          break;
            case  ACC_NATURAL: s = naturalSym;           break;

            case 11:           s = sharpSym;             break;
            case 12:           s = flatSym;              break;
            case 13:           s = sharpsharpSym;        break;
            case 14:           s = flatflatSym;          break;
            case 15:           s = naturalSym;           break;

            case 16:           s = flatslashSym;         break;
            case 17:           s = flatslash2Sym;        break;
            case 18:           s = mirroredflat2Sym;     break;
            case 19:           s = mirroredflatSym;      break;
            case 20:           s = mirroredflatslashSym; break;
            case 21:           s = flatflatslashSym;     break;
            case 22:           s = sharpslashSym;        break;
            case 23:           s = sharpslash2Sym;       break;
            case 24:           s = sharpslash3Sym;       break;
            case 25:           s = sharpslash4Sym;       break;
            case 26:           s = sharpArrowUpSym;      break;
            case 27:           s = sharpArrowDownSym;    break;
            case 28:           s = sharpArrowBothSym;    break;
            case 29:           s = flatArrowUpSym;       break;
            case 30:           s = flatArrowDownSym;     break;
            case 31:           s = flatArrowBothSym;     break;
            case 32:           s = naturalArrowUpSym;    break;
            case 33:           s = naturalArrowDownSym;  break;
            case 34:           s = naturalArrowBothSym;  break;
            }
      return s;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Accidental::layout()
      {
      int i = subtype();
      el.clear();

      double m = magS();
      QRectF r;

      QPointF pos;
      if (i & 0x8000) {
            SymElement e(leftparenSym, 0.0);
            el.append(e);
            r |= symbols[leftparenSym].bbox(m);
            pos = symbols[leftparenSym].attach(m);
            }

      int s = symbol();
      SymElement e(s, pos.x());
      el.append(e);
      r |= symbols[s].bbox(m);
      pos += symbols[s].attach(m);

      if (i & 0x8000) {
            double x = pos.x();     // symbols[s].width(m) + symbols[s].bbox(m).x();
            SymElement e(rightparenSym, x);
            el.append(e);
            r |= symbols[rightparenSym].bbox(m).translated(x, 0.0);
            }
      setbbox(r);
      }

//---------------------------------------------------------
//   subtype2value
//    returns the resulting pitch offset
//---------------------------------------------------------

int Accidental::subtype2value(int st)
      {
      switch(st & 0xfff) {
            case ACC_NONE:    return 0;
            case ACC_SHARP:   return 1;
            case ACC_SHARP2:  return 2;
            case ACC_FLAT:    return -1;
            case ACC_FLAT2:   return -2;
            }
      return 0;
      }

//---------------------------------------------------------
//   value2subtype
//---------------------------------------------------------

int Accidental::value2subtype(int v)
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
      return 0;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Accidental::draw(QPainter& painter) const
      {
      double m = magS();
      foreach(const SymElement& e, el)
            symbols[e.sym].draw(painter, m, e.x, 0.0);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Accidental::acceptDrop(ScoreView*, const QPointF&, int type, int /*subtype*/) const
      {
      return type == ACCIDENTAL_BRACKET;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Accidental::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      switch(e->type()) {
            case ACCIDENTAL_BRACKET:
                  score()->changeAccidental(note(), subtype() | 0x8000);
                  break;

            default:
                  break;
            }
      delete e;
      return 0;
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

