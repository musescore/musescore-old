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
#include "tempo.h"
#include "tempotext.h"

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
      _tuplet   = e._tuplet;
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
      _beam               = 0;
      _up                 = cr._up;
      _small              = cr._small;
      _beamMode           = cr._beamMode;
      _dots               = cr._dots;
      _extraLeadingSpace  = cr._extraLeadingSpace;
      _extraTrailingSpace = cr._extraTrailingSpace;

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
      return _beam ? (_beam->up() != -1 ? _beam->up() : _up) : _up;
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
      if (_tickLen)
            pl.append(Prop("ticklen", _tickLen));
      if (tuplet())
            pl.append(Prop("Tuplet", tuplet()->id()));
      if (_small)
            pl.append(Prop("small", _small));
      if (_extraLeadingSpace.val() != 0.0)
            pl.append(Prop("leadingSpace", _extraLeadingSpace.val()));
      if (_extraTrailingSpace.val() != 0.0)
            pl.append(Prop("trailingSpace", _extraTrailingSpace.val()));
      if (!clipboardmode) {
            if (tickLen() != duration().ticks(_dots)) {
                  if (_dots)
                        pl.append(Prop("dots", _dots));
                  pl.append(Prop("durationType", duration().name()));
                  }
            }
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(Xml& xml) const
      {
      QList<Prop> pl = properties(xml);
      xml.prop(pl);
      for (ciArticulation ia = articulations.begin(); ia != articulations.end(); ++ia)
            (*ia)->write(xml);
      if (!xml.noSlurs) {
            foreach(Slur* s, _slurFor)
                  xml.tagE(QString("Slur type=\"start\" number=\"%1\"").arg(s->id()+1));
            foreach(Slur* s, _slurBack)
                  xml.tagE(QString("Slur type=\"stop\" number=\"%1\"").arg(s->id()+1));
            }
      if (!xml.clipboardmode && _beam)
            xml.tag("Beam", _beam->id());
      xml.curTick = tick() + tickLen();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(QDomElement e, const QList<Tuplet*>& tuplets,
   const QList<Beam*>& beams)
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
      else if (tag == "Attribute" || tag == "Articulation") {     // obsolete: "Attribute"
            Articulation* atr = new Articulation(score());
            atr->read(e);
            add(atr);
            }
      else if (tag == "Tuplet") {
            setTuplet(0);
            foreach(Tuplet* t, tuplets) {
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
      else if (tag == "leadingSpace")
            _extraLeadingSpace = Spatium(val.toDouble());
      else if (tag == "trailingSpace")
            _extraTrailingSpace = Spatium(val.toDouble());
      else if (tag == "Beam") {
            foreach(Beam* b, beams) {
                  if (b->id() == i) {
                        setBeam(b);
                        b->add(this);
                        break;
                        }
                  }
            if (beam() == 0)
                  printf("Beam id %d not found\n", i);
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
      else if (tag == "ticklen")
            setTickLen(i);
      else if (tag == "tickLen")    // debug
            setTickLen(i);
      else if (tag == "dots")
            _dots = i;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void ChordRest::setSmall(bool val)
      {
      _small     = val;
      double m = 1.0;
      if (_small)
            m = score()->styleD(ST_smallNoteMag);
      if (staff()->small())
            m *= score()->styleD(ST_smallStaffMag);
      setMag(m);
      }

//---------------------------------------------------------
//   layoutArticulations
//    if ((a->subtype() == TenutoSym) || (a->subtype() == StaccatoSym))
//---------------------------------------------------------

void ChordRest::layoutArticulations(ScoreLayout* layout)
      {
      double _spatium  = layout->spatium();
      Measure* m       = measure();
      System* s        = m->system();
      int idx          = staff()->rstaff();
      qreal x          = centerX();

      double distance  = point(score()->styleS(ST_propertyDistance));
      double distance1 = point(score()->styleS(ST_propertyDistanceHead));
      double distance2 = point(score()->styleS(ST_propertyDistanceStem));

      qreal chordTopY = upPos();
      qreal chordBotY = downPos();

      qreal staffTopY = s->bboxStaff(idx).y() - pos().y();
      qreal staffBotY = staffTopY + 4.0 * _spatium + distance2;
      staffTopY      -= distance2;

      qreal dy = 0.0;

      //
      //    pass 1
      //    place all articulations with anchor at chord/rest
      //

      foreach (Articulation* a, articulations) {
            ArticulationAnchor aa = a->anchor();
            if (aa != A_CHORD)
                  continue;
            QRectF bb(a->bbox());

            dy += distance1;
            if (bb.height() > (_spatium * .3))   // hack
                  dy += bb.height() * .5;
            qreal y = dy;
            if (isUp()) {
                  y = chordBotY + dy;
                  //
                  // check for collision with staff line
                  //
                  if (y <= staffBotY-.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  += _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  }
            else {
                  y = chordTopY - dy;
                  //
                  // check for collision with staff line
                  //
                  if (y >= staffTopY+.1) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  -= _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  }
            a->setPos(x, y);
            }

      //
      //    pass 2
      //    now place all articulations with staff top or bottom anchor
      //
      qreal dyTop = 0.0;
      qreal dyBot = 0.0;

      for (iArticulation ia = articulations.begin(); ia != articulations.end(); ++ia) {
            Articulation* a = *ia;
            qreal y = 0;
            ArticulationAnchor aa = a->anchor();
            if (aa == A_TOP_STAFF) {
                  y = staffTopY - dyTop;
                  a->setPos(x, y);
                  dyTop += (point(score()->styleS(ST_propertyDistance)) + a->bbox().height());
                  }
            else if (aa == A_BOTTOM_STAFF) {
                  y = staffBotY + dyBot;
                  a->setPos(x, y);
                  dyBot += (point(score()->styleS(ST_propertyDistance)) + a->bbox().height());
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

            case TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(e);
                  score()->tempomap->addTempo(tick(), tt->tempo());
                  tt->setTick(tick());
                  tt->setTrack(0);
                  tt->setParent(m);
                  score()->undoAddElement(tt);
                  }
                  break;

            case DYNAMIC:
                  {
                  e->setTick(tick());
                  e->setTrack(staffIdx() * VOICES);
                  e->setParent(m);
                  score()->undoAddElement(e);
                  }
                  break;

            default:
                  printf("cannot drop %s\n", e->name());
                  return 0;
            }
      return 0;
      }

//---------------------------------------------------------
//   setBeam
//---------------------------------------------------------

void ChordRest::setBeam(Beam* b)
      {
      _beam = b;
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void ChordRest::toDefault()
      {
      score()->undoChangeChordRestSpace(this, Spatium(0.0), Spatium(0.0));
      score()->undoChangeUserOffset(this, QPointF());
      }

