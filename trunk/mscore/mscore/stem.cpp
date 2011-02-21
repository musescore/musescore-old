//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chord.h 3601 2010-10-22 12:46:05Z wschweer $
//
//  Copyright (C) 2010-2011 Werner Schweer and others
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

#include "stem.h"
#include "staff.h"
#include "chord.h"
#include "score.h"
#include "stafftype.h"
#include "hook.h"
#include "tremolo.h"
#include "painter.h"

// TEMPORARY HACK!!
#include "sym.h"
// END OF HACK

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      _len = 0.0;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
      bool useTab = false;
      Staff* st = staff();
      if (st && st->useTablature()) {     // stems used in palette do not have a staff
            useTab = true;
            if (st->staffType()->slashStyle())
                  return;
            }
      qreal lw = point(score()->styleS(ST_stemWidth));
      QPen pen(p.pen());
      pen.setWidthF(lw);
      pen.setCapStyle(Qt::RoundCap);
      p.setPen(pen);
      p.drawLine(QLineF(0.0, 0.0, 0.0, stemLen()) );
      // NOT THE BEST PLACE FOR THIS?
      // with tablatures, dots are not drawn near 'notes', but near stems
      if (useTab) {
            int nDots = chord()->dots();
            if (nDots > 0)
                  symbols[score()->symIdx()][dotSym].draw(p, magS(), spatium(), stemLen(), nDots);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Stem::write(Xml& xml) const
      {
      xml.stag("Stem");
      Element::writeProperties(xml);
      if (_userLen.val() != 0.0)
            xml.sTag("userLen", _userLen);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Stem::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "userLen")
                  _userLen = Spatium(e.text().toDouble());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Stem::setVisible(bool f)
      {
      Element::setVisible(f);
      Chord* chord = static_cast<Chord*>(parent());
      if (chord && chord->hook() && chord->hook()->visible() != f)
            chord->hook()->setVisible(f);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Stem::bbox() const
      {
      double w = point(score()->styleS(ST_stemWidth));
      double l = _len + point(_userLen);
      return QRectF(-w * .5, 0, w, l).normalized();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Stem::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      QPointF p(0.0, stemLen());
      grip[0].translate(canvasPos() + p);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Stem::editDrag(int, const QPointF& delta)
      {
      _userLen += Spatium(delta.y() / spatium());
      Chord* c = static_cast<Chord*>(parent());
      if (c->hook())
            c->hook()->move(0.0, delta.y());
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void Stem::toDefault()
      {
      _userLen = Spatium(0.0);
      setUserOff(QPointF());
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Stem::acceptDrop(ScoreView*, const QPointF&, int type, int subtype) const
      {
      if ((type == TREMOLO) && (subtype <= TREMOLO_R64)) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Stem::drop(const DropData& data)
      {
      Element* e = data.element;
      Chord* ch = chord();
      switch(e->type()) {
            case TREMOLO:
                  e->setParent(ch);
                  score()->setLayout(ch->measure());
                  score()->undoAddElement(e);
                  break;
            default:
                  delete e;
                  break;
            }
      return 0;
      }


