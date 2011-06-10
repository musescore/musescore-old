//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: slur.cpp 3705 2010-11-14 10:48:36Z wschweer $
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
#include "note.h"
#include "chord.h"
#include "m-al/xml.h"
#include "slur.h"
#include "measure.h"
#include "utils.h"
#include "score.h"
#include "preferences.h"
#include "system.h"
#include "segment.h"
#include "staff.h"
#include "articulation.h"
#include "painter.h"

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

SlurSegment::SlurSegment(Score* score)
   : SpannerSegment(score)
      {
      }

SlurSegment::SlurSegment(const SlurSegment& b)
   : SpannerSegment(b)
      {
      for (int i = 0; i < 4; ++i)
            ups[i] = b.ups[i];
//      path         = b.path;
      bow          = b.bow;
      _system      = b._system;
      }

//---------------------------------------------------------
//   updatePath
//---------------------------------------------------------

void SlurSegment::updatePath()
      {
      qreal _spatium = spatium();

      QPointF pp[4];
      for (int i = 0; i < 4; ++i)
            pp[i] = ups[i].p + ups[i].off * _spatium;
      path = PainterPath();
      QPointF t(0.0, score()->styleS(ST_SlurMidWidth).val() * _spatium);    // thickness of slur

      qreal x1 = 1000000;
      qreal y1 = 1000000;
      qreal x2 = -1000000;
      qreal y2 = -1000000;

      path.moveTo(pp[0].x(), pp[0].y());
      QPointF p1(pp[1]-t);
      QPointF p2(pp[2]-t);

      x1 = qMin(x1, pp[0].x());
      y1 = qMin(y1, pp[0].y());
      x2 = qMax(x2, pp[0].x());
      y2 = qMax(y2, pp[0].y());

      x1 = qMin(x1, p1.x());
      y1 = qMin(y1, p1.y());
      x2 = qMax(x2, p1.x());
      y2 = qMax(y2, p1.y());

      x1 = qMin(x1, p2.x());
      y1 = qMin(y1, p2.y());
      x2 = qMax(x2, p2.x());
      y2 = qMax(y2, p2.y());

      x1 = qMin(x1, pp[3].x());
      y1 = qMin(y1, pp[3].y());
      x2 = qMax(x2, pp[3].x());
      y2 = qMax(y2, pp[3].y());

      path.cubicTo(p1.x(), p1.y(), p2.x(), p2.y(), pp[3].x(), pp[3].y());
      if (slurTie()->lineType() == 0) {
            p1 = pp[2]+t;
            p2 = pp[1]+t;
            path.cubicTo(p1.x(), p1.y(), p2.x(), p2.y(), pp[0].x(), pp[0].y());
            x1 = qMin(x1, p1.x());
            y1 = qMin(y1, p1.y());
            x2 = qMax(x2, p1.x());
            y2 = qMax(y2, p1.y());

            x1 = qMin(x1, p2.x());
            y1 = qMin(y1, p2.y());
            x2 = qMax(x2, p2.x());
            y2 = qMax(y2, p2.y());
            }
      setbbox(QRectF(x1, y1, x2-x1, y2-y1));
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurSegment::move(const QPointF& s)
      {
      movePos(s);
      for (int k = 0; k < 4; ++k)
            ups[k].p += s;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SlurSegment::draw(Painter* p) const
      {
      if (slurTie()->lineType() == 0) {
            p->setBrush(curColor());
            qreal lw = point(score()->styleS(ST_SlurEndWidth));
            p->setPenWidth(lw);
            p->setLineCap(CAP_ROUND);
            p->setLineJoin(JOIN_ROUND);
            }
      else {
            // p->setBrush(QBrush());
            qreal lw = point(score()->styleS(ST_SlurDottedWidth));
            p->setLineStyle(Qt::DotLine);
            p->setPenWidth(lw);
            }
      p->drawPath(path);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

#if 0
QPainterPath SlurSegment::shape() const
      {
      return path;
      }
#endif

//---------------------------------------------------------
//   readSegment
//---------------------------------------------------------

void SlurSegment::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (r->readPoint("o1", &ups[0].off))
                  ;
            else if (r->readPoint("o2", &ups[1].off))
                  ;
            else if (r->readPoint("o3", &ups[2].off))
                  ;
            else if (r->readPoint("o4", &ups[3].off))
                  ;
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SlurSegment::layout(const QPointF& p1, const QPointF& p2, qreal b)
      {
      bow      = b;
      ups[0].p = p1;
      ups[3].p = p2;

      //
      //  compute bezier help points
      //
      qreal _spatium = spatium();
      qreal x0 = ups[0].p.x() + ups[0].off.x() * _spatium;
      qreal x3 = ups[3].p.x() + ups[3].off.x() * _spatium;

      qreal y0 = ups[0].p.y() + ups[0].off.y() * _spatium;
      qreal y3 = ups[3].p.y() + ups[3].off.y() * _spatium;

      qreal xdelta = x3 - x0;
      if (xdelta == 0.0 || (x0 > x3)) {
            printf("illegal slur width: %f-%f\n", x0, x3);
            return;
            }

      qreal d  = xdelta / 6.0;
      qreal x1 = x0 + d;
      qreal x2 = x3 - d;

      qreal maxBow = xdelta * .4;
      if (fabs(bow) > maxBow) {          // limit bow for small slurs
            if (bow > 0.0)
                  bow = maxBow;
            else
                  bow = -maxBow;
            }
      qreal slope = (y3 - y0) / xdelta;

      qreal y1 = y0 + (x1-x0) * slope + bow;
      qreal y2 = y0 + (x2-x0) * slope + bow;

      ups[1].p = QPointF(x1, y1);
      ups[2].p = QPointF(x2, y2);
      updatePath();
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(Score* s)
   : Spanner(s)
      {
      _slurDirection = AUTO;
      up             = true;
      _len           = 0;
      _lineType      = 0;     // default is solid
      }

SlurTie::SlurTie(const SlurTie& t)
   : Spanner(t)
      {
      up             = t.up;
      _slurDirection = t._slurDirection;
      _len           = t._len;
      _lineType      = t._lineType;
      // delSegments    = t.delSegments;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
      {
      }

//---------------------------------------------------------
//   slurPos
//---------------------------------------------------------

QPointF SlurTie::slurPos(Element* e, System*& s)
      {
      qreal _spatium = spatium();

      ChordRest* cr;
      if (e->type() == NOTE)
            cr = static_cast<Note*>(e)->chord();
      else
            cr = static_cast<ChordRest*>(e);
      s = cr->measure()->system();

      //-----------------------------------------
      //    off
      //-----------------------------------------

      qreal xo = cr->width() * .5;
      qreal yo = 0.0;
      if (cr->type() == CHORD) {
            Chord* c = static_cast<Chord*>(cr);
            Stem* stem = c->stem();
            Beam* beam = c->beam();

            Chord* sc;
            if (startElement()->type() == NOTE)
                  sc = ((Note*)startElement())->chord();
            else
                  sc = (Chord*)startElement();

            bool startIsGrace         = sc->type() == CHORD && sc->noteType() != NOTE_NORMAL;
            bool mainNoteOfGraceSlur  = startIsGrace && (c == endElement()) && (c->noteType() == NOTE_NORMAL);
            bool firstNoteOfGraceSlur = startIsGrace && (c == startElement()) && (c->noteType() != NOTE_NORMAL);

            if (up) {
                  yo = c->upNote()->pos().y() - c->upNote()->headHeight();
                  //
                  // handle special case of tenuto and staccato;
                  // should be generalized
                  //
                  QList<Articulation*>* al = c->getArticulations();
                  if (al->size() == 1) {
                        Articulation* a = al->at(0);
                        if (a->subtype() == TenutoSym || a->subtype() == StaccatoSym) {
                              yo = a->y() - _spatium * .5;
                              }
                        }
                  if (c->up() && stem) {
                        if (beam && !mainNoteOfGraceSlur)
                              yo = c->downNote()->pos().y() - stem->height() - _spatium;
                        else if (!startIsGrace) {
                              // slurs on the stem side of stemmed notes start half
                              // of a staff space from the end of the stem
                              yo = c->downNote()->pos().y() - stem->height() + _spatium * .5;
                              }
                        }
                  }
            else {
                  yo = c->downNote()->pos().y() + c->downNote()->headHeight();
                  //
                  // handle special case of tenuto and staccato;
                  // should be generalized
                  //
                  QList<Articulation*>* al = c->getArticulations();
                  if (al->size() == 1) {
                        Articulation* a = al->at(0);
                        if (a->subtype() == TenutoSym || a->subtype() == StaccatoSym)
                              yo = a->y() + a->height() + _spatium * .5;
                        }
                  if (!c->up() && stem) {
                        if (beam && !mainNoteOfGraceSlur)
                              yo = c->upNote()->pos().y() + stem->height() + _spatium;
                        else if (!startIsGrace) {
                              // slurs on the stem side of stemmed notes start half
                              // of a staff space from the end of the stem
                              yo = c->upNote()->pos().y() + stem->height() - _spatium * .5;
                              }
                        }
                  }
            if (up == c->up() && stem) {
                  if (firstNoteOfGraceSlur) {
                        xo -= c->upNote()->headWidth() * .5;
                        }
                  else if (beam && !mainNoteOfGraceSlur) {
                        // for beamed notes and slurs on stem side, slurs are aligned
                        // to the stem rather than the middle of the notehead
                        xo = stem->pos().x();
                        if (cr == startElement())
                              xo += stem->width() * .5;
                        else
                              xo -= stem->width() * .5;
                        }
                  else {
                        if (cr == startElement()) {
                              // don't collide with start stem
                              xo = stem->pos().x() + _spatium * .5;
                              }
                        if (cr == endElement()) {
                              // don't collide with end stem
                              xo = stem->pos().x() - _spatium * .5;
                              }
                        }
                  }
            if (firstNoteOfGraceSlur)
                  xo = c->downNote()->headWidth() * .5;
            if (!up && mainNoteOfGraceSlur)
                  xo = _spatium * .3;
            }
      return cr->canvasPos() + QPointF(xo, yo);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(XmlReader* r)
      {
      MString8 tag = r->tag();
      QString val;
      int i;

      if (tag == "SlurSegment") {
            int idx = 0;
            while (r->readAttribute()) {
                  if (r->tag() == "no")
                        idx = r->intValue();
                  }
            int n = spannerSegments().size();
            for (int i = n; i < idx; ++i)
                  add(new SlurSegment(score()));
            SlurSegment* segment = new SlurSegment(score());
            segment->read(r);
            add(segment);
            }
      else if (r->readInt("up", &i))
            _slurDirection = Direction(i);
      else if (r->readInt("lineType", &_lineType))
            ;
      else if (!Element::readProperties(r))
            return false;
      return true;
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(Score* s)
   : SlurTie(s)
      {
      setId(-1);
      _track2 = 0;
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::~Slur()
      {
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slur::read(XmlReader* r)
      {
      setTrack(0);
      while (r->readAttribute()) {
            if (r->tag() == "id")
                  setId(r->intValue());
            }
      while (r->readElement()) {
            if (!SlurTie::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   chordsHaveTie
//---------------------------------------------------------

static bool chordsHaveTie (Chord* c1, Chord* c2)
      {
      foreach(Note* n1, c1->notes()) {
            foreach(Note* n2, c2->notes()) {
                  if (n1->tieFor() && n1->tieFor() == n2->tieBack())
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   directionMixture
//---------------------------------------------------------

static bool isDirectionMixture (Chord* c1, Chord* c2)
      {
      bool up = c1->up();
      for (Segment* seg = c1->segment(); seg; seg = seg->next()) {
            if (seg->subtype() == SegChordRest) {
                  Element* e = seg->element(c1->track());
                  if (!e)
                        continue;
                  Chord* c = 0;
                  if (e->type() == NOTE)
                        c = static_cast<Note*>(e)->chord();
                  else if (e->type() == CHORD)
                        c = static_cast<Chord*>(e);
                  else
                        continue;
                  if (c && c->up() != up)
                        return true;
                  }
            if (seg == c2->segment())
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slur::layout()
      {
      qreal _spatium = spatium();

      switch (_slurDirection) {
            case UP:
                  up = true;
                  break;
            case DOWN:
                  up = false;
                  break;
            case AUTO:
                  {
                  ChordRest* cr1 = (ChordRest*)startElement();
                  ChordRest* cr2 = (ChordRest*)endElement();
                  Measure* m1    = cr1->measure();

                  Chord* c1;
                  if (cr1->type() == NOTE)
                        c1 = ((Note*)cr1)->chord();
                  else
                        c1 = (Chord*)cr1;
                  Chord* c2;
                  if (cr2->type() == NOTE)
                        c2 = ((Note*)cr2)->chord();
                  else
                        c2 = (Chord*)cr2;

                  up = !(cr1->up());

                  if ((cr2->tick() - cr1->tick()) > m1->ticks()) {
                        // long slurs are always above
                        up = true;
                        }
                  if (cr1->type() == CHORD && cr2->type() == CHORD) {
                        if (isDirectionMixture(c1, c2) && c1->noteType() == NOTE_NORMAL) {
                              // slurs go above if start and end note have different stem directions,
                              // but grace notes are exceptions
                              up = true;
                              }
                        else if (m1->mstaff(cr1->staffIdx())->hasVoices && c1->noteType() == NOTE_NORMAL) {
                              // in polyphonic passage, slurs go on the stem side
                              up = cr1->up();
                              }
                        else if (chordsHaveTie(c1, c2)) {
                              // could confuse slur with tie, put slur on stem side
                              up = cr1->up();
                              }
                        }
                  }
                  break;
            }

      System *s1, *s2;
      QPointF p1 = slurPos(startElement(), s1);
      QPointF p2 = slurPos(endElement(), s2);

      QList<System*>* sl = score()->systems();
      iSystem is = sl->begin();
      while (is != sl->end()) {
            if (*is == s1)
                  break;
            ++is;
            }
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      unsigned nsegs = 1;
      for (iSystem iis = is; iis != sl->end(); ++iis) {
            if (*iis == s2)
                  break;
            ++nsegs;
            }

      unsigned onsegs = spannerSegments().size();
      if (nsegs > onsegs) {
            for (unsigned i = onsegs; i < nsegs; ++i) {
                  SlurSegment* s;
                  if (!delSegments.isEmpty()) {
                        s = delSegments.dequeue();
                        }
                  else
                        s = new SlurSegment(score());
                  s->setTrack(track());
                  add(s);
                  }
            }
      else if (nsegs < onsegs) {
            for (unsigned i = nsegs; i < onsegs; ++i) {
                  SlurSegment* s = takeLastSegment();
                  delSegments.enqueue(s);  // cannot delete: used in SlurSegment->edit()
                  }
            }

      qreal bow = point(score()->styleS(ST_SlurBow));
      if (up)
            bow = -bow;

      for (int i = 0; is != sl->end(); ++i, ++is) {
            System* system  = *is;
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);
            ChordRest* cr1 = (ChordRest*)startElement();
            SysStaff* ss = system->staff(cr1->staffIdx());
            QPointF sp(system->canvasPos());
            sp.ry() += ss->y();

            // case 1: one segment
            if (s1 == s2) {
                  segment->setSubtype(SEGMENT_SINGLE);
                  segment->layout(p1, p2, bow);
                  }
            // case 2: start segment
            else if (i == 0) {
                  segment->setSubtype(SEGMENT_BEGIN);
                  qreal x = sp.x() + system->bbox().width();
                  segment->layout(p1, QPointF(x, p1.y()), bow);
                  }
            // case 3: middle segment
            else if (i != 0 && system != s2) {
                  segment->setSubtype(SEGMENT_MIDDLE);
                  qreal x1 = firstNoteRestSegmentX(system) - _spatium;
                  qreal x2 = sp.x() + system->bbox().width();
                  segment->layout(QPointF(x1, sp.y()), QPointF(x2, sp.y()), bow);
                  }
            // case 4: end segment
            else {
                  segment->setSubtype(SEGMENT_END);
                  qreal x = firstNoteRestSegmentX(system) - _spatium;
                  segment->layout(QPointF(x, p2.y()), p2, bow);
                  }
            if (system == s2)
                  break;
            }
      }

//---------------------------------------------------------
//   firstNoteRestSegmentX
//---------------------------------------------------------

qreal SlurTie::firstNoteRestSegmentX(System* system)
      {
      foreach(const MeasureBase* mb, system->measures()) {
            if (mb->type() == MEASURE) {
                  Measure* measure = (Measure*)mb;
                  for (Segment* seg = measure->first(); seg; seg = seg->next()) {
                        if (seg->subtype() == SegChordRest) {
                              return seg->canvasPos().x();
                              }
                        }
                  }
            }
      printf("firstNoteRestSegmentX: did not find segment\n");
      return 0.0;
      }

//---------------------------------------------------------
//   bbox
//    used in palette
//---------------------------------------------------------

QRectF Slur::bbox() const
      {
      if (spannerSegments().isEmpty())
            return QRectF();
      else
            return frontSegment()->bbox();
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Slur::setTrack(int n)
      {
      Element::setTrack(n);
      foreach(SpannerSegment* ss, spannerSegments())
            ss->setTrack(n);
      }

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(Score* s)
   : SlurTie(s)
      {
      }

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Tie::setStartNote(Note* note)
      {
      setStartElement(note);
      setParent(note);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tie::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (Element::readProperties(r))
                  ;
            else if (SlurTie::readProperties(r))
                  ;
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tie::layout()
      {
      //
      // TODO: if there is a startNote but no endNote
      //    show short bow
      if (startElement() == 0 || endElement() == 0) {
            printf("Tie::layout(): no start or end\n");
            return;
            }

      qreal _spatium = spatium();

      Chord* c1   = startNote()->chord();
      Measure* m1 = c1->measure();
      System* s1  = m1->system();
      Chord* c2   = endNote()->chord();
      Measure* m2 = c2->measure();
      System* s2  = m2->system();

      if (_slurDirection == AUTO)
            if (m1->mstaff(c1->staffIdx())->hasVoices) {
                  // in polyphonic passage, ties go on the stem side
                  up = c1->up();
                  }
            else
                  up = !(c1->up());
      else
            up = _slurDirection == UP ? true : false;
      qreal w   = startNote()->headWidth();
      qreal xo1 = w * 1.12;
      qreal h   = w * 0.3;
      qreal yo  = up ? -h : h;

      QPointF off1(xo1, yo);
      QPointF off2(0.0, yo);

      QPointF ppos(canvasPos());
      QPointF p1 = startNote()->canvasPos() + off1;
      QPointF p2 = endNote()->canvasPos()   + off2;

      QList<System*>* systems = score()->systems();
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      int sysIdx1      = systems->indexOf(s1);
      if (sysIdx1 == -1) {
            printf("system not found\n");
            foreach(System* s, *systems)
                  printf("   search %p in %p\n", s1, s);
            return;
            }

      int sysIdx2      = systems->indexOf(s2, sysIdx1);
      unsigned nsegs   = sysIdx2 - sysIdx1 + 1;
      unsigned onsegs  = spannerSegments().size();

      if (nsegs != onsegs) {
            if (nsegs > onsegs) {
                  int n = nsegs - onsegs;
                  for (int i = 0; i < n; ++i) {
                        SlurSegment* s = new SlurSegment(score());
                        s->setParent(this);
                        spannerSegments().append(s);
                        }
                  }
            else {
                  int n = onsegs - nsegs;
                  for (int i = 0; i < n; ++i) {
                        /* LineSegment* seg = */ takeLastSegment();
                        // delete seg;   // DEBUG: will be used later
                        }
                  }
            }

      qreal bow = up ? -_spatium : _spatium;

      p1 -= canvasPos();
      p2 -= canvasPos();
      for (unsigned int i = 0; i < nsegs; ++i) {
            System* system       = (*systems)[sysIdx1++];
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);
            QPointF sp(system->canvasPos() - canvasPos());

            // case 1: one segment
            if (s1 == s2) {
                  segment->layout(p1, p2, bow);
                  segment->setSpannerSegmentType(SEGMENT_SINGLE);
                  }
            // case 2: start segment
            else if (i == 0) {
                  qreal x = sp.x() + system->bbox().width();
                  segment->layout(p1, QPointF(x, p1.y()), bow);
                  segment->setSpannerSegmentType(SEGMENT_BEGIN);
                  }
            // case 3: middle segment
            else if (i != 0 && system != s2) {
                  // cannot happen
                  printf("sysIdx %d - %d\n", sysIdx1, sysIdx2);
                  Measure* m1 = c1->measure();
                  Measure* m2 = c2->measure();
                  printf("Measure %d - %d, %d %d\n", m1->no(), m2->no(), m1->tick(), m2->tick());
                  // abort();
                  }
            // case 4: end segment
            else {
                  // qreal x = sp.x();
                  qreal x = firstNoteRestSegmentX(system) - 2 * _spatium - canvasPos().x();

                  segment->layout(QPointF(x, p2.y()), p2, bow);
                  segment->setSpannerSegmentType(SEGMENT_END);
                  }
            }
      }

