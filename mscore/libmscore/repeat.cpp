//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: repeat.cpp 3646 2010-10-29 11:50:37Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "repeat.h"
#include "sym.h"
#include "score.h"
#include "system.h"
#include "measure.h"
#include "globals.h"
#include "al/xml.h"
#include "painter.h"

//---------------------------------------------------------
//   RepeatMeasure
//---------------------------------------------------------

RepeatMeasure::RepeatMeasure(Score* score)
   : Rest(score)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RepeatMeasure::draw(Painter* p) const
      {
      p->setBrush(p->penColor());
//      p->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RepeatMeasure::layout()
      {
#if 0
      qreal sp  = spatium();

      qreal y   = sp;
      qreal w   = sp * 2.0;
      qreal h   = sp * 2.0;
      qreal lw  = sp * .30;  // line width
      qreal r   = sp * .15;  // dot radius

      path       = QPainterPath();

      path.moveTo(w - lw, y);
      path.lineTo(w,  y);
      path.lineTo(lw,  h+y);
      path.lineTo(0.0, h+y);
      path.closeSubpath();
      path.addEllipse(QRectF(w * .25 - r, y+h * .25 - r, r * 2.0, r * 2.0 ));
      path.addEllipse(QRectF(w * .75 - r, y+h * .75 - r, r * 2.0, r * 2.0 ));

      setbbox(path.boundingRect());
#endif
      }

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setSubtype(TEXT_REPEAT);
      setTextStyle(TEXT_STYLE_REPEAT);
      }

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(MarkerType t)
      {
      _markerType = t;
      switch(t) {
            case MARKER_SEGNO:
//                  setHtml(symToHtml(symbols[score()->symIdx()][segnoSym], 8));
                  setLabel("segno");
                  break;

            case MARKER_CODA:
//                  setHtml(symToHtml(symbols[score()->symIdx()][codaSym], 8));
                  setLabel("codab");
                  break;

            case MARKER_VARCODA:
//                  setHtml(symToHtml(symbols[score()->symIdx()][varcodaSym], 8));
                  setLabel("varcoda");
                  break;

            case MARKER_CODETTA:
//                  setHtml(symToHtml(symbols[score()->symIdx()][codaSym], symbols[score()->symIdx()][codaSym], 8));
                  setLabel("codetta");
                  break;

            case MARKER_FINE:
                  setText("Fine");
                  setLabel("fine");
                  break;

            case MARKER_TOCODA:
                  setText("To Coda");
                  setLabel("coda");
                  break;

            case MARKER_USER:
                  break;

            default:
                  printf("unknown marker type %d\n", t);
                  break;
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Marker::styleChanged()
      {
      setMarkerType(_markerType);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Marker::layout()
      {
      Text::layout();
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Marker::canvasPos() const
      {
      if (parent())
            return measure()->canvasPos() + pos();
      return pos();
      }

//---------------------------------------------------------
//   markerType
//---------------------------------------------------------

MarkerType Marker::markerType(const QString& s) const
      {
      if (s == "segno")
            return MARKER_SEGNO;
      else if (s == "codab")
            return MARKER_CODA;
      else if (s == "varcoda")
            return MARKER_VARCODA;
      else if (s == "codetta")
            return MARKER_CODETTA;
      else if (s == "fine")
            return MARKER_FINE;
      else if (s == "coda")
            return MARKER_TOCODA;
      else
            return MARKER_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Marker::read(XmlReader* r)
      {
      MarkerType mt;
      while (r->readElement()) {
            QString s;
            if (r->readString("label", &s)) {
                  setLabel(s);
                  mt = markerType(s);
                  }
            else if (!Text::readProperties(r))
                  r->unknown();
            }
      switch(mt) {
            case MARKER_SEGNO:
            case MARKER_CODA:
            case MARKER_VARCODA:
            case MARKER_CODETTA:
                  setTextStyle(TEXT_STYLE_REPEAT_LEFT);
                  break;

            case MARKER_FINE:
            case MARKER_TOCODA:
                  setTextStyle(TEXT_STYLE_REPEAT_RIGHT);
                  break;

            case MARKER_USER:
                  setTextStyle(TEXT_STYLE_REPEAT);
                  break;
            }
      setMarkerType(mt);
      }

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setSubtype(TEXT_REPEAT);
      setTextStyle(TEXT_STYLE_REPEAT);
      }

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(int t)
      {
      switch(t) {
            case JUMP_DC:
                  setText("D.C.");
                  setJumpTo("start");
                  setPlayUntil("end");
                  break;

            case JUMP_DC_AL_FINE:
                  setText("D.C. al Fine");
                  setJumpTo("start");
                  setPlayUntil("fine");
                  break;

            case JUMP_DC_AL_CODA:
                  setText("D.C. al Coda");
                  setJumpTo("start");
                  setPlayUntil("coda");
                  setContinueAt("codab");
                  break;

            case JUMP_DS_AL_CODA:
                  setText("D.S. al Coda");
                  setJumpTo("segno");
                  setPlayUntil("coda");
                  setContinueAt("codab");
                  break;

            case JUMP_DS_AL_FINE:
                  setText("D.S. al Fine");
                  setJumpTo("segno");
                  setPlayUntil("fine");
                  break;

            case JUMP_DS:
                  setText("D.S.");
                  setJumpTo("segno");
                  setPlayUntil("end");
                  break;

            case JUMP_USER:
                  break;

            default:
                  printf("unknown jump type\n");
                  break;
            }
      }

//---------------------------------------------------------
//   jumpType
//---------------------------------------------------------

int Jump::jumpType() const
      {
      if (_jumpTo == "start" && _playUntil == "end" && _continueAt == "")
            return JUMP_DC;
      else if (_jumpTo == "start" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DC_AL_FINE;
      else if (_jumpTo == "start" && _playUntil == "coda" && _continueAt == "codab")
            return JUMP_DC_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "coda" && _continueAt == "codab")
            return JUMP_DS_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DS_AL_FINE;
      else if (_jumpTo == "segno" && _playUntil == "end" && _continueAt == "")
            return JUMP_DS;
      return JUMP_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Jump::read(XmlReader* r)
      {
      while (r->readElement()) {
            QString s;
            if (r->readString("jumpTo", &_jumpTo))
                  ;
            else if (r->readString("playUntil", &_playUntil))
                  ;
            else if (r->readString("continueAt", &_continueAt))
                  ;
            else if (!Text::readProperties(r))
                  r->unknown();
            }
      setTextStyle(TEXT_STYLE_REPEAT_RIGHT);
      }

