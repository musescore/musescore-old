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

#include "chordrest.h"
#include "chord.h"
#include "xml.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "tuplet.h"
#include "score.h"
#include "sym.h"
#include "slur.h"
#include "beam.h"
#include "breath.h"
#include "barline.h"
#include "articulation.h"
#include "tempo.h"
#include "tempotext.h"
#include "note.h"
#include "arpeggio.h"
#include "dynamic.h"
#include "stafftext.h"
#include "sig.h"
#include "clef.h"
#include "lyrics.h"
#include "segment.h"
#include "stafftype.h"
#include "undo.h"
#include "stem.h"
#include "page.h"

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
      _tuplet   = e._tuplet;
      _duration = e._duration;
      }

//---------------------------------------------------------
//   globalDuration
//---------------------------------------------------------

Fraction DurationElement::globalDuration() const
      {
      Fraction f(_duration);
      for (Tuplet* t = tuplet(); t; t = t->tuplet())
            f /= t->ratio();
      return f;
      }

//---------------------------------------------------------
//  actualTicks
//---------------------------------------------------------

int DurationElement::actualTicks() const
      {
      return Fraction(staff()->timeStretch(tick()) * globalDuration()).ticks();
      }

//---------------------------------------------------------
//   hasArticulation
//---------------------------------------------------------

Articulation* ChordRest::hasArticulation(const Articulation* aa)
      {
      int idx = aa->subtype();
      foreach(Articulation* a, articulations) {
            if (idx == a->subtype())
                  return a;
            }
      return 0;
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::ChordRest(Score* s)
   : DurationElement(s)
      {
      _beam      = 0;
      _small     = false;
      _beamMode  = BEAM_AUTO;
      _up        = true;
      _staffMove = 0;
      _tabDur    = 0;
      }

ChordRest::ChordRest(const ChordRest& cr)
   : DurationElement(cr)
      {
      _durationType = cr._durationType;
      _staffMove    = cr._staffMove;
      _tabDur       = 0;                  // tab sur. symb. depends upon context: can't be simply copied from another CR

      foreach(Articulation* a, cr.articulations) {    // make deep copy
            Articulation* na = new Articulation(*a);
            na->setParent(this);
            na->setTrack(track());
            articulations.append(na);
            }

      _beam               = 0;
      _beamMode           = cr._beamMode;
      _up                 = cr._up;
      _small              = cr._small;
      _extraLeadingSpace  = cr.extraLeadingSpace();
      _extraTrailingSpace = cr.extraTrailingSpace();
      _space              = cr._space;

      foreach(Lyrics* l, cr._lyricsList) {        // make deep copy
            Lyrics* nl = new Lyrics(*l);
            nl->setParent(this);
            nl->setTrack(track());
            _lyricsList.append(nl);
            }
      }

//---------------------------------------------------------
//   ChordRest
//---------------------------------------------------------

ChordRest::~ChordRest()
      {
      foreach(Articulation* a,  articulations)
            delete a;
      foreach(Lyrics* l, _lyricsList)
            delete l;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void ChordRest::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (_beam && (_beam->elements().front() == this))
            _beam->scanElements(data, func, all);
//    slur segments are collected from System:
//          foreach(Slur* slur, _slurFor)
//                slur->scanElements(data, func, all);
      foreach(Articulation* a, articulations)
            func(data, a);
      foreach(Lyrics* l, _lyricsList) {
            if (l)
                  l->scanElements(data, func, all);
            }
      if (_tabDur)
            func(data, _tabDur);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF ChordRest::pagePos() const
      {
      if (parent() == 0)
            return pos();
      System* system = measure()->system();
      if (system == 0)
            return QPointF();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF ChordRest::canvasPos() const
      {
      if (parent() == 0)
            return pos();
      System* system = measure()->system();
      if (system == 0 || system->page() == 0)
            return QPointF();
      qreal yp = y() + system->staff(staffIdx())->y() + system->y() + system->page()->y();
      return QPointF(pageX(), yp);
      }

//---------------------------------------------------------
//   properties
//---------------------------------------------------------

QList<Prop> ChordRest::properties(Xml& xml, bool /*clipboardmode*/) const
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
                  case BEAM_BEGIN64: s = "begin64"; break;
                  case BEAM_INVALID: s = "?"; break;
                  }
            pl.append(Prop("BeamMode", s));
            }
      if (tuplet())
            pl.append(Prop("Tuplet", tuplet()->id()));
      if (_small)
            pl.append(Prop("small", _small));
      if (_extraLeadingSpace.val() != 0.0)
            pl.append(Prop("leadingSpace", _extraLeadingSpace.val()));
      if (_extraTrailingSpace.val() != 0.0)
            pl.append(Prop("trailingSpace", _extraTrailingSpace.val()));
      if (durationType().dots())
            pl.append(Prop("dots", durationType().dots()));
      if (_staffMove)
            pl.append(Prop("move", _staffMove));
      if (durationType().isValid())
            pl.append(Prop("durationType", durationType().name()));
      return pl;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void ChordRest::writeProperties(Xml& xml) const
      {
      QList<Prop> pl = properties(xml);
      xml.prop(pl);
      if (!duration().isZero() && (!durationType().fraction().isValid()
         || (durationType().fraction() != duration())))
            xml.fTag("duration", duration());
      foreach(const Articulation* a, articulations)
            a->write(xml);
      foreach(Slur* s, _slurFor)
            xml.tagE(QString("Slur type=\"start\" number=\"%1\"").arg(s->id()+1));
      foreach(Slur* s, _slurBack)
            xml.tagE(QString("Slur type=\"stop\" number=\"%1\"").arg(s->id()+1));
      if (!xml.clipboardmode && _beam && !_beam->generated())
            xml.tag("Beam", _beam->id());
      foreach(Lyrics* lyrics, _lyricsList) {
            if (lyrics)
                  lyrics->write(xml);
            }
      Fraction t(globalDuration());
      if (staff())
            t *= staff()->timeStretch(xml.curTick);
      xml.curTick += t.ticks();
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ChordRest::readProperties(QDomElement e, const QList<Tuplet*>& tuplets, QList<Slur*>* slurs)
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
            else if (val == "begin64")
                  bm = BEAM_BEGIN64;
            else
                  bm = BeamMode(i);
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
            }
      else if (tag == "leadingSpace")
            _extraLeadingSpace = Spatium(val.toDouble());
      else if (tag == "trailingSpace")
            _extraTrailingSpace = Spatium(val.toDouble());
      else if (tag == "Beam") {
            Beam* beam = 0;
            foreach(Beam* b, score()->beams) {
                  if (b->id() == i) {
                        beam = b;
                        break;
                        }
                  }
            if (beam)
                  beam->add(this);        // also calls this->setBeam(beam)
            else
                  printf("Beam id %d not found\n", i);
            }
      else if (tag == "small")
            _small = i;
      else if (tag == "Slur") {
            int id = e.attribute("number").toInt();
            QString type = e.attribute("type");
            Slur* slur = 0;
            foreach(Slur* s, *slurs) {
                  if (s->id() == id) {
                        slur = s;
                        break;
                        }
                  }
            if (!slur) {
                  printf("ChordRest::read(): Slur id %d not found\n", id);
                  slur = new Slur(score());
                  slur->setId(id);
                  slurs->append(slur);
                  }
            if (type == "start") {
                  slur->setStartElement(this);
                  _slurFor.append(slur);
                  }
            else if (type == "stop") {
                  slur->setEndElement(this);
                  _slurBack.append(slur);
                  }
            else
                  printf("ChordRest::read(): unknown Slur type <%s>\n", qPrintable(type));
            }
      else if (tag == "durationType") {
            setDurationType(val);
            if (durationType().type() != Duration::V_MEASURE) {
                  if ((type() == REST) &&
                              // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                              duration().numerator() != 0 &&
                              // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                              (durationType()==Duration::V_WHOLE && duration() <= Fraction(4, 4)) ) {
                        // old pre 2.0 scores: convert
                        setDurationType(Duration::V_MEASURE);
                        }
                  else  // not from old score: set duration fraction from duration type
                        setDuration(durationType().fraction());
                  }
            else {
                  if (score()->mscVersion() < 115) {
                        SigEvent e = score()->sigmap()->timesig(score()->curTick);
                        setDuration(e.timesig());
                        }
                  }
            }
      else if (tag == "duration")
            setDuration(readFraction(e));
      else if (tag == "ticklen") {      // obsolete (version < 1.12)
            int mticks = score()->sigmap()->timesig(score()->curTick).timesig().ticks();
            if (i == 0)
                  i = mticks;
            // if ((type() == REST) && (mticks == i || (durationType()==Duration::V_WHOLE && mticks != 1920))) {
            if ((type() == REST) && (mticks == i)) {
                  setDurationType(Duration::V_MEASURE);
                  setDuration(Fraction::fromTicks(i));
                  }
            else {
                  Fraction f = Fraction::fromTicks(i);
                  setDuration(f);
                  setDurationType(Duration(f));
                  }
            }
      else if (tag == "dots")
            setDots(i);
      else if (tag == "move")
            _staffMove = i;
      else if (tag == "Lyrics") {
            Lyrics* lyrics = new Lyrics(score());
            lyrics->setTrack(score()->curTrack);
            lyrics->read(e);
            add(lyrics);
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
      _small   = val;
      qreal m = 1.0;
      if (_small)
            m = score()->styleD(ST_smallNoteMag);
      if (staff()->small())
            m *= score()->styleD(ST_smallStaffMag);
      setMag(m);
      }

//---------------------------------------------------------
//   layoutArticulations
//    called from chord()->layout()
//---------------------------------------------------------

void ChordRest::layoutArticulations()
      {
      if (parent() == 0 || articulations.isEmpty())
            return;
      qreal _spatium  = spatium();
      qreal x         = centerX();
      qreal distance0 = score()->styleS(ST_propertyDistance).val()     * _spatium;
      qreal distance1 = score()->styleS(ST_propertyDistanceHead).val() * _spatium;
      qreal distance2 = score()->styleS(ST_propertyDistanceStem).val() * _spatium;

      qreal chordTopY = upPos();    // note position of highest note
      qreal chordBotY = downPos();  // note position of lowest note

      qreal staffTopY = -distance2;
      qreal staffBotY = staff()->height() + distance2;

      // avoid collisions of staff articulations with chord notes:
      // gap between note and staff articulation is distance0 + 0.5 spatium

      if (type() == CHORD) {
            Chord* chord = static_cast<Chord*>(this);
            Stem* stem   = chord->stem();
            if (stem) {
                  qreal y = stem->pos().y() + pos().y();
                  if (up() && stem->stemLen() < 0.0)
                        y += stem->stemLen();
                  else if (!up() && stem->stemLen() > 0.0)
                        y -= stem->stemLen();
                  if (beam()) {
                        qreal bw = score()->styleS(ST_beamWidth).val() * _spatium;
                        y += up() ? -bw : bw;
                        }
                  if (up())
                        staffTopY = qMin(staffTopY, qreal(y - 0.5 * _spatium));
                  else
                        staffBotY = qMax(staffBotY, qreal(y + 0.5 * _spatium));
                  }
            }

      staffTopY = qMin(staffTopY, qreal(chordTopY - distance0 - 0.5 * _spatium));
      staffBotY = qMax(staffBotY, qreal(chordBotY + distance0 + 0.5 * _spatium));

      qreal dy = 0.0;

      foreach (Articulation* a, articulations) {
            if (a->direction() != AUTO) {
                  a->setUp(a->direction() == UP);
                  }
            else {
                  if (a->anchor() == A_CHORD)
                        a->setUp(!up());
                  else
                        a->setUp(a->anchor() == A_TOP_STAFF || a->anchor() == A_TOP_CHORD);
                  }
            }

      //
      //    pass 1
      //    place tenuto and staccato
      //

      foreach (Articulation* a, articulations) {
            a->layout();
            ArticulationAnchor aa = a->anchor();

            if ((a->subtype() != Articulation_Tenuto)
               && (a->subtype() != Articulation_Staccato))
                  continue;

            if (aa != A_CHORD && aa != A_TOP_CHORD && aa != A_BOTTOM_CHORD)
                  continue;

            // for tenuto and staccate check for staff line collision
            bool staffLineCT = a->subtype() == Articulation_Tenuto
                               || a->subtype() == Articulation_Staccato;

//            qreal sh = a->bbox().height() * mag();
            bool bottom = (aa == A_BOTTOM_CHORD) || (aa == A_CHORD && up());

            dy += distance1;
            if (bottom) {
                  qreal y = chordBotY + dy;
                  if (staffLineCT && (y <= staffBotY -.1 - dy)) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  += _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  a->setPos(x, y); // - a->bbox().y() + a->bbox().height() * .5);
                  }
            else {
                  qreal y = chordTopY - dy;
                  if (staffLineCT && (y >= (staffTopY +.1 + dy))) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  -= _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  a->setPos(x, y); // + a->bbox().y() - a->bbox().height() * .5);
                  }
            }

      // reserve space for slur
      bool botGap = false;
      bool topGap = false;
      foreach(Slur* s, _slurFor) {
            if (s->up())
                  topGap = true;
            else
                  botGap = true;
            }
      foreach(Slur* s, _slurBack) {
            if (s->up())
                  topGap = true;
            else
                  botGap = true;
            }
      if (botGap)
            chordBotY += _spatium;
      if (topGap)
            chordTopY -= _spatium;

      //
      //    pass 2
      //    place all articulations with anchor at chord/rest
      //
      foreach (Articulation* a, articulations) {
            a->layout();
            ArticulationAnchor aa = a->anchor();
            if ((a->subtype() == Articulation_Tenuto)
               || (a->subtype() == Articulation_Staccato))
                  continue;

            if (aa != A_CHORD && aa != A_TOP_CHORD && aa != A_BOTTOM_CHORD)
                  continue;

            // for tenuto and staccate check for staff line collision
            bool staffLineCT = a->subtype() == Articulation_Tenuto
                               || a->subtype() == Articulation_Staccato;

//            qreal sh = a->bbox().height() * mag();
            bool bottom = (aa == A_BOTTOM_CHORD) || (aa == A_CHORD && up());

            dy += distance1;
            if (bottom) {
                  qreal y = chordBotY + dy;
                  if (staffLineCT && (y <= staffBotY -.1 - dy)) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  += _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  a->setPos(x, y); // - a->bbox().y() + a->bbox().height() * .5);
                  }
            else {
                  qreal y = chordTopY - dy;
                  if (staffLineCT && (y >= (staffTopY +.1 + dy))) {
                        qreal l = y / _spatium;
                        qreal delta = fabs(l - round(l));
                        if (delta < 0.4) {
                              y  -= _spatium * .5;
                              dy += _spatium * .5;
                              }
                        }
                  a->setPos(x, y); // + a->bbox().y() - a->bbox().height() * .5);
                  }
            }

      //
      //    pass 3
      //    now place all articulations with staff top or bottom anchor
      //
      qreal dyTop = staffTopY;
      qreal dyBot = staffBotY;

/*      if ((upPos() - _spatium) < dyTop)
            dyTop = upPos() - _spatium;
      if ((downPos() + _spatium) > dyBot)
            dyBot = downPos() + _spatium;
  */
      foreach (Articulation* a, articulations) {
            ArticulationAnchor aa = a->anchor();
            if (aa == A_TOP_STAFF || aa == A_BOTTOM_STAFF) {
                  if (a->up()) {
                        a->setPos(x, dyTop);
                        dyTop -= distance0;
                        }
                  else {
                        a->setPos(x, dyBot);
                        dyBot += distance0;
                        }
                  }
            a->adjustReadPos();
            }
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
      if (!_slurFor.removeOne(s)) {
            printf("ChordRest<%p>::removeSlurFor(): %p not found\n", this, s);
            foreach(Slur* s, _slurFor)
                  printf("  %p\n", s);
            abort();
            }
      }

//---------------------------------------------------------
//   removeSlurBack
//---------------------------------------------------------

void ChordRest::removeSlurBack(Slur* s)
      {
      if (!_slurBack.removeOne(s)) {
            printf("ChordRest<%p>::removeSlurBack(): %p not found\n", this, s);
            foreach(Slur* s, _slurBack)
                  printf("  %p\n", s);
            }
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* ChordRest::drop(const DropData& data)
      {
      Element* e = data.element;
      Measure* m  = measure();
      switch (e->type()) {
            case BREATH:
                  {
                  Breath* b = static_cast<Breath*>(e);
                  b->setTrack(staffIdx() * VOICES);

                  // TODO: insert automatically in all staves?

                  Segment* seg = m->undoGetSegment(SegBreath, tick());
                  b->setParent(seg);
                  score()->undoAddElement(b);
                  }
                  break;

            case BAR_LINE:
                  {
                  BarLine* bl = static_cast<BarLine*>(e);
                  bl->setTrack(staffIdx() * VOICES);

//??                  if ((bl->tick() == m->tick()) || (bl->tick() == m->tick() + m->ticks())) {
//                        return m->drop(data);
//                        }
                  if (tick() == m->tick()) {
                        return m->drop(data);
                        }

                  Segment* seg = m->undoGetSegment(SegBarLine, tick());
                  bl->setParent(seg);
                  score()->undoAddElement(bl);
                  }
                  break;

            case CLEF:
                  score()->undoChangeClef(staff(), segment(), static_cast<Clef*>(e)->clefType());
                  delete e;
                  break;

            case TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(e);
                  tt->setParent(segment());
                  score()->undoAddElement(tt);
                  }
                  break;

            case DYNAMIC:
                  e->setTrack(track());
                  e->setParent(segment());
                  score()->undoAddElement(e);
                  return e;

            case NOTE:
                  {
                  Note* note = static_cast<Note*>(e);
                  NoteVal nval;
                  nval.pitch = note->pitch();
                  nval.headGroup = note->headGroup();
                  score()->setNoteRest(segment(), track(), nval, Fraction(1, 4), AUTO);
                  delete e;
                  }
                  break;

            case TEXT:
            case STAFF_TEXT:
            case HARMONY:
            case STAFF_STATE:
            case INSTRUMENT_CHANGE:
                  e->setParent(segment());
                  e->setTrack((track() / VOICES) * VOICES);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;
            default:
                  printf("cannot drop %s\n", e->name());
                  delete e;
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
      if (type() == CHORD) {
            score()->undo()->push(new SetStemDirection(static_cast<Chord*>(this), AUTO));
            score()->undo()->push(new ChangeBeamMode(this, BEAM_AUTO));
            }
      else {
            score()->undo()->push(new ChangeBeamMode(this, BEAM_NO));
            }
      }

//---------------------------------------------------------
//   setDurationType
//---------------------------------------------------------

void ChordRest::setDurationType(Duration::DurationType t)
      {
      _durationType.setType(t);
      }

void ChordRest::setDurationType(const QString& s)
      {
      _durationType.setType(s);
      }

void ChordRest::setDurationType(int ticks)
      {
      _durationType.setVal(ticks);
      }

void ChordRest::setDurationType(const Duration& v)
      {
      _durationType = v;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void ChordRest::setTrack(int val)
      {
      foreach(Articulation* a, articulations)
            a->setTrack(val);
      Element::setTrack(val);
      if (type() == CHORD) {
            foreach(Note* n, static_cast<Chord*>(this)->notes())
                  n->setTrack(val);
            }
      if (_beam)
            _beam->setTrack(val);
      foreach(Lyrics* l, _lyricsList)
            l->setTrack(val);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int ChordRest::tick() const
      {
      return segment() ? segment()->tick() : 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ChordRest::add(Element* e)
      {
      e->setParent(this);
      e->setTrack(track());
      switch(e->type()) {
            case ARTICULATION:
                  {
                  Articulation* a = static_cast<Articulation*>(e);
                  articulations.push_back(a);
                  if (a->timeStretch() > 0.0) {
                        qreal otempo = score()->tempo(tick());
                        qreal ntempo = otempo / a->timeStretch();
                        score()->setTempo(tick(), ntempo);
                        score()->setTempo(tick() + actualTicks(), otempo);
                        }
                  }
                  break;
            case LYRICS:
                  {
                  Lyrics* l = static_cast<Lyrics*>(e);
                  int size = _lyricsList.size();
                  if (l->no() >= size) {
                        for (int i = size-1; i < l->no(); ++i)
                              _lyricsList.append(0);
                        }
                  _lyricsList[l->no()] = l;
                  }
                  break;
            default:
                  printf("ChordRest::add: unknown element %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void ChordRest::remove(Element* e)
      {
      switch(e->type()) {
            case ARTICULATION:
                  {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (!articulations.removeOne(a))
                        printf("ChordRest::remove(): articulation not found\n");
                  if (a->timeStretch() > 0.0) {
                        score()->removeTempo(tick());
                        score()->removeTempo(tick() + actualTicks());
                        }
                  }
                  break;
            case LYRICS:
                  {
                  for (int i = 0; i < _lyricsList.size(); ++i) {
                        if (_lyricsList[i] != e)
                              continue;
                        _lyricsList[i] = 0;
                        // shrink list if possible ?
                        return;
                        }
                  }
                  printf("ChordRest::remove: %s %p not found\n", e->name(), e);
                  break;
            default:
                  printf("ChordRest::remove: unknown element %s\n", e->name());
                  break;
            }
      }

//---------------------------------------------------------
//   removeDeleteBeam
//    remove ChordRest from beam
//    delete beam if empty
//---------------------------------------------------------

void ChordRest::removeDeleteBeam()
      {
      if (_beam) {
            score()->deselect(_beam);
            Beam* b = _beam;
            b->remove(this);  // this sets _beam to zero
            if (b->isEmpty())
                  delete b;
            }
      }

