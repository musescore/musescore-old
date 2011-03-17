//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "stem.h"
#include "beam.h"
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
      path         = b.path;
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
      double w = (score()->styleS(ST_SlurMidWidth).val() - score()->styleS(ST_SlurEndWidth).val()) * _spatium;
      QPointF t(0.0, w);    // thickness of slur

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

void SlurSegment::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
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

      ChordRest* cr = 0;
      Element* e    = curGrip == 0 ? sl->startElement() : sl->endElement();
      Element* e1   = curGrip == 0 ? sl->endElement() : sl->startElement();

      if (key == Qt::Key_Left)
            cr = prevChordRest((ChordRest*)e);
      else if (key == Qt::Key_Right)
            cr = nextChordRest((ChordRest*)e);

      if (cr == 0 || cr == (ChordRest*)e1)
            return true;
      changeAnchor(viewer, curGrip, cr);
      return true;
      }

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void SlurSegment::changeAnchor(ScoreView* viewer, int curGrip, ChordRest* cr)
      {
      Slur* sl = static_cast<Slur*>(slurTie());
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

      int segments  = sl->spannerSegments().size();
      ups[curGrip].off = QPointF();
      sl->layout();
      if (sl->spannerSegments().size() != segments) {
            SlurSegment* newSegment = curGrip == 3 ? sl->backSegment() : sl->frontSegment();
            score()->endCmd();
            score()->startCmd();
            viewer->startEdit(newSegment, curGrip);
            score()->setLayoutAll(true);
            }
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF SlurSegment::gripAnchor(int grip) const
      {
      SlurPos spos;
      slurTie()->slurPos(&spos);

      QPointF sp(_system->canvasPos());
      switch(spannerSegmentType()) {
            case SEGMENT_SINGLE:
                  if (grip == 0)
                        return spos.p1;
                  else if (grip == 3)
                        return spos.p2;
                  return QPointF();

            case SEGMENT_BEGIN:
                  if (grip == 0)
                        return spos.p1;
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
                        return spos.p2;
                  break;
            }
      return QPointF();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void SlurSegment::editDrag(const EditData& ed)
      {
      ups[ed.curGrip].off += (ed.delta / spatium());
      if (ed.curGrip == 0 || ed.curGrip == 3) {
            computeBezier();
            //
            // move anchor for slurs
            //
            Slur* slur = static_cast<Slur*>(slurTie());
            if ((slur->type() == SLUR)
               && (
                  (ed.curGrip == 0 && (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN))
                  || (ed.curGrip == 3 && (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END))
                  )
               ) {
                  Element* e = ed.view->elementNear(ed.pos);
                  if (e && e->type() == NOTE) {
                        Chord* chord = static_cast<Note*>(e)->chord();
                        if ((ed.curGrip == 3 && chord != slur->endElement())
                           || (ed.curGrip == 0 && chord != slur->startElement())) {
                              changeAnchor(ed.view, ed.curGrip, chord);
                              QPointF p1 = ed.pos - ups[ed.curGrip].p + canvasPos();
                              ups[ed.curGrip].off = p1 / spatium();
                              return;
                              }
                        }
                  }
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
//   computeBezier
//    compute help points of slur bezier segment
//---------------------------------------------------------

void SlurSegment::computeBezier()
      {
      double _spatium = spatium();
      qreal bow       = score()->styleS(ST_SlurBow).val() * _spatium;
      //
      //  compute bezier help points
      //
      QPointF p1 = ups[0].p;
      QPointF p2 = ups[3].p;
      qreal x0   = p1.x() + ups[0].off.x() * _spatium;
      qreal y0   = p1.y() + ups[0].off.y() * _spatium;
      qreal x3   = p2.x() + ups[3].off.x() * _spatium;
      qreal y3   = p2.y() + ups[3].off.y() * _spatium;

      qreal dx = x3 - x0;
      if (dx <= 0.0 || (x0 > x3)) {
            if (debugMode)
                  printf("illegal slurSegment dx %f ---  x0 %f > x3 %f\n", dx, x0, x3);
            return;
            }
      if (bow > (dx * .2))       // limit bow for small slurs
            bow = dx * .2;

      qreal d     = dx / 5.0;
      qreal x1    = x0 + d;
      qreal x2    = x3 - d;
      qreal dy    = y3 - y0;
      qreal slope = dy / dx;

      qreal y1, y2;
      bool up   = slurTie()->isUp();
      qreal ddx = dx * .1 * slope;
      qreal ddy = bow * (1.0 - qAbs(slope) * 1.5);
      if (ddy < 0.5 * _spatium)
            ddy = 0.5 * _spatium;
      if (up) {
            if (slope > 0.0) {
                  y1 = y0 - ddy;
                  y2 = y3 - bow - dy * slope * .8;
                  if (y2 < y1)
                        y2 = y1;
                  x2 += ddx;
                  }
            else {
                  y2 = y3 - ddy;
                  y1 = y0 - bow - dy * slope * .8;
                  if (y1 < y2)
                        y1 = y2;
                  x1 -= ddx;
                  }
            }
      else {
            if (slope < 0.0) {
                  y1 = y0 + ddy;
                  y2 = y3 + bow + dy * slope * .8;
                  if (y2 > y1)
                        y2 = y1;
                  x2 += ddx;
                  }
            else {
                  y2 = y3 + ddy;
                  y1 = y0 + bow + dy * slope * .8;
                  if (y1 > y2)
                        y1 = y2;
                  x1 -= ddx;
                  }
            }

      ups[1].p = QPointF(x1, y1);
      ups[2].p = QPointF(x2, y2);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SlurSegment::layout(const QPointF& p1, const QPointF& p2)
      {
      ups[0].p = p1;
      ups[3].p = p2;
      computeBezier();
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
      // delSegments    = t.delSegments;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
      {
      }

//---------------------------------------------------------
//   fixArticulations
//---------------------------------------------------------

static qreal fixArticulations(qreal yo, Chord* c, qreal _up)
      {
      //
      // handle special case of tenuto and staccato;
      //
      QList<Articulation*>* al = c->getArticulations();
      if (al->size() == 1) {
            Articulation* a = al->at(0);
            if (a->subtype() == TenutoSym || a->subtype() == StaccatoSym)
                  yo = a->y() + (a->height() + c->score()->spatium() * .5) * _up;
            }
      return yo;
      }

//---------------------------------------------------------
//   slurPos
//    calculate position of start- and endpoint of slur
//---------------------------------------------------------

void SlurTie::slurPos(SlurPos* sp)
      {
      double _spatium = spatium();
      Element* e1 = startElement();
      Element* e2 = endElement();
      bool isTie  = e1->type() == NOTE;

      Note* note1;
      Note* note2;
      Chord* sc;
      Chord* ec;

      if (isTie) {
            note1 = static_cast<Note*>(e1);
            note2 = static_cast<Note*>(e2);
            sc    = note1->chord();
            ec    = note2->chord();
            }
      else {
            if ((e1->type() != CHORD) || (e2->type() != CHORD)) {
                  sp->p1 = e1->canvasPos();
                  sp->p2 = e2->canvasPos();
                  sp->p1.rx() += e1->width();
                  sp->p2.rx() += e2->width();
                  sp->system1 = static_cast<ChordRest*>(e1)->measure()->system();
                  sp->system2 = static_cast<ChordRest*>(e2)->measure()->system();
                  return;
                  }
            sc    = static_cast<Chord*>(e1);
            ec    = static_cast<Chord*>(e2);
            note1 = up ? sc->upNote() : sc->downNote();
            note2 = up ? ec->upNote() : ec->downNote();
            }
      sp->p1      = sc->canvasPos();
      sp->p2      = ec->canvasPos();
      sp->system1 = sc->measure()->system();
      sp->system2 = ec->measure()->system();

      qreal xo, yo;

      Stem* stem1 = sc->stem();
      Stem* stem2 = ec->stem();

      //
      // default position:
      //    horizontal: middle of note head
      //    vertical:   _spatium * .4 above/below note head
      //
      double hw  = note1->headWidth();
      double hh  = note1->headHeight();
      double _up = up ? -1.0 : 1.0;

      //------p1
      xo = hw * .5;
      yo = 0.0;
      if (isTie && sc->notes().size() > 1) {
            xo = hw * 1.12;
            yo = note1->pos().y() + hw * .3 * _up;
            }
      else {
            yo = note1->yPos() + (hh * .5 + _spatium * .4) * _up;
            if (stem1) {
                  // bool startIsGrace = sc->noteType() != NOTE_NORMAL;

                  Beam* beam1 = sc->beam();
                  if (beam1 && (beam1->elements().back() != sc) && (sc->up() == up)) {
                        double sh = stem1->height() + _spatium;
                        if (up)
                              yo = sc->downNote()->yPos() - sh;
                        else
                              yo = sc->upNote()->yPos() + sh;
                        xo = stem1->pos().x();
                        }
                  else {
                        if (sc->up() && up)
                              xo = note1->headWidth() + _spatium * .3;

                        //
                        // handle case: stem up   - stem down
                        //              stem down - stem up
                        //
                        if ((sc->up() != ec->up()) && (sc->up() == up)) {
                              Note* n1  = sc->up() ? sc->downNote() : sc->upNote();
                              Note* n2  = ec->up() ? ec->downNote() : ec->upNote();
                              double yd = n2->yPos() - n1->yPos();

                              double mh = stem1->height();    // limit y move
                              if (yd > 0.0) {
                                    if (yd > mh)
                                          yd = mh;
                                    }
                              else {
                                    if (yd < - mh)
                                          yd = -mh;
                                    }
                              if ((up && (yd < -_spatium)) || (!up && (yd > _spatium)))
                                    yo += yd;
                              }
                        else if (sc->up() != up)
                              yo = fixArticulations(yo, sc, _up);
                        }
                  }
            }
      sp->p1 += QPointF(xo, yo);

      //------p2
      xo = hw * .5;
      yo = 0.0;
      if (isTie && ec->notes().size() > 1) {
            xo = - hw * 0.12;
            yo = note2->pos().y() + hw * .3 * _up;
            }
      else {
            yo = note2->yPos() + (hh * .5 + _spatium * .4) * _up;
            if (stem2) {
                  Beam* beam2 = ec->beam();
                  if (beam2
                     && (!beam2->elements().isEmpty())
                     && (beam2->elements().front() != ec)
                     && (ec->up() == up)
                     && (sc->noteType() == NOTE_NORMAL)
                        ) {
                        double sh = stem2->height() + _spatium;
                        if (up)
                              yo = ec->downNote()->yPos() - sh;
                        else
                              yo = ec->upNote()->yPos() + sh;
                        xo = stem2->pos().x();
                        }
                  else if (!ec->up() && !up)
                        xo = -_spatium * .3;
                  //
                  // handle case: stem up   - stem down
                  //              stem down - stem up
                  //
                  if ((sc->up() != ec->up()) && (ec->up() == up)) {
                        Note* n1 = sc->up() ? sc->downNote() : sc->upNote();
                        Note* n2 = ec->up() ? ec->downNote() : ec->upNote();
                        double yd = n2->yPos() - n1->yPos();

                        double mh = stem2->height();    // limit y move
                        if (yd > 0.0) {
                              if (yd > mh)
                                    yd = mh;
                              }
                        else {
                              if (yd < - mh)
                                    yd = -mh;
                              }

                        if ((up && (yd > _spatium)) || (!up && (yd < -_spatium)))
                              yo -= yd;
                        }
                  else if (ec->up() != up)
                        yo = fixArticulations(yo, ec, _up);
                  }
            }
      sp->p2 += QPointF(xo, yo);
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
      parent()->layout();
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

static bool chordsHaveTie(Chord* c1, Chord* c2)
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

static bool isDirectionMixture(Chord* c1, Chord* c2)
      {
      bool up = c1->up();
      for (Segment* seg = c1->segment(); seg; seg = seg->next(SegChordRest)) {
            Chord* c = static_cast<Chord*>(seg->element(c1->track()));
            if (!c || c->type() != CHORD)
                  continue;
            if (c->up() != up)
                  return true;
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

      if (score() == gscore || !startElement()) {
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
            s->layout(QPointF(0, 0), QPointF(_len, 0));
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
                  //
                  // assumption:
                  // slurs have only chords or rests as start/end elements
                  //
                  ChordRest* cr1 = static_cast<ChordRest*>(startElement());
                  ChordRest* cr2 = static_cast<ChordRest*>(endElement());
                  Measure* m1    = cr1->measure();

                  Chord* c1 = (cr1->type() == CHORD) ? static_cast<Chord*>(cr1) : 0;
                  Chord* c2 = (cr2->type() == CHORD) ? static_cast<Chord*>(cr2) : 0;

                  up = !(cr1->up());

                  if ((cr2->tick() - cr1->tick()) > m1->ticks()) {
                        // long slurs are always above
                        up = true;
                        }
                  else
                        up = !(cr1->up());

                  if (c1 && c2 && isDirectionMixture(c1, c2) && (c1->noteType() == NOTE_NORMAL)) {
                        // slurs go above if start and end note have different stem directions,
                        // but grace notes are exceptions
                        up = true;
                        }
                  else if (m1->mstaff(cr1->staffIdx())->hasVoices && c1 && c1->noteType() == NOTE_NORMAL) {
                        // in polyphonic passage, slurs go on the stem side
                        up = cr1->up();
                        }
                  else if (c1 && c2 && chordsHaveTie(c1, c2)) {
                        // could confuse slur with tie, put slur on stem side
                        up = cr1->up();
                        }
                  }
                  break;
            }

      SlurPos sPos;
      slurPos(&sPos);

      QList<System*>* sl = score()->systems();
      iSystem is = sl->begin();
      while (is != sl->end()) {
            if (*is == sPos.system1)
                  break;
            ++is;
            }
      if (is == sl->end())
            printf("Slur::layout  first system not found\n");
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      unsigned nsegs = 1;
      for (iSystem iis = is; iis != sl->end(); ++iis) {
            if ((*iis)->isVbox())
                  continue;
            if (*iis == sPos.system2)
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
                  else {
                        s = new SlurSegment(score());
                        }
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

      for (int i = 0; is != sl->end(); ++i, ++is) {
            System* system  = *is;
            if (system->isVbox()) {
                  --i;
                  continue;
                  }
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);
            ChordRest* cr1 = (ChordRest*)startElement();
            SysStaff* ss = system->staff(cr1->staffIdx());
            QPointF sp(system->canvasPos());
            sp.ry() += ss->y();

            // case 1: one segment
            if (sPos.system1 == sPos.system2) {
                  segment->setSubtype(SEGMENT_SINGLE);
                  segment->layout(sPos.p1, sPos.p2);
                  }
            // case 2: start segment
            else if (i == 0) {
                  segment->setSubtype(SEGMENT_BEGIN);
                  qreal x = sp.x() + system->bbox().width();
                  segment->layout(sPos.p1, QPointF(x, sPos.p1.y()));
                  }
            // case 3: middle segment
            else if (i != 0 && system != sPos.system2) {
                  segment->setSubtype(SEGMENT_MIDDLE);
                  qreal x1 = firstNoteRestSegmentX(system) - _spatium;
                  qreal x2 = sp.x() + system->bbox().width();
                  segment->layout(QPointF(x1, sp.y()), QPointF(x2, sp.y()));
                  }
            // case 4: end segment
            else {
                  segment->setSubtype(SEGMENT_END);
                  qreal x = firstNoteRestSegmentX(system) - _spatium;
                  segment->layout(QPointF(x, sPos.p2.y()), sPos.p2);
                  }
            if (system == sPos.system2)
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
      if (startElement() == 0 || endElement() == 0) {
            printf("Tie::layout(): no start or end\n");
            return;
            }

      double _spatium = spatium();

      Chord* c1   = startNote()->chord();
      Measure* m1 = c1->measure();
//      System* s1  = m1->system();
      Chord* c2   = endNote()->chord();
//      Measure* m2 = c2->measure();
//      System* s2  = m2->system();

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

      // TODO: cleanup

      SlurPos sPos;
      slurPos(&sPos);

      // p1, p2, s1, s2

      QList<System*>* systems = score()->systems();
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      int sysIdx1 = systems->indexOf(sPos.system1);
      if (sysIdx1 == -1) {
            printf("system not found\n");
            foreach(System* s, *systems)
                  printf("   search %p in %p\n", sPos.system1, s);
            return;
            }

      int sysIdx2     = systems->indexOf(sPos.system2, sysIdx1);
      unsigned nsegs  = sysIdx2 - sysIdx1 + 1;
      unsigned onsegs = spannerSegments().size();

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

      sPos.p1 -= canvasPos();
      sPos.p2 -= canvasPos();
      for (unsigned int i = 0; i < nsegs; ++i) {
            System* system       = (*systems)[sysIdx1++];
            SlurSegment* segment = segmentAt(i);
            segment->setSystem(system);
            QPointF sp(system->canvasPos() - canvasPos());

            // case 1: one segment
            if (sPos.system1 == sPos.system2) {
                  segment->layout(sPos.p1, sPos.p2);
                  segment->setSpannerSegmentType(SEGMENT_SINGLE);
                  }
            // case 2: start segment
            else if (i == 0) {
                  qreal x = sp.x() + system->bbox().width();
                  segment->layout(sPos.p1, QPointF(x, sPos.p1.y()));
                  segment->setSpannerSegmentType(SEGMENT_BEGIN);
                  }
            // case 3: middle segment
            else if (i != 0 && system != sPos.system2) {
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

                  segment->layout(QPointF(x, sPos.p2.y()), sPos.p2);
                  segment->setSpannerSegmentType(SEGMENT_END);
                  }
            }
      }

