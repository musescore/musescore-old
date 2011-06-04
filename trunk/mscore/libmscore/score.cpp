//---------------------------------------------------------
//    MscorePlayer
//
//    Copyright (C) 2010 Werner Schweer
//---------------------------------------------------------

#include <QtCore/QFile>

#include "score.h"
#include "scoreproxy.h"
#include "measure.h"
#include "measurebase.h"
#include "segment.h"
#include "chordrest.h"
#include "slur.h"
#include "style.h"
#include "part.h"
#include "al/sig.h"
#include "al/tempo.h"
#include "al/xml.h"
#include "keysig.h"
#include "articulation.h"
#include "beam.h"
#include "tempotext.h"
#include "preferences.h"
#include "page.h"
#include "repeatlist.h"
#include "stafftype.h"
#include "staff.h"
#include "cursor.h"
#include "system.h"
#include "sym.h"
#include "clef.h"

qreal PDPI  = 72.0;
qreal DPI   = 72.0;
qreal DPMM  = DPI / INCH;
int division = 480;

//---------------------------------------------------------
//   ScoreProxy
//---------------------------------------------------------

ScoreProxy::ScoreProxy()
      {
      s = new Score;
      s->scoreProxy = this;
      }

ScoreProxy::~ScoreProxy()
      {
      delete s;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool ScoreProxy::read(const QString& name)
      {
      return s->read(name);
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
      {
      _parentScore   = 0;
      _symIdx        = 0;
      _spatium       = 5.0 / PPI * DPI;
      _pageFormat    = new PageFormat;
      _repeatList    = new RepeatList(this);
      _style         = defaultStyle;
      _staffTypes    = defaultStaffTypes;     // init with buildin types
      _mscVersion    = MSCVERSION;
      _showInvisible = true;
      _showFrames    = true;
      _printing      = false;
      _fileDivision  = AL::division;
      _creditsRead   = false;
      _defaultsRead  = false;
      _sigmap        = new TimeSigMap();
      _tempomap      = new AL::TempoMap;
      _cursor        = new Cursor(this);
      cursorSegment  = 0;
      }

Score::~Score()
      {
      for (MeasureBase* m = _measures.first(); m;) {
            MeasureBase* nm = m->next();
            delete m;
            m = nm;
            }
      foreach(Part* p, _parts)
            delete p;
      foreach(Staff* staff, _staves)
            delete staff;
      foreach(Page* page, _pages)
            delete page;
      foreach(System* s, _systems)
            delete s;
      delete _pageFormat;
      delete _repeatList;
      delete _sigmap;
      delete _tempomap;
      delete _cursor;
      }

//---------------------------------------------------------
//    load
//---------------------------------------------------------

bool Score::read(const QString& name)
      {
      QFileInfo info(name);
      bool rv;
      if (info.suffix() == "mscx")
            rv = loadMsc(name);
      else if (info.suffix() == "mscz")
            rv = loadCompressedMsc(name);
      if (!rv)
            return false;

      int staffIdx = 0;
      foreach(Staff* st, _staves) {
            st->clefList()->clear();
            st->keymap()->clear();
            int track = staffIdx * VOICES;
            KeySig* key1 = 0;
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->generated())
                              continue;
                        if (s->subtype() == SegClef) {
                              Clef* clef = static_cast<Clef*>(e);
                              st->setClef(s->tick(), clef->clefType());
                              }
                        else if (s->subtype() == SegKeySig) {
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
            ++staffIdx;
            }
      updateNotes();
      doLayout();
      return true;
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
      if (nb->type() == HBOX || nb->type() == VBOX || nb->type() == TBOX || nb->type() == FBOX)
            nb->setSystem(ob->system());
      foreach(Element* e, *nb->el())
            e->setParent(nb);
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
//   first
//---------------------------------------------------------

MeasureBase* Score::first() const
      {
      return _measures.first();
      }

//---------------------------------------------------------
//   last
//---------------------------------------------------------

MeasureBase* Score::last()  const
      {
      return _measures.last();
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Score::midiChannel(int idx) const
      {
      return _midiMapping[idx].channel;
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
//   deselect
//---------------------------------------------------------

void Score::deselect(Element*)
      {

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
      printf("Score::findSpanner() id %d not found\n", id);
      return 0;
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
//   sigmap
//---------------------------------------------------------

TimeSigMap* Score::sigmap() const
      {
      const Score* score = this;
      while (score->parentScore())
            score = parentScore();
      return score->_sigmap;
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
//   customKeySig
//---------------------------------------------------------

KeySig* Score::customKeySig(int idx) const
      {
      return customKeysigs[idx];
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
                                    tempomap()->addTempo(tt->segment()->tick(), tt->tempo());
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
                        tempomap()->addPause(m->tick() + m->ticks(), m->pause());

                  //
                  // implement fermata as a tempo change
                  //
                  SegmentTypes st = SegChordRest | SegBreath;

                  for (Segment* s = m->first(st); s; s = s->next(st)) {
                        if (s->subtype() == SegBreath) {
                              tempomap()->addPause(s->tick(), .1);
                              continue;
                              }
                        foreach(Element* e, s->elist()) {
                              if (!e)
                                    continue;
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              qreal stretch = -1.0;
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
                                    qreal otempo = tempomap()->tempo(cr->tick());
                                    qreal ntempo = otempo / stretch;
                                    tempomap()->addTempo(cr->tick(), ntempo);
                                    tempomap()->addTempo(cr->tick() + cr->ticks(), otempo);
                                    break;      // do not consider more staves/voices
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
//   repeatList
//---------------------------------------------------------

RepeatList* Score::repeatList()  const
      {
      const Score* score = this;
      while (score->parentScore())
            score = parentScore();
      return score->_repeatList;
      }

//---------------------------------------------------------
//   tempomap
//---------------------------------------------------------

AL::TempoMap* Score::tempomap() const
      {
      const Score* score = this;
      while (score->parentScore())
            score = parentScore();
      return score->_tempomap;
      }

//---------------------------------------------------------
//   pos2sel
//---------------------------------------------------------

int Measure::snap(int tick, const QPointF p) const
      {
      Segment* s = _first;
      for (; s->next(); s = s->next()) {
            qreal x  = s->x();
            qreal dx = s->next()->x() - x;
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
            qreal x  = s->x();
            qreal nx = x + (ns->x() - x) * .5;
            if (p.x() < nx)
                  break;
            s = ns;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   ImagePath
//---------------------------------------------------------

ImagePath::ImagePath(const QString& p)
   : _path(p), _references(0), _loaded(false)
      {
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
//   setInputTrack
//---------------------------------------------------------

void Score::setInputTrack(int v)
      {
      if (v < 0) {
            printf("setInputTrack: bad value: %d\n", v);
            return;
            }
      }

//---------------------------------------------------------
//   setSpatium
//---------------------------------------------------------

void Score::setSpatium(qreal v)
      {
      _spatium = v;
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Score::readStaff(XmlReader* r)
      {
      MeasureBase* mb = first();
      int staff = 0;
      while (r->readAttribute()) {
            if (r->tag() == "id")
                  staff = r->intValue() - 1;
            }
      curTick  = 0;
      curTrack = staff * VOICES;

      while (r->readElement()) {
            MString8 tag(r->tag());
            if (tag == "Measure") {
                  Measure* measure = 0;
                  if (staff == 0) {
                        measure = new Measure(this);
                        measure->setTick(curTick);
                        add(measure);
                        //
                        // inherit timesig from previous measure
                        //
                        Measure* m = measure->prevMeasure();
                        Fraction f(m ? m->timesig() : Fraction(4,4));
                        measure->setLen(f);
                        measure->setTimesig(f);
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
                  measure->read(r, staff);
                  curTick = measure->tick() + measure->ticks();
                  }
            else if ((tag == "HBox") || (tag == "VBox") || (tag == "TBox") || (tag == "FBox")) {
                  QString s = tag.string();
                  MeasureBase* mb = static_cast<MeasureBase*>(Element::name2Element(s, this));
                  mb->read(r);
                  mb->setTick(curTick);
                  add(mb);
                  }
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   updateNotes
///   calculate note lines and accidental
//---------------------------------------------------------

void Score::updateNotes()
      {
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  KeySigEvent key = staff(staffIdx)->keymap()->key(m->tick());

                  char tversatz[75];      // list of already set accidentals for this measure
                  initLineList(tversatz, key.accidentalType());

                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        if (!(segment->subtype() & (SegChordRest | SegGrace)))
                              continue;
                        m->layoutChords10(segment, staffIdx * VOICES, tversatz);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   utick2utime
//---------------------------------------------------------

qreal Score::utick2utime(int tick) const
      {
      return repeatList()->utick2utime(tick);
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

MRect Score::moveCursor(Segment* segment)
      {
      MRect mr;
      mr.w = 0.0;

      if (cursorSegment == segment)
            return mr;
      cursorSegment = segment;

      System* system = segment->measure()->system();
      if (system == 0)
            return mr;

      qreal x        = segment->canvasPos().x();
      qreal y        = system->staffY(0);
      qreal _spatium = _cursor->spatium();

      qreal w;
      qreal h  = 8 * _spatium;
      //
      // set cursor height for whole system
      //
      qreal y2 = 0.0;
      for (int i = 0; i < nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            y2 = ss->y();
            }
      h += y2;
      w  = _spatium * 4.0;
      x -= _spatium;
      y -= 2*_spatium;
      _cursor->setPos(x, y);
      QRectF r(0.0, 0.0, w, h);
      _cursor->setbbox(r);

      mr.x = x;
      mr.y = y;
      mr.w = w;
      mr.h = h;
      return mr;
      }

//---------------------------------------------------------
//   showCursor
//---------------------------------------------------------

void Score::showCursor(bool val)
      {
      _cursor->setVisible(val);
      }

//---------------------------------------------------------
//   pageWidth
//---------------------------------------------------------

qreal Score::pageWidth() const
      {
      return _pageFormat->width() * DPI;
      }

//---------------------------------------------------------
//   pageHeight
//---------------------------------------------------------

qreal Score::pageHeight() const
      {
      return _pageFormat->height() * DPI;
      }

//---------------------------------------------------------
//   pageIdx
//---------------------------------------------------------

int Score::pageIdx(Page* page) const
      {
      return _pages.indexOf(page);
      }

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int Score::utime2utick(qreal utime) const
      {
      return repeatList()->utime2utick(utime);
      }


