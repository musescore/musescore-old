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

#include "tremolo.h"
#include "layout.h"
#include "score.h"
#include "style.h"
#include "chord.h"
#include "note.h"

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

void Tremolo::draw(QPainter& p) const
      {
      p.setBrush(p.pen().color());
      p.drawPath(path);
      if ((parent() == 0) && (subtype() < 3)) {
            double x = 0.0; // bbox().width() * .25;
            QPen pen(p.pen());
            pen.setWidthF(score()->style()->stemWidth.point());
            p.setPen(pen);
            p.drawLine(x, -_spatium*.5, x, bbox().height() + _spatium);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tremolo::layout(ScoreLayout* layout)
      {
      double sp  = layout->spatium();
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
      }

//---------------------------------------------------------
//   layout2
//    called after notes have their x position
//---------------------------------------------------------

void Tremolo::layout2(ScoreLayout*)
      {
      _chord2 = static_cast<Chord*>(parent());
      if (_chord2 == 0)
            return;
      Stem* stem = _chord2->stem();
      qreal x, y, h;
      if (stem) {
            x = stem->pos().x();
            y  = stem->pos().y();
            h  = stem->stemLen().point();
            }
      else {
            // center tremolo above note
            Note* upnote = _chord2->upNote();
            x = upnote->x() + upnote->headWidth() * .5;
            y = 0.0;
            h = 3 * _spatium;
            }
      y += (h - bbox().height()) * .5;
      if (!twoNotes()) {
            setPos(x, y);
            return;
            }
      Note* anchor2   = _chord2->upNote();
      Segment* s = _chord2->segment();
      s = s->prev();
      while (s) {
            if (s->subtype() == Segment::SegChordRest && s->element(track()))
                  break;
            s = s->prev();
            }
      if (s == 0) {
            printf("no segment for first note of tremolo found\n");
            return;
            }
      ChordRest* cr = static_cast<ChordRest*>(s->element(track()));
      if (cr == 0 || cr->type() != CHORD) {
            printf("no first note for tremolo found, track %d\n", track());
            return;
            }
      _chord1       = static_cast<Chord*>(cr);
      Note* anchor1 = _chord1->upNote();
      QPointF p1    = anchor1->canvasPos();
      QPointF p2    = anchor2->canvasPos();
      double x1     = anchor1->headWidth() - ((p2.x() - p1.x()) * .5);

      setPos(x1, y);
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

