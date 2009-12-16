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

#include "sym.h"
#include "staff.h"
#include "clef.h"
#include "keysig.h"
// #include "scoreview.h"
#include "system.h"
#include "segment.h"
#include "measure.h"
#include "score.h"

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
      pt *= spatium();
      KeySym* ks = new KeySym;
      ks->sym = sym;
      ks->pos = pt;
      keySymbols.append(ks);
      Sym* s = &symbols[sym];
      _bbox |= s->bbox(magS()).translated(pt);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout()
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


      int accidentals = 0;
      switch(t1) {
            case 7:
            case -7:
                  accidentals = 0x7f;
                  break;
            case 6:
            case -6:
                  accidentals = 0x3f;
                  break;
            case 5:
            case -5:
                  accidentals = 0x1f;
                  break;
            case 4:
            case -4:
                  accidentals = 0xf;
                  break;
            case 3:
            case -3:
                  accidentals = 0x7;
                  break;
            case 2:
            case -2:
                  accidentals = 0x3;
                  break;
            case 1:
            case -1:
                  accidentals = 0x1;
                  break;

            case 0:   break;
            default:
                  printf("illegal t2 key %d (t1=%d) subtype 0x%04x\n", t2, t1, subtype());
                  break;
            }

      int naturals = 0;
      switch(t2) {
            case 7:
            case -7:
                  naturals = 0x7f;
                  break;
            case 6:
            case -6:
                  naturals = 0x3f;
                  break;
            case 5:
            case -5:
                  naturals = 0x1f;
                  break;
            case 4:
            case -4:
                  naturals = 0xf;
                  break;
            case 3:
            case -3:
                  naturals = 0x7;
                  break;
            case 2:
            case -2:
                  naturals = 0x3;
                  break;
            case 1:
            case -1:
                  naturals = 0x1;
                  break;

            case 0:   break;
            default:
                  printf("illegal t2 key %d (t1=%d) subtype 0x%04x\n", t2, t1, subtype());
                  break;
            }

      xo = 0.0;
      int coffset = t2 < 0 ? 7 : 0;

      if (!((t1 > 0) ^ (t2 > 0)))
            naturals &= ~accidentals;

      for (int i = 0; i < 7; ++i) {
            if (naturals & (1 << i)) {
                  addLayout(naturalSym, xo, clefTable[clef].lines[i + coffset]);
                  xo += 1.0;
                  }
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
            symbols[ks->sym].draw(p, magS(), ks->pos.x(), ks->pos.y());
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool KeySig::acceptDrop(ScoreView*, const QPointF&, int type, int) const
      {
      return type == KEYSIG;
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

//---------------------------------------------------------
//   setOldSig
//---------------------------------------------------------

void KeySig::setOldSig(int old)
      {
      int newSig = subtype() & 0xff;
      setSubtype(((old & 0xff) << 8) | (newSig & 0xff));
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space KeySig::space() const
      {
      return Space(point(score()->styleS(ST_keysigLeftMargin)), width());
      }


