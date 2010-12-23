//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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

#include "accidental.h"
#include "symbol.h"
#include "sym.h"

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

Accidental::Accidental(Score* sc)
  : Compound(sc)
      {
      }

//---------------------------------------------------------
//   setSubtype
//    0 - no accidental
//    1 - sharp
//    2 - flat
//    3 - double sharp
//    4 - double flat
//    5 - natural
//    6 - (sharp)          11 - [sharp]
//    7 - (flat)           12 - [flat]
//    8 - (double sharp)   13 - [double sharp]
//    9 - (double flat)    14 - [double flat]
//    10 - (natural)       15 - [natural]
//
//    16 - flat-slash
//    17 - flat-slash2
//    18 - mirrored-flat2
//    19 - mirrored-flat
//    20 - mirrored-flat-slash
//    21 - flat-flat-slash
//
//    22 - sharp-slash
//    23 - sharp-slash2
//    24 - sharp-slash3
//    25 - sharp-slash4
//---------------------------------------------------------

void Accidental::setSubtype(int i)
      {
      if (subtype() == i)
            return;
      Element::setSubtype(i);
      clear();

      Symbol* s = new Symbol(score());
      switch(i) {
            default:
                  printf("illegal accidental %d\n", i);
                  abort();

            case  ACC_NONE:    delete s; return;
            case  ACC_SHARP:   s->setSym(sharpSym);      break;
            case  ACC_FLAT:    s->setSym(flatSym);       break;
            case  ACC_SHARP2:  s->setSym(sharpsharpSym); break;
            case  ACC_FLAT2:   s->setSym(flatflatSym);   break;
            case  ACC_NATURAL: s->setSym(naturalSym);    break;

            case  6 ... 10:
                  {
                  s->setSym(leftparenSym);
                  addElement(s, 0.0, 0.0);
                  double x = symbols[leftparenSym].width(mag());

                  s = new Symbol(score());
                  switch(i) {
                        case  6: s->setSym(sharpSym);      break;
                        case  7: s->setSym(flatSym);       break;
                        case  8: s->setSym(sharpsharpSym); break;
                        case  9: s->setSym(flatflatSym);   break;
                        case 10: s->setSym(naturalSym);    break;
                        }
                  addElement(s, x - s->bbox().x(), 0.0);
                  x += s->width();
                  s = new Symbol(score());
                  s->setSym(rightparenSym);
                  addElement(s, x, 0.0);
                  }
                  return;

            case 11: s->setSym(sharpSym);      break;
            case 12: s->setSym(flatSym);       break;
            case 13: s->setSym(sharpsharpSym); break;
            case 14: s->setSym(flatflatSym);   break;
            case 15: s->setSym(naturalSym);    break;

            case 16: s->setSym(flatslashSym);         break;
            case 17: s->setSym(flatslash2Sym);        break;
            case 18: s->setSym(mirroredflat2Sym);     break;
            case 19: s->setSym(mirroredflatSym);      break;
            case 20: s->setSym(mirroredflatslashSym); break;
            case 21: s->setSym(flatflatslashSym);     break;

            case 22: s->setSym(sharpslashSym); break;
            case 23: s->setSym(sharpslash2Sym); break;
            case 24: s->setSym(sharpslash3Sym); break;
            case 25: s->setSym(sharpslash4Sym); break;
            }
      addElement(s, 0.0, 0.0);
      setMag(mag());
      }

//---------------------------------------------------------
//   subtype2value
//    returns the resulting pitch offset
//---------------------------------------------------------

int Accidental::subtype2value(int st)
      {
      static const int preTab[] = {
            0,  // ACC_NONE
            1,  // ACC_SHARP
            -1, // ACC_FLAT
            2,  // ACC_SHARP2
            -2, // ACC_FLAT2
            0,  // ACC_NAT
            1, -1, 2, -2, 0,  // () brackets
            1, -1, 2, -2, 0,  // [] brackets
            0, 0, 0, 0, 0, 0,  // special flats
            0, 0, 0, 0        // spacial sharps
            };

      if (st < 0 || st >= int(sizeof(preTab)/sizeof(*preTab)))
            abort();

      return preTab[st];
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
//   setMag
//---------------------------------------------------------

void Accidental::setMag(double v)
      {
      Element::setMag(v);
      foreach(Element* e, getElemente())
            e->setMag(v);
      }
