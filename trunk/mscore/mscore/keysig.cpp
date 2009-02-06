//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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

#include "sym.h"
#include "staff.h"
#include "clef.h"
#include "keysig.h"
#include "viewer.h"
#include "system.h"
#include "segment.h"
#include "measure.h"

const char* keyNames[15] = {
      QT_TRANSLATE_NOOP("MuseScore", "G major, E minor"),   QT_TRANSLATE_NOOP("MuseScore", "Cb major, Ab minor"),
      QT_TRANSLATE_NOOP("MuseScore", "D major, B minor"),   QT_TRANSLATE_NOOP("MuseScore", "Gb major, Eb minor"),
      QT_TRANSLATE_NOOP("MuseScore", "A major, F# minor"),  QT_TRANSLATE_NOOP("MuseScore", "Db major, Bb minor"),
      QT_TRANSLATE_NOOP("MuseScore", "E major, C# minor"),  QT_TRANSLATE_NOOP("MuseScore", "Ab major, F minor"),
      QT_TRANSLATE_NOOP("MuseScore", "B major, G# minor"),  QT_TRANSLATE_NOOP("MuseScore", "Eb major, C minor"),
      QT_TRANSLATE_NOOP("MuseScore", "F# major, D# minor"), QT_TRANSLATE_NOOP("MuseScore", "Bb major, G minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C# major, A# minor"), QT_TRANSLATE_NOOP("MuseScore", "F major,  D minor"),
      QT_TRANSLATE_NOOP("MuseScore", "C major, A minor")
      };

//---------------------------------------------------------
//   KeySig
//---------------------------------------------------------

KeySig::KeySig(Score* s)
  : Element(s)
      {
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF KeySig::canvasPos() const
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
//   add
//---------------------------------------------------------

void KeySig::addLayout(int sym, double x, int line)
      {
      double y = double(line) * .5;
      QPointF pt(x, y);
      pt *= _spatium * mag();
      KeySym* ks = new KeySym;
      ks->sym = sym;
      ks->pos = pt;
      keySymbols.append(ks);
      Sym* s = &symbols[sym];
      _bbox |= s->bbox(mag()).translated(pt);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout(ScoreLayout*)
      {
      foreach(KeySym* ks, keySymbols)
            delete ks;
      keySymbols.clear();

      int yoff = 0;
      int clef = 0;
      if (staff()) {
            clef = staff()->clefList()->clef(tick());
            yoff = clefTable[clef].yOffset;
            }

      _bbox    = QRectF(0, 0, 0, 0);
      char t1  = subtype() & 0xff;
      char t2  = (subtype() & 0xff00) >> 8;
      qreal xo = 0.0;


      switch(t2) {
            case 7:   addLayout(naturalSym, 6.0, clefTable[clef].lines[6]);
            case 6:   addLayout(naturalSym, 5.0, clefTable[clef].lines[5]);
            case 5:   addLayout(naturalSym, 4.0, clefTable[clef].lines[4]);
            case 4:   addLayout(naturalSym, 3.0, clefTable[clef].lines[3]);
            case 3:   addLayout(naturalSym, 2.0, clefTable[clef].lines[2]);
            case 2:   addLayout(naturalSym, 1.0, clefTable[clef].lines[1]);
            case 1:   addLayout(naturalSym, 0.0, clefTable[clef].lines[0]);
                  xo = double(t2) + .5;
                  break;
            case -7:  addLayout(naturalSym, 6.0, clefTable[clef].lines[13]);
            case -6:  addLayout(naturalSym, 5.0, clefTable[clef].lines[12]);
            case -5:  addLayout(naturalSym, 4.0, clefTable[clef].lines[11]);
            case -4:  addLayout(naturalSym, 3.0, clefTable[clef].lines[10]);
            case -3:  addLayout(naturalSym, 2.0, clefTable[clef].lines[9]);
            case -2:  addLayout(naturalSym, 1.0, clefTable[clef].lines[8]);
            case -1:  addLayout(naturalSym, 0.0, clefTable[clef].lines[7]);
            case 0:
                  xo = double(-t2) + .5;
                  break;
            default:
                  printf("illegal t2 key %d (t1=%d) subtype 0x%04x\n", t2, t1, subtype());
                  break;
            }

      switch(t1) {
            case 7:  addLayout(sharpSym, xo + 6.0, clefTable[clef].lines[6]);
            case 6:  addLayout(sharpSym, xo + 5.0, clefTable[clef].lines[5]);
            case 5:  addLayout(sharpSym, xo + 4.0, clefTable[clef].lines[4]);
            case 4:  addLayout(sharpSym, xo + 3.0, clefTable[clef].lines[3]);
            case 3:  addLayout(sharpSym, xo + 2.0, clefTable[clef].lines[2]);
            case 2:  addLayout(sharpSym, xo + 1.0, clefTable[clef].lines[1]);
            case 1:  addLayout(sharpSym, xo + 0.0, clefTable[clef].lines[0]);
                     break;
            case -7: addLayout(flatSym, xo + 6.0, clefTable[clef].lines[13]);
            case -6: addLayout(flatSym, xo + 5.0, clefTable[clef].lines[12]);
            case -5: addLayout(flatSym, xo + 4.0, clefTable[clef].lines[11]);
            case -4: addLayout(flatSym, xo + 3.0, clefTable[clef].lines[10]);
            case -3: addLayout(flatSym, xo + 2.0, clefTable[clef].lines[9]);
            case -2: addLayout(flatSym, xo + 1.0, clefTable[clef].lines[8]);
            case -1: addLayout(flatSym, xo + 0.0, clefTable[clef].lines[7]);
            case 0:
                  break;
            default:
                  printf("illegal t1 key %d (t2=%d) subtype 0x%04x\n", t1, t2, subtype());
                  break;
            }
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void KeySig::draw(QPainter& p) const
      {
      foreach(const KeySym* ks, keySymbols) {
            symbols[ks->sym].draw(p, mag(), ks->pos.x(), ks->pos.y());
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(Viewer* v, const QPointF&, int type, int) const
      {
      if (type == KEYSIG) {
            v->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* KeySig::drop(const QPointF&, const QPointF&, Element* e)
      {
      if (e->type() == KEYSIG) {
            KeySig* k = static_cast<KeySig*>(e);
            char stype = k->subtype() & 0xff;
            delete k;
            int st = subtype();
            if (st != stype)
                  staff()->changeKeySig(tick(), stype);
            return this;
            }
      delete e;
      return 0;
      }

//---------------------------------------------------------
//   setSig
//---------------------------------------------------------

void KeySig::setSig(int old, int newSig)
      {
      setSubtype(((old & 0xff) << 8) | (newSig & 0xff));
      }


