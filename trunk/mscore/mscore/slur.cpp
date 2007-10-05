//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: slur.cpp,v 1.53 2006/03/28 14:58:58 wschweer Exp $
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
#include "layout.h"
#include "viewer.h"

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

SlurSegment::SlurSegment(Score* score)
   : Element(score)
      {
      _segmentType = SEGMENT_SINGLE;
      _system      = 0;
      }

SlurSegment::SlurSegment(const SlurSegment& b)
   : Element(b)
      {
      for (int i = 0; i < 4; ++i)
            ups[i] = b.ups[i];
      path         = b.path;
      bow          = b.bow;
      _segmentType = b._segmentType;
      _system      = b._system;
      }

//---------------------------------------------------------
//   updatePath
//---------------------------------------------------------

void SlurSegment::updatePath()
      {
      QPointF pp[4];
      for (int i = 0; i < 4; ++i)
            pp[i] = ups[i].pos();
      path = QPainterPath();
      QPointF t(0.0, _spatium * .08);    // thickness of slur
      path.moveTo(pp[0]);
      path.cubicTo(pp[1]-t, pp[2]-t, pp[3]);
      path.cubicTo(pp[2]+t, pp[1]+t, pp[0]);
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurSegment::move(const QPointF& s)
      {
      _pos += s;
      for (int k = 0; k < 4; ++k)
            ups[k].p += s;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SlurSegment::draw(QPainter& p)
      {
      p.setBrush(color());
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
            r[i].translate(ups[i].pos() + p);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool SlurSegment::startEdit(const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(int curGrip, QKeyEvent* ev)
      {
      if (slurTie()->type() != SLUR)
            return false;
      Slur* sl = (Slur*) slurTie();

      if ((ev->modifiers() & Qt::ShiftModifier)
         && ((_segmentType == SEGMENT_SINGLE)
              || (_segmentType == SEGMENT_BEGIN && curGrip == 0)
              || (_segmentType == SEGMENT_END && curGrip == 3)
            )
         ) {
            int track1 = sl->track1();
            int track2 = sl->track2();
            int tick1  = sl->tick1();
            int tick2  = sl->tick2();

            if (ev->key() == Qt::Key_Left) {
                  if (curGrip == 0)
                        tick1 = score()->prevSeg1(tick1, track1);
                  else if (curGrip == 3)
                        tick2 = score()->prevSeg1(tick2, track2);
                  }
            else if (ev->key() == Qt::Key_Right) {
                  if (curGrip == 0)
                        tick1 = score()->nextSeg1(tick1, track1);
                  else if (curGrip == 3)
                        tick2 = score()->nextSeg1(tick2, track2);
                  }
            else {
                  return false;
                  }
            sl->setTrack1(track1);
            sl->setTrack2(track2);
            sl->setTick1(tick1);
            sl->setTick2(tick2);
            return true;
            }
      if (ev->key() == Qt::Key_X) {
            slurTie()->setSlurDirection(slurTie()->isUp() ? DOWN : UP);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF SlurSegment::gripAnchor(int grip) const
      {
      Slur* sl = (Slur*) slurTie();

      QPointF sp(_system->canvasPos());
      System* s;  // dummy
      switch(_segmentType) {
            case SEGMENT_SINGLE:
                  if (grip == 0)
                        return sl->slurPos(sl->tick1(), sl->track1(), s);
                  else if (grip == 3)
                        return sl->slurPos(sl->tick2(), sl->track2(), s);
                  return QPointF();
            case SEGMENT_BEGIN:
                  if (grip == 0)
                        return sl->slurPos(sl->tick1(), sl->track1(), s);
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
                        return sl->slurPos(sl->tick2(), sl->track2(), s);
                  break;
            }
      return QPointF();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void SlurSegment::editDrag(int curGrip, const QPointF&, const QPointF& delta)
      {
      ups[curGrip].off += (delta / _spatium);

      if (curGrip == 0 || curGrip == 3) {
            //
            //  compute bezier help points
            //
            QPointF p0 = ups[0].pos();
            QPointF p3 = ups[3].pos();

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
      if (empty)
            return;
      xml.stag(QString("SlurSegment no=\"%1\"").arg(no));
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
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SlurSegment::layout(ScoreLayout*, const QPointF& p1, const QPointF& p2, qreal b)
      {
// printf("SlurSegment %p %p layout\n", slur, this);
      bow = b;
      ups[0].p = p1;
      ups[3].p = p2;

      //
      //  compute bezier help points
      //
      qreal x0 = ups[0].pos().x();
      qreal x3 = ups[3].pos().x();
      qreal y0 = ups[0].pos().y();
      qreal y3 = ups[3].pos().y();

      qreal xdelta = x3 - x0;
      if (xdelta == 0.0) {
            printf("bad slur slope\n");
            return;
            }

      qreal d = xdelta / 4.0;
      qreal x1 = x0 + d;
      qreal x2 = x3 - d;

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
      QAction* a = popup->addAction(QT_TR_NOOP("Edit Mode"));
      a->setData("edit");
      return true;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(Score* s)
   : Element(s)
      {
      _slurDirection = AUTO;
      up = true;
      }

SlurTie::SlurTie(const SlurTie& t)
   : Element(t)
      {
      up             = t.up;
      _slurDirection = t._slurDirection;
      //
      // duplicate segments
      //
      foreach(const Element* s, t.segments) {
            SlurSegment* ss = new SlurSegment(*(const SlurSegment*)s);
            ss->setParent(this);
            segments.append(ss);
            }
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
      {
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SlurTie::add(Element* s)
      {
//      printf("SlurTie: %d vor add %p\n", segments.size(), s);
      s->setParent(this);
      segments.push_back((SlurSegment*)s);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SlurTie::remove(Element* s)
      {
      int idx = segments.indexOf((SlurSegment*)s);
      if (idx == -1)
            printf("SlurTie: cannot remove %p\n", s);
      else
            segments.removeAt(idx);
//      printf("SlurTie: %d nach remove %p\n", segments.size(), s);
      }

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
      segments[idx] = (SlurSegment*)n;
      }

//---------------------------------------------------------
//   slurPos
//---------------------------------------------------------

QPointF SlurTie::slurPos(int tick, int track, System*& s)
      {
      Measure* m = _score->tick2measure(tick);
      if (m == 0) {
            printf("SlurTie: cannot find measure for tick %d\n", tick);
            return QPointF(0,0);
            }
      s = m->system();
      ChordRest* cr = m->findChordRest(tick, track);
      if (cr == 0) {
            printf("SlurTie: cannot find chord/rest at tick:%d track:%d, measure %d-%d\n",
               tick, track, m->tick(), m->tick() + m->tickLen());
abort();
            return QPointF(0,0);
            }

      //-----------------------------------------
      //    off
      //-----------------------------------------

      qreal w  = cr->width() / 2.0;
      qreal yo = 0.0;
      if (cr->type() == CHORD) {
            Chord* c = (Chord*)cr;
            Stem* stem = c->stem();
            if (up) {
                  yo = c->upNote()->pos().y() - _spatium;
                  if (c->isUp() && stem)
                        yo = c->downNote()->pos().y() - stem->height() - _spatium;
                  }
            else {
                  yo = c->downNote()->pos().y() + _spatium;
                  if (!c->isUp() && stem)
                        yo = c->upNote()->pos().y() + stem->height() + _spatium;
                  }
            }
      QPointF off(w, yo);

      //-----------------------------------------
      //    p
      //-----------------------------------------

      return cr->canvasPos() + off;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTie::writeProperties(Xml& xml) const
      {
      int idx = 0;
      foreach(SlurSegment* ss, segments)
            ss->write(xml, idx++);
      if (_slurDirection)
            xml.tag("up", _slurDirection);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void SlurTie::setSelected(bool f)
      {
      foreach(SlurSegment* ss, segments)
            ss->setSelected(f);
      Element::setSelected(f);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "SlurSegment") {
            SlurSegment* segment = new SlurSegment(score());
            segment->read(e);
            add(segment);
            }
      else if (tag == "up")
            _slurDirection = Direction(val.toInt());
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(Score* s)
   : SlurTie(s)
      {
      _tick1  = 0;
      _tick2  = 0;
      _track1 = 0;
      _track2 = 0;
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::~Slur()
      {
      foreach(SlurSegment* ss, segments)
            delete ss;
      }

//---------------------------------------------------------
//   setTick1
//---------------------------------------------------------

void Slur::setTick1(int val)
      {
      if (val != -1)
            _tick1 = val;
      }

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void Slur::setTick2(int val)
      {
      if (val != -1)
            _tick2 = val;
      }

//---------------------------------------------------------
//   setStart
//---------------------------------------------------------

void Slur::setStart(int t, int track)
      {
      _tick1  = t;
      _track1 = track;
      }

//---------------------------------------------------------
//   setEnd
//---------------------------------------------------------

void Slur::setEnd(int t, int track)
      {
      _tick2  = t;
      _track2 = track;
      }

//---------------------------------------------------------
//   startsAt
//---------------------------------------------------------

bool Slur::startsAt(int t, int track)
      {
      return ((_tick1 == t) && (_track1 == track));
      }

//---------------------------------------------------------
//   endsAt
//---------------------------------------------------------

bool Slur::endsAt(int t, int track)
      {
      return ((_tick2 == t) && (_track2 == track));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Slur::write(Xml& xml) const
      {
      xml.stag("Slur");
      xml.tag("startTick", _tick1);
      xml.tag("endTick", _tick2);
      if (_track1)
            xml.tag("startTrack", _track1);
      if (_track2)
            xml.tag("endTrack", _track2);
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slur::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "startTick")
                  _tick1 = score()->fileDivision(i);
            else if (tag == "endTick")
                  _tick2 = score()->fileDivision(i);
            else if (tag == "startTrack")
                  _track1 = i;
            else if (tag == "endTrack")
                  _track2 = i;
            else if (!SlurTie::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slur::layout(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();
      switch (_slurDirection) {
            case UP:    up = true; break;
            case DOWN:  up = false; break;
            case AUTO:
                  {
                  Measure* m1 = _score->tick2measure(_tick1);
                  if (m1 == 0) {
                        printf("Slur: cannot find measure for tick %d\n", _tick1);
                        return;
                        }
                  if ((_tick2 - _tick1) > m1->tickLen()) {
                        // long slurs are always above
                        up = true;
                        }
                  else {
                        ChordRest* c1 = m1->findChordRest(_tick1, _track1);
                        if (c1 == 0) {
                              printf("Slur-1: cannot find chord/rest at tick:%d track:%d, measure %d-%d\n",
                                 _tick1, _track1, m1->tick(), m1->tick() + m1->tickLen());
                              return;
                              }

                        Measure* m2 = _score->tick2measure(_tick2);
                        if (m2 == 0) {
                              printf("Slur: cannot find measure for tick %d\n", _tick2);
                              return;
                              }
                        ChordRest* c2 = m2->findChordRest(_tick2, _track2);
                        if (c2 == 0) {
                              printf("Slur-2: cannot find chord/rest at tick:%d track:%d, measure %d-%d\n",
                                 _tick2, _track2, m2->tick(), m2->tick() + m2->tickLen());
                              return;
                              }
                        if (c1->isUp())
                              up = false;
                        else
                              up = true;
                        }
                  }
                  break;
            }

      System *s1, *s2;

      QPointF p1 = slurPos(_tick1, _track1, s1);
      QPointF p2 = slurPos(_tick2, _track2, s2);

      QList<System*>* sl = layout->systems();
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

      unsigned onsegs = segments.size();
      if (nsegs > onsegs) {
            for (unsigned i = onsegs; i < nsegs; ++i) {
                  SlurSegment* s = new SlurSegment(score());
                  add(s);
                  }
            }
      else if (nsegs < onsegs) {
            for (unsigned i = nsegs; i < onsegs; ++i) {
                  SlurSegment* s = (SlurSegment*)(segments.takeLast());
                  remove(s);
                  }
            }

      qreal bow = up ? 2*-_spatium : 2*_spatium;
      for (int i = 0; is != layout->systems()->end(); ++i, ++is) {
            System* system  = *is;
            SlurSegment* segment = segments[i];
            segment->setSystem(system);
            QPointF sp(system->canvasPos());

            // case 1: one segment
            if (s1 == s2) {
                  segment->layout(layout, p1, p2, bow);
                  segment->setSegmentType(SEGMENT_SINGLE);
                  }
            // case 2: start segment
            else if (i == 0) {
                  qreal x = sp.x() + system->bbox().width();
                  segment->layout(layout, p1, QPointF(x, p1.y()), bow);
                  segment->setSegmentType(SEGMENT_BEGIN);
                  }
            // case 3: middle segment
            else if (i != 0 && system != s2) {
                  qreal x1 = sp.x();
                  qreal x2 = x1 + system->bbox().width();
                  segment->layout(layout, QPointF(x1, sp.y()), QPointF(x2, sp.y()), bow);
                  segment->setSegmentType(SEGMENT_MIDDLE);
                  }
            // case 4: end segment
            else {
                  qreal x = sp.x();
                  segment->layout(layout, QPointF(x, p2.y()), p2, bow);
                  segment->setSegmentType(SEGMENT_END);
                  }
            if (system == s2)
                  break;
            }
      }

//---------------------------------------------------------
//   bbox
//    DEBUG: needed?
//---------------------------------------------------------

QRectF Slur::bbox() const
      {
      QRectF r;
      foreach(SlurSegment* ss, segments)
            r |= ss->abbox().translated(canvasPos());
      return r;
      }

//---------------------------------------------------------
//   layout2
//    snap to next tick positions
//---------------------------------------------------------

void Slur::layout2(ScoreLayout* /*layout*/, const QPointF /*ppos*/, int /*mode*/, struct UP& /*ups*/)
      {
#if 0
      double _spatium = layout->spatium();
      //
      // compute absolute position of control point on canvas:
      //
      QPointF p(ups.p + ups.off * _spatium + ppos);

      System* s;
      if (mode == 1) {
            int tick = _score->snapNote(_tick1, p, _track1 / VOICES);
            if (tick != _tick1) {
                  _tick1     = tick;
                  QPointF p2 = slurPos(_tick1, _track1, s);
                  ups.p      = p2 - ppos;    // relative to parent position
                  ups.off    = (p - p2) / _spatium;
                  }
            }
      else if (mode == 4) {
            //
            // search for nearest segment at p
            //
            int tick = _score->snapNote(_tick2, p, _track2 / VOICES);

            //
            // if found new segment, and reference point is different,
            // then update ups.p and ups.off
            //
            if (tick != _tick2) {
                  _tick2     = tick;
                  QPointF p2 = slurPos(_tick2, _track2, s);
                  ups.p      = p2 - ppos;    // relative to parent position
                  ups.off    = (p - p2) / _spatium;
                  }
            }
#endif
      }

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(Score* s)
   : SlurTie(s)
      {
      _startNote = 0;
      _endNote   = 0;
      }

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Tie::setStartNote(Note* note)
      {
      _startNote = note;
      setParent(note);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tie::write(Xml& xml) const
      {
      xml.stag("Tie");
      Element::writeProperties(xml);
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

void Tie::layout(ScoreLayout* layout)
      {
      //
      // TODO: if there is a startNote but no endNote
      //    show short bow
      if (_startNote == 0 || _endNote == 0)
            return;

      double _spatium = layout->spatium();
      Chord* c1   = _startNote->chord();
      Measure* m1 = c1->measure();
      System* s1  = m1->system();
      Chord* c2   = _endNote->chord();
      Measure* m2 = c2->measure();
      System* s2  = m2->system();

      if (_slurDirection == AUTO)
            up = !c1->isUp();
      else
            up = _slurDirection == UP ? true : false;
      qreal w   = _startNote->headWidth();
      qreal h   = w * 0.3;
      qreal yo  = up ? -h : h;

      QPointF off1(w, yo);
      QPointF off2(0.0, yo);

      QPointF ppos(canvasPos());
      QPointF p1 = _startNote->canvasPos() + off1;
      QPointF p2 = _endNote->canvasPos()   + off2;

      iSystem is = layout->systems()->begin();
      while (is != layout->systems()->end()) {
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
      for (iSystem iis = is; (iis != layout->systems()->end()) && (*iis != s2); ++iis)
            ++nsegs;

      unsigned onsegs = segments.size();
      QList<SlurSegment*> segments1 = segments;
      foreach (SlurSegment* ss, segments1) {
            Element* pa = ss->parent();
            pa->remove(ss);
            }
      segments.clear();
      if (nsegs != onsegs) {
            foreach(SlurSegment* ss, segments1)
                  delete ss;
            segments1.clear();
            for (unsigned i = 0; i < nsegs; ++i) {
                  SlurSegment* s = new SlurSegment(score());
                  s->setParent(this);
                  segments1.push_back(s);
                  }
            }

      qreal bow = up ? -_spatium : _spatium;
      for (int i = 0; is != layout->systems()->end(); ++i, ++is) {
            System* system = *is;
            QPointF sp(system->canvasPos());
            SlurSegment* bs = segments1[i];
            // case 1: one segment
            if (s1 == s2) {
                  bs->layout(layout, p1, p2, bow);
                  }
            // case 2: start segment
            else if (i == 0) {
                  bs->layout(layout, p1, QPointF(p1.x()+2*_spatium, p1.y()), bow);
                  }
            // case 3: end segment
            else {
                  bs->layout(layout, QPointF(p2.x()-2*_spatium, p2.y()), p2, bow);
                  }
            segments.push_back(bs);
            bs->parent()->add(bs);  // puts segment also on segments list
            if (*is == s2)
                  break;
            }
      }

