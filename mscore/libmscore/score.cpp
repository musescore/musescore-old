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

/**
 \file
 Implementation of class Score (partial).
*/

#include <assert.h>
#include "score.h"
#include "key.h"
#include "sig.h"
#include "clef.h"
#include "tempo.h"
#include "measure.h"
#include "page.h"
#include "undo.h"
#include "system.h"
#include "select.h"
#include "segment.h"
#include "xml.h"
#include "text.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "slur.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "tuplet.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "line.h"
#include "volta.h"
#include "event.h"
#include "repeat.h"
#include "ottava.h"
#include "barline.h"
#include "box.h"
#include "utils.h"
#include "excerpt.h"
#include "stafftext.h"
#include "repeatlist.h"
#include "keysig.h"
#include "beam.h"
#include "stafftype.h"
#include "tempotext.h"
#include "articulation.h"
#include "revisions.h"
#include "slurmap.h"
#include "tiemap.h"
#include "spannermap.h"
#include "layoutbreak.h"
#include "harmony.h"
#include "mscore.h"
#include "omr/omr.h"

Score* gscore;                 ///< system score, used for palettes etc.
QPoint scorePos(0,0);
QSize  scoreSize(950, 500);

bool layoutDebug     = false;
bool scriptDebug     = false;
bool noSeq           = false;
bool noMidi          = false;
bool midiInputTrace  = false;
bool midiOutputTrace = false;
bool showRubberBand  = true;

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

MeasureBaseList::MeasureBaseList()
      {
      _first = 0;
      _last  = 0;
      _size  = 0;
      };

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void MeasureBaseList::push_back(MeasureBase* e)
      {
      ++_size;
      if (_last) {
            _last->setNext(e);
            e->setPrev(_last);
            e->setNext(0);
            }
      else {
            _first = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _last = e;
      }

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void MeasureBaseList::push_front(MeasureBase* e)
      {
      ++_size;
      if (_first) {
            _first->setPrev(e);
            e->setNext(_first);
            e->setPrev(0);
            }
      else {
            _last = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _first = e;
      }

//---------------------------------------------------------
//   add
//    insert e before e->next()
//---------------------------------------------------------

void MeasureBaseList::add(MeasureBase* e)
      {
      MeasureBase* el = e->next();
      if (el == 0) {
            push_back(e);
            return;
            }
      if (el == _first) {
            push_front(e);
            return;
            }
      ++_size;
//      e->setNext(el);
      e->setPrev(el->prev());
      el->prev()->setNext(e);
      el->setPrev(e);
      }

//---------------------------------------------------------
//   erase
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* el)
      {
      --_size;
      if (el->prev())
            el->prev()->setNext(el->next());
      else
            _first = el->next();
      if (el->next())
            el->next()->setPrev(el->prev());
      else
            _last = el->prev();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void MeasureBaseList::insert(MeasureBase* fm, MeasureBase* lm)
      {
      ++_size;
      for (MeasureBase* m = fm; m != lm; m = m->next())
            ++_size;
      MeasureBase* pm = fm->prev();
      if (pm)
            pm->setNext(fm);
      else
            _first = fm;
      MeasureBase* nm = lm->next();
      if (nm)
            nm->setPrev(lm);
      else
            _last = lm;
      for (MeasureBase* mb = fm;;) {
            if (mb->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  SegmentTypes st = SegChordRest | SegGrace;
                  for (Segment* s = m->first(st); s; s = s->next(st)) {
                        foreach(Element* e, s->elist()) {
                              if (e) {
                                    ChordRest* cr = static_cast<ChordRest*>(e);
                                    foreach(Slur* s, cr->slurFor())
                                          s->setStartElement(cr);
                                    foreach(Slur* s, cr->slurBack())
                                          s->setEndElement(cr);
                                    }
                              }
                        }
                  }
            if (mb == lm)
                  break;
            mb = mb->next();
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* fm, MeasureBase* lm)
      {
      --_size;
      for (MeasureBase* m = fm; m != lm; m = m->next())
            --_size;
      MeasureBase* pm = fm->prev();
      MeasureBase* nm = lm->next();
      if (pm)
            pm->setNext(nm);
      else
            _first = nm;
      if (nm)
            nm->setPrev(pm);
      else
            _last = pm;
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void MeasureBaseList::change(MeasureBase* ob, MeasureBase* nb)
      {
      nb->setPrev(ob->prev());
      nb->setNext(ob->next());
      if (ob->prev())
            ob->prev()->setNext(nb);
      if (ob->next())
            ob->next()->setPrev(nb);
      if (ob == _last)
            _last = nb;
      if (ob == _first)
            _first = nb;
      if (nb->type() == HBOX || nb->type() == VBOX || nb->type() == TBOX || nb->type() == FBOX)
            nb->setSystem(ob->system());
      foreach(Element* e, *nb->el())
            e->setParent(nb);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Score::init()
      {
      _parentScore    = 0;
      _currentLayer   = 0;
      Layer l;
      l.name          = "default";
      l.tags          = 1;
      _layer.append(l);
      _layerTags[0]   = "default";

      _creationDate   = QDate::currentDate();
      _revisions      = new Revisions;
      _symIdx         = 0;
      _pageNumberOffset = 0;
      startLayout     = 0;
      _undo           = new UndoStack();
      _repeatList     = new RepeatList(this);
      foreach(StaffType* st, ::staffTypes)
             _staffTypes.append(st->clone());
      _swingRatio     = 0.0;

      _mscVersion     = MSCVERSION;
      _created        = false;

      _updateAll      = true;
      _layoutAll      = true;
      layoutFlags     = 0;
      _playNote       = false;
      _excerptsChanged = false;
      _instrumentsChanged = false;
      _selectionChanged   = false;

      keyState        = 0;
      _showInvisible  = true;
      _showUnprintable = true;
      _showFrames     = true;
      _showPageborders = false;
      _printing       = false;
      _playlistDirty  = false;
      _autosaveDirty  = false;
      _dirty          = false;
      _saved          = false;
      _playPos        = 0;
      _fileDivision   = MScore::division;
      _creditsRead    = false;
      _defaultsRead   = false;
      _omr            = 0;
      _showOmr        = false;
      _sigmap         = 0; // new TimeSigMap();
      _tempomap       = 0; // new TempoMap;
      _playRepeats    = true;
      _layoutMode     = LayoutPage;
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score(const Style* s)
   : _selection(this)
      {
      init();
      _tempomap  = new TempoMap;
      _sigmap    = new TimeSigMap();
      _style = *s;
      }

//
//  a linked score shares some properties with parentScore():
//    _undo
//    _sigmap
//    _tempomap
//    _repeatList
//    _links
//    _staffTypes
//
Score::Score(Score* parent)
   : _selection(this)
      {
      init();
      _parentScore    = parent;
      _undo           = 0;
      _repeatList     = 0;

      _style = *parent->style();
      if (!MScore::partStyle.isEmpty()) {
            QFile f(MScore::partStyle);
            if (f.open(QIODevice::ReadOnly))
                  _style.load(&f);
            }
      _syntiState     = parent->_syntiState;
      }

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
      {
      foreach(MuseScoreView* v, viewer)
            v->removeScore();
      deselectAll();
      for (MeasureBase* m = _measures.first(); m;) {
            MeasureBase* nm = m->next();
            delete m;
            m = nm;
            }
      foreach(Part* p, _parts)
            delete p;
      foreach(Staff* staff, _staves)
            delete staff;
      foreach(System* s, _systems)
            delete s;
      foreach(Page* page, _pages)
            delete page;
      foreach(Excerpt* e, _excerpts)
            delete e;
      delete _revisions;
      delete _undo;           // this also removes _undoStack from Mscore::_undoGroup
      delete _tempomap;
      delete _sigmap;
      delete _repeatList;
      foreach(StaffType* st, _staffTypes)
            delete st;
      }

//---------------------------------------------------------
//   renumberMeasures
//---------------------------------------------------------

void Score::renumberMeasures()
      {
      int measureNo = 0;
      for (Measure* measure = firstMeasure(); measure; measure = measure->nextMeasure()) {
            measureNo += measure->noOffset();
            measure->setNo(measureNo);
            if (measure->sectionBreak() && measure->sectionBreak()->startWithMeasureOne())
                  measureNo = 0;
            else if (measure->irregular())      // dont count measure
                  ;
            else
                  ++measureNo;
            }
      }

//---------------------------------------------------------
//   elementAdjustReadPos
//---------------------------------------------------------

static void elementAdjustReadPos(void*, Element* e)
      {
      if (e->isMovable())
            e->adjustReadPos();
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Part* Score::part(int n)
      {
      int idx = 0;
      foreach (Part* part, _parts) {
            idx += part->nstaves();
            if (n < idx)
                  return part;
            }
      return 0;
      }

//---------------------------------------------------------
//   addMeasure
//---------------------------------------------------------

void Score::addMeasure(MeasureBase* m, MeasureBase* pos)
      {
      // if (!m->next())
      m->setNext(pos);
      _measures.add(m);
      addLayoutFlags(LAYOUT_FIX_TICKS);
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Score::insertTime(int /*tick*/, int /*len*/)
      {
#if 0
      if (len < 0) {
            //
            // remove time
            //
            len = -len;
            tempomap()->removeTime(tick, len);
            foreach(Staff* staff, _staves)
                  staff->keymap()->removeTime(tick, len);
            }
      else {
            //
            // insert time
            //
            tempomap()->insertTime(tick, len);
            foreach(Staff* staff, _staves)
                  staff->keymap()->insertTime(tick, len);
            }
      addLayoutFlags(LAYOUT_FIX_TICKS);
#endif
      }

//---------------------------------------------------------
//    fixTicks
//    update:
//      - measure ticks
//      - tempo map
//      - time signature map
//      - measure numbers
//---------------------------------------------------------

/**
 This is needed after
      - inserting or removing a measure.
      - changing the sigmap
      - after inserting/deleting time (changes the sigmap)
*/

void Score::fixTicks()
      {
      int number = 0;
      int tick   = 0;
      Measure* fm = firstMeasure();
      if (fm == 0)
            return;

      Fraction sig(fm->timesig());
      if (!parentScore()) {
            tempomap()->clear();
            sigmap()->clear();
            sigmap()->add(0, SigEvent(sig,  number));
            }

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE) {
                  mb->setTick(tick);
                  continue;
                  }
            Measure* m = static_cast<Measure*>(mb);
            if (!parentScore()) {
                  for (Segment* s = m->first(SegChordRest); s; s = s->next(SegChordRest)) {
                        foreach(Element* e, s->annotations()) {
                              if (e->type() == TEMPO_TEXT) {
                                    const TempoText* tt = static_cast<const TempoText*>(e);
                                    setTempo(tt->segment(), tt->tempo());
                                    }
                              }
                        }
                  }

            //
            // fix ticks
            //
            int mtick = m->tick();
            int diff  = tick - mtick;
            int measureTicks = m->ticks();
            tick += measureTicks;
            m->moveTicks(diff);

            if (!parentScore()) {
                  //
                  //  implement section break rest
                  //
                  if (m->sectionBreak())
                        setPause(m->tick() + m->ticks(), m->pause());

                  //
                  // implement fermata as a tempo change
                  //
                  SegmentTypes st = SegChordRest | SegBreath;

                  for (Segment* s = m->first(st); s; s = s->next(st)) {
                        if (s->subtype() == SegBreath) {
                              setPause(s->tick(), .1);
                              continue;
                              }
                        qreal stretch = -1.0;
                        foreach(Element* e, s->elist()) {
                              if (!e)
                                    continue;
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              foreach(Articulation* a, *cr->getArticulations())
                                    stretch = qMax(a->timeStretch(), stretch);
                              if (stretch > 0.0) {
                                    qreal otempo = tempomap()->tempo(cr->tick());
                                    qreal ntempo = otempo / stretch;
                                    setTempo(cr->tick(), ntempo);
                                    setTempo(cr->tick() + cr->actualTicks(), otempo);
                                    break;
                                    }
                              }
                        }
                  }

            //
            // calculate measure number
            //
            number += m->noOffset();
            if (m->no() != number)
                  m->setNo(number);
            if (m->sectionBreak())
                  number = 0;
            else if (m->irregular())      // dont count measure
                  ;
            else
                  ++number;

            //
            // update time signature map
            //
            if (!parentScore() && (m->timesig() != sig)) {
                  sig = m->timesig();
                  sigmap()->add(tick, SigEvent(sig,  number));
                  }
            }
      if (tempomap()->empty())
            tempomap()->setTempo(0, 2.0);
      }

//---------------------------------------------------------
//   pos2measure
//---------------------------------------------------------

/**
 Return measure for canvas relative position \a p.
*/

MeasureBase* Score::pos2measure(const QPointF& p, int* rst, int* pitch,
   Segment** seg, QPointF* offset) const
      {
      Measure* m = searchMeasure(p);
      if (m == 0)
            return 0;

      System* s = m->system();
//      qreal sy1 = 0;
      qreal y   = p.y() - s->canvasPos().y();

      int i;
      for (i = 0; i < nstaves();) {
            SysStaff* staff = s->staff(i);
            if (!staff->show()) {
                  ++i;
                  continue;
                  }
            int ni = i;
            for (;;) {
                  ++ni;
                  if (ni == nstaves() || s->staff(ni)->show())
                        break;
                  }

            qreal sy2;
            if (ni != nstaves()) {
                  SysStaff* nstaff = s->staff(ni);
                  qreal s1y2 = staff->bbox().y() + staff->bbox().height();
                  sy2 = s1y2 + (nstaff->bbox().y() - s1y2)/2;
                  }
            else
                  sy2 = s->page()->height() - s->pos().y();   // s->height();
            if (y > sy2) {
//                  sy1 = sy2;
                  i   = ni;
                  continue;
                  }
            break;
            }

      // search for segment + offset
      QPointF pppp = p - m->pagePos();
      int track = i * VOICES;

      SysStaff* sstaff = m->system()->staff(i);
      for (Segment* segment = m->first(); segment; segment = segment->next()) {
            if (segment->subtype() != SegChordRest)
                  continue;
            if ((segment->element(track) == 0)
               && (segment->element(track+1) == 0)
               && (segment->element(track+2) == 0)
               && (segment->element(track+3) == 0)
               )
                  continue;
            Segment* ns = segment->next();
            for (; ns; ns = ns->next()) {
                  if (ns->subtype() != SegChordRest)
                        continue;
                  if (ns->element(track)
                     || ns->element(track+1)
                     || ns->element(track+2)
                     || ns->element(track+3))
                        break;
                  }
            if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                  *rst = i;
                  if (pitch) {
                        Staff* s = _staves[i];
                        int clef = s->clef(segment->tick());
                        *pitch = y2pitch(pppp.y() - sstaff->bbox().y(), clef, s->spatium());
                        }
                  if (offset)
                        *offset = pppp - QPointF(segment->x(), sstaff->bbox().y());
                  if (seg)
                        *seg = segment;
                  return m;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   staffIdx
//
///  Return index for the first staff of \a part.
//---------------------------------------------------------

int Score::staffIdx(const Part* part) const
      {
      int idx = 0;
      foreach(Part* p, _parts) {
            if (p == part)
                  break;
            idx += p->nstaves();
            }
      return idx;
      }

//---------------------------------------------------------
//   setShowInvisible
//---------------------------------------------------------

void Score::setShowInvisible(bool v)
      {
      _showInvisible = v;
      _updateAll     = true;
      end();
      }

//---------------------------------------------------------
//   setShowUnprintable
//---------------------------------------------------------

void Score::setShowUnprintable(bool v)
      {
      _showUnprintable = v;
      _updateAll      = true;
      end();
      }

//---------------------------------------------------------
//   setShowFrames
//---------------------------------------------------------

void Score::setShowFrames(bool v)
      {
      _showFrames = v;
      _updateAll  = true;
      end();
      }

//---------------------------------------------------------
//   setShowPageborders
//---------------------------------------------------------

void Score::setShowPageborders(bool v)
      {
      _showPageborders = v;
      _updateAll = true;
      end();
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void Score::setDirty(bool val)
      {
      if (_dirty != val) {
            _dirty         = val;
            _playlistDirty = true;
            }
      if (_dirty) {
            _playlistDirty = true;
            _autosaveDirty = true;
            }
      }

//---------------------------------------------------------
//   playlistDirty
//---------------------------------------------------------

bool Score::playlistDirty()
      {
      bool val = _playlistDirty;
      _playlistDirty = false;
      return val;
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell()
      {
      for (int i = 0; i < nstaves(); ++i) {
            QList<Note*> notes;
            for (Segment* s = firstSegment(); s; s = s->next1()) {
                  int strack = i * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (e && e->type() == CHORD)
                              notes.append(static_cast<Chord*>(e)->notes());
                        }
                  }
            spellNotelist(notes);
            }
      }

void Score::spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment)
      {
      for (int i = startStaff; i < endStaff; ++i) {
            QList<Note*> notes;
            for (Segment* s = startSegment; s && s != endSegment; s = s->next()) {
                  int strack = i * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (e && e->type() == CHORD)
                              notes.append(static_cast<Chord*>(e)->notes());
                        }
                  }
            spellNotelist(notes);
            }
      }

//---------------------------------------------------------
//   prevNote
//---------------------------------------------------------

Note* prevNote(Note* n)
      {
      Chord* chord = n->chord();
      Segment* seg = chord->segment();
      const QList<Note*> nl = chord->notes();
      int i = nl.indexOf(n);
      if (i)
            return nl[i-1];
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() - 1;
      int endTrack   = 0;
      while (seg) {
            if (seg->subtype() == SegChordRest) {
                  for (int track = startTrack; track >= endTrack; --track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == CHORD)
                              return static_cast<Chord*>(e)->upNote();
                        }
                  }
            seg = seg->prev1();
            startTrack = staff * VOICES + VOICES - 1;
            }
      return n;
      }

//---------------------------------------------------------
//   nextNote
//---------------------------------------------------------

Note* nextNote(Note* n)
      {
      Chord* chord = n->chord();
      const QList<Note*> nl = chord->notes();
      int i = nl.indexOf(n);
      ++i;
      if (i < nl.size())
            return nl[i];
      Segment* seg   = chord->segment();
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() + 1;
      int endTrack   = staff * VOICES + VOICES;
      while (seg) {
            if (seg->subtype() == SegChordRest) {
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == CHORD) {
                              return ((Chord*)e)->downNote();
                              }
                        }
                  }
            seg = seg->next1();
            startTrack = staff * VOICES;
            }
      return n;
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spell(Note* note)
      {
      QList<Note*> notes;

      notes.append(note);
      Note* nn = nextNote(note);
      notes.append(nn);
      nn = nextNote(nn);
      notes.append(nn);
      nn = nextNote(nn);
      notes.append(nn);

      nn = prevNote(note);
      notes.prepend(nn);
      nn = prevNote(nn);
      notes.prepend(nn);
      nn = prevNote(nn);
      notes.prepend(nn);

      int opt = ::computeWindow(notes, 0, 7);
      note->setTpc(::tpc(3, note->pitch(), opt));
      }

//---------------------------------------------------------
//   isSavable
//---------------------------------------------------------

bool Score::isSavable() const
      {
      // TODO: check if file can be created if it does not exist
      return info.isWritable() || !info.exists();
      }

//---------------------------------------------------------
//   setInputTrack
//---------------------------------------------------------

void Score::setInputTrack(int v)
      {
      if (v < 0) {
            printf("setInputTrack: bad value: %d\n", v);
            return;
            }
      _is.setTrack(v);
      }

//---------------------------------------------------------
//   setLayout
//---------------------------------------------------------

void Score::setLayout(Measure* m)
      {
      if (m)
            m->setDirty();
      if (startLayout && startLayout != m)
            setLayoutAll(true);
      else
            startLayout = m;
      }

//---------------------------------------------------------
//   appendPart
//---------------------------------------------------------

void Score::appendPart(Part* p)
      {
      _parts.append(p);
      }

//---------------------------------------------------------
//   rebuildMidiMapping
//---------------------------------------------------------

void Score::rebuildMidiMapping()
      {
      _midiMapping.clear();
      int port    = 0;
      int channel = 0;
      int idx     = 0;
      foreach(Part* part, _parts) {
            InstrumentList* il = part->instrList();
            for (iInstrument i = il->begin(); i != il->end(); ++i) {
                  bool drum = i->second.useDrumset();
                  for (int k = 0; k < i->second.channel().size(); ++k) {
                        Channel* a = &(i->second.channel(k));
                        MidiMapping mm;
                        if (drum) {
                              mm.port    = port;
                              mm.channel = 9;
                              }
                        else {
                              mm.port    = port;
                              mm.channel = channel;
                              if (channel == 15) {
                                    channel = 0;
                                    ++port;
                                    }
                              else {
                                    ++channel;
                                    if (channel == 9)
                                          ++channel;
                                    }
                              }
                        mm.part         = part;
                        mm.articulation = a;
                        _midiMapping.append(mm);
                        a->channel = idx;
                        ++idx;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   midiPort
//---------------------------------------------------------

int Score::midiPort(int idx) const
      {
      return _midiMapping[idx].port;
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Score::midiChannel(int idx) const
      {
      return _midiMapping[idx].channel;
      }

//---------------------------------------------------------
//   searchPage
//    p is in canvas coordinates
//---------------------------------------------------------

Page* Score::searchPage(const QPointF& p) const
      {
      foreach(Page* page, pages()) {
            if (page->bbox().translated(page->pos()).contains(p))
                  return page;
            }
      return 0;
      }

//---------------------------------------------------------
//   searchSystem
//    return list of systems as there may be more than
//    one system in a row
//    p is in canvas coordinates
//---------------------------------------------------------

QList<System*> Score::searchSystem(const QPointF& pos) const
      {
      QList<System*> systems;
      Page* page = searchPage(pos);
      if (page == 0)
            return systems;
      qreal y = pos.y() - page->pos().y();  // transform to page relative
      const QList<System*>* sl = page->systems();
      qreal y2;
      int n = sl->size();
      for (int i = 0; i < n; ++i) {
            System* s = sl->at(i);
            System* ns = 0;               // next system row
            int ii = i + 1;
            for (; ii < n; ++ii) {
                  ns = sl->at(ii);
                  if (ns->y() != s->y())
                        break;
                  }
            if ((ii == n) || (ns == 0))
                  y2 = page->height();
            else  {
                  qreal sy2 = s->y() + s->bbox().height();
                  y2         = sy2 + (ns->y() - sy2) * .5;
                  }
            if (y < y2) {
                  systems.append(s);
                  for (int ii = i+1; ii < n; ++ii) {
                        if (sl->at(ii)->y() != s->y())
                              break;
                        systems.append(sl->at(ii));
                        }
                  return systems;
                  }
            }
      return systems;
      }

//---------------------------------------------------------
//   searchMeasure
//    p is in canvas coordinates
//---------------------------------------------------------

Measure* Score::searchMeasure(const QPointF& p) const
      {
      QList<System*> systems = searchSystem(p);
      if (systems.isEmpty())
            return 0;

      foreach(System* system, systems) {
            qreal x = p.x() - system->canvasPos().x();
            foreach(MeasureBase* mb, system->measures()) {
                  if (mb->type() != MEASURE)
                        continue;
                  if (x < (mb->x() + mb->bbox().width()))
                        return static_cast<Measure*>(mb);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//    getNextValidInputSegment
//    - s is of type SegChordRest
//---------------------------------------------------------

static Segment* getNextValidInputSegment(Segment* s, int track, int voice)
      {
      if (s == 0)
            return 0;
      assert(s->subtype() == SegChordRest);
      // Segment* s1 = s;
      ChordRest* cr1;
      for (Segment* s1 = s; s1; s1 = s1->prev(SegChordRest)) {
            cr1 = static_cast<ChordRest*>(s1->element(track + voice));
            if (cr1)
                  break;
            }
      int nextTick = (cr1 == 0) ? s->measure()->tick() : cr1->tick() + cr1->actualTicks();

      static const SegmentTypes st = SegChordRest;
      while (s) {
            if (s->element(track + voice))
                  break;
            if (voice && s->tick() == nextTick)
                  return s;
#if 0
            int v;
            for (v = 0; v < VOICES; ++v) {
                  if (s->element(track + v))
                        break;
                  }
            if ((v != VOICES) && voice) {
                  int ntick;
                  bool skipChord = false;
                  bool ns        = false;
                  for (Segment* s1 = s->measure()->first(st); s1; s1 = s1->next(st)) {
                        ChordRest* cr = static_cast<ChordRest*>(s1->element(track + voice));
                        if (cr) {
                              if (ns)
                                    return s1;
                              ntick = s1->tick() + cr->actualTicks();
                              skipChord = true;
                              }
                        if (s1 == s)
                              ns = true;
                        if (skipChord) {
                              if (s->tick() >= ntick)
                                    skipChord = false;
                              }
                        if (!skipChord && ns)
                              return s1;
                        }
                  if (!skipChord)
                        return s;
                  }
#endif
            s = s->next(st);
            }
      return s;
      }

//---------------------------------------------------------
//   getPosition
//    return true if valid position found
//---------------------------------------------------------

bool Score::getPosition(Position* pos, const QPointF& p, int voice) const
      {
      pos->measure = searchMeasure(p);
      if (pos->measure == 0)
            return false;

      //
      //    search staff
      //
//      qreal sy1         = 0;
      pos->staffIdx      = 0;
      SysStaff* sstaff   = 0;
      System* system     = pos->measure->system();
      qreal y           = p.y() - system->pagePos().y();
      for (; pos->staffIdx < nstaves(); ++pos->staffIdx) {
            qreal sy2;
            SysStaff* ss = system->staff(pos->staffIdx);
            if ((pos->staffIdx+1) != nstaves()) {
                  SysStaff* nstaff = system->staff(pos->staffIdx+1);
                  qreal s1y2 = ss->bbox().y() + ss->bbox().height();
                  sy2         = s1y2 + (nstaff->bbox().y() - s1y2) * .5;
                  }
            else
                  sy2 = system->page()->height() - system->pos().y();   // system->height();
            if (y < sy2) {
                  sstaff = ss;
                  break;
                  }
//            sy1 = sy2;
            }
      if (sstaff == 0)
            return false;

      //
      //    search segment
      //
      QPointF pppp(p - pos->measure->canvasPos());
      qreal x         = pppp.x();
      Segment* segment = 0;
      pos->segment     = 0;

      // int track = pos->staffIdx * VOICES + voice;
      int track = pos->staffIdx * VOICES;

      for (segment = pos->measure->first(SegChordRest); segment;) {
            segment = getNextValidInputSegment(segment, track, voice);
            if (segment == 0)
                  break;
            Segment* ns = getNextValidInputSegment(segment->next(SegChordRest), track, voice);

            qreal x1 = segment->x();
            qreal x2;
            qreal d;
            if (ns) {
                  x2    = ns->x();
                  d     = x2 - x1;
                  }
            else {
                  x2    = pos->measure->bbox().width();
                  d     = (x2 - x1) * 2.0;
                  x     = x1;
                  pos->segment = segment;
                  break;
                  }

            if (x < (x1 + d * .5)) {
                  x = x1;
                  pos->segment = segment;
                  break;
                  }
            segment = ns;
            }
      if (segment == 0)
            return false;
      //
      // TODO: restrict to reasonable values (pitch 0-127)
      //
      Staff* s    = staff(pos->staffIdx);
      qreal mag = staff(pos->staffIdx)->mag();
      qreal lineDist = (s->useTablature() ? 1.5 * spatium() : spatium() * .5) * mag;

      pos->line  = lrint((pppp.y() - sstaff->bbox().y()) / lineDist);
      if (s->useTablature()) {
            if (pos->line < -1 || pos->line > s->lines()+1)
                  return false;
            if (pos->line < 0)
                  pos->line = 0;
            else if (pos->line >= s->lines())
                  pos->line = s->lines() - 1;
            }
      else {
            int minLine = pitch2line(0);
            int clef    = s->clef(pos->segment->tick());
            minLine     = 127 - minLine - 82 + clefTable[clef].yOffset;
            int maxLine = pitch2line(127);
            maxLine     = 127 - maxLine - 82 + clefTable[clef].yOffset;

            if (pos->line > minLine || pos->line < maxLine)
                  return false;
            }

      y         = sstaff->y() + pos->line * lineDist;
      pos->pos  = QPointF(x, y) + pos->measure->canvasPos();
      return true;
      }

//---------------------------------------------------------
//   checkHasMeasures
//---------------------------------------------------------

bool Score::checkHasMeasures() const
      {
      Page* page = pages().front();
      const QList<System*>* sl = page->systems();
      if (sl == 0 || sl->empty() || sl->front()->measures().empty()) {
            printf("first create measure, then repeat operation\n");
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   ImagePath
//---------------------------------------------------------

ImagePath::ImagePath(const QString& p)
   : _path(p), _references(0), _loaded(false)
      {
      }

//---------------------------------------------------------
//   addImage
//---------------------------------------------------------

ImagePath* Score::addImage(const QString& s)
      {
      foreach(ImagePath* p, imagePathList) {
            if (p->path() == s)
                  return p;
            }
      ImagePath* p = new ImagePath(s);
      imagePathList.append(p);
      return p;
      }

//---------------------------------------------------------
//   dereference
//    decrement usage count of image in score
//---------------------------------------------------------

void ImagePath::dereference()
      {
      --_references;
      }

//---------------------------------------------------------
//   reference
//    increment usage count of image in score
//---------------------------------------------------------

void ImagePath::reference()
      {
      ++_references;
      }

//---------------------------------------------------------
//   moveBracket
//    columns are counted from right to left
//---------------------------------------------------------

void Score::moveBracket(int staffIdx, int srcCol, int dstCol)
      {
      foreach(System* system, *systems()) {
            if (system->isVbox())
                  continue;
            SysStaff* s = system->staff(staffIdx);
            for (int i = s->brackets.size(); i <= dstCol; ++i)
                  s->brackets.append(0);

            s->brackets[dstCol] = s->brackets[srcCol];
            s->brackets[srcCol] = 0;

            while (!s->brackets.isEmpty() && (s->brackets.last() == 0))
                  s->brackets.removeLast();
            }
      }

//---------------------------------------------------------
//   spatiumHasChanged
//---------------------------------------------------------

static void spatiumHasChanged(void* data, Element* e)
      {
      qreal* val = (qreal*)data;
      e->spatiumChanged(val[0], val[1]);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Score::spatiumChanged(qreal oldValue, qreal newValue)
      {
      qreal data[2];
      data[0] = oldValue;
      data[1] = newValue;
      scanElements(data, spatiumHasChanged, true);
      }

//---------------------------------------------------------
//   getCreateMeasure
//    - return Measure for tick
//    - create new Measure(s) if there is no measure for
//      this tick
//---------------------------------------------------------

Measure* Score::getCreateMeasure(int tick)
      {
      Measure* last = lastMeasure();
      if (last == 0 || ((last->tick() + last->ticks()) <= tick)) {
            int lastTick  = last ? (last->tick()+last->ticks()) : 0;
            while (tick >= lastTick) {
                  Measure* m = new Measure(this);
                  Fraction ts = _sigmap->timesig(lastTick).timesig();
// printf("getCreateMeasure %d  %d/%d\n", tick, ts.numerator(), ts.denominator());
                  m->setTick(lastTick);
                  m->setTimesig(ts);
                  m->setLen(ts);
                  add(m);
                  lastTick += ts.ticks();
                  }
            }
      return tick2measure(tick);
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 Add \a element to its parent.

 Several elements (clef, keysig, timesig) need special handling, as they may cause
 changes throughout the score.
*/

void Score::addElement(Element* element)
      {
      if (debugMode) {
            printf("   Score(%p)::addElement %p(%s) parent %p(%s)\n",
               this, element, element->name(), element->parent(),
               element->parent() ? element->parent()->name() : "");
            }
      ElementType et = element->type();
      if (et == TREMOLO) {
            Chord* chord = static_cast<Chord*>(element->parent());
            setLayout(chord->measure());
            }

      else if (et == MEASURE
         || (et == HBOX && element->parent()->type() != VBOX)
         || et == VBOX
         || et == TBOX
         || et == FBOX
         ) {
            add(element);
            addLayoutFlags(LAYOUT_FIX_TICKS);
            return;
            }

      if (element->parent() == 0)
            add(element);
      else
            element->parent()->add(element);

      switch(et) {
            case VOLTA:
            case TRILL:
            case PEDAL:
            case TEXTLINE:
//            case HAIRPIN:
//            case TIE:
                  {
                  Spanner* spanner = static_cast<Spanner*>(element);
                  foreach(SpannerSegment* ss, spanner->spannerSegments()) {
                        Q_ASSERT(ss->spanner() == spanner);
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
                  break;

            case SLUR:
                  {
                  Slur* slur = static_cast<Slur*>(element);
                  foreach(SpannerSegment* ss, slur->spannerSegments()) {
                        Q_ASSERT(ss->spanner() == slur);
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  if (slur->startElement())
                        static_cast<ChordRest*>(slur->startElement())->addSlurFor(slur);
                  if (slur->endElement())
                        static_cast<ChordRest*>(slur->endElement())->addSlurBack(slur);
                  }
                  break;

            case OTTAVA:
                  {
                  Ottava* o = static_cast<Ottava*>(element);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  Staff* s  = o->staff();
                  if (o->startElement()) {
                        int tick = static_cast<Segment*>(o->startElement())->tick();
                        s->pitchOffsets().setPitchOffset(tick, o->pitchShift());
                        }
                  if (o->endElement()) {
                        int tick = static_cast<Segment*>(o->endElement())->tick();
                        s->pitchOffsets().setPitchOffset(tick, 0);
                        }
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  }
                  break;

            case DYNAMIC:
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;
            case CLEF:
                  {
                  Clef* clef       = static_cast<Clef*>(element);
                  Segment* segment = clef->segment();
                  // Staff* staff     = clef->staff();
                  // staff->setClef(segment->tick(), clef->clefTypeList());
                  updateNoteLines(segment, clef->track());
                  }
                  break;
            case KEYSIG:
                  {
                  KeySig* ks = static_cast<KeySig*>(element);
                  Staff*  staff = element->staff();
                  KeySigEvent keySigEvent = ks->keySigEvent();
                  staff->setKey(ks->segment()->tick(), keySigEvent);
                  }
                  break;
            case TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(element);
                  setTempo(tt->segment(), tt->tempo());
                  }
                  break;
            case INSTRUMENT_CHANGE:
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
                  break;

            default:
                  break;
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   removeElement
///   Remove \a element from its parent.
///   Several elements (clef, keysig, timesig) need special handling, as they may cause
///   changes throughout the score.
//---------------------------------------------------------

void Score::removeElement(Element* element)
      {
      Element* parent = element->parent();

      if (debugMode) {
            printf("   Score(%p)::removeElement %p(%s) parent %p(%s)\n",
               this, element, element->name(), parent, parent ? parent->name() : "");
            }

      // special for MEASURE, HBOX, VBOX
      // their parent is not static

      ElementType et = element->type();
      if (et == TREMOLO) {
            Chord* chord = static_cast<Chord*>(element->parent());
            setLayout(chord->measure());
            }
      else if (et == MEASURE
         || (et == HBOX && parent->type() != VBOX)
         || et == VBOX
         || et == TBOX
         || et == FBOX
            ) {
            remove(element);
            addLayoutFlags(LAYOUT_FIX_TICKS);
            return;
            }

      if (et == BEAM)          // beam parent does not survive layout
            element->setParent(0);

      if (parent)
            parent->remove(element);
      else
            remove(element);

      switch(et) {
            case SLUR:
                  {
                  Slur* slur = static_cast<Slur*>(element);
                  foreach(SpannerSegment* ss, slur->spannerSegments()) {
                        Q_ASSERT(ss->spanner() == slur);
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  static_cast<ChordRest*>(slur->startElement())->removeSlurFor(slur);
                  static_cast<ChordRest*>(slur->endElement())->removeSlurBack(slur);
                  }
                  break;

            case VOLTA:
            case TRILL:
            case PEDAL:
            case TEXTLINE:
//            case HAIRPIN:
//            case TIE:
                  {
                  Spanner* spanner = static_cast<Spanner*>(element);
                  foreach(SpannerSegment* ss, spanner->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  }
                  break;

            case OTTAVA:
                  {
                  Ottava* o = static_cast<Ottava*>(element);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  Staff* s = o->staff();
                  int tick1 = static_cast<Segment*>(o->startElement())->tick();
                  int tick2 = static_cast<Segment*>(o->endElement())->tick();
                  s->pitchOffsets().remove(tick1);
                  s->pitchOffsets().remove(tick2);
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  }
                  break;

            case DYNAMIC:
                  layoutFlags |= LAYOUT_FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;

            case CHORD:
            case REST:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(element);
                  if (cr->beam())
                        cr->beam()->remove(cr);
                  // cr->setBeam(0);
                  }
                  break;
            case CLEF:
                  {
                  Clef* clef = static_cast<Clef*>(element);
                  updateNoteLines(clef->segment(), clef->track());
                  }
                  break;
            case KEYSIG:
                  {
                  KeySig* ks    = static_cast<KeySig*>(element);
                  Staff*  staff = element->staff();
                  staff->removeKey(ks->segment()->tick());
                  }
                  break;
            case TEMPO_TEXT:
                  {
                  TempoText* tt = static_cast<TempoText*>(element);
                  int tick = tt->segment()->tick();
                  tempomap()->delTempo(tick);
                  }
                  break;
            case INSTRUMENT_CHANGE:
                  rebuildMidiMapping();
                  _instrumentsChanged = true;
                  break;
            default:
                  break;
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   firstMeasure
//---------------------------------------------------------

Measure* Score::firstMeasure() const
      {
      MeasureBase* mb = _measures.first();
      while (mb && mb->type() != MEASURE)
            mb = mb->next();
      return static_cast<Measure*>(mb);
      }

//---------------------------------------------------------
//   lastMeasure
//---------------------------------------------------------

Measure* Score::lastMeasure() const
      {
      MeasureBase* mb = _measures.last();
      while (mb && mb->type() != MEASURE)
            mb = mb->prev();
      return static_cast<Measure*>(mb);
      }

//---------------------------------------------------------
//   firstSegment
//---------------------------------------------------------

Segment* Score::firstSegment(SegmentTypes segType) const
      {
      Measure* m = firstMeasure();
      return m ? m->first(segType) : 0;
      }

//---------------------------------------------------------
//   lastSegment
//---------------------------------------------------------

Segment* Score::lastSegment() const
      {
      Measure* m = lastMeasure();
      return m ? m->last() : 0;
      }

//---------------------------------------------------------
//   utick2utime
//---------------------------------------------------------

qreal Score::utick2utime(int tick) const
      {
      return repeatList()->utick2utime(tick);
      }

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int Score::utime2utick(qreal utime) const
      {
      return repeatList()->utime2utick(utime);
      }

//---------------------------------------------------------
//   inputPos
//---------------------------------------------------------

int Score::inputPos() const
      {
      return _is.tick();
      }

//---------------------------------------------------------
//   scanElements
//    scan all elements
//---------------------------------------------------------

void Score::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      for(MeasureBase* m = first(); m; m = m->next())
            m->scanElements(data, func, all);
      foreach(Page* page, pages())
            page->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   customKeySigIdx
//    try to find custom key signature in table,
//    return index or -1 if not found
//---------------------------------------------------------

int Score::customKeySigIdx(KeySig* ks) const
      {
      int idx = 0;
      foreach(KeySig* k, customKeysigs) {
            if (*k == *ks)
                  return idx;
            ++idx;
            }
      printf("  not found\n");
      return -1;
      }

//---------------------------------------------------------
//   addCustomKeySig
//---------------------------------------------------------

int Score::addCustomKeySig(KeySig* ks)
      {
      customKeysigs.append(ks);
      int idx = customKeysigs.size() - 1;
      KeySigEvent k = ks->keySigEvent();
      k.setCustomType(idx);
      ks->setKeySigEvent(k);
      ks->setScore(this);
      return idx;
      }

//---------------------------------------------------------
//   customKeySig
//---------------------------------------------------------

KeySig* Score::customKeySig(int idx) const
      {
      return customKeysigs[idx];
      }

//---------------------------------------------------------
//   keySigFactory
//---------------------------------------------------------

KeySig* Score::keySigFactory(const KeySigEvent& e)
      {
      KeySig* ks;
      if (!e.isValid())
            return 0;
      if (e.custom()) {
            ks = new KeySig(*customKeysigs[e.customType()]);
            }
      else {
            ks = new KeySig(this);
            ks->setKeySigEvent(e);
            }
      return ks;
      }

//---------------------------------------------------------
//   setSelection
//---------------------------------------------------------

void Score::setSelection(const Selection& s)
      {
      deselectAll();
      _selection = s;
      foreach(Element* e, _selection.elements())
            e->setSelected(true);
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

Text* Score::getText(int subtype)
      {
      MeasureBase* m = measures()->first();
      if (m) {
            foreach(Element* e, *m->el()) {
                  if (e->type() == TEXT && e->subtype() == subtype)
                        return static_cast<Text*>(e);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   rootScore
//---------------------------------------------------------

Score* Score::rootScore()
      {
      Score* score = this;
      while (score->parentScore())
            score = parentScore();
      return score;
      }

const Score* Score::rootScore() const
      {
      const Score* score = this;
      while (score->parentScore())
            score = parentScore();
      return score;
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

UndoStack* Score::undo() const
      {
      return rootScore()->_undo;
      }

//---------------------------------------------------------
//   repeatList
//---------------------------------------------------------

RepeatList* Score::repeatList()  const
      {
      return rootScore()->_repeatList;
      }

//---------------------------------------------------------
//   links
//---------------------------------------------------------

QHash<int, LinkedElements*>& Score::links()
      {
      return rootScore()->_elinks;
      }

//---------------------------------------------------------
//   tempomap
//---------------------------------------------------------

TempoMap* Score::tempomap() const
      {
      return rootScore()->_tempomap;
      }

//---------------------------------------------------------
//   sigmap
//---------------------------------------------------------

TimeSigMap* Score::sigmap() const
      {
      return rootScore()->_sigmap;
      }

//---------------------------------------------------------
//   staffTypes
//---------------------------------------------------------

const QList<StaffType*>& Score::staffTypes() const
      {
      return rootScore()->_staffTypes;
      }

QList<StaffType*>& Score::staffTypes()
      {
      return rootScore()->_staffTypes;
      }

//---------------------------------------------------------
//   setStaffTypes
//---------------------------------------------------------

void Score::addStaffTypes(const QList<StaffType*>& tl)
      {
      Score* score = rootScore();
      foreach(StaffType* st, tl)
            score->_staffTypes.append(st->clone());
      }

//---------------------------------------------------------
//   replaceStaffTypes
//---------------------------------------------------------

void Score::replaceStaffTypes(const QList<StaffType*>& tl)
      {
      rootScore()->_staffTypes = tl;
      }

//---------------------------------------------------------
//   addExcerpt
//---------------------------------------------------------

void Score::addExcerpt(Score* score)
      {
      score->setParentScore(this);
      Excerpt* ex = new Excerpt(score);
      excerpts()->append(ex);
      ex->setTitle(score->name());
      foreach(Staff* s, score->staves()) {
            LinkedStaves* ls = s->linkedStaves();
            if (ls == 0)
                  continue;
            foreach(Staff* ps, ls->staves()) {
                  if (ps->score() == this) {
                        ex->parts()->append(ps->part());
                        break;
                        }
                  }
            }
      setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   removeExcerpt
//---------------------------------------------------------

void Score::removeExcerpt(Score* score)
      {
      foreach (Excerpt* ex, *excerpts()) {
            if (ex->score() == score) {
                  if (excerpts()->removeOne(ex)) {
                        delete ex;
                        return;
                        }
                  else
                        printf("removeExcerpt:: ex not found\n");
                  }
            }
      printf("Score::removeExcerpt: excerpt not found\n");
      }

//---------------------------------------------------------
//   findSpanner
//---------------------------------------------------------

Spanner* Score::findSpanner(int id) const
      {
      static const SegmentTypes st = SegChordRest;
      for (Segment* s = firstMeasure()->first(st); s; s = s->next1(st)) {
            foreach(Spanner* e, s->spannerFor()) {
                  if (e->id() == id)
                        return e;
                  }
            }
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Spanner* e, m->spannerFor()) {
                  if (e->id() == id)
                        return e;
                  }
            }
      printf("Score::findSpanner() id %d not found\n", id);
      return 0;
      }

//---------------------------------------------------------
//   updateNotes
///   calculate note lines and accidental
//---------------------------------------------------------

void Score::updateNotes()
      {
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  AccidentalState tversatz;      // state of already set accidentals for this measure
                  tversatz.init(staff(staffIdx)->keymap()->key(m->tick()));

                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        if (!(segment->subtype() & (SegChordRest | SegGrace)))
                              continue;
                        m->layoutChords10(segment, staffIdx * VOICES, &tversatz);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdUpdateNotes
///   calculate note lines and accidental
//---------------------------------------------------------

void Score::cmdUpdateNotes()
      {
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
                  updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   updateAccidentals
//---------------------------------------------------------

void Score::updateAccidentals(Measure* m, int staffIdx)
      {
// printf("updateAccidentals measure %d staff %d\n", m->no(), staffIdx);
      Staff* st = staff(staffIdx);
      AccidentalState tversatz;      // list of already set accidentals for this measure
      tversatz.init(st->keymap()->key(m->tick()));

      for (Segment* segment = m->first(); segment; segment = segment->next()) {
            if (segment->subtype() & (SegChordRest | SegGrace))
                  m->updateAccidentals(segment, staffIdx, &tversatz);
            }
      }

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

Score* Score::clone()
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.header();

      xml.stag("museScore version=\"" MSC_VERSION "\"");
      write(xml, false);
      xml.etag();

      buffer.close();

      QDomDocument doc;
      int line, column;
      QString err;
// printf("buffer <%s>\n", buffer.buffer().data());
      if (!doc.setContent(buffer.buffer(), &err, &line, &column)) {
            printf("error cloning score %d/%d: %s\n<%s>\n",
               line, column, err.toLatin1().data(), buffer.buffer().data());
            return 0;
            }
      Score* score = new Score(style());
      docName = "--";
      score->read1(doc.documentElement());

      score->renumberMeasures();

      int staffIdx = 0;
      foreach(Staff* st, score->staves()) {
            if (st->updateKeymap())
                  st->keymap()->clear();
            int track = staffIdx * VOICES;
            KeySig* key1 = 0;
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->generated())
                              continue;
                        if ((s->subtype() == SegKeySig) && st->updateKeymap()) {
                              KeySig* ks = static_cast<KeySig*>(e);
                              int naturals = key1 ? key1->keySigEvent().accidentalType() : 0;
                              ks->setOldSig(naturals);
                              st->setKey(s->tick(), ks->keySigEvent());
                              key1 = ks;
                              }
                        }
                  if (m->sectionBreak())
                        key1 = 0;
                  }
            st->setUpdateKeymap(false);
            ++staffIdx;
            }
      score->updateNotes();
      score->addLayoutFlags(LAYOUT_FIX_TICKS | LAYOUT_FIX_PITCH_VELO);
      score->doLayout();
      score->scanElements(0, elementAdjustReadPos);
      return score;
      }

//---------------------------------------------------------
//   setSyntiSettings
//---------------------------------------------------------

void Score::setSyntiState(const SyntiState& s)
      {
      if (!(_syntiState == s)) {
            // _dirty = true;       // DEBUG: conflicts with setting of default sound font
            _syntiState = s;
            }
      }

//---------------------------------------------------------
//   setLayoutAll
//---------------------------------------------------------

void Score::setLayoutAll(bool val)
      {
      Score* score = this;
      while (score->parentScore())
            score = parentScore();
      foreach(Excerpt* excerpt, score->_excerpts)
            excerpt->score()->_layoutAll = val;
      score->_layoutAll = val;
      }

//---------------------------------------------------------
//   removeOmr
//---------------------------------------------------------

void Score::removeOmr()
      {
      _showOmr = false;
      delete _omr;
      _omr = 0;
      }

//---------------------------------------------------------
//   appendScore
//---------------------------------------------------------

bool Score::appendScore(Score* score)
      {
      int tracks       = score->nstaves() * VOICES;
      SlurMap* slurMap = new SlurMap[tracks];
      TieMap  tieMap;
      SpannerMap spannerMap;

      MeasureBaseList* ml = &score->_measures;
      for (MeasureBase* mb = ml->first(); mb; mb = mb->next()) {
            MeasureBase* nmb;
            if (mb->type() == MEASURE)
                  nmb = static_cast<Measure*>(mb)->cloneMeasure(this, slurMap, &tieMap, &spannerMap);
            else
                  nmb = mb->clone();
            nmb->setNext(0);
            nmb->setPrev(0);
            nmb->setScore(this);
            _measures.add(nmb);
            }
      fixTicks();
      renumberMeasures();
      delete[] slurMap;
      return true;
      }

//---------------------------------------------------------
//   splitStaff
//---------------------------------------------------------

void Score::splitStaff(int staffIdx, int splitPoint)
      {
      printf("split staff %d point %d\n", staffIdx, splitPoint);

      //
      // create second staff
      //
      Staff* s  = staff(staffIdx);
      Part*  p  = s->part();
      int rstaff = s->rstaff();
      Staff* ns = new Staff(this, p, rstaff + 1);
      ns->setRstaff(rstaff + 1);
//      ns->setClef(0, CLEF_F);

      undoInsertStaff(ns, staffIdx+1);

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            m->cmdAddStaves(staffIdx+1, staffIdx+2, false);

//      undoChangeBarLineSpan(s, p->nstaves());
      adjustBracketsIns(staffIdx+1, staffIdx+2);
      undoChangeKeySig(ns, 0, s->key(0));

//      Bracket* b = new Bracket(this);
//      b->setSubtype(BRACKET_AKKOLADE);
//      b->setTrack(staffIdx * VOICES);
//      b->setParent(firstMeasure()->system());
//      b->setLevel(-1);  // add bracket
//      b->setSpan(2);
//      cmdAdd(b);

      rebuildMidiMapping();
      _instrumentsChanged = true;
      startLayout = 0;
      doLayout();

      //
      // move notes
      //
      select(0, SELECT_SINGLE, 0);
      int strack = staffIdx * VOICES;
      int dtrack = (staffIdx + 1) * VOICES;

      for (Segment* s = firstSegment(SegChordRest); s; s = s->next1(SegChordRest)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(strack + voice));
                  if (cr == 0 || cr->type() == REST)
                        continue;
                  Chord* c = static_cast<Chord*>(cr);
                  QList<Note*> removeNotes;
                  foreach(Note* note, c->notes()) {
                        if (note->pitch() >= splitPoint)
                              continue;
                        Chord* chord = static_cast<Chord*>(s->element(dtrack + voice));
                        if (chord && (chord->type() != CHORD))
                              abort();
                        if (chord == 0) {
                              chord = new Chord(*c);
                              foreach(Note* note, chord->notes())
                                    delete note;
                              chord->notes().clear();
                              chord->setTrack(dtrack + voice);
                              undoAddElement(chord);
                              }
                        Note* nnote = new Note(*note);
                        nnote->setTrack(dtrack + voice);
                        chord->add(nnote);
                        removeNotes.append(note);
                        }
                  foreach(Note* note, removeNotes) {
                        undoRemoveElement(note);
                        if (note->chord()->notes().isEmpty())
                              undoRemoveElement(note->chord());
                        }
                  }
            }
      //
      // make sure that the timeline for dtrack
      // has no gaps
      //
      int ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegChordRest); s; s = s->next1(SegChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(dtrack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                        // insert Rest
                        Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              printf("no segment at %d\n", ctick);
                              continue;
                              }
                        setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->actualTicks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
      //
      // same for strack
      //
      ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegChordRest); s; s = s->next1(SegChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(strack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                        // insert Rest
                        Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              printf("no segment at %d\n", ctick);
                              continue;
                              }
                        setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->actualTicks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
      }

//---------------------------------------------------------
//   cmdInsertPart
//    insert before staffIdx
//---------------------------------------------------------

void Score::cmdInsertPart(Part* part, int staffIdx)
      {
      undoInsertPart(part, staffIdx);

      int sidx = this->staffIdx(part);
      int eidx = sidx + part->nstaves();
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            m->cmdAddStaves(sidx, eidx, true);
      adjustBracketsIns(sidx, eidx);
      }

//---------------------------------------------------------
//   cmdRemovePart
//---------------------------------------------------------

void Score::cmdRemovePart(Part* part)
      {
      int sidx   = staffIdx(part);
      int n      = part->nstaves();
      int eidx   = sidx + n;

// printf("cmdRemovePart %d-%d\n", sidx, eidx);

      //
      //    adjust measures
      //
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            m->cmdRemoveStaves(sidx, eidx);

      for (int i = 0; i < n; ++i)
            cmdRemoveStaff(sidx);
      undoRemovePart(part, sidx);
      }

//---------------------------------------------------------
//   insertPart
//---------------------------------------------------------

void Score::insertPart(Part* part, int idx)
      {
      int staff = 0;
      for (QList<Part*>::iterator i = _parts.begin(); i != _parts.end(); ++i) {
            if (staff >= idx) {
                  _parts.insert(i, part);
                  return;
                  }
            staff += (*i)->nstaves();
            }
      _parts.push_back(part);
      }

//---------------------------------------------------------
//   removePart
//---------------------------------------------------------

void Score::removePart(Part* part)
      {
      _parts.removeAt(_parts.indexOf(part));
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Score::insertStaff(Staff* staff, int idx)
      {
      _staves.insert(idx, staff);
      staff->part()->insertStaff(staff);
      }

//---------------------------------------------------------
//   adjustBracketsDel
//---------------------------------------------------------

void Score::adjustBracketsDel(int sidx, int eidx)
      {
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            for (int i = 0; i < staff->bracketLevels(); ++i) {
                  int span = staff->bracketSpan(i);
                  if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx))
                        continue;
                  if ((sidx >= staffIdx) && (eidx <= (staffIdx + span)))
                        undoChangeBracketSpan(staff, i, span - (eidx-sidx));
//                  else {
//                        printf("TODO: adjust brackets, span %d\n", span);
//                        }
                  }
            int span = staff->barLineSpan();
            if ((sidx >= staffIdx) && (eidx <= (staffIdx + span)))
                  undoChangeBarLineSpan(staff, span - (eidx-sidx));
            }
      }

//---------------------------------------------------------
//   adjustBracketsIns
//---------------------------------------------------------

void Score::adjustBracketsIns(int sidx, int eidx)
      {
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            int bl = staff->bracketLevels();
            for (int i = 0; i < bl; ++i) {
                  int span = staff->bracketSpan(i);
                  if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx))
                        continue;
                  if ((sidx >= staffIdx) && (eidx < (staffIdx + span)))
                        undoChangeBracketSpan(staff, i, span + (eidx-sidx));
//                  else {
//                        printf("TODO: adjust brackets\n");
//                        }
                  }
            int span = staff->barLineSpan();
            if ((sidx >= staffIdx) && (eidx < (staffIdx + span)))
                  undoChangeBarLineSpan(staff, span + (eidx-sidx));
            }
      }

//---------------------------------------------------------
//   cmdRemoveStaff
//---------------------------------------------------------

void Score::cmdRemoveStaff(int staffIdx)
      {
      adjustBracketsDel(staffIdx, staffIdx+1);
      Staff* s = staff(staffIdx);
      undoRemoveStaff(s, staffIdx);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Score::removeStaff(Staff* staff)
      {
      _staves.removeAll(staff);
      staff->part()->removeStaff(staff);
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Score::sortStaves(QList<int>& dst)
      {
      systems()->clear();  //??
      _parts.clear();
      Part* curPart = 0;
      QList<Staff*> dl;
      foreach(int idx, dst) {
            Staff* staff = _staves[idx];
            if (staff->part() != curPart) {
                  curPart = staff->part();
                  curPart->staves()->clear();
                  _parts.push_back(curPart);
                  }
            curPart->staves()->push_back(staff);
            dl.push_back(staff);
            }
      _staves = dl;

      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = static_cast<Measure*>(mb);
            m->sortStaves(dst);
            }
      }

//---------------------------------------------------------
//   keydiff2Interval
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------

static Interval keydiff2Interval(int oKey, int nKey, TransposeDirection dir)
      {
      static int stepTable[15] = {
            // C  G  D  A  E  B Fis
               0, 4, 1, 5, 2, 6, 3,
            };

      int cofSteps;     // circle of fifth steps
      int diatonic;
      if (nKey > oKey)
            cofSteps = nKey - oKey;
      else
            cofSteps = 12 - (oKey - nKey);
      diatonic = stepTable[(nKey + 7) % 7] - stepTable[(oKey + 7) % 7];
      if (diatonic < 0)
            diatonic += 7;
      diatonic %= 7;
      int chromatic = (cofSteps * 7) % 12;


      if ((dir == TRANSPOSE_CLOSEST) && (chromatic > 6))
            dir = TRANSPOSE_DOWN;

      if (dir == TRANSPOSE_DOWN) {
            chromatic = chromatic - 12;
            diatonic  = diatonic - 7;
            if (diatonic == -7)
                  diatonic = 0;
            if (chromatic == -12)
                  chromatic = 0;
            }
printf("TransposeByKey %d -> %d   chromatic %d diatonic %d\n", oKey, nKey, chromatic, diatonic);
      return Interval(diatonic, chromatic);
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose(int mode, TransposeDirection direction, int transposeKey, int transposeInterval,
   bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats)
      {
      bool rangeSelection = selection().state() == SEL_RANGE;
      int startStaffIdx = 0;
      int startTick     = 0;
      if (rangeSelection) {
            startStaffIdx = selection().staffStart();
            startTick     = selection().tickStart();
            }
      KeyList* km = staff(startStaffIdx)->keymap();
//      int key     = km->key(startTick).accidentalType();

      Interval interval;
      if (mode == TRANSPOSE_BY_KEY) {
            // calculate interval from "transpose by key"
            km       = staff(startStaffIdx)->keymap();
            int oKey = km->key(startTick).accidentalType();
            interval = keydiff2Interval(oKey, transposeKey, direction);
            }
      else {
            interval = intervalList[transposeInterval];
            if (direction == TRANSPOSE_DOWN)
                  interval.flip();
            }

      if (!rangeSelection)
            trKeys = false;
      bool fullOctave = (interval.chromatic % 12) == 0;
      if (fullOctave && (mode != TRANSPOSE_BY_KEY)) {
            trKeys = false;
            transposeChordNames = false;
            }

      if (_selection.state() == SEL_LIST) {
            foreach(Element* e, _selection.elements()) {
                  if (e->staff()->staffType()->group() == PERCUSSION_STAFF)
                        continue;
                  if (e->type() == NOTE)
                        transpose(static_cast<Note*>(e), interval, useDoubleSharpsFlats);
                  else if ((e->type() == HARMONY) && transposeChordNames) {
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  else if ((e->type() == KEYSIG) && trKeys) {
                        KeySig* ks = static_cast<KeySig*>(e);
                        KeySigEvent key  = km->key(ks->tick());
                        KeySigEvent okey = km->key(ks->tick() - 1);
                        key.setNaturalType(okey.accidentalType());
                        undo()->push(new ChangeKeySig(ks, key, ks->showCourtesySig(),
                           ks->showNaturals()));
                        }
                  }
            return;
            }

      int startTrack = _selection.staffStart() * VOICES;
      int endTrack   = _selection.staffEnd() * VOICES;

      for (Segment* segment = _selection.startSegment(); segment && segment != _selection.endSegment(); segment = segment->next1()) {
            for (int st = startTrack; st < endTrack; ++st) {
                  if (staff(st/VOICES)->staffType()->group() == PERCUSSION_STAFF)
                        continue;
                  Element* e = segment->element(st);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  QList<Note*> nl = chord->notes();
                  foreach (Note* n, nl)
                        transpose(n, interval, useDoubleSharpsFlats);
                  }
            if (transposeChordNames) {
                  foreach (Element* e, segment->annotations()) {
                        if ((e->type() != HARMONY) || (e->track() < startTrack) || (e->track() >= endTrack))
                              continue;
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  }
            }

      if (trKeys) {
            transposeKeys(_selection.staffStart(), _selection.staffEnd(),
               _selection.tickStart(), _selection.tickEnd(), interval.chromatic);
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   transposeStaff
//---------------------------------------------------------

void Score::cmdTransposeStaff(int staffIdx, Interval interval, bool useDoubleSharpsFlats)
      {
      if (staff(staffIdx)->staffType()->group() == PERCUSSION_STAFF)
            return;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      transposeKeys(staffIdx, staffIdx+1, 0, lastSegment()->tick(), interval.chromatic);

      for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
           for (int st = startTrack; st < endTrack; ++st) {
                  Element* e = segment->element(st);
                  if (!e || e->type() != CHORD)
                      continue;

                  Chord* chord = static_cast<Chord*>(e);
                  QList<Note*> nl = chord->notes();
                  foreach(Note* n, nl)
                      transpose(n, interval, useDoubleSharpsFlats);
                  }
            }

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach (Element* e, *m->el()) {
                  if (e->type() != HARMONY)
                        continue;
                  if (e->track() >= startTrack && e->track() < endTrack) {
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag, bool useDoubleSharpsFlats)
      {
      undo()->push(new ChangeConcertPitch(this, flag));

      foreach(Staff* staff, _staves) {
            if (staff->staffType()->group() == PERCUSSION_STAFF)
                  continue;
            Instrument* instr = staff->part()->instr();
            Interval interval = instr->transpose();
            if (interval.isZero())
                  continue;
            if (!flag)
                  interval.flip();
            cmdTransposeStaff(staff->idx(), interval, useDoubleSharpsFlats);
            }
      for (Segment* s = firstMeasure()->first(SegClef); s; s = s->next1(SegClef)) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Clef* clef = static_cast<Clef*>(s->element(staffIdx * VOICES));
                  if (!clef)
                        continue;
                  clef->setClefType(flag ? clef->concertClef() : clef->transposingClef());
                  }
            }
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose(Note* n, Interval interval, bool useDoubleSharpsFlats)
      {
      int npitch;
      int ntpc;
      transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc, interval,
        useDoubleSharpsFlats);
      undoChangePitch(n, npitch, ntpc, n->line(), n->fret(), n->string());
      }

//---------------------------------------------------------
//   transposeKeys
//    key -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Score::transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, int /*semitones*/)
      {
      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            if (staff(staffIdx)->staffType()->group() == PERCUSSION_STAFF)
                  continue;
            for (Segment* s = firstSegment(); s; s = s->next1()) {
                  if (s->subtype() != SegKeySig)
                        continue;
                  if (s->tick() < tickStart)
                        continue;
                  if (s->tick() >= tickEnd)
                        break;
                  KeySig* ks = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                  if (ks) {
                        KeyList* km      = staff(staffIdx)->keymap();
                        KeySigEvent key  = km->key(s->tick());
                        KeySigEvent okey = km->key(s->tick() - 1);
                        key.setNaturalType(okey.accidentalType());
                        undo()->push(new ChangeKeySig(ks, key, ks->showCourtesySig(),
                           ks->showNaturals()));
                        }
                  }
            }
      }


//---------------------------------------------------------
//   addAudioTrack
//---------------------------------------------------------

void Score::addAudioTrack()
      {
      // TODO
      }

//---------------------------------------------------------
//   padToggle
//---------------------------------------------------------

void Score::padToggle(int n)
      {
      switch (n) {
            case PAD_NOTE00:
                  _is.setDuration(Duration::V_LONG);
                  break;
            case PAD_NOTE0:
                  _is.setDuration(Duration::V_BREVE);
                  break;
            case PAD_NOTE1:
                  _is.setDuration(Duration::V_WHOLE);
                  break;
            case PAD_NOTE2:
                  _is.setDuration(Duration::V_HALF);
                  break;
            case PAD_NOTE4:
                  _is.setDuration(Duration::V_QUARTER);
                  break;
            case PAD_NOTE8:
                  _is.setDuration(Duration::V_EIGHT);
                  break;
            case PAD_NOTE16:
                  _is.setDuration(Duration::V_16TH);
                  break;
            case PAD_NOTE32:
                  _is.setDuration(Duration::V_32ND);
                  break;
            case PAD_NOTE64:
                  _is.setDuration(Duration::V_64TH);
                  break;
            case PAD_NOTE128:
                  _is.setDuration(Duration::V_128TH);
                  break;
            case PAD_REST:
                  _is.rest = !_is.rest;
                  break;
            case PAD_DOT:
                  if (_is.duration().dots() == 1)
                        _is.setDots(0);
                  else
                        _is.setDots(1);
                  break;
            case PAD_DOTDOT:
                  if (_is.duration().dots() == 2)
                        _is.setDots(0);
                  else
                        _is.setDots(2);
                  break;
            }
      if (n >= PAD_NOTE00 && n <= PAD_NOTE128) {
            _is.setDots(0);
            //
            // if in "note enter" mode, reset
            // rest flag
            //
            if (noteEntryMode())
                  _is.rest = false;
            }

      if (noteEntryMode() || !selection().isSingle()) {
            return;
            }

      //do not allow to add a dot on a full measure rest
      Element* e = selection().element();
      if (e && e->type() == REST) {
            Rest* r = static_cast<Rest*>(e);
            Duration d = r->durationType();
            if (d.type() == Duration::V_MEASURE) {
                  _is.setDots(0);
                  // return;
                  }
            }

      Element* el = selection().element();
      if (el->type() == NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      if (cr->type() == CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            _undo->push(new ChangeDurationType(cr, _is.duration()));
            }
      else
            changeCRlen(cr, _is.duration());
      }

//---------------------------------------------------------
//   setInputState
//---------------------------------------------------------

void Score::setInputState(Element* e)
      {
// printf("setInputState %s\n", e ? e->name() : "--");

      if (e == 0)
            return;
      if (e && e->type() == CHORD)
            e = static_cast<Chord*>(e)->upNote();

      _is.setDrumNote(-1);
//      _is.setDrumset(0);
      if (e->type() == NOTE) {
            Note* note    = static_cast<Note*>(e);
            Chord* chord  = note->chord();
            _is.setDuration(chord->durationType());
            _is.rest      = false;
            _is.setTrack(note->track());
            _is.pitch     = note->pitch();
            _is.noteType  = note->noteType();
            _is.beamMode  = chord->beamMode();
            }
      else if (e->type() == REST) {
            Rest* rest   = static_cast<Rest*>(e);
            if (rest->durationType().type() == Duration::V_MEASURE)
                  _is.setDuration(Duration::V_QUARTER);
            else
                  _is.setDuration(rest->durationType());
            _is.rest     = true;
            _is.setTrack(rest->track());
            _is.beamMode = rest->beamMode();
            _is.noteType = NOTE_NORMAL;
            }
      else {
/*            _is.rest     = false;
            _is.setDots(0);
            _is.setDuration(Duration::V_INVALID);
            _is.noteType = NOTE_INVALID;
            _is.beamMode = BEAM_INVALID;
            _is.noteType = NOTE_NORMAL;
*/
            }
      if (e->type() == NOTE || e->type() == REST) {
            const Instrument* instr   = e->staff()->part()->instr();
            if (instr->useDrumset()) {
                  if (e->type() == NOTE)
                        _is.setDrumNote(static_cast<Note*>(e)->pitch());
                  else
                        _is.setDrumNote(-1);
//                  _is.setDrumset(instr->drumset());
                  }
            }
//      mscore->updateInputState(this);
      }

//---------------------------------------------------------
//   deselect
//---------------------------------------------------------

void Score::deselect(Element* el)
      {
      refresh |= el->abbox();
      _selection.remove(el);
      }

//---------------------------------------------------------
//   select
//    staffIdx is valid, if element is of type MEASURE
//---------------------------------------------------------

void Score::select(Element* e, SelectType type, int staffIdx)
      {
      if (e && (e->type() == NOTE || e->type() == REST)) {
            Element* ee = e;
            if (ee->type() == NOTE)
                  ee = ee->parent();
            setPlayPos(static_cast<ChordRest*>(ee)->segment()->tick());
            }
      if (debugMode)
            printf("select element <%s> type %d(state %d) staff %d\n",
               e ? e->name() : "", type, selection().state(), e ? e->staffIdx() : -1);

      SelState selState = _selection.state();

      if (type == SELECT_SINGLE) {
            deselectAll();
            if (e == 0) {
                  selState = SEL_NONE;
                  _updateAll = true;
                  }
            else {
                  if (e->type() == MEASURE) {
                        select(e, SELECT_RANGE, staffIdx);
                        return;
                        }
                  refresh |= e->abbox();
                  _selection.add(e);
                  _is.setTrack(e->track());
                  selState = SEL_LIST;
                  if (e->type() == NOTE || e->type() == REST || e->type() == CHORD) {
                        if (e->type() == NOTE)
                              e = e->parent();
                        _is.setSegment(static_cast<ChordRest*>(e)->segment());
                        }
                  }
            _selection.setActiveSegment(0);
            _selection.setActiveTrack(0);
            }
      else if (type == SELECT_ADD) {
            if (e->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  int etick = tick + m->ticks();
                  if (_selection.state() == SEL_NONE) {
                        _selection.setStartSegment(m->tick2segment(tick, true));
                        _selection.setEndSegment(tick2segment(etick));
                        }
                  else {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  _updateAll = true;
                  selState = SEL_RANGE;
                  _selection.setStaffStart(0);
                  _selection.setStaffEnd(nstaves());
                  _selection.updateSelectedElements();
                  }
            else {
                  if (_selection.state() == SEL_RANGE) {
                        select(0, SELECT_SINGLE, 0);
                        return;
                        }
                  else {
                        refresh |= e->abbox();
                        if (_selection.elements().contains(e))
                              _selection.remove(e);
                        else {
                            _selection.add(e);
                            selState = SEL_LIST;
                            }
                        }
                  }
            }
      else if (type == SELECT_RANGE) {
            bool activeIsFirst = false;
            int activeTrack = e->track();
            if (e->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(e);
                  int tick  = m->tick();
                  int etick = tick + m->ticks();
                  activeTrack = staffIdx * VOICES;
                  if (_selection.state() == SEL_NONE) {
                        _selection.setStaffStart(staffIdx);
                        _selection.setStaffEnd(staffIdx + 1);
                        //_selection.setStartSegment(m->tick2segment(tick, true));
                        _selection.setStartSegment(m->first());
                        // _selection.setEndSegment(tick2segment(etick, true));
                        _selection.setEndSegment(m->last());
                        }
                  else if (_selection.state() == SEL_RANGE) {
                        if (staffIdx < _selection.staffStart())
                              _selection.setStaffStart(staffIdx);
                        else if (staffIdx >= _selection.staffEnd())
                              _selection.setStaffEnd(staffIdx + 1);
                        if (tick < _selection.tickStart()) {
                              _selection.setStartSegment(m->tick2segment(tick, true));
                              activeIsFirst = true;
                              }
                        else if (etick >= _selection.tickEnd())
                              _selection.setEndSegment(tick2segment(etick, true));
                        else {
                              if (_selection.activeSegment() == _selection.startSegment()) {
                                    _selection.setStartSegment(m->tick2segment(tick, true));
                                    activeIsFirst = true;
                                    }
                              else
                                    _selection.setEndSegment(tick2segment(etick, true));
                              }
                        }
                  else if (_selection.isSingle()) {
                        Segment* seg = 0;
                        Element* oe  = _selection.element();
                        bool reverse = false;
                        int ticks    = 0;
                        if (oe->isChordRest())
                              ticks = static_cast<ChordRest*>(oe)->actualTicks();
                        int oetick = 0;
                        if (oe->parent()->type() == SEGMENT)
                              oetick = static_cast<Segment*>(oe->parent())->tick();
                        if (tick < oetick)
                              seg = m->first();
                        else if (etick >= oetick + ticks) {
                              seg = m->last();
                              reverse = true;
                              }
                        int track = staffIdx * VOICES;
                        Element* el = 0;
                        // find first or last chord/rest in measure
                        for (;;) {
                              el = seg->element(track);
                              if (el && el->isChordRest())
                                    break;
                              if (reverse)
                                    seg = seg->prev1();
                              else
                                    seg = seg->next1();
                              if (!seg)
                                    break;
                              }
                        if (el)
                              select(el, SELECT_RANGE, staffIdx);
                        return;
                        }
                  else {
                        printf("SELECT_RANGE: measure: sel state %d\n", _selection.state());
                        }
                  }
            else if (e->type() == NOTE || e->type() == REST || e->type() == CHORD) {
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  ChordRest* cr = static_cast<ChordRest*>(e);

                  if (_selection.state() == SEL_NONE) {
                        _selection.setStaffStart(e->staffIdx());
                        _selection.setStaffEnd(_selection.staffStart() + 1);
                        _selection.setStartSegment(cr->segment());
                        activeTrack = cr->track();
                        _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                        }
                  else if (_selection.isSingle()) {
                        Element* oe = _selection.element();
                        if (oe && (oe->type() == NOTE || oe->type() == REST || oe->type() == CHORD)) {
                              if (oe->type() == NOTE)
                                    oe = oe->parent();
                              ChordRest* ocr = static_cast<ChordRest*>(oe);
                              _selection.setStaffStart(oe->staffIdx());
                              _selection.setStaffEnd(_selection.staffStart() + 1);
                              _selection.setStartSegment(ocr->segment());
                              _selection.setEndSegment(ocr->segment()->nextCR(ocr->track()));
                              if (!_selection.endSegment())
                                    _selection.setEndSegment(ocr->segment()->next());

                              staffIdx = cr->staffIdx();
                              int tick = cr->tick();
                              if (staffIdx < _selection.staffStart())
                                    _selection.setStaffStart(staffIdx);
                              else if (staffIdx >= _selection.staffEnd())
                                    _selection.setStaffEnd(staffIdx + 1);
                              if (tick < _selection.tickStart()) {
                                    _selection.setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else if (tick >= _selection.tickEnd())
                                    _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                              else {
                                    if (_selection.activeSegment() == _selection.startSegment()) {
                                          _selection.setStartSegment(cr->segment());
                                          activeIsFirst = true;
                                          }
                                    else
                                          _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                                    }
                              }
                        else {
                              select(e, SELECT_SINGLE, 0);
                              return;
                              }
                        }
                  else if (_selection.state() == SEL_RANGE) {
                        staffIdx = cr->staffIdx();
                        int tick = cr->tick();
                        if (staffIdx < _selection.staffStart())
                              _selection.setStaffStart(staffIdx);
                        else if (staffIdx >= _selection.staffEnd())
                              _selection.setStaffEnd(staffIdx + 1);
                        if (tick < _selection.tickStart()) {
                              if (_selection.activeSegment() == _selection.endSegment())
                                    _selection.setEndSegment(_selection.startSegment());
                              _selection.setStartSegment(cr->segment());
                              activeIsFirst = true;
                              }
                        else if (_selection.endSegment() && tick >= _selection.tickEnd()) {
                              if (_selection.activeSegment() == _selection.startSegment())
                                    _selection.setStartSegment(_selection.endSegment());
                              Segment* s = cr->segment()->nextCR(cr->track());
                              _selection.setEndSegment(s);
                              }
                        else {
                              if (_selection.activeSegment() == _selection.startSegment()) {
                                    _selection.setStartSegment(cr->segment());
                                    activeIsFirst = true;
                                    }
                              else {
                                    _selection.setEndSegment(cr->segment()->nextCR(cr->track()));
                                    }
                              }
                        }
                  else {
                        printf("sel state %d\n", _selection.state());
                        }
                  selState = SEL_RANGE;
                  if (!_selection.endSegment())
                        _selection.setEndSegment(cr->segment()->nextCR());
                  if (!_selection.startSegment())
                        _selection.setStartSegment(cr->segment());
                  }
            else {
                  select(e, SELECT_SINGLE, staffIdx);
                  return;
                  }

            if (activeIsFirst)
                  _selection.setActiveSegment(_selection.startSegment());
            else
                  _selection.setActiveSegment(_selection.endSegment());

            _selection.setActiveTrack(activeTrack);

            selState = SEL_RANGE;
            _selection.updateSelectedElements();
            }
      _selection.setState(selState);
      }

//---------------------------------------------------------
//   lassoSelect
//---------------------------------------------------------

void Score::lassoSelect(const QRectF& bbox)
      {
      select(0, SELECT_SINGLE, 0);
      QRectF fr(bbox.normalized());
      foreach(Page* page, _pages) {
            QRectF pr(page->bbox());
            QRectF frr(fr.translated(-page->pos()));
            if (pr.right() < frr.left())
                  continue;
            if (pr.left() > frr.right())
                  break;

            QList<const Element*> el = page->items(frr);
            for (int i = 0; i < el.size(); ++i) {
                  const Element* e = el.at(i);
                  e->itemDiscovered = 0;
                  if (frr.contains(e->abbox())) {
                        if (e->type() != MEASURE && e->selectable())
                              select(const_cast<Element*>(e), SELECT_ADD, 0);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   lassoSelectEnd
//---------------------------------------------------------

void Score::lassoSelectEnd()
      {
      int noteRestCount     = 0;
      Segment* startSegment = 0;
      Segment* endSegment   = 0;
      int startStaff        = 0x7fffffff;
      int endStaff          = 0;
      int endTrack          = 0;

      if (_selection.elements().isEmpty()) {
            _selection.setState(SEL_NONE);
            return;
            }
      _selection.setState(SEL_LIST);

      foreach(const Element* e, _selection.elements()) {
            if (e->type() != NOTE && e->type() != REST)
                  continue;
            ++noteRestCount;
            if (e->type() == NOTE)
                  e = e->parent();
            Segment* seg = static_cast<const ChordRest*>(e)->segment();
            if ((startSegment == 0) || (seg->tick() < startSegment->tick()))
                  startSegment = seg;
            if ((endSegment == 0) || (seg->tick() > endSegment->tick())) {
                  endSegment = seg;
                  endTrack = e->track();
                  }
            int idx = e->staffIdx();
            if (idx < startStaff)
                  startStaff = idx;
            if (idx > endStaff)
                  endStaff = idx;
            }
      if (noteRestCount > 0) {
            endSegment = endSegment->nextCR(endTrack);
            _selection.setRange(startSegment, endSegment, startStaff, endStaff+1);
            if (_selection.state() != SEL_RANGE)
                  _selection.setState(SEL_RANGE);
            }
      _updateAll = true;
      }

//---------------------------------------------------------
//   searchSelectedElements
//    "ElementList selected"
//---------------------------------------------------------

void Score::searchSelectedElements()
      {
      _selection.searchSelectedElements();
      _selectionChanged = true;
      }

//---------------------------------------------------------
//   addLyrics
//---------------------------------------------------------

void Score::addLyrics(int tick, int staffIdx, const QString& txt)
      {
      if (txt.trimmed().isEmpty())
            return;
      Measure* measure = tick2measure(tick);
      Segment* seg     = measure->findSegment(SegChordRest, tick);
      if (seg == 0) {
            printf("no segment found for lyrics<%s> at tick %d\n",
               qPrintable(txt), tick);
            return;
            }
      int track = staffIdx * VOICES;
      ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
      if (cr) {
            Lyrics* l = new Lyrics(this);
            l->setText(txt);
            l->setTrack(staffIdx * VOICES);
            cr->add(l);
            }
      else {
            printf("no chord/rest for lyrics<%s> at tick %d, staff %d\n",
               qPrintable(txt), tick, staffIdx);
            }
      }

//---------------------------------------------------------
//   setTempo
//    convenience function to access TempoMap
//---------------------------------------------------------

void Score::setTempo(Segment* segment, qreal tempo)
      {
      setTempo(segment->tick(), tempo);
      }

void Score::setTempo(int tick, qreal tempo)
      {
      tempomap()->setTempo(tick, tempo);
      _playlistDirty = true;
      }

//---------------------------------------------------------
//   removeTempo
//---------------------------------------------------------

void Score::removeTempo(int tick)
      {
      tempomap()->delTempo(tick);
      _playlistDirty = true;
      }

//---------------------------------------------------------
//   setPause
//---------------------------------------------------------

void Score::setPause(int tick, qreal seconds)
      {
      tempomap()->setPause(tick, seconds);
      _playlistDirty = true;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

qreal Score::tempo(int tick) const
      {
      return tempomap()->tempo(tick);
      }

