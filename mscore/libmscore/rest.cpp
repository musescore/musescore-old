//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "rest.h"
#include "score.h"
#include "xml.h"
#include "style.h"
#include "utils.h"
#include "tuplet.h"
#include "sym.h"
#include "stafftext.h"
#include "articulation.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"
#include "harmony.h"
#include "lyrics.h"
#include "segment.h"
#include "painter.h"
#include "stafftype.h"

//---------------------------------------------------------
//    Rest
//--------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
      _beamMode  = BEAM_NO;
      dotline    = -1;
      _sym       = rest4Sym;
      }

Rest::Rest(Score* s, const Duration& d)
  : ChordRest(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_ON_STAFF);
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

void Rest::draw(Painter* painter) const
      {
      if (staff()->useTablature() || generated())
            return;
      qreal _spatium = spatium();

      Measure* m = measure();
      if (m && m->multiMeasure()) {
            int n     = m->multiMeasure();
            qreal pw = _spatium * .7;
            painter->setLineWidth(pw);

            qreal w  = _mmWidth;
            qreal y  = _spatium;
            qreal x1 = 0.0;
            qreal x2 =  w;
            pw *= .5;
            painter->drawLine(x1 + pw, y, x2 - pw, y);

            // draw vertical lines:
            painter->setLineWidth(_spatium * .2);
            painter->drawLine(x1, y-_spatium, x1, y+_spatium);
            painter->drawLine(x2, y-_spatium, x2, y+_spatium);

            QFont font = symbols[score()->symIdx()][allabreveSym].font();
            painter->setFont(font);
            QFontMetricsF fm(font);
            y  = -_spatium * .5 - fm.ascent();
            painter->drawTextHCentered(center(x1, x2), y, QString("%1").arg(n));
            }
      else {
            qreal mag = magS();
            symbols[score()->symIdx()][_sym].draw(painter, mag);
            int dots = durationType().dots();
            if (dots) {
                  qreal y = dotline * _spatium * .5;
                  for (int i = 1; i <= dots; ++i) {
                        qreal x = symbols[score()->symIdx()][_sym].width(mag)
                                   + point(score()->styleS(ST_dotNoteDistance)) * i;
                        symbols[score()->symIdx()][dotSym].draw(painter, mag, x, y);
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
      layout();
      return abbox() | r;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(MuseScoreView*, const QPointF&, int type, int subtype) const
      {
      if (
         (type == ICON && subtype == ICON_SBEAM)
         || (type == ICON && subtype == ICON_MBEAM)
         || (type == ICON && subtype == ICON_NBEAM)
         || (type == ICON && subtype == ICON_BEAM32)
         || (type == ICON && subtype == ICON_BEAM64)
         || (type == ICON && subtype == ICON_AUTOBEAM)
         || (type == ARTICULATION && subtype == Articulation_Fermata)
         || (type == CLEF)
         || (type == STAFF_TEXT)
         || (type == BAR_LINE)
         || (type == BREATH)
         || (type == CHORD)
         || (type == STAFF_STATE)
         || (type == INSTRUMENT_CHANGE)
         || (type == DYNAMIC)
         || (type == HARMONY)
         || (type == TEMPO_TEXT)
         || (type == STAFF_TEXT)
         ) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Rest::drop(const DropData& data)
      {
      Element* e = data.element;
      switch (e->type()) {
            case ARTICULATION:
                  if (e->subtype() == Articulation_Fermata)
                        score()->addArticulation(this, (Articulation*)e);
                  return 0;
            case ICON:
                  {
                  switch(e->subtype()) {
                        case ICON_SBEAM:
                              score()->undoChangeBeamMode(this, BEAM_BEGIN);
                              break;
                        case ICON_MBEAM:
                              score()->undoChangeBeamMode(this, BEAM_MID);
                              break;
                        case ICON_NBEAM:
                              score()->undoChangeBeamMode(this, BEAM_NO);
                              break;
                        case ICON_BEAM32:
                              score()->undoChangeBeamMode(this, BEAM_BEGIN32);
                              break;
                        case ICON_BEAM64:
                              score()->undoChangeBeamMode(this, BEAM_BEGIN64);
                              break;
                        case ICON_AUTOBEAM:
                              score()->undoChangeBeamMode(this, BEAM_AUTO);
                              break;
                        }
                  }
                  delete e;
                  break;

            case CHORD:
                  {
                  Chord* c      = static_cast<Chord*>(e);
                  Note* n       = c->upNote();
                  Direction dir = c->stemDirection();
                  score()->select(0, SELECT_SINGLE, 0);
                  NoteVal nval;
                  nval.pitch = n->pitch();
                  nval.headGroup = n->headGroup();
                  Fraction d = score()->inputState().duration().fraction();
                  if (!d.isZero()) {
                        Segment* seg = score()->setNoteRest(segment(), track(), nval, d, dir);
                        if (seg) {
                              ChordRest* cr = static_cast<ChordRest*>(seg->element(track()));
                              if (cr)
                                    score()->nextInputPos(cr, true);
                              }
                        }
                  delete e;
                  }
                  break;
            default:
                  return ChordRest::drop(data);
            }
      return 0;
      }

//---------------------------------------------------------
//   write Rest
//---------------------------------------------------------

void Rest::write(Xml& xml) const
      {
      xml.stag(name());
      ChordRest::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void Rest::read(QDomElement e, const QList<Tuplet*>& tuplets, QList<Slur*>* slurs)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!ChordRest::readProperties(e, tuplets, slurs))
                  domError(e);
            }
      QPointF off(userOff());
      setUserOffset(off.x(), off.y());
      }

//---------------------------------------------------------
//   getSymbol
//---------------------------------------------------------

int Rest::getSymbol(Duration::DurationType type, int line, int lines, int* yoffset)
      {
      *yoffset = 2;
      switch(type) {
            case Duration::V_LONG:
                  return longarestSym;
            case Duration::V_BREVE:
                  return breverestSym;
            case Duration::V_MEASURE:
            case Duration::V_WHOLE:
                  *yoffset = 1;
                  return (line <= -2 || line >= (lines - 1)) ? outsidewholerestSym : wholerestSym;
            case Duration::V_HALF:
                  return (line <= -3 || line >= (lines - 2)) ? outsidehalfrestSym : halfrestSym;
            case Duration::V_EIGHT:
                  return rest8Sym;
            case Duration::V_16TH:
                  return rest16Sym;
            case Duration::V_32ND:
                  return rest32Sym;
            case Duration::V_64TH:
                  return rest64Sym;
            case Duration::V_128TH:
                  return rest128Sym;
            case Duration::V_256TH:
qDebug("Rest: no symbol for 1/256\n");
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
      int lines = staff()->lines();

      switch(durationType().type()) {
            case Duration::V_64TH:
            case Duration::V_32ND:
                  dotline = -3;
                  break;
            case Duration::V_256TH:
            case Duration::V_128TH:
                  dotline = -5;
                  break;
            default:
                  dotline = -1;
                  break;
            }
      qreal _spatium = spatium();
      int line        = lrint(userOff().y() / _spatium); //  + ((staff()->lines()-1) * 2);
      int lineOffset  = 0;

      if (measure()->mstaff(staffIdx())->hasVoices) {
            // move rests in a multi voice context
            bool up = (voice() == 0) || (voice() == 2);       // TODO: use style values
            switch(durationType().type()) {
                  case Duration::V_LONG:
                        lineOffset = up ? -3 : 5;
                        break;
                  case Duration::V_BREVE:
                        lineOffset = up ? -3 : 5;
                        break;
                  case Duration::V_MEASURE:
                  case Duration::V_WHOLE:
                        lineOffset = up ? -4 : 6;
                        break;
                  case Duration::V_HALF:
                        lineOffset = up ? -4 : 4;
                        break;
                  case Duration::V_QUARTER:
                        lineOffset = up ? -4 : 4;
                        break;
                  case Duration::V_EIGHT:
                        lineOffset = up ? -4 : 4;
                        break;
                  case Duration::V_16TH:
                        lineOffset = up ? -6 : 4;
                        break;
                  case Duration::V_32ND:
                        lineOffset = up ? -6 : 6;
                        break;
                  case Duration::V_64TH:
                        lineOffset = up ? -8 : 6;
                        break;
                  case Duration::V_128TH:
                        lineOffset = up ? -8 : 8;
                        break;
                  case Duration::V_256TH:             // not available
                        lineOffset = up ? -10 : 6;
                        break;
                  default:
                        break;
                  }
            }
      else {
            switch(durationType().type()) {
                  case Duration::V_LONG:
                  case Duration::V_BREVE:
                  case Duration::V_MEASURE:
                  case Duration::V_WHOLE:
                        if (lines == 1)
                              lineOffset = -2;
                        break;
                  case Duration::V_HALF:
                  case Duration::V_QUARTER:
                  case Duration::V_EIGHT:
                  case Duration::V_16TH:
                  case Duration::V_32ND:
                  case Duration::V_64TH:
                  case Duration::V_128TH:
                  case Duration::V_256TH:             // not available
                        if (lines == 1)
                              lineOffset = -4;
                        break;
                  default:
                        break;
                  }
            }

      int yo;
      _sym = getSymbol(durationType().type(), line + lineOffset/2, lines, &yo);
      layoutArticulations();
      setPos(0.0, (qreal(yo) + qreal(lineOffset) * .5) * _spatium);

      Spatium rs;
      if (dots()) {
            rs = Spatium(score()->styleS(ST_dotNoteDistance)
               + dots() * score()->styleS(ST_dotDotDistance));
            }
      Segment* s = segment();
      if (s && s->measure() && s->measure()->multiMeasure()) {
            qreal _spatium = spatium();
            qreal h = _spatium * 6.5;
            qreal w = point(score()->styleS(ST_minMMRestWidth));
            setbbox(QRectF(-w * .5, -h + 2 * _spatium, w, h));
            }
      else {
            if (dots()) {
                  rs = Spatium(score()->styleS(ST_dotNoteDistance)
                     + dots() * score()->styleS(ST_dotDotDistance));
                  }
            setbbox(symbols[score()->symIdx()][_sym].bbox(magS()));
            }
      _space.setLw(point(_extraLeadingSpace));
      _space.setRw(width() + point(_extraTrailingSpace + rs));
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

void Rest::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      ChordRest::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   setMMWidth
//---------------------------------------------------------

void Rest::setMMWidth(qreal val)
      {
      _mmWidth = val;
      Segment* s = segment();
      if (s && s->measure() && s->measure()->multiMeasure()) {
            qreal _spatium = spatium();
            qreal h = _spatium * 6.5;
            qreal w = _mmWidth;
            // setbbox(QRectF(-w * .5, -h + 2 * _spatium, w, h));
            setbbox(QRectF(0.0, -h + 2 * _spatium, w, h));
            }
      }

