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

void KeySig::addLayout(Sym* s, double x, double y)
      {
      _bbox |= s->bbox().translated(x * _spatium, y * _spatium);
      }

//---------------------------------------------------------
//   yoffset
//---------------------------------------------------------

double KeySig::yoffset() const
      {
      if (staff()) {
            int clef       = staff()->clef()->clef(tick());
            int clefOffset = clefTable[clef].yOffset;
            while (clefOffset >= 7)
                  clefOffset -= 7;
            while (clefOffset < 0)
                  clefOffset += 7;
            return double(clefOffset / 2.0);
            }
      return .0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void KeySig::layout(ScoreLayout*)
      {
      Sym* ss     = &symbols[sharpSym];
      Sym* fs     = &symbols[flatSym];
      Sym* ns     = &symbols[naturalSym];
      double yoff = yoffset();
      _bbox       = QRectF(0, 0, 0, 0);

      char t1  = subtype() & 0xff;
      char t2  = (subtype() & 0xff00) >> 8;
      qreal xo = 0.0;

      switch(t2) {
            case 7:
                  xo += 1.0;
                  addLayout(ns, 6.0, yoff + 2.0);
            case 6:
                  xo += 1.0;
                  addLayout(ns, 5.0, yoff + 0.5);
            case 5:
                  xo += 1.0;
                  addLayout(ns, 4.0, yoff + 2.5);
            case 4:
                  xo += 1.0;
                  addLayout(ns, 3.0, yoff + 1.0);
            case 3:
                  xo += 1.0;
                  addLayout(ns, 2.0, yoff - 0.5);
            case 2:
                  xo += 1.0;
                  addLayout(ns, 1.0, yoff + 1.5);
            case 1:
                  xo += 1.5;
                  addLayout(ns, 0.0, yoff * _spatium);
                  break;
            case -7:    addLayout(ns, 6.0, yoff + 3.5); xo += 1.0;
            case -6:    addLayout(ns, 5.0, yoff + 1.5); xo += 1.0;
            case -5:    addLayout(ns, 4.0, yoff + 3);   xo += 1.0;
            case -4:    addLayout(ns, 3.0, yoff + 1);   xo += 1.0;
            case -3:    addLayout(ns, 2.0, yoff + 2.5); xo += 1.0;
            case -2:    addLayout(ns, 1.0, yoff + .5);  xo += 1.0;
            case -1:    addLayout(ns, 0.0, yoff + 2.0); xo += 1.5;
            case 0:
                  break;
            default:
                  printf("illegal t2 key %02x\n", t2);
                  break;
            }

      switch(t1) {
            case 7:     addLayout(ss, xo + 6.0, yoff + 2.0);
            case 6:     addLayout(ss, xo + 5.0, yoff + .5);
            case 5:     addLayout(ss, xo + 4.0, yoff + 2.5);
            case 4:     addLayout(ss, xo + 3.0, yoff + 1);
            case 3:     addLayout(ss, xo + 2.0, yoff - .5);
            case 2:     addLayout(ss, xo + 1.0, yoff + 1.5);
            case 1:     addLayout(ss, xo + 0.0, yoff);
                        break;
            case -7:    addLayout(fs, xo + 6.0, yoff + 3.5);
            case -6:    addLayout(fs, xo + 5.0, yoff + 1.5);
            case -5:    addLayout(fs, xo + 4.0, yoff + 3);
            case -4:    addLayout(fs, xo + 3.0, yoff + 1);
            case -3:    addLayout(fs, xo + 2.0, yoff + 2.5);
            case -2:    addLayout(fs, xo + 1.0, yoff + .5);
            case -1:    addLayout(fs, xo + 0.0, yoff + 2.0);
            case 0:
                  break;
            default:
                  printf("illegal t1 key %02x\n", t1);
                  break;
            }
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void KeySig::draw(QPainter& p) const
      {
      double yoff = yoffset();
      Sym* ss     = &symbols[sharpSym];
      Sym* fs     = &symbols[flatSym];
      Sym* ns     = &symbols[naturalSym];

      char t1     = subtype() & 0xff;
      char t2     = (subtype() & 0xff00) >> 8;
      qreal xo    = 0.0;
      double ld   = _spatium * mag();

      switch(t2) {
            case 7: ns->draw(p, mag(), 6.0 * ld, (yoff + 2.0) * ld); xo += 1.0;
            case 6: ns->draw(p, mag(), 5.0 * ld, (yoff + 0.5) * ld); xo += 1.0;
            case 5: ns->draw(p, mag(), 4.0 * ld, (yoff + 2.5) * ld); xo += 1.0;
            case 4: ns->draw(p, mag(), 3.0 * ld, (yoff + 1.0) * ld); xo += 1.0;
            case 3: ns->draw(p, mag(), 2.0 * ld, (yoff - 0.5) * ld); xo += 1.0;
            case 2: ns->draw(p, mag(), 1.0 * ld, (yoff + 1.5) * ld); xo += 1.0;
            case 1: ns->draw(p, mag(), 0.0, yoff * ld);              xo += 1.5;
                  break;
            case -7: ns->draw(p, mag(), 6.0 * ld, (yoff + 3.5) * ld); xo += 1.0;
            case -6: ns->draw(p, mag(), 5.0 * ld, (yoff + 1.5) * ld); xo += 1.0;
            case -5: ns->draw(p, mag(), 4.0 * ld, (yoff + 3.0) * ld); xo += 1.0;
            case -4: ns->draw(p, mag(), 3.0 * ld, (yoff + 1.0) * ld); xo += 1.0;
            case -3: ns->draw(p, mag(), 2.0 * ld, (yoff + 2.5) * ld); xo += 1.0;
            case -2: ns->draw(p, mag(), 1.0 * ld, (yoff + 0.5) * ld); xo += 1.0;
            case -1: ns->draw(p, mag(), 0.0 * ld, (yoff + 2.0) * ld); xo += 1.5;
            case 0:
                  break;
            default:
                  printf("illegal t2 key %02x\n", t2);
                  break;
            }

      switch(t1) {
            case 7:     ss->draw(p, mag(), (xo + 6.0) * ld, (yoff + 2.0) * ld);
            case 6:     ss->draw(p, mag(), (xo + 5.0) * ld, (yoff + 0.5) * ld);
            case 5:     ss->draw(p, mag(), (xo + 4.0) * ld, (yoff + 2.5) * ld);
            case 4:     ss->draw(p, mag(), (xo + 3.0) * ld, (yoff + 1.0) * ld);
            case 3:     ss->draw(p, mag(), (xo + 2.0) * ld, (yoff - 0.5) * ld);
            case 2:     ss->draw(p, mag(), (xo + 1.0) * ld, (yoff + 1.5) * ld);
            case 1:     ss->draw(p, mag(), (xo + 0.0) * ld, (yoff)       * ld);
                  break;
            case -7:    fs->draw(p, mag(), (xo + 6.0) * ld, (yoff + 3.5) * ld);
            case -6:    fs->draw(p, mag(), (xo + 5.0) * ld, (yoff + 1.5) * ld);
            case -5:    fs->draw(p, mag(), (xo + 4.0) * ld, (yoff + 3.0) * ld);
            case -4:    fs->draw(p, mag(), (xo + 3.0) * ld, (yoff + 1.0) * ld);
            case -3:    fs->draw(p, mag(), (xo + 2.0) * ld, (yoff + 2.5) * ld);
            case -2:    fs->draw(p, mag(), (xo + 1.0) * ld, (yoff + 0.5) * ld);
            case -1:    fs->draw(p, mag(), (xo + 0.0) * ld, (yoff + 2.0) * ld);
            case 0:
                  break;
            default:
                  printf("illegal t1 key %02x\n", t1);
                  break;
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
            KeySig* k = (KeySig*)e;
            char stype = k->subtype() & 0xff;
            delete k;
            int st = subtype();
            if (st != stype) {
                  // change keysig applies to all staves, can't simply set subtype
                  // for this one only
                  staff()->changeKeySig(tick(), stype);
                  }
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
      setSubtype((old << 8) | (newSig & 0xff));
      }


