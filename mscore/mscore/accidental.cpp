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
      Acc("natural arrow both",  QT_TRANSLATE_NOOP("accidental", "natural arrow both"),  0, 0, naturalArrowBothSym),
      Acc("sori",                QT_TRANSLATE_NOOP("accidental", "sori"),                0, 0, soriSym),
      Acc("koron",               QT_TRANSLATE_NOOP("accidental", "koron"),               0, 0, koronSym)
      };

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Accidental::subtypeName() const
      {
      return accList[subtype()].tag;
      }

//---------------------------------------------------------
//   subTypeUseName
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

      double m = magS();
      QRectF r;

      QPointF pos;
      if (_hasBracket) {
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

      if (_hasBracket) {
            double x = pos.x();
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
      return accList[st].offset;
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
                  _hasBracket = true;     // TODO: make undoable
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

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Accidental::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            bool isInt;
            int i = e.text().toInt(&isInt);
            if (tag == "bracket") {
                  if (i == 0 || i == 1)
                        _hasBracket = i;
                  }
            else if (tag == "subtype") {
                  if (isInt) {
                        _hasBracket = i & 0x8000;
                        i &= ~0x8000;
                        switch(i) {
                              case 0:
                                    i = ACC_NONE;
                                    break;
                              case 1:
                              case 11:
                                    i = ACC_SHARP;
                                    break;
                              case 2:
                              case 12:
                                    i = ACC_FLAT;
                                    break;
                              case 3:
                              case 13:
                                    i = ACC_SHARP2;
                                    break;
                              case 4:
                              case 14:
                                    i = ACC_FLAT2;
                                    break;
                              case 5:
                              case 15:
                                    i = ACC_NATURAL;
                                    break;
                              case 6:
                                    i = ACC_SHARP;
                                    _hasBracket = true;
                                    break;
                              case 7:
                                    i = ACC_FLAT;
                                    _hasBracket = true;
                                    break;
                              case 8:
                                    i = ACC_SHARP2;
                                    _hasBracket = true;
                                    break;
                              case 9:
                                    i = ACC_FLAT2;
                                    _hasBracket = true;
                                    break;
                              case 10:
                                    i = ACC_NATURAL;
                                    _hasBracket = true;
                                    break;
                              case 16:
                                    i = ACC_FLAT_SLASH;
                                    break;
                              case 17:
                                    i = ACC_FLAT_SLASH2;
                                    break;
                              case 18:
                                    i = ACC_MIRRORED_FLAT2;
                                    break;
                              case 19:
                                    i = ACC_MIRRORED_FLAT;
                                    break;
                              case 20:
                                    i = ACC_MIRRIRED_FLAT_SLASH;
                                    break;
                              case 21:
                                    i = ACC_FLAT_FLAT_SLASH;
                                    break;
                              case 22:
                                    i = ACC_SHARP_SLASH;
                                    break;
                              case 23:
                                    i = ACC_SHARP_SLASH2;
                                    break;
                              case 24:
                                    i = ACC_SHARP_SLASH3;
                                    break;
                              case 25:
                                    i = ACC_SHARP_SLASH4;
                                    break;
                              case 26:
                                    i = ACC_SHARP_ARROW_UP;
                                    break;
                              case 27:
                                    i = ACC_SHARP_ARROW_DOWN;
                                    break;
                              case 28:
                                    i = ACC_SHARP_ARROW_BOTH;
                                    break;
                              case 29:
                                    i = ACC_FLAT_ARROW_UP;
                                    break;
                              case 30:
                                    i = ACC_FLAT_ARROW_DOWN;
                                    break;
                              case 31:
                                    i = ACC_FLAT_ARROW_BOTH;
                                    break;
                              case 32:
                                    i = ACC_NATURAL_ARROW_UP;
                                    break;
                              case 33:
                                    i = ACC_NATURAL_ARROW_DOWN;
                                    break;
                              case 34:
                                    i = ACC_NATURAL_ARROW_BOTH;
                                    break;
                               case 35:
                                    i = ACC_SORI;
                                    break;
                               case 36:
                                    i = ACC_KORON;
                                    break;                                                                        
                              default:
                                    i = 0;
                                    break;
                              }
                        Element::setSubtype(i);
                        }
                  else
                        setSubtype(e.text());
                  }
            else if (Element::readProperties(e))
                  ;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Accidental::write(Xml& xml) const
      {
      xml.stag(name());
      if (_hasBracket)
            xml.tag("bracket", _hasBracket);
      Element::writeProperties(xml);
      xml.etag();
      }
