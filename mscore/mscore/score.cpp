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
#include "stafftype.h"
#include "tempotext.h"
#include "articulation.h"

#ifdef OMR
#include "omr/omr.h"
#endif

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
   _drumNote(-1),
   _drumset(0),
   _track(0),
   _segment(0),
   rest(false),
   pad(0),
   pitch(72),
   noteType(NOTE_NORMAL),
   beamMode(BEAM_AUTO),
   noteEntryMode(false),
   slur(0)
      {
      }

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* InputState::cr() const
      {
      return _segment ? static_cast<ChordRest*>(_segment->element(_track)) : 0;
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
      _spatium = v;
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
      _style          = s;
      _staffTypes     = ::staffTypes;     // init with buildin types
      _swingRatio     = 0.0;
      // deep copy of defaultTextStyles:
      for (int i = 0; i < TEXT_STYLES; ++i)
            _textStyles.append(new TextStyle(defaultTextStyles[i]));

      _mscVersion     = MSCVERSION;
      _created        = false;
      _updateAll      = false;
      layoutAll       = false;
      layoutFlags     = 0;
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
      _omr            = 0;
      _showOmr        = false;
      _sigmap         = new AL::TimeSigMap();
      _tempomap       = new AL::TempoMap;
      connect(_undo, SIGNAL(cleanChanged(bool)), SLOT(setClean(bool)));
      }

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
      {
      delete _pageFormat;
      delete rights;
      delete _undo;           // this also removes _undoStack from Mscore::_undoGroup
      delete _sigmap;
      delete _tempomap;
      delete _repeatList;
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
            if (measure->sectionBreak())
                  measureNo = 0;
            else if (measure->irregular())      // dont count measure
                  ;
            else
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

      if (cs == "mscz") {
            if (!loadCompressedMsc(name))
                  return false;
            }
      else if (cs.toLower() == "msc" || cs.toLower() == "mscx") {
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

            if (cs == "xml") {
                  importMusicXml(name);
                  connectSlurs();
                  }
            else if (cs == "mxl")
                  importCompressedMusicXml(name);
            else if (cs.toLower() == "mid" || cs.toLower() == "midi" || cs.toLower() == "kar") {
                  if (!importMidi(name))
                        return false;
                  }
            else if (cs == "md") {
                  if (!importMuseData(name))
                        return false;
                  }
            else if (cs == "ly") {
                  if (!importLilypond(name))
                        return false;
                  }
            else if (cs.toLower() == "mgu" || cs.toLower() == "sgu") {
                  if (!importBB(name))
                        return false;
                  }
            else if (cs.toLower() == "cap") {
                  if (!importCapella(name))
                        return false;
                  connectSlurs();
                  }
            else if (cs.toLower() == "ove") {
            	if(!importOve(name))
            		return false;
			}
#ifdef OMR
            else if (cs.toLower() == "pdf") {
                  if (!importPdf(name))
                        return false;
                  }
#endif
            else if (cs.toLower() == "bww") {
                  if (!importBww(name))
                        return false;
                  }
            else {
                  printf("unknown file suffix <%s>, name <%s>\n", qPrintable(cs), qPrintable(name));
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
      checkScore();

      rebuildMidiMapping();
      updateChannel();
      fixPpitch();

      mscore->updateRecentScores(this);

      startCmd();
      _needLayout = true;
      layoutFlags |= LAYOUT_FIX_TICKS;
      endCmd();

      return true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml, bool /*autosave*/)
      {
#ifdef OMR
      if (_omr)
            _omr->write(xml);
      if (_showOmr)
            xml.tag("showOmr", _showOmr);
#endif

      _syntiSettings.write(xml);

      xml.tag("Spatium", _spatium / DPMM);
      xml.tag("Division", AL::division);
      xml.curTrack = -1;

      _style.save(xml, true);      // save only differences to buildin style
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (*_textStyles[i] != defaultTextStyleArray[i])
                  _textStyles[i]->write(xml);
            }
      int idx = 0;
      foreach(StaffType* st, _staffTypes) {
            if (st->modified())
                  st->write(xml, idx);
            ++idx;
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
      foreach(const Part* part, _parts)
            part->write(xml);
      foreach(const Excerpt* excerpt, _excerpts)
            excerpt->write(xml);

      // to serialize slurs/tuplets/beams, they need an id; this id is referenced
      // in begin-end elements
//      foreach(Element* el, _gel) {
//            if (el->type() == SLUR)
//                  static_cast<Slur*>(el)->setId(slurId++);
//            }
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* tuplet, *m->tuplets())
                  tuplet->setId(xml.tupletId++);
            }
      xml.curTrack = 0;
      foreach(Beam* beam, _beams)
            beam->setId(xml.beamId++);
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
                        xml.curTick = m->tick() + m->ticks();
                  }
            xml.etag();
            }
      xml.curTrack = -1;
      xml.tag("cursorTrack", _is.track());
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
      addLayoutFlag(LAYOUT_FIX_TICKS);
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
#if 0  // WS1
                  if (el->type() == SLUR) {
                        Slur* s = static_cast<Slur*>(el);
                        if (s->tick() >= tick + len) {
                              s->setTick(s->tick() - len);
                              }
                        if (s->tick2() >= tick + len) {
                              s->setTick2(s->tick2() - len);
                              }
                        }
                  else if (el->isSLine()) {
                        SLine* s = static_cast<SLine*>(el);
                        if (s->tick() >= tick + len)
                              s->setTick(s->tick() - len);
                        if (s->tick2() >= tick + len)
                              s->setTick2(s->tick2() - len);
                        }
#endif
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
#if 0 // WS1
                  if (el->type() == SLUR) {
                        Slur* s = static_cast<Slur*>(el);
                        if (s->tick() >= tick) {
                              s->setTick(s->tick() + len);
                              }
                        if (s->tick2() >= tick) {
                              s->setTick2(s->tick2() + len);
                              }
                        }
                  else if (el->isSLine()) {
                        SLine* s = static_cast<SLine*>(el);
                        if (s->tick() >= tick)
                              s->setTick(s->tick() + len);
                        if (s->tick2() >= tick)
                              s->setTick2(s->tick2() + len);
                        }
#endif
                  }
            }
      addLayoutFlag(LAYOUT_FIX_TICKS);
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

      _tempomap->clear();
      _sigmap->clear();
      Fraction sig(fm->timesig());
      _sigmap->add(0, AL::SigEvent(sig,  number));

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {

            foreach(const Element* e, *mb->el()) {
                  if (e->type() == TEMPO_TEXT) {
                        const TempoText* tt = static_cast<const TempoText*>(e);
                        _tempomap->addTempo(tt->segment()->tick(), tt->tempo());
                        }
                  }
            if (mb->type() != MEASURE) {
                  mb->setTick(tick);
                  continue;
                  }

            Measure* m = static_cast<Measure*>(mb);

            //
            // fix ticks
            //
            int mtick = m->tick();
            int diff  = tick - mtick;
            int measureTicks = m->ticks();
            tick += measureTicks;
            m->moveTicks(diff);

            //
            //  implement section break rest
            //
            if (m->sectionBreak())
                  _tempomap->addPause(m->tick() + m->ticks(), m->pause());

            //
            // implement fermata as a tempo change
            //
            SegmentTypes st = SegChordRest | SegBreath;

            for (Segment* s = m->first(st); s; s = s->next(st)) {
                  if (s->subtype() == SegBreath) {
                        _tempomap->addPause(s->tick(), .1);
                        continue;
                        }
                  foreach(Element* e, s->elist()) {
                        if (!e)
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        double stretch = -1.0;
                        foreach(Articulation* a, *cr->getArticulations()) {
                              switch(a->subtype()) {
                                    case UshortfermataSym:
                                    case DshortfermataSym:
                                          stretch = 1.5;
                                          break;
                                    case UfermataSym:
                                    case DfermataSym:
                                          stretch = 2.0;
                                          break;
                                    case UlongfermataSym:
                                    case DlongfermataSym:
                                          stretch = 3.0;
                                          break;
                                    case UverylongfermataSym:
                                    case DverylongfermataSym:
                                          stretch = 4.0;
                                          break;
                                    default:
                                          break;
                                    }
                              }
                        if (stretch > 0.0) {
                              double otempo = _tempomap->tempo(cr->tick());
                              double ntempo = otempo / stretch;
                              _tempomap->addTempo(cr->tick(), ntempo);
                              _tempomap->addTempo(cr->tick() + cr->ticks(), otempo);
                              break;      // do not consider more staves/voices
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
            if (m->timesig() != sig) {
                  sig = m->timesig();
                  _sigmap->add(tick, AL::SigEvent(sig,  number));
                  }
            }
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
      double sy1 = 0;
      double y   = p.y() - s->canvasPos().y();

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

            double sy2;
            if (ni != nstaves()) {
                  SysStaff* nstaff = s->staff(ni);
                  double s1y2 = staff->bbox().y() + staff->bbox().height();
                  sy2 = s1y2 + (nstaff->bbox().y() - s1y2)/2;
                  }
            else
                  sy2 = s->page()->height() - s->pos().y();   // s->height();
            if (y > sy2) {
                  sy1 = sy2;
                  i   = ni;
                  continue;
                  }
            break;
            }

      // search for segment + offset
      QPointF pppp = p - m->canvasPos();
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
                        int clef = s->clefList()->clef(segment->tick());
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
//   readStaff
//---------------------------------------------------------

void Score::readStaff(QDomElement e)
      {
      MeasureBase* mb = first();
      int staff       = e.attribute("id", "1").toInt() - 1;
      curTick         = 0;
      curTrack        = staff * VOICES;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());

            if (tag == "Measure") {
                  Measure* measure = 0;
                  if (staff == 0) {
                        measure = new Measure(this);
                        measure->setTick(curTick);
                        add(measure);
                        if (_mscVersion < 115) {
                              const AL::SigEvent& ev = _sigmap->timesig(measure->tick());
                              measure->setLen(ev.timesig());
                              measure->setTimesig(ev.nominal());
                              }
                        else {
                              //
                              // inherit timesig from previous measure
                              //
                              Measure* m = measure->prevMeasure();
                              Fraction f(m ? m->timesig() : Fraction(4,4));
                              measure->setLen(f);
                              measure->setTimesig(f);
                              }
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
                  curTick = measure->tick() + measure->ticks();
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
            _autosaveDirty = true;
            emit dirtyChanged(this);
            }
      if (_dirty)
            _playlistDirty = true;
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
      _is.setTrack(v);
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
            bool drum = part->instr()->useDrumset();

            for (int k = 0; k < part->instr()->channel().size(); ++k) {
                  Channel* a = &part->instr()->channel(k);
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
//   searchSystem
//    return list of systems as there may be more than
//    one system in a row
//---------------------------------------------------------

QList<System*> Score::searchSystem(const QPointF& pos) const
      {
      QList<System*> systems;
      Page* page = searchPage(pos);
      if (page == 0)
            return systems;
      double y = pos.y() - page->pos().y();  // transform to page relative
      const QList<System*>* sl = page->systems();
      double y2;
      int n = sl->size();
      for (int i = 0; i < n; ++i) {
            System* s = sl->at(i);
            System* ns = 0;               // next system row
            for (int ii = i+1; ii < n; ++ii) {
                  ns = sl->at(ii);
                  if (ns->y() != s->y())
                        break;
                  }
            if (ns == 0)
                  y2 = page->height();
            else  {
                  double sy2 = s->y() + s->bbox().height();
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
//---------------------------------------------------------

Measure* Score::searchMeasure(const QPointF& p) const
      {
      QList<System*> systems = searchSystem(p);
      if (systems.isEmpty())
            return 0;

      foreach(System* system, systems) {
            double x = p.x() - system->canvasPos().x();
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
      static const SegmentTypes st = SegChordRest;
      while (s) {
            if (s->element(track + voice))
                  break;
            int v;
            for (v = 0; v < VOICES; ++v) {
                  if (s->element(track + v))
                        break;
                  }
            if ((v != VOICES) && voice) {
                  Segment* s1;
                  // Measure* m = s->measure();
                  int ntick;
                  bool skipChord = false;
                  bool ns        = false;
                  for (s1 = s->measure()->first(st); s1; s1 = s1->next(st)) {
                        ChordRest* cr = static_cast<ChordRest*>(s1->element(track + voice));
                        if (cr) {
                              if (ns)
                                    return s1;
                              ntick = s1->tick() + cr->ticks();
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
      double sy1         = 0;
      pos->staffIdx      = 0;
      SysStaff* sstaff   = 0;
      System* system     = pos->measure->system();
      double y           = p.y() - system->canvasPos().y();
      for (; pos->staffIdx < nstaves(); ++pos->staffIdx) {
            double sy2;
            SysStaff* ss = system->staff(pos->staffIdx);
            if ((pos->staffIdx+1) != nstaves()) {
                  SysStaff* nstaff = system->staff(pos->staffIdx+1);
                  double s1y2 = ss->bbox().y() + ss->bbox().height();
                  sy2         = s1y2 + (nstaff->bbox().y() - s1y2) * .5;
                  }
            else
                  sy2 = system->page()->height() - system->pos().y();   // system->height();
            if (y < sy2) {
                  sstaff = ss;
                  break;
                  }
            sy1 = sy2;
            }
      if (sstaff == 0)
            return false;

      //
      //    search segment
      //
      QPointF pppp(p - pos->measure->canvasPos());
      double x         = pppp.x();
      Segment* segment = 0;
      pos->segment     = 0;

      // int track = pos->staffIdx * VOICES + voice;
      int track = pos->staffIdx * VOICES;
      for (segment = pos->measure->first(SegChordRest); segment;) {
            segment = getNextValidInputSegment(segment, track, voice);
            if (segment == 0)
                  break;
            Segment* ns = getNextValidInputSegment(segment->next(SegChordRest), track, voice);

            double x1 = segment->x();
            double x2;
            int ntick;
            double d;
            if (ns) {
                  x2    = ns->x();
                  ntick = ns->tick();
                  d     = x2 - x1;
                  }
            else {
                  x2    = pos->measure->bbox().width();
                  ntick = pos->measure->tick() + pos->measure->ticks();
                  d     = (x2 - x1) * 2.0;
                  x     = x1;
                  pos->segment = segment;
                  break;
                  }

//            if (x < (x1 + d * .5) && segment->element(track)) {
//            if (x < (x1 + d * .5) && isStaffElement(segment, track)) {
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
      double mag = staff(pos->staffIdx)->mag();
      pos->line  = lrint((pppp.y() - sstaff->bbox().y()) / (_spatium * mag) * 2.0);
      y          = pos->measure->canvasPos().y() + sstaff->y();
      y         += pos->line * _spatium * .5 * mag;
      pos->pos  = QPointF(x + pos->measure->canvasPos().x(), y);

      int minLine = pitch2line(0);
      Staff* s    = staff(pos->staffIdx);
      int clef    = s->clefList()->clef(pos->segment->tick());
      minLine     = 127 - minLine - 82 + clefTable[clef].yOffset;
      int maxLine = pitch2line(127);
      maxLine     = 127 - maxLine - 82 + clefTable[clef].yOffset;

      if (pos->line > minLine || pos->line < maxLine)
            return false;

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
      Fraction ts;
      int lastTick;
      Measure* last = lastMeasure();
      if (last) {
            lastTick = last->tick();
            ts = last->timesig();
            }
      else {
            lastTick = 0;
            ts = Fraction(4,4);
            }
      while (tick >= lastTick) {
            Measure* m = new Measure(this);
            m->setTick(lastTick);
            m->setTimesig(ts);
            m->setLen(ts);
            add(m);
            lastTick += ts.ticks();
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
            printf("   Score::addElement %p(%s) parent %p(%s)\n",
               element, element->name(), element->parent(),
               element->parent() ? element->parent()->name() : "");
            }

      if (element->type() == MEASURE
         || (element->type() == HBOX && element->parent()->type() != VBOX)
         || element->type() == VBOX
         ) {
            add(element);
            addLayoutFlag(LAYOUT_FIX_TICKS);
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
                        if (ie && ie->type() == CLEF && segment->tick() > tick) {
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
            printf("   Score::removeElement %p(%s) parent %p(%s)\n",
               element, element->name(), parent, parent ? parent->name() : "");

      // special for MEASURE, HBOX, VBOX
      // their parent is not static

      if (element->type() == MEASURE
         || (element->type() == HBOX && parent->type() != VBOX)
         || element->type() == VBOX) {
            remove(element);
            addLayoutFlag(LAYOUT_FIX_TICKS);
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
                              if (ie && ie->type() == CLEF && segment->tick() > tick) {
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
      if (e.custom) {
            ks = new KeySig(*customKeysigs[e.customType]);
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

