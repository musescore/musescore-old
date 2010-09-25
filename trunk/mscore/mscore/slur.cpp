//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "note.h"
#include "chord.h"
#include "xml.h"
#include "slur.h"
#include "measure.h"
#include "utils.h"
#include "score.h"
#include "preferences.h"
#include "system.h"
#include "segment.h"
#include "staff.h"
#include "scoreview.h"
#include "navigate.h"
#include "scoreview.h"
#include "articulation.h"
#include "slurproperties.h"
#include "undo.h"

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
      path         = b.path;
      bow          = b.bow;
      _system      = b._system;
      }

//---------------------------------------------------------
//   updatePath
//---------------------------------------------------------

void SlurSegment::updatePath()
      {
      double _spatium = spatium();

      QPointF pp[4];
      for (int i = 0; i < 4; ++i)
            pp[i] = ups[i].p + ups[i].off * _spatium;
      path = QPainterPath();
      QPointF t(0.0, score()->styleS(ST_SlurMidWidth).val() * _spatium);    // thickness of slur

      path.moveTo(pp[0]);
      path.cubicTo(pp[1]-t, pp[2]-t, pp[3]);
      if (slurTie()->lineType() == 0)
            path.cubicTo(pp[2]+t, pp[1]+t, pp[0]);
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

void SlurSegment::draw(QPainter& p, ScoreView*) const
      {
      if (slurTie()->lineType() == 0) {
            p.setBrush(curColor());
            QPen pen(p.pen());
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            qreal lw = point(score()->styleS(ST_SlurEndWidth));
            pen.setWidthF(lw);
            p.setPen(pen);
            }
      else {
            p.setBrush(QBrush());
            QPen pen(p.pen());
            qreal lw = point(score()->styleS(ST_SlurDottedWidth));
            pen.setWidthF(lw);
            pen.setStyle(Qt::DotLine);
            p.setPen(pen);
            }
      p.drawPath(path);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void SlurSegment::updateGrips(int* n, QRectF* r) const
      {
      *n = 4;
      QPointF p(canvasPos());
      for (int i = 0; i < 4; ++i)
            r[i].translate(ups[i].p + ups[i].off * spatium() + p);
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(ScoreView* viewer, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (slurTie()->type() != SLUR)
            return false;

      Slur* sl = static_cast<Slur*>(slurTie());

      if (key == Qt::Key_X) {
            sl->setSlurDirection(sl->isUp() ? DOWN : UP);
            sl->layout();
            return true;
            }
      if (!((modifiers & Qt::ShiftModifier)
         && ((spannerSegmentType() == SEGMENT_SINGLE)
              || (spannerSegmentType() == SEGMENT_BEGIN && curGrip == 0)
              || (spannerSegmentType() == SEGMENT_END && curGrip == 3)
            )))
            return false;

      int segments  = sl->spannerSegments().size();
      ChordRest* cr = 0;
      Element* e    = curGrip == 0 ? sl->startElement() : sl->endElement();
      Element* e1   = curGrip == 0 ? sl->endElement() : sl->startElement();

      if (key == Qt::Key_Left)
            cr = prevChordRest((ChordRest*)e);
      else if (key == Qt::Key_Right)
            cr = nextChordRest((ChordRest*)e);

      if (cr == 0 || cr == (ChordRest*)e1)
            return true;
      if (curGrip == 0) {
            ((ChordRest*)sl->startElement())->removeSlurFor(sl);
            sl->setStartElement(cr);
            cr->addSlurFor(sl);
            }
      else {
            ((ChordRest*)sl->endElement())->removeSlurBack(sl);
            sl->setEndElement(cr);
            cr->addSlurBack(sl);
            }

      ups[curGrip].off = QPointF();
      sl->layout();
      if (sl->spannerSegments().size() != segments) {
            SlurSegment* newSegment = curGrip == 3 ? sl->backSegment() : sl->frontSegment();
            score()->endCmd();
            score()->startCmd();
            viewer->startEdit(newSegment, curGrip);
            score()->setLayoutAll(true);
            }
      return true;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF SlurSegment::gripAnchor(int grip) const
      {
      Slur* sl = (Slur*) slurTie();

      QPointF sp(_system->canvasPos());
      System* s;  // dummy
      switch(spannerSegmentType()) {
            case SEGMENT_SINGLE:
                  if (grip == 0)
                        return sl->slurPos(sl->startElement(), s);
                  else if (grip == 3)
                        return sl->slurPos(sl->endElement(), s);
                  return QPointF();
            case SEGMENT_BEGIN:
                  if (grip == 0)
                        return sl->slurPos(sl->startElement(), s);
                  else if (grip == 3)
                        return _system->abbox().topRight();
                  break;
            case SEGMENT_MIDDLE:
                  if (grip == 0)
                        return sp;
                  else if (grip == 3)
                        return _system->abbox().topRight();
                  break;
            case SEGMENT_END:
                  if (grip == 0)
                        return sp;
                  else if (grip == 3)
                        return sl->slurPos(sl->endElement(), s);
                  break;
            }
      return QPointF();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void SlurSegment::editDrag(int curGrip, const QPointF& delta)
      {
      double _spatium = spatium();

      ups[curGrip].off += (delta / spatium());

      if (curGrip == 0 || curGrip == 3) {
            //
            //  compute bezier help points
            //
            QPointF p0 = ups[0].p + ups[0].off * _spatium;
            QPointF p3 = ups[3].p + ups[3].off * _spatium;

            qreal xdelta = p3.x() - p0.x();
            if (xdelta == 0.0) {
                  printf("bad slur slope\n");
                  return;
                  }

            qreal d    = xdelta / 4.0;
            qreal x1   = p0.x() + d;
            qreal x2   = p3.x() - d;

            qreal slope = (p3.y() - p0.y()) / xdelta;
            qreal y1    = p0.y() + (x1-p0.x()) * slope + bow;
            qreal y2    = p0.y() + (x2-p0.x()) * slope + bow;
            ups[1].p    = QPointF(x1, y1);
            ups[2].p    = QPointF(x2, y2);
            }
      updatePath();
      }

//---------------------------------------------------------
//    bbox
//---------------------------------------------------------

QRectF SlurSegment::bbox() const
      {
      return path.boundingRect();
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath SlurSegment::shape() const
      {
      return path;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurSegment::write(Xml& xml, int no) const
      {
      bool empty = true;
      for (int i = 0; i < 4; ++i) {
            if (!(ups[i].off.isNull())) {
                  empty = false;
                  break;
                  }
            }
      if (!userOff().isNull() || !visible())
            empty = false;
      if (empty)
            return;

      xml.stag(QString("SlurSegment no=\"%1\"").arg(no));
      if (!visible())
            xml.tag("visible", visible());
      if (!userOff().isNull())
            xml.tag("offset", userOff() / spatium());
      if (!(ups[0].off.isNull()))
            xml.tag("o1", ups[0].off);
      if (!(ups[1].off.isNull()))
            xml.tag("o2", ups[1].off);
      if (!(ups[2].off.isNull()))
            xml.tag("o3", ups[2].off);
      if (!(ups[3].off.isNull()))
            xml.tag("o4", ups[3].off);
      xml.etag();
      }

//---------------------------------------------------------
//   readSegment
//---------------------------------------------------------

void SlurSegment::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "o1")
                  ups[0].off = readPoint(e);
            else if (tag == "o2")
                  ups[1].off = readPoint(e);
            else if (tag == "o3")
                  ups[2].off = readPoint(e);
            else if (tag == "o4")
                  ups[3].off = readPoint(e);
            else if (!Element::readProperties(e))
                  domError(e);
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
      double _spatium = spatium();
      qreal x0 = ups[0].p.x() + ups[0].off.x() * _spatium;
      qreal x3 = ups[3].p.x() + ups[3].off.x() * _spatium;

      qreal y0 = ups[0].p.y() + ups[0].off.y() * _spatium;
      qreal y3 = ups[3].p.y() + ups[3].off.y() * _spatium;

      qreal xdelta = x3 - x0;
      if (xdelta == 0.0 || (x0 > x3)) {
            printf("illegal slurSegment\n");
            return;
            }

      qreal d  = xdelta / 6.0;
      qreal x1 = x0 + d;
      qreal x2 = x3 - d;

      double maxBow = xdelta * .4;
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
//   dump
//---------------------------------------------------------

void SlurSegment::dump() const
      {
      printf("SlurSegment %f/%f %f/%f %f/%f %f/%f\n",
            ups[0].off.x(), ups[0].off.y(),
            ups[1].off.x(), ups[1].off.y(),
            ups[2].off.x(), ups[2].off.y(),
            ups[3].off.x(), ups[3].off.y());
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool SlurSegment::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Edit Mode"));
      a->setData("edit");
      a = popup->addAction(tr("Slur Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void SlurSegment::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            SlurProperties sp(0);
            sp.setLineType(slurTie()->lineType());
            int rv = sp.exec();
            if (rv) {
                  int lt = sp.getLineType();
                  if (lt != slurTie()->lineType()) {
                        score()->undo()->push(new ChangeSlurProperties(slurTie(), lt));
                        }
                  }
            }
      else
            Element::propertyAction(viewer, s);
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
      delSegments    = t.delSegments;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
      {
      }

#if 0
//---------------------------------------------------------
//   change
//---------------------------------------------------------

void SlurTie::change(Element* o, Element* n)
      {
      int idx = segments.indexOf((SlurSegment*)o);
      if (idx == -1) {
            printf("SlurTie: cannot change %p\n", o);
            return;
            }
      n->setParent(this);
      segments[idx] = (SlurSegment*)n;
      }
#endif

//---------------------------------------------------------
//   slurPos
//---------------------------------------------------------

QPointF SlurTie::slurPos(Element* e, System*& s)
      {
      double _spatium = spatium();

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
//   writeProperties
//---------------------------------------------------------

void SlurTie::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      int idx = 0;
      foreach(const SpannerSegment* ss, spannerSegments())
            ((SlurSegment*)ss)->write(xml, idx++);
      if (_slurDirection)
            xml.tag("up", _slurDirection);
      if (_lineType)
            xml.tag("lineType", _lineType);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "SlurSegment") {
            int idx = e.attribute("no", 0).toInt();
            int n = spannerSegments().size();
            for (int i = n; i < idx; ++i)
                  add(new SlurSegment(score()));
            SlurSegment* segment = new SlurSegment(score());
            segment->read(e);
            add(segment);
            }
      else if (tag == "up")
            _slurDirection = Direction(val.toInt());
      else if (tag == "lineType")
            _lineType = val.toInt();
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void SlurTie::toDefault()
      {
      score()->undoChangeUserOffset(this, QPointF());
      }

void SlurSegment::toDefault()
      {
      score()->undoChangeUserOffset(this, QPointF());
      score()->undo()->push(new ChangeSlurOffsets(this, QPointF(), QPointF(), QPointF(), QPointF()));
      for (int i = 0; i < 4; ++i)
            ups[i].off = QPointF();
      parent()->toDefault();
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
//   write
//---------------------------------------------------------

void Slur::write(Xml& xml) const
      {
      xml.stag(QString("Slur id=\"%1\"").arg(id() + 1));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slur::read(QDomElement e)
      {
      setTrack(0);      // set staff
      setId(e.attribute("id").toInt());
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
//            if (tag == "tick2")
//                  _tick2 = score()->fileDivision(i);
            if (tag == "track2")
                  _track2 = i;
//            else if (tag == "startTick")        // obsolete
//                  ; //                  setTick(i);
//            else if (tag == "endTick")          // obsolete
//                  setTick2(i);
            else if (tag == "startTrack")       // obsolete
                  setTrack(i);
            else if (tag == "endTrack")         // obsolete
                  setTrack2(i);
            else if (!SlurTie::readProperties(e))
                  domError(e);
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
      double _spatium = spatium();

      if (score() == gscore || !startElement()) {      // HACK
            //
            // when used in a palette, slur has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            setLen(_spatium * 7);
            SlurSegment* s;
            if (spannerSegments().isEmpty()) {
                  s = new SlurSegment(score());
                  s->setTrack(track());
                  add(s);
                  }
            else {
                  s = frontSegment();
                  }
            s->setSpannerSegmentType(SEGMENT_SINGLE);
            qreal bow = up ? 1.5 * -_spatium : 1.5 * _spatium;

            s->layout(QPointF(0, 0), QPointF(_len, 0), bow);
            return;
            }
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

double SlurTie::firstNoteRestSegmentX(System* system)
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
//   write
//---------------------------------------------------------

void Tie::write(Xml& xml) const
      {
      xml.stag("Tie");
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tie::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (Element::readProperties(e))
                  ;
            else if (SlurTie::readProperties(e))
                  ;
            else
                  domError(e);
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
      if (startElement() == 0 || endElement() == 0)
            return;

      double _spatium = spatium();

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

