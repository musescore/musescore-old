//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "tremolobar.h"
#include "score.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
#include "al/xml.h"
#include "painter.h"

//---------------------------------------------------------
//   TremoloBar
//---------------------------------------------------------

TremoloBar::TremoloBar(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TremoloBar::layout()
      {
      qreal _spatium = spatium();

      if (staff() && !staff()->useTablature()) {
            setbbox(QRectF());
            if (!parent()) {
                  noteWidth = -_spatium*2;
                  notePos   = QPointF(0.0, _spatium*3);
                  }
            return;
            }

      _lw = _spatium * 0.1;
      Note* note = 0;
      if (note == 0) {
            noteWidth = 0.0;
            notePos = QPointF();
            }
      else {
            noteWidth = note->width();
            notePos = note->pos();
            }
//      int n    = _points.size();
//      int pt   = 0;
//      qreal x = noteWidth * .5;
//      qreal y = notePos.y() - _spatium;
//      qreal x2, y2;

      QRectF bb (0, 0, _spatium*3, -_spatium * 4);
#if 0
      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            x = x2;
            y = y2;
            }
#endif
      bb.adjust(-_lw, -_lw, _lw, _lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TremoloBar::draw(Painter* p) const
      {
      if (staff() && !staff()->useTablature())
            return;
      p->setLineCap(Qt::RoundCap);
      p->setLineJoin(Qt::RoundJoin);
      p->setPenWidth(_lw);
      p->setBrush(Color(0, 0, 0));

      qreal _spatium = spatium();
//      const TextStyle* st = &score()->textStyle(TEXT_STYLE_BENCH);
//      Font f = st->fontPx(_spatium);
//      p->setFont(f);

      int n    = _points.size();

      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
            }
      //debug:
      p->drawLine(0.0, 0.0, _spatium*1.5, _spatium*3);
      p->drawLine(_spatium*1.5, _spatium*3, _spatium*3, 0.0);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TremoloBar::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (r->tag() == "point") {
                  PitchValue pv;
                  while (r->readAttribute()) {
                        if (r->tag() == "time")
                              pv.time = r->intValue();
                        else if (r->tag() == "pitch")
                              pv.pitch = r->intValue();
                        else if (r->tag() == "vibrato")
                              pv.vibrato = r->intValue();
                        }
                  _points.append(pv);
                  }
            else
                  r->unknown();
            }
      }

