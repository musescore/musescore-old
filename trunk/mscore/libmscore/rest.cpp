//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: rest.cpp 3708 2010-11-16 09:54:31Z wschweer $
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

#include <math.h>

#include "rest.h"
#include "score.h"
#include "al/xml.h"
#include "style.h"
#include "utils.h"
#include "tuplet.h"
#include "sym.h"
#include "stafftext.h"
#include "articulation.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "staff.h"
#include "harmony.h"
#include "lyrics.h"
#include "segment.h"
#include "painter.h"

//---------------------------------------------------------
//    Rest
//--------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _beamMode  = BEAM_NO;
      dotline    = -1;
      _sym       = rest4Sym;
      }

Rest::Rest(Score* s, const TimeDuration& d)
  : ChordRest(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _beamMode  = BEAM_NO;
      dotline    = -1;
      _sym       = rest4Sym;
      setDurationType(d);
      if (d.fraction().isValid())
            setDuration(d.fraction());
      }

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw(Painter* p) const
      {
      if (staff()->useTablature() || generated())
            return;
      qreal _spatium = spatium();

      Measure* m = measure();
      qreal mag = magS();
      if (m && m->multiMeasure()) {
#if 0
            int n = m->multiMeasure();

            qreal pw = _spatium * .7;
            p->setPenWidth(pw);

            qreal w  = _mmWidth;
            qreal y  = _spatium;
            qreal x1 = 0.0;
            qreal x2 =  w;
            pw *= .5;
            p->drawLine(x1 + pw, y, x2 - pw, y);

            p->setPenWidth(_spatium * .2);
            p->drawLine(QLineF(x1, y-_spatium, x1, y+_spatium));
            p->drawLine(QLineF(x2, y-_spatium, x2, y+_spatium));

            p->setFont(symbols[score()->symIdx()][allabreveSym].font());
            p->scale(mag, mag);
            qreal imag = 1.0 / mag;

            y = -_spatium * 6.75 * imag;
            x1 = x1 + (x2 - x1) * .5 * imag;
            p->drawText(QRectF(x1, y, 0.0, 0.0), Qt::AlignHCenter|Qt::TextDontClip,
               QString("%1").arg(n));
            p->scale(imag, imag);
#endif
            }
      else {
            symbols[score()->symIdx()][_sym].draw(p, mag);
            int dots = durationType().dots();
            if (dots) {
                  qreal y = dotline * _spatium * .5;
                  for (int i = 1; i <= dots; ++i) {
                        qreal x = symbols[score()->symIdx()][_sym].width(magS())
                                   + point(score()->styleS(ST_dotNoteDistance)) * i;
                        symbols[score()->symIdx()][dotSym].draw(p, magS(), x, y);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setUserOffset
//    - raster vertical position in spatium units
//    - half rests and whole rests outside the staff are
//      replaced by special symbols with ledger lines
//---------------------------------------------------------

void Rest::setUserOffset(qreal x, qreal y)
      {
      qreal _spatium = spatium();
      int line = lrint(y/_spatium);

      if (_sym == wholerestSym && (line <= -2 || line >= 3))
            _sym = outsidewholerestSym;
      else if (_sym == outsidewholerestSym && (line > -2 && line < 4))
            _sym = wholerestSym;
      else if (_sym == halfrestSym && (line <= -3 || line >= 3))
            _sym = outsidehalfrestSym;
      else if (_sym == outsidehalfrestSym && (line > -3 && line < 3))
            _sym = halfrestSym;

      setUserOff(QPointF(x, qreal(line) * _spatium));
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF Rest::drag(const QPointF& s)
      {
      QRectF r(abbox());

      // Limit horizontal drag range
      const qreal xDragRange = 250.0;
      qreal xoff = (fabs(s.x()) > xDragRange) ? xDragRange : fabs(s.x());
      if (s.x() < 0)
            xoff *= -1;
      setUserOffset(xoff, s.y());
      return abbox() | r;
      }

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void Rest::read(XmlReader* r, const QList<Tuplet*>& tuplets, const QList<Slur*>& slurs)
      {
      while (r->readElement()) {
            if (!ChordRest::readProperties(r, tuplets, slurs))
                  r->unknown();
            }
      QPointF off(userOff());
      setUserOffset(off.x(), off.y());
      }

//---------------------------------------------------------
//   getSymbol
//---------------------------------------------------------

int Rest::getSymbol(TimeDuration::DurationType type, int line, int* yoffset)
      {
      *yoffset = 2;
      switch(type) {
            case TimeDuration::V_LONG:
                  return longarestSym;
            case TimeDuration::V_BREVE:
                  return breverestSym;
            case TimeDuration::V_MEASURE:
            case TimeDuration::V_WHOLE:
                  *yoffset = 1;
                  return (line <= -2 || line >= 4) ? outsidewholerestSym : wholerestSym;
            case TimeDuration::V_HALF:
                  return (line <= -3 || line >= 3) ? outsidehalfrestSym : halfrestSym;
            case TimeDuration::V_EIGHT:
                  return rest8Sym;
            case TimeDuration::V_16TH:
                  return rest16Sym;
            case TimeDuration::V_32ND:
                  return rest32Sym;
            case TimeDuration::V_64TH:
                  return rest64Sym;
            case TimeDuration::V_256TH:
            case TimeDuration::V_128TH:
                  return rest128Sym;
            default:
                  return rest4Sym;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout()
      {
      switch(durationType().type()) {
            case TimeDuration::V_64TH:
            case TimeDuration::V_32ND:
                  dotline = -3;
                  break;
            case TimeDuration::V_256TH:
            case TimeDuration::V_128TH:
                  dotline = -5;
                  break;
            default:
                  dotline = -1;
                  break;
            }
      qreal _spatium = spatium();
      int line       = lrint(userOff().y() / _spatium);
      int lineOffset = 0;
      if (measure()->mstaff(staffIdx())->hasVoices) {
            // move rests in a multi voice context
            bool up = (voice() == 0) || (voice() == 2);       // TODO: use style values
            switch(durationType().type()) {
                  case TimeDuration::V_LONG:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TimeDuration::V_BREVE:
                        lineOffset = up ? -3 : 5;
                        break;
                  case TimeDuration::V_MEASURE:
                        lineOffset = up ? -4 : 6;
                        break;
                  case TimeDuration::V_WHOLE:
                        lineOffset = up ? -4 : 6;
                        break;
                  case TimeDuration::V_HALF:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TimeDuration::V_QUARTER:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TimeDuration::V_EIGHT:
                        lineOffset = up ? -4 : 4;
                        break;
                  case TimeDuration::V_16TH:
                        lineOffset = up ? -6 : 4;
                        break;
                  case TimeDuration::V_32ND:
                        lineOffset = up ? -6 : 6;
                        break;
                  case TimeDuration::V_64TH:
                        lineOffset = up ? -8 : 6;
                        break;
                  case TimeDuration::V_128TH:
                        lineOffset = up ? -8 : 8;
                        break;
                  case TimeDuration::V_256TH:             // not available
                        lineOffset = up ? -10 : 6;
                        break;
                  default:
                        break;
                  }
            }
      int yo;
      _sym = getSymbol(durationType().type(), line + lineOffset/2, &yo);
      setYoff(qreal(yo) + qreal(lineOffset) * .5);
      layoutArticulations();
      setPos(0.0, yoff() * _spatium);

      Spatium rs;
      if (dots()) {
            rs = Spatium(score()->styleS(ST_dotNoteDistance)
               + dots() * score()->styleS(ST_dotDotDistance));
            }
      _space.setLw(point(_extraLeadingSpace));
      _space.setRw(width() + point(_extraTrailingSpace + rs));
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Rest::bbox() const
      {
      Segment* s = segment();
      if (s && s->measure() && s->measure()->multiMeasure()) {
            qreal _spatium = spatium();
            qreal h = _spatium * 6.5;
            qreal w = point(score()->styleS(ST_minMMRestWidth));
            return QRectF(-w * .5, -h + 2 * _spatium, w, h);
            }
      return symbols[score()->symIdx()][_sym].bbox(magS());
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal Rest::centerX() const
      {
      return symbols[score()->symIdx()][_sym].width(magS())*.5;
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Rest::upPos() const
      {
      return symbols[score()->symIdx()][_sym].bbox(magS()).y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Rest::downPos() const
      {
      return symbols[score()->symIdx()][_sym].bbox(magS()).y() + symbols[score()->symIdx()][_sym].height(magS());
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Rest::scanElements(void* data, void (*func)(void*, Element*))
      {
      func(data, this);
      ChordRest::scanElements(data, func);
      }


