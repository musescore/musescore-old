//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: slur.cpp,v 1.53 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "painter.h"

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

SlurSegment::SlurSegment(SlurTie* st)
   : Element(st->score())
      {
      slur = st;
      mode = 0;
      rb   = new RubberBand(st->score());
      path = 0;
      }

SlurSegment::SlurSegment(const SlurSegment& b)
   : Element(b)
      {
      for (int i = 0; i < 4; ++i)
            ups[i] = b.ups[i];
      path = new QPainterPath(*(b.path));
      slur = b.slur;
      rb   = b.rb->clone();
      bow  = b.bow;
      mode = b.mode;
      }

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

SlurSegment::~SlurSegment()
      {
      delete rb;
      if (path)
            delete path;
      }

//---------------------------------------------------------
//   resetMode
//---------------------------------------------------------

void SlurSegment::resetMode()
      {
      mode = 0;
      }

//---------------------------------------------------------
//   updatePath
//---------------------------------------------------------

void SlurSegment::updatePath()
      {
      QPointF pp[4];
      for (int i = 0; i < 4; ++i)
            pp[i] = ups[i].p + ups[i].off * _spatium;
      if (path)
            delete path;
      path = new QPainterPath;
      QPointF t(0.0, _spatium * .08);    // thickness of slur
      path->moveTo(pp[0]);
      path->cubicTo(pp[1]-t, pp[2]-t, pp[3]);
      path->cubicTo(pp[2]+t, pp[1]+t, pp[0]);
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

void SlurSegment::draw1(Painter& p)
      {
      if (path == 0)
            return;
//      int seg = 0;      // todo
// printf("draw\n");
      p.setBrush(selected() ? preferences.selectColor[0] : Qt::black);
      p.drawPath(*path);
      if (selected() && mode) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            for (int i = 0; i < 4; ++i) {
                  if (i == (mode-1))
                        p.setBrush(Qt::blue);
                  else
                        p.setBrush(Qt::NoBrush);
                  p.drawRect(ups[i].r);
                  }
            }
      if (showRubberBand && (mode == 1 || mode == 4))
            rb->draw(p);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool SlurSegment::startEdit(QMatrix& matrix)
      {
// printf("SlurSegment:: start edit\n");
      mode = 4;
      updateGrips(matrix);
      return true;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void SlurSegment::updateGrips(QMatrix& matrix)
      {
      qreal w = 8.0 / matrix.m11();
      qreal h = 8.0 / matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);

      for (int i = 0; i < 4; ++i) {
            ups[i].r = r.translated(ups[i].p + ups[i].off * _spatium);
            }
      updatePath();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SlurSegment::endEdit()
      {
      mode = 0;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

bool SlurSegment::startEditDrag(const QPointF& p)
      {
      // search for the clicked grip
      mode = 1;
      for (int i = 0; i < 4; ++i) {
            if (ups[i].r.contains(p)) {
                  return true;
                  }
            ++mode;
            }
      mode = 0;
      return false;
      }

//---------------------------------------------------------
//   dragOff
//---------------------------------------------------------

QPointF SlurSegment::dragOff() const
      {
      if (mode)
            return -ups[mode - 1].off;
      else
            return QPointF(0,0);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

bool SlurSegment::editDrag(QMatrix& matrix, QPointF*, const QPointF& delta)
      {
      if (!mode)
            return false;
      int n = mode - 1;

      ups[n].r.translate(delta);
      ups[n].off += (delta / _spatium);

      if (mode == 1 || mode == 4) {
            slur->layout2(apos(), mode, ups[n]);
            //
            //  compute bezier help points
            //
            QPointF p0 = ups[0].pos();
            QPointF p3 = ups[3].pos();

            qreal xdelta = p3.x() - p0.x();
            if (xdelta == 0.0) {
                  printf("bad slur slope\n");
                  return true;
                  }

            qreal d    = xdelta / 4.0;
            qreal x1   = p0.x() + d;
            qreal x2   = p3.x() - d;

            qreal slope = (p3.y() - p0.y()) / xdelta;
            qreal y1    = p0.y() + (x1-p0.x()) * slope + bow;
            qreal y2    = p0.y() + (x2-p0.x()) * slope + bow;
            ups[1].p    = QPointF(x1, y1);
            ups[2].p    = QPointF(x2, y2);

            updateGrips(matrix);

            if (showRubberBand) {
                  QPointF ppos(apos());
                  QPointF rp1( ups[n].p + ups[n].off * _spatium + ppos);
                  QPointF rp2(ups[n].p + ppos);
                  rb->set(rp1, rp2);
                  QPointF ppp(rp2-ppos);
                  //TODO bbox?
                  }
            }
      updatePath();
      return true;
      }

//---------------------------------------------------------
//    bbox
//---------------------------------------------------------

QRectF SlurSegment::bbox() const
      {
      QRectF r;
      if (path == 0)
            return r;
      r = path->boundingRect();
      if (mode) {
            for (int i = 0; i < 4; ++i)
                  r |= ups[i].r;
            }
      return r;
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
      xml.etag("SlurSegment");
      }

//---------------------------------------------------------
//   readSegment
//---------------------------------------------------------

void SlurSegment::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "o1")
                  ups[0].off = readPoint(node);
            else if (tag == "o2")
                  ups[1].off = readPoint(node);
            else if (tag == "o3")
                  ups[2].off = readPoint(node);
            else if (tag == "o4")
                  ups[3].off = readPoint(node);
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   contains
//    return true if p is inside of bounding box of object
//    p is relative to the coordinate system of parent()
//---------------------------------------------------------

#if 0
bool SlurSegment::contains(const QPointF& p) const
      {
      QPointF pp(p);
      pp -= pos();
      QPointF ffp[4];
      for (int i = 0; i < 4; ++i)
            ffp[i] = ups[i].p + ups[i].off * _spatium;

      QRectF r(ffp[0], QSizeF(ffp[1].x(), ffp[1].y()));
      for (int i = 2; i < 4; ++i)
            r |= QRectF(ffp[i], QSizeF(1, 1));
      if (mode) {
            for (int i = 0; i < 4; ++i)
                  r |= ups[i].r;
            }
//      QRectF rr(r.expand(1));
      if (r.contains(pp))
            return true;
      return false;
      }
#endif

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool SlurSegment::edit(QKeyEvent* ev)
      {
      QPointF ppos(apos());

      if ((ev->modifiers() & Qt::ShiftModifier)) {
            if (ev->key() == Qt::Key_Left)
                  slur->prevSeg(apos(), mode, ups[mode-1]);
            else if (ev->key() == Qt::Key_Right)
                  slur->nextSeg(apos(), mode, ups[mode-1]);
            return false;
            }

      QPointF delta;
      qreal val = 1.0;
      if (ev->modifiers() & Qt::ControlModifier)
            val = 0.1;
      switch (ev->key()) {
            case Qt::Key_Left:
                  delta = QPointF(-val, 0);
                  break;
            case Qt::Key_Right:
                  delta = QPointF(val, 0);
                  break;
            case Qt::Key_Up:
                  delta = QPointF(0, -val);
                  break;
            case Qt::Key_Down:
                  delta = QPointF(0, val);
                  break;
            case Qt::Key_Tab:
                  if (mode < 4)
                        ++mode;
                  else
                        mode = 1;
                  break;
            case Qt::Key_X:
                  slur->setSlurDirection(slur->isUp() ? DOWN : UP);
                  break;
            }
      if (mode == 0)
            return false;

      int idx       = (mode-1) % 4;
      ups[idx].off += delta;
      ups[idx].r.translate(delta * _spatium);

      if (mode == 1 || mode == 4) {
            slur->layout2(apos(), mode, ups[idx]);
            if (!showRubberBand || (mode != 1 && mode != 4))
                  return true;
            QPointF ppos(apos());
            QPointF rp1, rp2;
            rp1 = ups[idx].p + ups[idx].off * _spatium + ppos;
            rp2 = ups[idx].p + ppos;
            rb->set(rp1, rp2);
            rp1 -= ppos;
            rp2 -= ppos;
//??            orBbox(QRectF(rp1, QSizeF(rp2.x() - rp1.x(), rp2.y() - rp1.y())));
            }
      return false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SlurSegment::layout(const QPointF& p1, const QPointF& p2, qreal b)
      {
// printf("SlurSegment %p %p layout\n", slur, this);
      bow = b;
      QPointF ppos(apos());
      ups[0].p = p1 - ppos;
      ups[3].p = p2 - ppos;

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
//   endEditDrag
//---------------------------------------------------------

bool SlurSegment::endEditDrag()
      {
      return true;
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
      segments.clear();
      for (ciElement ie = t.segments.begin(); ie != t.segments.end(); ++ie) {
            SlurSegment* ss = new SlurSegment(*(SlurSegment*)(*ie));
            ss->setSlurTie(this);
            segments.push_back(ss);
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
      segments.push_back(s);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SlurTie::remove(Element* s)
      {
      if (!segments.remove(s))
            printf("SlurTie: cannot remove %p\n", s);
//      printf("SlurTie: %d nach remove %p\n", segments.size(), s);
      }

//---------------------------------------------------------
//   slurPos
//---------------------------------------------------------

QPointF SlurTie::slurPos(int tick, Staff* staff, int voice, System*& s)
      {
      Measure* m = _score->tick2measure(tick);
      if (m == 0) {
            printf("SlurTie: cannot find measure for tick %d\n", tick);
            return QPointF(0,0);
            }
      s = m->system();
      ChordRest* cr = m->findChordRest(tick, staff, voice, false);
      if (cr == 0) {
            printf("SlurTie: cannot find chord/rest at tick:%d staff:%d voice:%d, measure %d-%d\n",
               tick, staff->idx(), voice, m->tick(), m->tick() + m->tickLen());
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

      return cr->apos() + off;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTie::writeProperties(Xml& xml) const
      {
      int idx = 0;
      for (ciElement i = segments.begin(); i != segments.end(); ++i, ++idx)
            ((SlurSegment*)*i)->write(xml, idx);
      if (_slurDirection)
            xml.tag("up", _slurDirection);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void SlurTie::setSelected(bool f)
      {
      Element::setSelected(f);
      for (ciElement i = segments.begin(); i != segments.end(); ++i)
            (*i)->setSelected(f);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(QDomNode node)
      {
      QDomElement e = node.toElement();
      if (e.isNull())
            return true;
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "SlurSegment") {
            SlurSegment* segment = new SlurSegment(this);
            segment->read(node);
            segments.push_back(segment);
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
      _staff1 = 0;
      _staff2 = 0;
      _voice1 = 0;
      _voice2 = 0;
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::~Slur()
      {
      for (iElement e = segments.begin(); e != segments.end(); ++e)
            delete *e;
      }

//---------------------------------------------------------
//   nextSeg
//---------------------------------------------------------

void Slur::nextSeg(const QPointF ppos, int mode, struct UP& ups)
      {
      if (mode != 1 && mode != 4)
            return;
      int tick = mode == 1 ? _tick1 : _tick2;
      Segment* seg = _score->tick2segment(tick);
      seg = seg->next1();
      if (seg == 0) {
            printf("no seg found\n");
            return;
            }
      if (tick == seg->tick())
            return;

      System* s;
      if (mode == 1) {
            QPointF p(ups.p + ups.off * _spatium + ppos);
            _tick1 = seg->tick();
            ups.p = slurPos(_tick1, _staff1, _voice1, s);
            }
      else {
            QPointF p(ups.p + ups.off * _spatium + ppos);
            _tick2  = seg->tick();
            ups.p = slurPos(_tick2, _staff2, _voice2, s);
            }
      }

//---------------------------------------------------------
//   prevSeg
//---------------------------------------------------------

void Slur::prevSeg(const QPointF ppos, int mode, struct UP& ups)
      {
      if (mode != 1 && mode != 4)
            return;
      int tick = mode == 1 ? _tick1 : _tick2;
      Segment* seg = _score->tick2segment(tick);
      seg = seg->prev1();
      if (seg == 0) {
            printf("no seg found\n");
            return;
            }
      if (tick == seg->tick())
            return;

      System* s;
      if (mode == 1) {
            QPointF p(ups.p + ups.off * _spatium + ppos);
            _tick1 = seg->tick();
            ups.p = slurPos(_tick1, _staff1, _voice1, s);
            }
      else {
            QPointF p(ups.p + ups.off * _spatium + ppos);
            _tick2  = seg->tick();
            ups.p = slurPos(_tick2, _staff2, _voice2, s);
            }
      }

//---------------------------------------------------------
//   setStart
//---------------------------------------------------------

void Slur::setStart(int t, Staff* staff, int voice)
      {
      _tick1  = t;
      _staff1 = staff;
      _voice1 = voice;
      }

//---------------------------------------------------------
//   setEnd
//---------------------------------------------------------

void Slur::setEnd(int t, Staff* staff, int voice)
      {
      _tick2  = t;
      _staff2 = staff;
      _voice2 = voice;
      }

//---------------------------------------------------------
//   startsAt
//---------------------------------------------------------

bool Slur::startsAt(int t, Staff* staff, int voice)
      {
      return ((_tick1 == t) && (_staff1 == staff) && (_voice1 == voice));
      }

//---------------------------------------------------------
//   endsAt
//---------------------------------------------------------

bool Slur::endsAt(int t, Staff* staff, int voice)
      {
      return ((_tick2 == t) && (_staff2 == staff) && (_voice2 == voice));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Slur::write(Xml& xml) const
      {
      xml.stag("Slur");
      xml.tag("startTick", _tick1);
      xml.tag("endTick", _tick2);
      if (_voice1)
            xml.tag("startVoice", _voice1);
      if (_voice2)
            xml.tag("endVoice", _voice2);
      int s1 = _staff1->idx();
      int s2 = _staff2->idx();
      if (s1)
            xml.tag("startStaff", s1);
      if (s2)
            xml.tag("endStaff", s2);
      SlurTie::writeProperties(xml);
      xml.etag("Slur");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slur::read(Score* score, QDomNode node)
      {
      _staff1 = 0;
      _staff2 = 0;
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "startTick")
                  _tick1 = score->fileDivision(i);
            else if (tag == "endTick")
                  _tick2 = score->fileDivision(i);
            else if (tag == "startVoice")
                  _voice1 = i;
            else if (tag == "endVoice")
                  _voice2 = i;
            else if (tag == "startStaff")
                  _staff1 = score->staff(i);
            else if (tag == "endStaff")
                  _staff2 = score->staff(i);
            else if (SlurTie::readProperties(node))
                  ;
            else
                  printf("Mscore:Slur: unknown tag %s\n",
                     tag.toLatin1().data());
            }
      if (_staff1 == 0)
            _staff1 = score->staff(0);
      if (_staff2 == 0)
            _staff2 = score->staff(0);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slur::layout()
      {
// printf("Slur layout\n");
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
                        ChordRest* c1 = m1->findChordRest(_tick1, _staff1, _voice1, false);
                        if (c1 == 0) {
                              printf("Slur-1: cannot find chord/rest at tick:%d staff:%d voice:%d, measure %d-%d\n",
                                 _tick1, _staff1->idx(), _voice1, m1->tick(), m1->tick() + m1->tickLen());
                              return;
                              }

                        Measure* m2 = _score->tick2measure(_tick2);
                        if (m2 == 0) {
                              printf("Slur: cannot find measure for tick %d\n", _tick2);
                              return;
                              }
                        ChordRest* c2 = m2->findChordRest(_tick2, _staff2, _voice2, false);
                        if (c2 == 0) {
                              printf("Slur-2: cannot find chord/rest at tick:%d staff:%d voice:%d, measure %d-%d\n",
                                 _tick2, _staff2->idx(), _voice2, m2->tick(), m2->tick() + m2->tickLen());
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

      QPointF ppos(apos());
      System *s1, *s2;

      QPointF p1 = slurPos(_tick1, _staff1, _voice1, s1);
      QPointF p2 = slurPos(_tick2, _staff2, _voice2, s2);

      iSystem is = _score->systems()->begin();
      while (is != _score->systems()->end()) {
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
      for (iSystem iis = is; (iis != _score->systems()->end()) && (*iis != s2); ++iis)
            ++nsegs;

      unsigned onsegs = segments.size();
      ElementList segments1 = segments;
      for (iElement i = segments1.begin(); i != segments1.end(); ++i) {
            Element* el = *i;
            Element* parent = el->parent();
// printf("Slur layout %d: remove segment %p from parent %p\n", segments.size(), el, parent);

            if (parent)           // if layout was not called, parent is zero
                  parent->remove(el);
            }
      segments.clear();
      if (nsegs != onsegs) {
// printf("Slur layout: create new segment list\n");
            for (iElement i = segments1.begin(); i != segments1.end(); ++i)
                  delete *i;
            segments1.clear();
            for (unsigned i = 0; i < nsegs; ++i) {
                  SlurSegment* s = new SlurSegment(this);
                  segments1.push_back(s);
                  }
            }
      qreal bow = up ? 2*-_spatium : 2*_spatium;
      iElement ibss = segments1.begin();
      for (int i = 0; is != _score->systems()->end(); ++i, ++is, ++ibss) {
            System* system = *is;
            QPointF sp(system->apos());
            SlurSegment* bs = (SlurSegment*)*ibss;
            bs->setParent(parent());
            bs->setStaff(staff());
            // case 1: one segment
            if (s1 == s2) {
                  bs->layout(p1, p2, bow);
                  }
            // case 2: start segment
            else if (i == 0) {
                  System* system = *is;
                  qreal x       = system->apos().x() + system->bbox().width();
                  bs->layout(p1, QPointF(x, p1.y()), bow);
                  }
            // case 3: middle segment
            else if (i != 0 && *is != s2) {
                  System* system = *is;
                  Measure* m = system->measures()->front();
                  bs->setParent(m);
                  QPointF p = system->apos();
                  qreal x1 = p.x();
                  qreal x2 = x1 + system->bbox().width();
                  bs->layout(QPointF(x1, p.y()), QPointF(x2, p.y()), bow);
                  }
            // case 4: end segment
            else {
                  System* system = *is;
                  Measure* m = system->measures()->front();
                  bs->setParent(m);
                  qreal x = system->apos().x();
                  bs->layout(QPointF(x, p2.y()), p2, bow);
                  }
// printf("Slur layout: add segment %p to parent %p\n", bs, bs->parent());
            bs->parent()->add(bs);
            segments.push_back(bs);
            if (*is == s2)
                  break;
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Slur::bbox() const
      {
      QRectF r;
      for (ciElement i = segments.begin(); i != segments.end(); ++i)
            r |= (*i)->abbox().translated(apos());
      return r;
      }

//---------------------------------------------------------
//   layout2
//    snap to next tick positions
//---------------------------------------------------------

void Slur::layout2(const QPointF ppos, int mode, struct UP& ups)
      {
      //
      // compute absolute position of control point on canvas:
      //
      QPointF p(ups.p + ups.off * _spatium + ppos);

      System* s;
      if (mode == 1) {
            int tick = _score->snapNote(_tick1, p, _staff1->idx());
            if (tick != _tick1) {
                  _tick1     = tick;
                  QPointF p2 = slurPos(_tick1, _staff1, _voice1, s);
                  ups.p      = p2 - ppos;    // relative to parent position
                  ups.off    = (p - p2) / _spatium;
                  }
            }
      else if (mode == 4) {
            //
            // search for nearest segment at p
            //
            int tick = _score->snapNote(_tick2, p, _staff2->idx());

            //
            // if found new segment, and reference point is different,
            // then update ups.p and ups.off
            //
            if (tick != _tick2) {
                  _tick2         = tick;
                  QPointF p2         = slurPos(_tick2, _staff2, _voice2, s);
                  ups.p   = p2 - ppos;    // relative to parent position
                  ups.off = (p - p2) / _spatium;
                  }
            }
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
      xml.etag("Tie");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tie::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            if (SlurTie::readProperties(node))
                  ;
            else
                  printf("Mscore:Tie: unknown tag %s\n",
                     tag.toLatin1().data());
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
      if (_startNote == 0 || _endNote == 0)
            return;

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

      QPointF ppos(apos());
      QPointF p1 = _startNote->apos() + off1;
      QPointF p2 = _endNote->apos()   + off2;

      iSystem is = _score->systems()->begin();
      while (is != _score->systems()->end()) {
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
      for (iSystem iis = is; (iis != _score->systems()->end()) && (*iis != s2); ++iis)
            ++nsegs;

      unsigned onsegs = segments.size();
      ElementList segments1 = segments;
      for (iElement i = segments1.begin(); i != segments1.end(); ++i) {
            Element* el = *i;
            Element* pa = el->parent();
            if (pa) {          // if layout was not called, parent is zero
                  pa->remove(el);
                  }
            }
      segments.clear();
      if (nsegs != onsegs) {
            for (iElement i = segments1.begin(); i != segments1.end(); ++i)
                  delete *i;
            segments1.clear();
            for (unsigned i = 0; i < nsegs; ++i) {
                  SlurSegment* s = new SlurSegment(this);
                  segments1.push_back(s);
                  }
            }

      qreal bow = up ? -_spatium : _spatium;
      iElement ibss = segments1.begin();
      for (int i = 0; is != _score->systems()->end(); ++i, ++is, ++ibss) {
            System* system = *is;
            QPointF sp(system->apos());
            SlurSegment* bs = (SlurSegment*)*ibss;
            bs->setParent(m1);
            bs->setStaff(staff());
            // case 1: one segment
            if (s1 == s2) {
                  bs->layout(p1, p2, bow);
                  }
            // case 2: start segment
            else if (i == 0) {
                  bs->layout(p1, QPointF(p1.x()+2*_spatium, p1.y()), bow);
                  }
            // case 3: end segment
            else {
                  Measure* m = (*is)->measures()->front();
                  bs->setParent(m);
                  bs->layout(QPointF(p2.x()-2*_spatium, p2.y()), p2, bow);
                  }
            segments.push_back(bs);
            bs->parent()->add(bs);  // puts segment also on segments list
            if (*is == s2)
                  break;
            }
      }

