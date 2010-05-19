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

#include "tremolo.h"
#include "score.h"
#include "style.h"
#include "chord.h"
#include "note.h"
#include "measure.h"

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

Tremolo::Tremolo(Score* score)
   : Element(score)
      {
      _chord1 = 0;
      _chord2 = 0;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Tremolo::draw(QPainter& p, ScoreView*) const
      {
      p.setBrush(p.pen().color());
      p.drawPath(path);
      if ((parent() == 0) && (subtype() < 3)) {
            double x = 0.0; // bbox().width() * .25;
            QPen pen(p.pen());
            pen.setWidthF(point(score()->styleS(ST_stemWidth)));
            p.setPen(pen);
            double _spatium = spatium();
            p.drawLine(QLineF(x, -_spatium*.5, x, bbox().height() + _spatium));
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tremolo::layout()
      {
      double sp  = spatium();
      double w   = sp * 1.2;
      double h   = sp * .8;
      double lw  = sp * .35;
      double d   = sp * 0.8;
      path       = QPainterPath();

      qreal y = 0.0;
      int lines = (subtype() % 3) + 1;
      for (int i = 0; i < lines; ++i) {
            path.moveTo(-w*.5, y + h - lw);
            path.lineTo(w*.5,  y);
            path.lineTo(w*.5,  y + lw);
            path.lineTo(-w*.5, y + h);
            path.closeSubpath();
            y += d;
            }
      setbbox(path.boundingRect());

      _chord2 = static_cast<Chord*>(parent());
      if (_chord2 == 0)
            return;
      Note* anchor2 = _chord2->upNote();
      Stem* stem    = _chord2->stem();
      qreal x;
      if (stem) {
            x = stem->pos().x();
            y  = stem->pos().y();
            h  = stem->stemLen();
            }
      else {
            // center tremolo above note
            x = anchor2->x() + anchor2->headWidth() * .5;
            y = anchor2->y();
            h = 2.0 * spatium() + bbox().height();
            if (anchor2->line() > 4)
                  h *= -1;
            }
      y += (h - bbox().height()) * .5;
      if (!twoNotes()) {
            if (_chord2->hook())
                  y -= spatium() * .5 * (_chord2->up() ? -1.0 : 1.0);
            setPos(x, y);
            return;
            }
      Segment* s = _chord2->segment()->prev();
      while (s) {
            if (s->element(track()) && (s->element(track())->type() == CHORD))
                  break;
            s = s->prev();
            }
      if (s == 0) {
            printf("no first note of tremolo found\n");
            return;
            }
      _chord1       = static_cast<Chord*>(s->element(track()));
      Note* anchor1 = _chord1->upNote();
      double x1     = anchor1->canvasPos().x();
      double x2     = anchor2->canvasPos().x();
      x             = anchor2->pos().x() + (x1 - x2) * .5;
      if (_chord1->up())
            x += anchor1->headWidth();
      setPos(x, y);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tremolo::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tremolo::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      }

