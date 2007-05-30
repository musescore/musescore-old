//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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
            case  0: delete s; return;
            case  1: s->setSym(sharpSym);      break;
            case  2: s->setSym(flatSym);       break;
            case  3: s->setSym(sharpsharpSym); break;
            case  4: s->setSym(flatflatSym);   break;
            case  5: s->setSym(naturalSym);    break;

            case  6 ... 10:
                  {
                  s->setSym(leftparenSym);
                  addElement(s, 0.0, 0.0);
                  double x = symbols[leftparenSym].width();

                  s = new Symbol(score());
                  switch(i) {
                        case  6: s->setSym(s_sharpSym);    break;
                        case  7: s->setSym(flatSym);       break;
                        case  8: s->setSym(sharpsharpSym); break;
                        case  9: s->setSym(flatflatSym);   break;
                        case 10: s->setSym(naturalSym);    break;
                        }
                  addElement(s, x, 0.0);
                  x += s->width();
                  s = new Symbol(score());
                  s->setSym(rightparenSym);
                  addElement(s, x, 0.0);
                  }
                  return;

            // small symbols:

            case 11: s->setSym(sharpSym);      break;
            case 12: s->setSym(flatSym);       break;
            case 13: s->setSym(sharpsharpSym); break;
            case 14: s->setSym(flatflatSym);   break;
            case 15: s->setSym(naturalSym);    break;

            case 101: s->setSym(s_sharpSym);      break;
            case 102: s->setSym(s_flatSym);       break;
            case 103: s->setSym(s_sharpsharpSym); break;
            case 104: s->setSym(s_flatflatSym);   break;
            case 105: s->setSym(s_naturalSym);    break;

            case 106: s->setSym(s_sharpSym);      break;
            case 107: s->setSym(s_flatSym);       break;
            case 108: s->setSym(s_sharpsharpSym); break;
            case 109: s->setSym(s_flatflatSym);   break;
            case 110: s->setSym(s_naturalSym);    break;

            case 111: s->setSym(s_sharpSym);      break;
            case 112: s->setSym(s_flatSym);       break;
            case 113: s->setSym(s_sharpsharpSym); break;
            case 114: s->setSym(s_flatflatSym);   break;
            case 115: s->setSym(s_naturalSym);    break;
            }
      addElement(s, 0.0, 0.0);
      }

//---------------------------------------------------------
//   value
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
            0, 0, 0, 0, 0,  // () brackets
            0, 0, 0, 0, 0,  // [] brackets
            };
      return preTab[st % 100];
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Accidental::startEdit(QMatrix&, const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Accidental::edit(QKeyEvent* ev)
      {
      int key = ev->key();

      qreal o = 0.2;
      if (ev->modifiers() & Qt::ControlModifier)
            o = 0.02;
      QPointF p = userOff();
      switch (key) {
            case Qt::Key_Left:
                  p.setX(p.x() - o);
                  break;

            case Qt::Key_Right:
                  p.setX(p.x() + o);
                  break;

            case Qt::Key_Up:
                  p.setY(p.y() - o);
                  break;

            case Qt::Key_Down:
                  p.setY(p.y() + o);
                  break;

            case Qt::Key_Home:
                  p = QPointF(0.0, 0.0);    // reset to zero
                  break;
            }
      setUserOff(p);
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Accidental::endEdit()
      {
      }
