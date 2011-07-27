//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "rest.h"
#include "score.h"
#include "xml.h"
#include "style.h"
#include "restproperties.h"
#include "utils.h"
#include "tuplet.h"
#include "sym.h"
#include "icons.h"
#include "stafftext.h"
#include "articulation.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "undo.h"
#include "staff.h"

//---------------------------------------------------------
//    Rest
//--------------------------------------------------------

Rest::Rest(Score* s)
  : ChordRest(s)
      {
      setOffsetType(OFFSET_SPATIUM);
      _beamMode  = BEAM_NO;
      dotline    = -1;
      _sym       = rest4Sym;
      }

Rest::Rest(Score* s, int tick, const Duration& d)
  : ChordRest(s)
      {
      _beamMode  = BEAM_NO;
      dotline    = -1;
      setOffsetType(OFFSET_SPATIUM);
      _sym       = rest4Sym;
      setTick(tick);
      setDuration(d);
      }

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw(QPainter& p) const
      {
      if (generated())
            return;
      double _spatium = spatium();

      Measure* m = measure();
      double mag = magS();
      if (m && m->multiMeasure()) {
            int n = m->multiMeasure();

            QPen pen(p.pen());
            double pw = _spatium * .7;
            pen.setWidthF(pw);
            p.setPen(pen);

            double w  = _mmWidth;
            double y  = _spatium;
            double x1 = 0.0;
            double x2 =  w;
            pw *= .5;
            p.drawLine(QLineF(x1 + pw, y, x2 - pw, y));

            pen.setWidthF(_spatium * .2);
            p.setPen(pen);
            p.drawLine(QLineF(x1, y-_spatium, x1, y+_spatium));
            p.drawLine(QLineF(x2, y-_spatium, x2, y+_spatium));

            p.setFont(symbols[allabreveSym].font());
            p.scale(mag, mag);
            double imag = 1.0 / mag;

            y = -_spatium * 6.75 * imag;
            x1 = x1 + (x2 - x1) * .5 * imag;
            p.drawText(QRectF(x1, y, 0.0, 0.0), Qt::AlignHCenter|Qt::TextDontClip,
               QString("%1").arg(n));
            p.scale(imag, imag);
            }
      else {
            symbols[_sym].draw(p, mag);
            int dots = duration().dots();
            if (dots) {
                  double y = dotline * _spatium * .5;
                  for (int i = 1; i <= dots; ++i) {
                        double x = symbols[_sym].width(magS())
                                   + point(score()->styleS(ST_dotNoteDistance)) * i;
                        symbols[dotSym].draw(p, magS(), x, y);
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

void Rest::setUserOffset(double x, double y)
      {
      double _spatium = spatium();
      int line = lrint(y/_spatium);

      if (_sym == wholerestSym && (line <= -2 || line >= 3))
            _sym = outsidewholerestSym;
      else if (_sym == outsidewholerestSym && (line > -2 && line < 4))
            _sym = wholerestSym;
      else if (_sym == halfrestSym && (line <= -3 || line >= 3))
            _sym = outsidehalfrestSym;
      else if (_sym == outsidehalfrestSym && (line > -3 && line < 3))
            _sym = halfrestSym;

      setUserOff(QPointF(x, double(line) * _spatium));
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
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(ScoreView*, const QPointF&, int type, int subtype) const
      {
      if (
         (type == ICON && subtype == ICON_SBEAM)
         || (type == ICON && subtype == ICON_MBEAM)
         || (type == ICON && subtype == ICON_NBEAM)
         || (type == ICON && subtype == ICON_BEAM32)
         || (type == ICON && subtype == ICON_AUTOBEAM)
         || (type == ARTICULATION && subtype == UfermataSym)
         || (type == ARTICULATION && subtype == DfermataSym)
         || (type == CLEF)
         || (type == STAFF_TEXT)
         || (type == BAR_LINE)
         || (type == BREATH)
         || (type == CHORD)
         || (type == DYNAMIC)
         || (type == HARMONY)
         || (type == STAFF_TEXT)
         ) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Rest::drop(ScoreView* view, const QPointF& p1, const QPointF& p2, Element* e)
      {
      switch (e->type()) {
            case ARTICULATION:
                  if (e->subtype() == UfermataSym || e->subtype() == DfermataSym)
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
                        case ICON_AUTOBEAM:
                              score()->undoChangeBeamMode(this, BEAM_AUTO);
                              break;
                        }
                  }
                  delete e;
                  break;

            case STAFF_TEXT:
                  {
                  StaffText* s = static_cast<StaffText*>(e);
                  s->setTrack(track());
                  s->setSystemFlag(false);
//                  s->setSubtype(STAFF_TEXT);
                  s->setParent(measure());
                  s->setTick(tick());
                  score()->undoAddElement(s);
                  score()->setLayoutAll(true);
                  }
                  break;

            case CHORD:
                  {
                  Chord* c      = static_cast<Chord*>(e);
                  Note* n       = c->upNote();
                  int headGroup = n->headGroup();
                  Direction dir = c->stemDirection();
                  score()->select(0, SELECT_SINGLE, 0);
                  Segment* seg = score()->setNoteRest(this, track(), n->pitch(),
                     c->fraction(), headGroup, dir);
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(track()));
                  if (cr)
                        score()->nextInputPos(cr, true);
                  delete e;
                  }
                  break;
            case HARMONY:
                  e->setParent(measure());
                  e->setTick(tick());
                  e->setTrack((track() / VOICES) * VOICES);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;
            default:
                  return ChordRest::drop(view, p1, p2, e);
            }
      return 0;
      }

//---------------------------------------------------------
//   write Rest
//---------------------------------------------------------

void Rest::write(Xml& xml) const
      {
      xml.stag("Rest");
      ChordRest::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void Rest::read(QDomElement e, const QList<Tuplet*>& tuplets)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!ChordRest::readProperties(e, tuplets))
                  domError(e);
            }
      if (!duration().isValid()) {
            if (_ticks <= 0)
                  setDuration(Duration(Duration::V_MEASURE));
            else
                  convertTicks();
            }
      QPointF off(userOff());
      setUserOffset(off.x(), off.y());
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Rest::add(Element* e)
      {
      if (e->type() != ARTICULATION)
            return;
      e->setParent(this);
      e->setTrack(track());
      articulations.push_back((Articulation*)e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Rest::remove(Element* e)
      {
      if (e->type() != ARTICULATION)
            return;
      int idx = articulations.indexOf((Articulation*)e);
      if (idx == -1)
            printf("Rest::remove(): attribute not found\n");
      else
            articulations.removeAt(idx);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout()
      {
      double _spatium = spatium();
      int line = lrint(userOff().y() / _spatium);
      int lines = staff()->lines();
      
      setYoff(2.0);
      switch(duration().type()) {
            case Duration::V_LONG:
                  _sym = longarestSym;
                  break;
            case Duration::V_BREVE:
                  _sym = breverestSym;
                  break;
            case Duration::V_MEASURE:
            case Duration::V_WHOLE:
                  _sym = (line <= -2 || line >= 4) ? outsidewholerestSym : wholerestSym;
                  if(lines != 1 && lines != 3)
                        setYoff(1.0);
                  break;
            case Duration::V_HALF:
                  _sym = (line <= -3 || line >= 3) ? outsidehalfrestSym : halfrestSym;
                  break;
            case Duration::V_INVALID:
            case Duration::V_QUARTER:
            case Duration::V_ZERO:
                  _sym = rest4Sym;
                  break;
            case Duration::V_EIGHT:
                  _sym = rest8Sym;
                  break;
            case Duration::V_16TH:
                  _sym = rest16Sym;
                  break;
            case Duration::V_32ND:
                  _sym = rest32Sym;
                  break;
            case Duration::V_64TH:
                  _sym = rest64Sym;
                  break;
            case Duration::V_128TH:
                  _sym = rest128Sym;
                  break;
            case Duration::V_256TH:
printf("Rest: no symbol for 1/256\n");
                  _sym = rest128Sym;  // TODO
                  break;
            }
//      layoutArticulations();
      Element::layout();
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Rest::bbox() const
      {
      Segment* s = segment();
      if (s && s->measure() && s->measure()->multiMeasure()) {
            double h = spatium() * 6.5;
            double w = point(score()->styleS(ST_minMMRestWidth));
            return QRectF(-w * .5, -h + 2 * spatium(), w, h);
            }
      return symbols[_sym].bbox(mag());
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal Rest::centerX() const
      {
      return symbols[_sym].width(magS())*.5;
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Rest::upPos() const
      {
      return symbols[_sym].bbox(magS()).y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Rest::downPos() const
      {
      return symbols[_sym].bbox(magS()).y() + symbols[_sym].height(magS());
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Rest::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addSeparator();
      a->setText(tr("Rest"));
      if (tuplet()) {
            QMenu* menuTuplet = popup->addMenu(tr("Tuplet..."));
            a = menuTuplet->addAction(tr("Tuplet Properties..."));
            a->setData("tupletProps");
            a = menuTuplet->addAction(tr("Delete Tuplet"));
            a->setData("tupletDelete");
            }
      a = popup->addAction(tr("Rest Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Rest::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            Rest r(*this);
            RestProperties vp(&r);
            int rv = vp.exec();
            if (rv) {
                  if (r.small() != small())
                        score()->undoChangeChordRestSize(this, r.small());
                  if (r.extraLeadingSpace() != extraLeadingSpace()
                     || r.extraTrailingSpace() != extraTrailingSpace()) {
                        score()->undoChangeChordRestSpace(this, r.extraLeadingSpace(),
                           r.extraTrailingSpace());
                        }
                  }
            }
      else if (s == "tupletProps") {
            TupletProperties vp(tuplet());
            if (vp.exec()) {
                  int bracketType = vp.bracketType();
                  int numberType  = vp.numberType();

                  foreach(Element* e, score()->selection().elements()) {
                        if (e->type() == REST) {
                              Rest* r = static_cast<Rest*>(e);
                              if (r->tuplet()) {
                                    Tuplet* tuplet = r->tuplet();
                                    if ((bracketType != tuplet->bracketType()) || (numberType != tuplet->numberType()))
                                          score()->undo()->push(new ChangeTupletProperties(tuplet, numberType, bracketType));
                                    }
                              }
                        }
                  }
            }
      else if (s == "tupletDelete") {
            foreach(Element* e, score()->selection().elements()) {
                  if (e->type() == REST) {
                        Rest* r = static_cast<Rest*>(e);
                        if (r->tuplet())
                              score()->cmdDeleteTuplet(r->tuplet(), true);
                        }
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Rest::space() const
      {
      return Space(point(_extraLeadingSpace), width() + point(_extraTrailingSpace));
      }

