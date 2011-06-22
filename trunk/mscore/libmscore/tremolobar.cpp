//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#include "tremolobar.h"
#include "score.h"
#include "undo.h"
#include "staff.h"
#include "chord.h"
#include "note.h"
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
      double _spatium = spatium();

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
//      double x = noteWidth * .5;
//      double y = notePos.y() - _spatium;
//      double x2, y2;

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

void TremoloBar::draw(Painter* painter) const
      {
      if (staff() && !staff()->useTablature())
            return;
      painter->setLineWidth(_lw);
      painter->setCapStyle(Qt::RoundCap);
      painter->setJoinStyle(Qt::RoundJoin);
      painter->setBrushColor(Qt::black);

      double _spatium = spatium();
      const TextStyle* st = &score()->textStyle(TEXT_STYLE_BENCH);
      QFont f = st->fontPx(_spatium);
      painter->setFont(f);

      int n    = _points.size();
//      int pt   = 0;
//      double x = noteWidth;
//      double y = -_spatium * .8;
//      double x2, y2;

      for (int pt = 0; pt < n; ++pt) {
            if (pt == (n-1))
                  break;
//            int pitch = _points[pt].pitch;
            }
      //debug:
      painter->drawLine(0.0, 0.0, _spatium*1.5, _spatium*3);
      painter->drawLine(_spatium*1.5, _spatium*3, _spatium*3, 0.0);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TremoloBar::write(Xml& xml) const
      {
      xml.stag("TremoloBar");
      foreach(const PitchValue& v, _points) {
            xml.tagE(QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
               .arg(v.time).arg(v.pitch).arg(v.vibrato));
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TremoloBar::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull();  e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "point") {
                  PitchValue pv;
                  pv.time    = e.attribute("time").toInt();
                  pv.pitch   = e.attribute("pitch").toInt();
                  pv.vibrato = e.attribute("vibrato").toInt();
                  _points.append(pv);
                  }
            else
                  domError(e);
            }
      }

