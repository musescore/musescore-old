//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chordrest.cpp,v 1.7 2006/03/28 14:58:58 wschweer Exp $
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

#include "chordrest.h"
#include "chord.h"
#include "xml.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "tuplet.h"
#include "layout.h"
#include "chordlist.h"
#include "score.h"
#include "sym.h"
#include "slur.h"
#include "beam.h"
#include "breath.h"
#include "barline.h"
#include "articulation.h"


//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(Score* s)
   : Element(s)
      {
      _tuplet = 0;
      }

//---------------------------------------------------------
//   DurationElement
//---------------------------------------------------------

DurationElement::DurationElement(const DurationElement& e)
   : Element(e)
      {
      _duration = e._duration;
      }

//---------------------------------------------------------
//   hasArticulation
//---------------------------------------------------------

Articulation* ChordRest::hasArticulation(const Articulation* a)
      {
      int idx = a->subtype();
      for (iArticulation l = articulations.begin(); l != articulations.end(); ++l) {
            if (idx == (*l)->subtype())
                  return *l;
            }
      return 0;
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::ChordRest(Score* s)
   : DurationElement(s)
      {
      _beam     = 0;
      _small    = false;
      _beamMode = BEAM_AUTO;
      _dots     = 0;
      _up       = true;
      }

ChordRest::ChordRest(const ChordRest& cr)
   : DurationElement(cr)
      {
      _beam     = 0;
      _up       = cr._up;
      _small    = cr._small;
      _beamMode = cr._beamMode;
      _dots     = cr._dots;

      foreach(Articulation* a, cr.articulations) {            // make deep copy
            Articulation* na = new Articulation(*a);
            na->setParent(this);
            na->setTrack(track());
            articulations.append(na);
            }
      }

//---------------------------------------------------------
//   isUp
//---------------------------------------------------------

bool ChordRest::isUp() const
      {
      return _beam ? _beam->up() : _up;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF ChordRest::canvasPos() const
      {
      if (parent() == 0)
            return QPointF(x(), 0.0);
      double xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      System* system = measure()->system();
      if (system == 0)
            return QPointF();
      double yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(xp, yp);
      }

//---------------------------------------------------------
//   properties
//---------------------------------------------------------

QList<Prop> ChordRest::properties(Xml& xml, bool clipboardmode) const
      {
      QList<Prop> pl = Element::properties(xml);
      //
      // BeamMode default:
      //    REST  - BEAM_NO
      //    CHORD - BEAM_AUTO
      //
      if ((type() == REST && _beamMode != BEAM_NO)
         || (type() == CHORD && _beamMode != BEAM_AUTO)) {
            QString s;
            switch(_beamMode) {
                  case BEAM_AUTO:    s = "auto"; break;
                  case BEAM_BEGIN:   s = "begin"; break;
                  case BEAM_MID:     s = "mid"; break;
                  case BEAM_END:     s = "end"; break;
                  case BEAM_NO:      s = "no"; break;
                  case BEAM_BEGIN32: s = "begin32"; break;
                  case BEAM_INVALID: s = "?"; break;
                  }
            pl.append(Prop("BeamMode", s));
            }
      if (tuplet()) {
            int idx = measure()->tuplets()->indexOf(tuplet());
            if (idx == -1)
                  printf("ChordRest::writeProperties(): tuplet not found\n");
            else
                  pl.append(Prop("Tuplet", idx));
            }
      if (_small)
            pl.append(Prop("small", _small));
      if (!clipboardmode) {
            Duration d;
            d.setVal(tickLen());
            if (duration() != d)
                  pl.append(Prop("durationType", duration().name()));
            }
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(Xml& xml, bool clipboardmode) const
      {
      QList<Prop> pl = properties(xml, clipboardmode);
      xml.prop(pl);
      for (ciArticulation ia = articulations.begin(); ia != articulations.end(); ++ia)
            (*ia)->write(xml);
      if (!xml.noSlurs) {
            foreach(Slur* s, _slurFor)
                  xml.tagE(QString("Slur type=\"start\" number=\"%1\"").arg(s->id()+1));
            foreach(Slur* s, _slurBack)
                  xml.tagE(QString("Slur type=\"stop\" number=\"%1\"").arg(s->id()+1));
            }
      xml.curTick = tick() + tickLen();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(QDomElement e)
      {
      if (Element::readProperties(e))
            return true;
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "BeamMode") {
            int bm = BEAM_AUTO;
            if (val == "auto")
                  bm = BEAM_AUTO;
            else if (val == "begin")
                  bm = BEAM_BEGIN;
            else if (val == "mid")
                  bm = BEAM_MID;
            else if (val == "end")
                  bm = BEAM_END;
            else if (val == "no")
                  bm = BEAM_NO;
            else if (val == "begin32")
                  bm = BEAM_BEGIN32;
            else
                  bm = i;
            _beamMode = BeamMode(bm);
            }
      else if (tag == "Attribute") {
            Articulation* atr = new Articulation(score());
            atr->read(e);
            add(atr);
            }
      else if (tag == "Tuplet") {
            // while reading Measure, parent of Chord or Rest is set
            // to measure; after inserting Chord or Rest into Measure
            // parent is Segment
            Measure* m = (Measure*)parent();
            setTuplet(0);
            foreach(Tuplet* t, *m->tuplets()) {
                  if (t->id() == i) {
                        setTuplet(t);
                        break;
                        }
                  }
            if (tuplet() == 0)
                  printf("Tuplet id %d not found\n", i);
            else
                  setTickLen(tickLen());  // set right symbol + dots
            }
      else if (tag == "small")
            _small = i;
      else if (tag == "Slur") {
            int id = e.attribute("number").toInt() - 1;
            QString type = e.attribute("type");
            Slur* slur = 0;
            foreach(Element* e, *score()->gel()) {
                  if (e->type() == SLUR && ((Slur*)e)->id() == id) {
                        slur = (Slur*)e;
                        break;
                        }
                  }
            if (slur) {
                  if (type == "start") {
                        slur->setStartElement(this);
                        _slurFor.append(slur);
                        }
                  else if (type == "stop") {
                        slur->setEndElement(this);
                        _slurBack.append(slur);
                        }
                  else
                        printf("Note::read(): unknown Slur type <%s>\n", qPrintable(type));
                  }
            else {
                  printf("Note::read(): Slur not found\n");
                  }
            }
      else if (tag == "durationType") {
            Duration d;
            d.setVal(val);
            setDuration(d);
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void ChordRest::setSmall(bool val)
      {
      _small = val;
      double m   = _small ? .7 : 1.0;
      if (staff()->small())
            m *= .7;
      setMag(m);
      }

//---------------------------------------------------------
//   layoutAttributes
//---------------------------------------------------------

void ChordRest::layoutAttributes(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();

      Measure* m = measure();
      System* s  = m->system();
      int idx    = staff()->rstaff();
      qreal x    = centerX();
      qreal sy   = _spatium;      // TODO: style parameter: distance to top/bottom line
      qreal sy2  = _spatium * .5; // TODO: style parameter: distance to top/bottom note

      qreal chordTopY = upPos()   - point(score()->style()->propertyDistanceStem) - sy2;
      qreal chordBotY = downPos() + point(score()->style()->propertyDistanceHead) + sy2;
      qreal staffTopY = s->bboxStaff(idx).y() - pos().y()      - sy;
      qreal staffBotY = staffTopY + s->bboxStaff(idx).height() + sy;

      qreal dyTop = 0;
      qreal dyBot = 0;

      //
      //    pass 1
      //    place all articulations with anchor at chord/rest
      //
      for (iArticulation ia = articulations.begin(); ia != articulations.end(); ++ia) {
            Articulation* a = *ia;
            qreal y = 0;
            ArticulationAnchor aa = Articulation::articulationList[a->subtype()].anchor;
            if (aa == A_CHORD)
                  aa = isUp() ? A_BOTTOM_CHORD : A_TOP_CHORD;
            if (aa == A_TOP_CHORD) {
                  y = chordTopY - dyTop;
                  //
                  // check for collision with staff line
                  //
                  if (y >= staffTopY+.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4)
                              y -= _spatium * .5;
                        }
                  a->setPos(x, y);
                  dyTop += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
            else if (aa == A_BOTTOM_CHORD) {
                  y = chordBotY + dyBot;
                  //
                  // check for collision with staff line
                  //
                  if (y <= staffBotY+.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4)
                              y += _spatium * .5;
                        }
                  a->setPos(x, y);
                  dyBot += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
            }

      //
      //    pass 1
      //    now place all articulations with staff top or bottom anchor
      //
      if (chordTopY - dyTop > staffTopY)
            dyTop = 0;
      else
            dyTop = staffTopY - (chordTopY - dyTop);

      if (chordBotY + dyBot < staffBotY)
            dyBot = 0;
      else
            dyBot = (chordBotY + dyBot) - staffBotY;

      for (iArticulation ia = articulations.begin(); ia != articulations.end(); ++ia) {
            Articulation* a = *ia;
            qreal y = 0;
            ArticulationAnchor aa = Articulation::articulationList[a->subtype()].anchor;
            if (aa == A_TOP_STAFF) {
                  y = staffTopY - dyTop;
                  a->setPos(x, y);
                  dyTop += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
            else if (aa == A_BOTTOM_STAFF) {
                  y = staffBotY + dyBot;
                  a->setPos(x, y);
                  dyBot += (point(score()->style()->propertyDistance) + a->bbox().height());
                  }
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ChordRestList::add(ChordRest* n)
      {
      std::multimap<const int, ChordRest*, std::less<int> >::insert(std::pair<const int, ChordRest*> (n->tick(), n));
      }

//---------------------------------------------------------
//   addSlurFor
//---------------------------------------------------------

void ChordRest::addSlurFor(Slur* s)
      {
      int idx = _slurFor.indexOf(s);
      if (idx >= 0) {
            printf("ChordRest::setSlurFor(): already there\n");
            return;
            }
      _slurFor.append(s);
      }

//---------------------------------------------------------
//   addSlurBack
//---------------------------------------------------------

void ChordRest::addSlurBack(Slur* s)
      {
      int idx = _slurBack.indexOf(s);
      if (idx >= 0) {
            printf("ChordRest::setSlurBack(): already there\n");
            return;
            }
      _slurBack.append(s);
      }

//---------------------------------------------------------
//   removeSlurFor
//---------------------------------------------------------

void ChordRest::removeSlurFor(Slur* s)
      {
      int idx = _slurFor.indexOf(s);
      if (idx < 0) {
            printf("ChordRest::removeSlurFor(): not found\n");
            return;
            }
      _slurFor.removeAt(idx);
      }

//---------------------------------------------------------
//   removeSlurBack
//---------------------------------------------------------

void ChordRest::removeSlurBack(Slur* s)
      {
      int idx = _slurBack.indexOf(s);
      if (idx < 0) {
            printf("ChordRest::removeSlurBack(): not found\n");
            return;
            }
      _slurBack.removeAt(idx);
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void ChordRest::setLen(int ticks)
      {
      setTickLen(ticks);
      Duration dt;
      int dts;
      headType(ticks, &dt, &dts);
      setDuration(dt);
      setDots(dts);
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* ChordRest::drop(const QPointF& p1, const QPointF& p2, Element* e)
      {
      Measure* m  = measure();
      switch (e->type()) {
            case BREATH:
                  {
                  Breath* b = static_cast<Breath*>(e);
                  b->setTrack(staffIdx() * VOICES);

                  // TODO: insert automatically in all staves?

                  Segment* seg = m->findSegment(Segment::SegBreath, tick());
                  if (seg == 0) {
                        seg = m->createSegment(Segment::SegBreath, tick());
                        score()->undoAddElement(seg);
                        }
                  b->setParent(seg);
                  score()->undoAddElement(b);
                  }
                  break;

            case BAR_LINE:
                  {
                  BarLine* bl = static_cast<BarLine*>(e);
                  bl->setTrack(staffIdx() * VOICES);

                  if ((bl->tick() == m->tick())
                     || (bl->tick() == m->tick() + m->tickLen())) {
                        return m->drop(p1, p2, e);
                        }

                  Segment* seg = m->findSegment(Segment::SegBarLine, tick());
                  if (seg == 0) {
                        seg = m->createSegment(Segment::SegBarLine, tick());
                        score()->undoAddElement(seg);
                        }
                  bl->setParent(seg);
                  score()->undoAddElement(bl);
                  }
                  break;

            case CLEF:
                  staff()->changeClef(tick(), e->subtype());
                  break;

            default:
                  printf("cannot drop %s\n", e->name());
                  return 0;
            }
      return 0;
      }

