//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

/**
 \file
 Implementation of class Score (partial).
*/

#include "score.h"
#include "key.h"
#include "al/sig.h"
#include "clef.h"
#include "al/tempo.h"
#include "measure.h"
#include "page.h"
#include "undo.h"
#include "system.h"
#include "select.h"
#include "mscore.h"
#include "scoreview.h"
#include "segment.h"
#include "xml.h"
#include "text.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "slur.h"
#include "seq.h"
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
#include "magbox.h"
#include "textpalette.h"
#include "preferences.h"
#include "repeatlist.h"
#include "keysig.h"
#include "beam.h"

Score* gscore;                 ///< system score, used for palettes etc.

QPoint scorePos(0,0);
QSize  scoreSize(950, 500);

MuseScore* mscore;
bool layoutDebug     = false;
bool scriptDebug     = false;
bool noSeq           = false;
bool noMidi          = false;
bool midiInputTrace  = false;
bool midiOutputTrace = false;
bool showRubberBand  = true;

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

InputState::InputState() :
   _duration(Duration::V_INVALID),
   rest(false),
   pad(0),
   pitch(72),
   noteType(NOTE_NORMAL),
   beamMode(BEAM_AUTO),
   _drumNote(-1),
   _drumset(0),
   track(0),
   _segment(0),
   noteEntryMode(false),
   slur(0)
      {
      }

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* InputState::cr() const
      {
      return _segment ? (static_cast<ChordRest*>(_segment->element(track))) : 0;
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int InputState::tick() const
      {
      return _segment ? _segment->tick() : 0;
      }

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
      e->setNext(el);
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
      if (nb->type() == HBOX || nb->type() == VBOX)
            nb->setSystem(ob->system());
      foreach(Element* e, *nb->el())
            e->setParent(nb);
      }

//---------------------------------------------------------
//   setSpatium
//---------------------------------------------------------

void Score::setSpatium(double v)
      {
      _spatium    = v;
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score(const Style& s)
   : _selection(this)
      {
      _spatium        = preferences.spatium * DPI;
      _pageFormat     = new PageFormat;
      _paintDevice    = 0;
      _needLayout     = false;
      startLayout     = 0;
      _undo           = new UndoStack();
      _repeatList     = new RepeatList(this);
      _style  = s;
      _swingRatio     = 0.0;
      // deep copy of defaultTextStyles:
      for (int i = 0; i < TEXT_STYLES; ++i)
            _textStyles.append(new TextStyle(defaultTextStyles[i]));

      _mscVersion     = MSCVERSION;
      _created        = false;
      _updateAll      = false;
      layoutAll       = false;
      keyState        = 0;
      _showInvisible  = true;
      _showFrames     = true;
      editTempo       = 0;
      _printing       = false;
      _playlistDirty  = false;
      _autosaveDirty  = false;
      _dirty          = false;
      _saved          = false;
      _playPos        = 0;
      _fileDivision   = AL::division;
      _creditsRead    = false;
      _defaultsRead   = false;
      rights          = 0;
      _pageOffset     = 0;
      _tempomap       = new AL::TempoMap;
      _sigmap         = new AL::TimeSigMap;
      _sigmap->add(0, Fraction(4, 4));
      curTick = 0;
      connect(_undo, SIGNAL(cleanChanged(bool)), SLOT(setClean(bool)));
      }

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
      {
      for (MeasureBase* m = _measures.first(); m;) {
            MeasureBase* nm = m->next();
            delete m;
            m = nm;
            }
      foreach(Element* e, _gel)
            delete e;
      /*foreach(Beam* b, _beams)
            delete b;*/
      foreach(Part* p, _parts)
            delete p;
      foreach(Staff* staff, _staves)
            delete staff;
      foreach(Page* page, _pages)
            delete page;
      foreach(System* s, _systems)
            delete s;
      foreach(Excerpt* e, _excerpts)
            delete e;

      delete _pageFormat;
      delete rights;
      delete _undo;           // this also removes _undoStack from Mscore::_undoGroup
      delete _tempomap;
      delete _sigmap;
      delete _repeatList;

      foreach(TextStyle* ts, _textStyles)
            delete ts;
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
            if (!measure->irregular())
                  ++measureNo;
            }
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

/**
 Import file \a name.
 */

bool Score::read(QString name)
      {
      _mscVersion = MSCVERSION;
      _saved      = false;
      info.setFile(name);

      QString cs = info.suffix();
      QString csl = cs.toLower();
      
      if (csl == "mscz") {
            if (!loadCompressedMsc(name))
                  return false;
            }
      else if (csl == "msc" || csl == "mscx") {
            if (!loadMsc(name))
                  return false;
            }
      else {
            // import
            if (!preferences.importStyleFile.isEmpty()) {
                  QFile f(preferences.importStyleFile);
                  // silently ignore style file on error
                  if (f.open(QIODevice::ReadOnly))
                        loadStyle(&f);
                  }

            if (csl == "xml") {
                  importMusicXml(name);
                  connectSlurs();
                  }
            else if (csl == "mxl")
                  importCompressedMusicXml(name);
            else if (csl == "mid" || csl == "midi" || csl == "kar") {
                  if (!importMidi(name))
                        return false;
                  }
            else if (csl == "md") {
                  if (!importMuseData(name))
                        return false;
                  }
            else if (csl == "ly") {
                  if (!importLilypond(name))
                        return false;
                  }
            else if (csl == "mgu" || csl == "sgu") {
                  if (!importBB(name))
                        return false;
                  }
            else if (csl == "cap") {
                  if (!importCapella(name))
                        return false;
                  connectSlurs();
                  }
            else if (csl == "ove") {
                  if(!importOve(name))
            		        return false;
				          }
            else if (csl == "bww") {
                  if (!importBww(name))
                        return false;
                  }
            else {
                  printf("unknown file suffix <%s>, name <%s>\n", qPrintable(cs), qPrintable(name));
                  return false;
                  }
            }

      renumberMeasures();
      if (_mscVersion < 103) {
            foreach(Staff* staff, _staves) {
                  Part* part = staff->part();
                  if (part->staves()->size() == 1)
                        staff->setBarLineSpan(1);
                  else {
                        if (staff == part->staves()->front())
                              staff->setBarLineSpan(part->staves()->size());
                        else
                              staff->setBarLineSpan(0);
                        }
                  }
            }

      checkSlurs();
      checkTuplets();
      rebuildMidiMapping();
      updateChannel();
      fixPpitch();

      mscore->updateRecentScores(this);

      startCmd();
      _needLayout = true;
      endCmd();

      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml, bool /*autosave*/)
      {
      xml.tag("Spatium", _spatium / DPMM);
      xml.tag("Division", AL::division);
      xml.curTrack = -1;
#if 0 // TODO-S
      if (!autosave && editObject) {                          // in edit mode?
            endCmd();
            setState(STATE_NORMAL);
            }
#endif
      _style.save(xml, true);      // save only differences to buildin style
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (*_textStyles[i] != defaultTextStyleArray[i])
                  _textStyles[i]->write(xml);
            }
      xml.tag("showInvisible", _showInvisible);
      xml.tag("showFrames", _showFrames);
      pageFormat()->write(xml);
      if (rights)
            rights->write(xml, "copyright");
      if (!_movementNumber.isEmpty())
            xml.tag("movement-number", _movementNumber);
      if (!_movementTitle.isEmpty())
            xml.tag("movement-title", _movementTitle);
      if (!_workNumber.isEmpty())
            xml.tag("work-number", _workNumber);
      if (!_workTitle.isEmpty())
            xml.tag("work-title", _workTitle);
      if (!_source.isEmpty())
            xml.tag("source", _source);

      foreach(KeySig* ks, customKeysigs)
            ks->write(xml);

      _sigmap->write(xml);
      _tempomap->write(xml);
      foreach(const Part* part, _parts)
            part->write(xml);
      foreach(const Excerpt* excerpt, _excerpts)
            excerpt->write(xml);

      // to serialize slurs/tuplets/beams, they need an id; this id is referenced
      // in begin-end elements
      int slurId   = 0;
      int tupletId = 0;
      int beamId   = 0;
      foreach(Element* el, _gel) {
            if (el->type() == SLUR)
                  static_cast<Slur*>(el)->setId(slurId++);
            }
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* tuplet, *m->tuplets())
                  tuplet->setId(tupletId++);
            }
      xml.curTrack = 0;
      foreach(Beam* beam, _beams) {
            beam->setId(beamId++);
//            beam->write(xml);
            }
      foreach(Element* el, _gel)
            el->write(xml);
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1));
            xml.curTick  = 0;
            xml.curTrack = staffIdx * VOICES;
            for (MeasureBase* m = first(); m; m = m->next()) {
                  if (m->type() == MEASURE || staffIdx == 0)
                        m->write(xml, staffIdx, staffIdx == 0);
                  if (m->type() == MEASURE)
                        xml.curTick = m->tick() + _sigmap->ticksMeasure(m->tick());
                  }
            xml.etag();
            }
      xml.curTrack = -1;
      xml.tag("cursorTrack", _is.track);
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

void Score::addMeasure(MeasureBase* m)
      {
      if (!m->next())
            m->setNext(tick2measureBase(m->tick()));
      _measures.add(m);
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Score::insertTime(int tick, int len)
      {
      if (len < 0) {
            //
            // remove time
            //
            len = -len;
            _tempomap->removeTime(tick, len);
            foreach(Staff* staff, _staves) {
                  staff->clefList()->removeTime(tick, len);
                  staff->keymap()->removeTime(tick, len);
                  }
            foreach(Element* el, _gel) {
                  if (el->type() == SLUR) {
                        Slur* s = (Slur*) el;
                        if (s->tick() >= tick + len) {
                              s->setTick(s->tick() - len);
                              }
                        if (s->tick2() >= tick + len) {
                              s->setTick2(s->tick2() - len);
                              }
                        }
                  else if (el->isSLine()) {
                        SLine* s = (SLine*) el;
                        if (s->tick() >= tick + len)
                              s->setTick(s->tick() - len);
                        if (s->tick2() >= tick + len)
                              s->setTick2(s->tick2() - len);
                        }
                  }
            }
      else {
            //
            // insert time
            //
            _tempomap->insertTime(tick, len);
            foreach(Staff* staff, _staves) {
                  staff->clefList()->insertTime(tick, len);
                  staff->keymap()->insertTime(tick, len);
                  }
            foreach(Element* el, _gel) {
                  if (el->type() == SLUR) {
                        Slur* s = (Slur*) el;
                        if (s->tick() >= tick) {
                              s->setTick(s->tick() + len);
                              }
                        if (s->tick2() >= tick) {
                              s->setTick2(s->tick2() + len);
                              }
                        }
                  else if (el->isSLine()) {
                        SLine* s = (SLine*) el;
                        if (s->tick() >= tick)
                              s->setTick(s->tick() + len);
                        if (s->tick2() >= tick)
                              s->setTick2(s->tick2() + len);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   fixTicks
//---------------------------------------------------------

/**
 Recalculate all ticks and measure numbers.

 This is needed after
      - inserting or removing a measure.
      - changing the sigmap
      - after inserting/deleting time (changes the sigmap)
*/

void Score::fixTicks()
      {
      int number = 0;
      int tick   = 0;
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE) {
                  mb->setTick(tick);
                  continue;
                  }
            Measure* m = static_cast<Measure*>(mb);
            number += m->noOffset();
            if (m->no() != number)
                  m->setNo(number);
            if (!m->irregular())
                  ++number;
            int mtick = m->tick();
            int diff  = tick - mtick;
            int measureTicks = _sigmap->ticksMeasure(tick);
            tick += measureTicks;
            m->moveTicks(diff);
            }
      }

//---------------------------------------------------------
//   pos2measure
//---------------------------------------------------------

/**
 Return measure for canvas relative position \a p.

 If *rst != -1, then staff is fixed.
*/

MeasureBase* Score::pos2measure(const QPointF& p, int* tick, int* rst, int* pitch,
   Segment** seg, QPointF* offset) const
      {
      foreach (const Page* page, pages()) {
            if (!page->abbox().contains(p))
                  continue;

            QPointF pp = p - page->pos();  // transform to page relative
            const QList<System*>* sl = page->systems();
            double y1 = 0.0;
            for (ciSystem is = sl->begin(); is != sl->end();) {
                  double y2;
                  System* s = *is;
                  ++is;
                  if (is != sl->end()) {
                        double sy2 = s->y() + s->bbox().height();
                        y2 = sy2 + ((*is)->y() - sy2)/2;
                        }
                  else
                        y2 = page->height();
                  if (pp.y() > y2) {
                        y1 = y2;
                        continue;
                        }
                  QPointF ppp = pp - s->pos();   // system relative
                  foreach(MeasureBase* mb, s->measures()) {
                        if (ppp.x() >= (mb->x() + mb->bbox().width()))
                              continue;
                        if (mb->type() != MEASURE)
                              return mb;
                        Measure* m = static_cast<Measure*>(mb);
                        double sy1 = 0;
                        if (rst && *rst == -1) {
                              for (int i = 0; i < nstaves();) {
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

                                    double sy2;
                                    if (ni != nstaves()) {
                                          SysStaff* nstaff = s->staff(ni);
                                          double s1y2 = staff->bbox().y() + staff->bbox().height();
                                          sy2 = s1y2 + (nstaff->bbox().y() - s1y2)/2;
                                          }
                                    else
                                          sy2 = y2 - s->pos().y();   // s->height();
                                    if (ppp.y() > sy2) {
                                          sy1 = sy2;
                                          i = ni;
                                          continue;
                                          }
                                    // search for segment + offset
                                    QPointF pppp = ppp - m->pos();
                                    int track = i * VOICES;
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
                                                if (tick)
                                                      *tick = segment->tick();
                                                if (pitch) {
                                                      Staff* s = _staves[i];
                                                      int clef = s->clefList()->clef(*tick);
                                                      *pitch = y2pitch(pppp.y() - staff->bbox().y(), clef, s->spatium());
                                                      }
                                                if (offset)
                                                      *offset = pppp - QPointF(segment->x(), staff->bbox().y());
                                                if (seg)
                                                      *seg = segment;
                                                return m;
                                                }
                                          }
                                    break;
                                    }
                              }
                        else {
                              //
                              // staff is fixed
                              //
                              // search for segment + offset
                              QPointF pppp = ppp - m->pos();
                              for (Segment* segment = m->first(); segment; segment = segment->next()) {
                                    if (segment->subtype() != SegChordRest)
                                          continue;
                                    Segment* ns = segment->next();
                                    while (ns && ns->subtype() != SegChordRest)
                                          ns = ns->next();
                                    if (ns) {
                                          double x1 = segment->x();
                                          double x2 = ns->x();
                                          if (pppp.x() >= (x1 + (x2 - x1) / 2))
                                                continue;
                                          }
                                    if (tick)
                                          *tick = segment->tick();
                                    if (pitch) {
                                          Staff* s = staff(*rst);
                                          // int clef = staff(*rst)->clefList()->clef(*tick);
                                          int clef = s->clefList()->clef(*tick);
                                          *pitch = y2pitch(pppp.y(), clef, s->spatium());
                                          }
                                    if (offset) {
                                          SysStaff* staff = s->staff(*rst);
                                          *offset = pppp - QPointF(segment->x(), staff->bbox().y());
                                          }
                                    if (seg)
                                          *seg = segment;
                                    return m;
                                    }
                              break;
                              }
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   pos2measure2
//---------------------------------------------------------

/**
 Return measure for canvas relative position \a p.

 Sets \a *tick to the nearest notes tick,
 \a *rst to the nearest staff,
 \a *line to the nearest staff line.
*/

Measure* Score::pos2measure2(const QPointF& p, int* tick, int* rst, int* line,
   Segment** seg) const
      {
      int voice = _is.voice();

      foreach(const Page* page, pages()) {
            if (!page->contains(p))
                  continue;
            QPointF pp = p - page->pos();  // transform to page relative

            const QList<System*>* sl = page->systems();
            double y1 = 0;
            for (ciSystem is = sl->begin(); is != sl->end();) {
                  double y2;
                  System* s = *is;
                  ++is;
                  if (is != sl->end()) {
                        double sy2 = s->y() + s->bbox().height();
                        y2 = sy2 + ((*is)->y() - sy2)/2;
                        }
                  else
                        y2 = page->height();
                  if (pp.y() > y2) {
                        y1 = y2;
                        continue;
                        }
                  QPointF ppp = pp - s->pos();   // system relative
                  foreach(MeasureBase* mb, s->measures()) {
                        if (mb->type() != MEASURE)
                              continue;
                        Measure* m = static_cast<Measure*>(mb);
                        if (ppp.x() > (m->x() + m->bbox().width()))
                              continue;
                        double sy1 = 0;
                        for (int i = 0; i < nstaves();) {
                              double sy2;

                              SysStaff* staff = s->staff(i);
                              ++i;
                              if (i != nstaves()) {
                                    SysStaff* nstaff = s->staff(i);
                                    double s1y2 = staff->bbox().y() + staff->bbox().height();
                                    sy2 = s1y2 + (nstaff->bbox().y() - s1y2)/2;
                                    }
                              else
                                    sy2 = y2 - s->pos().y();   // s->height();
                              if (ppp.y() > sy2) {
                                    sy1 = sy2;
                                    continue;
                                    }
                              // search for segment + offset
                              QPointF pppp = ppp - m->pos();
                              int track = (i-1) * VOICES + voice;
                              for (Segment* segment = m->first(); segment;) {
                                    if (segment->subtype() != SegChordRest || (segment->element(track) == 0 && voice == 0)) {
                                          segment = segment->next();
                                          continue;
                                          }
                                    Segment* ns = segment->next();
                                    for (; ns; ns = ns->next()) {
                                          if (ns->subtype() == SegChordRest && (ns->element(track) || voice))
                                                break;
                                          }
                                    if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                                          i     -= 1;
                                          *rst   = i;
                                          *tick  = segment->tick();
                                          //
                                          // TODO: restrict to reasonable values (pitch 0-127)
                                          //
                                          *line  = lrint((pppp.y()-staff->bbox().y())/_spatium * 2);
                                          *seg   = segment;
                                          return m;
                                          }
                                    segment = ns;
                                    }
                              break;
                              }
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   pos2measure3
//---------------------------------------------------------

/**
 Return nearest measure start for canvas relative position \a p.
*/

Measure* Score::pos2measure3(const QPointF& p, int* tick) const
      {
      foreach(const Page* page, pages()) {
            if (!page->contains(p))
                  continue;
            QPointF pp = p - page->pos();  // transform to page relative

            const QList<System*>* sl = page->systems();
            double y1 = 0;
            for (ciSystem is = sl->begin(); is != sl->end();) {
                  double y2;
                  System* s = *is;
                  ++is;
                  if (is != sl->end()) {
                        double sy2 = s->y() + s->bbox().height();
                        y2 = sy2 + ((*is)->y() - sy2)/2;
                        }
                  else
                        y2 = page->height();
                  if (pp.y() > y2) {
                        y1 = y2;
                        continue;
                        }
                  QPointF ppp = pp - s->pos();   // system relative
                  foreach(MeasureBase* m, s->measures()) {
                        if (m->type() != MEASURE)
                              continue;
                        if (ppp.x() > (m->x() + m->bbox().width()))
                              continue;
                        if (ppp.x() < (m->x() + m->bbox().width()*.5)) {
                              *tick = m->tick();
                              return (Measure*)m;
                              }
                        else {
                              MeasureBase* pm = m;
                              while (m && m->next() && m->next()->type() != MEASURE)
                                    m = m->next();
                              if (m) {
                                    *tick = m->tick();
                                    return (Measure*)m;
                                    }
                              else {
                                    *tick = pm->tick() + pm->tickLen();
                                    return (Measure*) pm;
                                    }
                              }
                        }
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
//   readStaff
//---------------------------------------------------------

void Score::readStaff(QDomElement e)
      {
      MeasureBase* mb = first();
      int staff       = e.attribute("id", "1").toInt() - 1;

      curTick  = 0;
      curTrack = staff * VOICES;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());

            if (tag == "Measure") {
                  Measure* measure = 0;
                  if (staff == 0) {
                        measure = new Measure(this);
                        measure->setTick(curTick);
                        add(measure);
                        }
                  else {
                        while (mb) {
                              if (mb->type() != MEASURE) {
                                    mb = mb->next();
                                    }
                              else {
                                    measure = (Measure*)mb;
                                    mb      = mb->next();
                                    break;
                                    }
                              }
                        if (measure == 0) {
                              printf("Score::readStaff(): missing measure!\n");
                              measure = new Measure(this);
                              measure->setTick(curTick);
                              add(measure);
                              }
                        }
                  measure->read(e, staff);
                  curTick = measure->tick() + measure->tickLen();
                  }
            else if (tag == "HBox") {
                  HBox* hbox = new HBox(this);
                  hbox->read(e);
                  hbox->setTick(curTick);
                  add(hbox);
                  }
            else if (tag == "VBox") {
                  VBox* vbox = new VBox(this);
                  vbox->read(e);
                  vbox->setTick(curTick);
                  add(vbox);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   pos2sel
//---------------------------------------------------------

int Measure::snap(int tick, const QPointF p) const
      {
      Segment* s = _first;
      for (; s->next(); s = s->next()) {
            double x  = s->x();
            double dx = s->next()->x() - x;
            if (s->tick() == tick)
                  x += dx / 3.0 * 2.0;
            else  if (s->next()->tick() == tick)
                  x += dx / 3.0;
            else
                  x += dx * .5;
            if (p.x() < x)
                  break;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   textUndoLevelAdded
//---------------------------------------------------------

// void Score::textUndoLevelAdded()
//      {
//      ++textUndoLevel;
//      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void ScoreView::midiNoteReceived(int pitch, bool chord)
      {
      MidiInputEvent ev;
      ev.pitch = pitch;
      ev.chord = chord;

      score()->enqueueMidiEvent(ev);
      if (!score()->undo()->active())
            cmd(0);
      }

//---------------------------------------------------------
//   snapNote
//---------------------------------------------------------

int Measure::snapNote(int /*tick*/, const QPointF p, int staff) const
      {
      Segment* s = _first;
      for (;;) {
            Segment* ns = s->next();
            while (ns && ns->element(staff) == 0)
                  ns = ns->next();
            if (ns == 0)
                  break;
            double x  = s->x();
            double nx = x + (ns->x() - x) * .5;
            if (p.x() < nx)
                  break;
            s = ns;
            }
      return s->tick();
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
//   setShowFrames
//---------------------------------------------------------

void Score::setShowFrames(bool v)
      {
      _showFrames = v;
      _updateAll  = true;
      end();
      }

//---------------------------------------------------------
//   setClean
//---------------------------------------------------------

void Score::setClean(bool val)
      {
      val = !val;
      if (_dirty != val) {
            _dirty         = val;
            _playlistDirty = true;
            emit dirtyChanged(this);
            }
      if (_dirty) {
            _autosaveDirty = true;
            _playlistDirty = true;
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
//   pos2TickAnchor
//    Calculates anchor position and tick for a
//    given position+staff in global coordinates.
//
//    return false if no anchor found
//---------------------------------------------------------

bool Score::pos2TickAnchor(const QPointF& pos, int staffIdx, int* tick, QPointF* anchor) const
      {
      Segment* seg = 0;
      MeasureBase* m = pos2measure(pos, tick, &staffIdx, 0, &seg, 0);
      if (!m || m->type() != MEASURE) {
            printf("pos2TickAnchor: no measure found\n");
            return false;
            }
      System* system = m->system();
      qreal y = system->staff(staffIdx)->bbox().y();
      *anchor = QPointF(seg->abbox().x(), y + system->canvasPos().y());
      return true;
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
//   setTextStyles
//---------------------------------------------------------

void Score::setTextStyles(const QVector<TextStyle*>& s)
      {
      foreach(TextStyle* ts, _textStyles)
            delete ts;
      _textStyles.clear();
      foreach(TextStyle* ts, s)
            _textStyles.append(new TextStyle(*ts));
      }

//---------------------------------------------------------
//   swapTextStyles
//---------------------------------------------------------

QVector<TextStyle*> Score::swapTextStyles(QVector<TextStyle*> s)
      {
      QVector<TextStyle*> tmp = _textStyles;
      _textStyles = s;
      return tmp;
      }

//---------------------------------------------------------
//   setCopyright
//---------------------------------------------------------

void Score::setCopyright(const QString& s)
      {
      if (rights == 0) {
            rights = new TextC(this);
            rights->setSubtype(TEXT_COPYRIGHT);
            rights->setTextStyle(TEXT_STYLE_COPYRIGHT);
            }
      rights->setText(s);
      }

//---------------------------------------------------------
//   setCopyrightHtml
//---------------------------------------------------------

void Score::setCopyrightHtml(const QString& s)
      {
      if (rights == 0) {
            rights = new TextC(this);
            rights->setSubtype(TEXT_COPYRIGHT);
            rights->setTextStyle(TEXT_STYLE_COPYRIGHT);
            }
      rights->setHtml(s);
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
      _is.track = v;
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
      if (!doc.setContent(buffer.buffer(), &err, &line, &column)) {
            printf("error cloning score %d/%d: %s\n<%s>\n",
               line, column, err.toLatin1().data(), buffer.buffer().data());
            return 0;
            }
      Score* score = new Score(_style);
      docName = "--";
      score->read(doc.documentElement());
      return score;
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
            bool drum = part->useDrumset();

            for (int k = 0; k < part->channel().size(); ++k) {
                  Channel* a = &part->channel(k);
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
//---------------------------------------------------------

Page* Score::searchPage(const QPointF& p) const
      {
      for (int i = 0; i < _pages.size(); ++i) {
            if (_pages[i]->contains(p))
                  return _pages[i];
            }
      return 0;
      }

//---------------------------------------------------------
//   getNextCRSegment
//---------------------------------------------------------

static Segment* getNextCRSegment(Segment* s, int track)
      {
      while (s && ((s->subtype() != SegChordRest) || !s->element(track)))
            s = s->next();
      return s;
      }

//---------------------------------------------------------
//   getPosition
//    return true if valid position found
//---------------------------------------------------------

bool Score::getPosition(Position* pos, const QPointF& p, int voice) const
      {
      const Page* page = searchPage(p);
      if (page == 0 || voice == -1)
            return false;
      //
      //    search system
      //
      QPointF pp               = p - page->pos();  // transform to page relative
      const QList<System*>* sl = page->systems();
      int n                    = sl->size();
      System* system           = 0;
      double y2;
      for (int i = 0; i < n; ++i) {
            System* s = (*sl)[i];
            if ((i+1) == n)
                  y2 = page->height();
            else  {
                  System* ns = (*sl)[i+1];
                  double sy2 = s->y() + s->bbox().height();
                  y2         = sy2 + (ns->y() - sy2) * .5;
                  }
            if (pp.y() < y2) {
                  system = s;
                  break;
                  }
            }
      if (system == 0) {
//            printf("no system\n");
            return false;
            }

      //
      //    search measure
      //
      QPointF ppp  = pp - system->pos();   // system relative
      pos->measure = 0;
      foreach(MeasureBase* mb, system->measures()) {
            if (mb->type() != MEASURE)
                  continue;
            if (ppp.x() < (mb->x() + mb->bbox().width())) {
                  pos->measure = static_cast<Measure*>(mb);
                  break;
                  }
            }
      if (pos->measure == 0) {
//            printf("no measure\n");
            return false;
            }

      //
      //    search staff
      //
      double sy1         = 0;
      pos->staffIdx      = 0;
      SysStaff* sstaff   = 0;
      for (; pos->staffIdx < nstaves(); ++pos->staffIdx) {
            double sy2;
            SysStaff* ss = system->staff(pos->staffIdx);
            if ((pos->staffIdx+1) != nstaves()) {
                  SysStaff* nstaff = system->staff(pos->staffIdx+1);
                  double s1y2 = ss->bbox().y() + ss->bbox().height();
                  sy2         = s1y2 + (nstaff->bbox().y() - s1y2) * .5;
                  }
            else
                  sy2 = y2 - system->pos().y();   // system->height();
            if (ppp.y() < sy2) {
                  sstaff = ss;
                  break;
                  }
            sy1 = sy2;
            }
      if (sstaff == 0) {
// printf("no sys staff\n");
            return false;
            }

      //
      //    search segment
      //
      QPointF pppp     = ppp - pos->measure->pos();
      double x         = pppp.x();
      Segment* segment = 0;
      pos->tick        = -1;

      int track = pos->staffIdx * VOICES + voice;
      for (segment = pos->measure->first(); segment;) {
            segment = getNextCRSegment(segment, track);
            if (segment == 0)
                  break;
            Segment* ns = getNextCRSegment(segment->next(), track);

            double x1 = segment->x();
            double x2;
            int ntick;
            double d;
            if (ns) {
                  x2    = ns->x();
                  ntick = ns->tick();
                  d = x2 - x1;
                  }
            else {
                  x2    = pos->measure->bbox().width();
                  ntick = pos->measure->tick() + pos->measure->tickLen();
                  d = (x2 - x1) * 2.0;
                  x = x1;
                  pos->tick = segment->tick();
                  break;      ///?
                  }

            if (x < (x1 + d * .5) && segment->element(track)) {
                  x = x1;
                  pos->tick = segment->tick();
                  break;
                  }
            segment = ns;
            }
      if (segment == 0) {
            if (voice) {
                  //
                  // first chord/rest segment of measure is a valid position
                  // for voice > 0 even if there is no chord/rest
                  //
                  for (segment = pos->measure->first(); segment;) {
                        if (segment->subtype() == SegChordRest)
                              break;
                        segment = segment->next();
                        }
                  x = segment->x();
                  pos->tick = pos->measure->tick();
                  }
            else {
// printf("no segment+ track %d voice %d itrack %d\n", track, voice, _is.track);
                  return false;
                  }
            }
      //
      // TODO: restrict to reasonable values (pitch 0-127)
      //
      double mag = staff(pos->staffIdx)->mag();
      pos->line  = lrint((pppp.y() - sstaff->bbox().y()) / (_spatium * mag) * 2.0);
      double y   = pos->measure->canvasPos().y() + sstaff->y();
      y         += pos->line * _spatium * .5 * mag;
      pos->pos  = QPointF(x + pos->measure->canvasPos().x(), y);
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
//   spatiumChanged
//---------------------------------------------------------

void Score::spatiumChanged(double oldValue, double newValue)
      {
      foreach(Element* e, _gel)
            e->spatiumChanged(oldValue, newValue);
      for(MeasureBase* mb = _measures.first(); mb; mb = mb->next())
            mb->spatiumChanged(oldValue, newValue);

      foreach(Part* part, _parts) {
            if (part->longName())
                  part->longName()->spatiumChanged(oldValue, newValue);
            if (part->shortName())
                  part->shortName()->spatiumChanged(oldValue, newValue);
            }
      if (rights)
            rights->spatiumChanged(oldValue, newValue);
      foreach(Page* p, pages()) {
            if (p->pageNo())
                  p->pageNo()->spatiumChanged(oldValue, newValue);
            }
      }

//---------------------------------------------------------
//   getCreateMeasure
//    - return Measure for tick
//    - create new Measure(s) if there is no measure for
//      this tick
//---------------------------------------------------------

Measure* Score::getCreateMeasure(int tick)
      {
      MeasureBase* last = _measures.last();
      int lastTick = last ? last->tick() + last->tickLen() : 0;
      while (tick >= lastTick) {
            Measure* m = new Measure(this);
            m->setTick(lastTick);
            int ticks = _sigmap->ticksMeasure(lastTick);
            add(m);
            lastTick += ticks;
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
      if (debugMode)
            printf("   Score::addElement %p %s parent %s\n",
               element, element->name(), element->parent() ? element->parent()->name() : "null");

      if (element->type() == MEASURE
         || (element->type() == HBOX && element->parent()->type() != VBOX)
         || element->type() == VBOX
         ) {
            add(element);
            return;
            }

      if (element->parent() == 0)
            add(element);
      else
            element->parent()->add(element);

      if (element->type() == CLEF) {
            int staffIdx = element->staffIdx();
            Clef* clef   = (Clef*) element;
            int tick     = clef->tick();

            //-----------------------------------------------
            //   move notes
            //-----------------------------------------------

            bool endFound = false;
            for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
                  int startTrack = staffIdx * VOICES;
                  int endTrack   = startTrack + VOICES;
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* ie = segment->element(track);
                        if (ie && ie->type() == CLEF && ie->tick() > tick) {
                              endFound = true;
                              break;
                              }
                        }
                  if (endFound)
                        break;
                  }
            }
      else if (element->type() == KEYSIG)
            layoutAll = true;
      else if (element->type() == SLUR) {
            Slur* s = static_cast<Slur*>(element);
            if (s->startElement())
                  static_cast<ChordRest*>(s->startElement())->addSlurFor(s);
            if (s->endElement())
                  static_cast<ChordRest*>(s->endElement())->addSlurBack(s);
            }
      else if ((element->type() == OTTAVA) || (element->type() == DYNAMIC)) {
            fixPpitch();      // recalculate all velocities
            _playlistDirty = true;
            }
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

      if (debugMode)
            printf("   Score::removeElement %p %s parent %p %s\n",
               element, element->name(), parent, parent ? parent->name() : "");

      // special for MEASURE, HBOX, VBOX
      // their parent is not static

      if (element->type() == MEASURE
         || (element->type() == HBOX && parent->type() != VBOX)
         || element->type() == VBOX) {
            remove(element);
            return;
            }
      if (element->type() == BEAM)          // beam parent does not survive layout
            element->setParent(0);

      if (parent)
            parent->remove(element);
      else
            remove(element);

      switch(element->type()) {
            case OTTAVA:
            case DYNAMIC:
                  fixPpitch();
                  _playlistDirty = true;
                  break;

            case CHORD:
            case REST:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(element);
                  cr->setBeam(0);
                  }
                  break;
            case CLEF:
                  {
                  Clef* clef   = static_cast<Clef*>(element);
                  int tick     = clef->tick();
                  int staffIdx = clef->staffIdx();

                  //-----------------------------------------------
                  //   move notes
                  //-----------------------------------------------

                  bool endFound = false;
                  for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
                        int startTrack = staffIdx * VOICES;
                        int endTrack   = startTrack + VOICES;
                        for (int track = startTrack; track < endTrack; ++track) {
                              Element* ie = segment->element(track);
                              if (ie && ie->type() == CLEF && ie->tick() > tick) {
                                    endFound = true;
                                    break;
                                    }
                              }
                        if (endFound)
                              break;
                        }
                  }
                  break;
            case KEYSIG:
                  layoutAll = true;
                  break;
            case SLUR:
                  {
                  Slur* s = static_cast<Slur*>(element);
                  static_cast<ChordRest*>(s->startElement())->removeSlurFor(s);
                  static_cast<ChordRest*>(s->endElement())->removeSlurBack(s);
                  }
                  break;
            default:
                  break;
            }
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

Segment* Score::firstSegment() const
      {
      Measure* m = firstMeasure();
      return m ? m->first() : 0;
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

double Score::utick2utime(int tick) const
      {
      return _repeatList->utick2utime(tick);
      }

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int Score::utime2utick(double utime)
      {
      return _repeatList->utime2utick(utime);
      }

StyleVal Score::style(StyleIdx idx) const
      {
      return _style[idx];
      }

Spatium Score::styleS(StyleIdx idx) const
      {
      return _style[idx].toSpatium();
      }

bool Score::styleB(StyleIdx idx) const
      {
      return _style[idx].toBool();
      }

double Score::styleD(StyleIdx idx) const
      {
      return _style[idx].toDouble();
      }

int Score::styleI(StyleIdx idx) const
      {
      return _style[idx].toInt();
      }

void Score::setStyle(StyleIdx idx, const StyleVal& v)
      {
      _style[idx] = v;
      }

int Score::inputPos() const
      {
      return _is.tick();
      }

//---------------------------------------------------------
//   scanElements
//    scan all elements
//---------------------------------------------------------

void Score::scanElements(void* data, void (*func)(void*, Element*))
      {
      foreach (Element* element, _gel) {
            if (element->type() == SLUR)
                  continue;
            element->scanElements(data, func);
            }
      foreach(Beam* b, _beams)
            func(data, b);
      for(MeasureBase* m = first(); m; m = m->next())
            m->scanElements(data, func);
      foreach(Page* page, pages())
            page->scanElements(data, func);
      }

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

Beam* Score::beam(int id) const
      {
      foreach(Beam* b, _beams) {
            if (b->id() == id)
                  return b;
            }
      return 0;
      }

//---------------------------------------------------------
//   customKeySigIdx
//    try to find custom key signature in table,
//    return index or -1 if not found
//---------------------------------------------------------

int Score::customKeySigIdx(KeySig* ks) const
      {
printf("Score::customKeySigIdx\n");
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
      ks->setSubtype(k);
      ks->setScore(this);
printf("Score::addCustomKeySig idx %d\n", idx);
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

KeySig* Score::keySigFactory(KeySigEvent e)
      {
      KeySig* ks;
      if (!e.isValid())
            return 0;
      if (e.custom()) {
            ks = new KeySig(*customKeysigs[e.customType()]);
            }
      else {
            ks = new KeySig(this);
            ks->setSubtype(e);
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

