//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: score.cpp,v 1.24 2006/04/12 14:58:10 wschweer Exp $
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
#include "sig.h"
#include "clef.h"
#include "tempo.h"
#include "measure.h"
#include "page.h"
#include "undo.h"
#include "system.h"
#include "select.h"
#include "mscore.h"
#include "canvas.h"
#include "segment.h"
#include "xml.h"
#include "text.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "padstate.h"
#include "slur.h"
#include "seq.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "layout.h"
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
#include "mididriver.h"
#include "excerpt.h"
#include "stafftext.h"

Score* gscore;                 ///< system score, used for palettes etc.

QPoint scorePos(0,0);
QSize  scoreSize(950, 500);

MuseScore* mscore;
bool debugMode       = false;
bool layoutDebug     = false;
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
      foreach(Element* e, *nb->el())
            e->setParent(nb);
      }

#if 0
//---------------------------------------------------------
//   doLayout
//---------------------------------------------------------

void Score::doLayout()
      {
      _layout->doLayout();
      }
#endif

//---------------------------------------------------------
//   pageFormat
//---------------------------------------------------------

PageFormat* Score::pageFormat() const
      {
      return _layout->pageFormat();
      }

//---------------------------------------------------------
//   setSpatium
//---------------------------------------------------------

void Score::setSpatium(double v)
      {
      _layout->setSpatium(v);
      _spatium = v;
      }

//---------------------------------------------------------
//   needLayout
//---------------------------------------------------------

bool Score::needLayout() const
      {
      return _layout->needLayout();
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
      {
      info.setFile("");
      _layout           = new ScoreLayout(this);
      _style            = new Style(defaultStyle);

      // deep copy of defaultTextStyles:
      for (int i = 0; i < TEXT_STYLES; ++i)
            _textStyles.append(new TextStyle(defaultTextStyles[i]));

      tempomap          = new TempoList;
      sigmap            = new SigList;
      sel               = new Selection(this);
      _dirty            = false;
      _saved            = false;
      sel->setState(SEL_NONE);
      editObject        = 0;
      origDragObject    = 0;
      _dragObject       = 0;
      keyState          = 0;
      editTempo         = 0;
      updateAll         = false;
      _pageOffset       = 0;
      _fileDivision     = division;
      _printing         = false;
      cmdActive         = false;
      _playlistDirty    = false;
      rights            = 0;
      clear();
      }

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
      {
      if (rights)
            delete rights;
      delete tempomap;
      delete sigmap;
      delete sel;
      delete _layout;
      delete _style;
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void Score::setStyle(const Style& s)
      {
      delete _style;
      _style = new Style(s);
      }

//---------------------------------------------------------
//   addViewer
//---------------------------------------------------------

void Score::addViewer(Viewer* v)
      {
      viewer.push_back(v);
      }

//---------------------------------------------------------
//   clearViewer
//---------------------------------------------------------

void Score::clearViewer()
      {
      viewer.clear();
      }

//---------------------------------------------------------
//   canvas
//---------------------------------------------------------

Canvas* Score::canvas() const
      {
      return  mscore->getCanvas();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Score::clear()
      {
      foreach(Excerpt* e, _excerpts)
            delete e;
      _excerpts.clear();

      _padState.pitch = 60;
      info.setFile("");
      _dirty          = false;
      _saved          = false;
      _created        = false;
      if (rights)
            delete rights;
      rights          = 0;
      movementNumber  = "";
      movementTitle   = "";
      _pageOffset     = 0;
      _playPos        = 0;

      foreach(Staff* staff, _staves)
            delete staff;
      _staves.clear();
      _measures.clear();
      foreach(Part* p, _parts)
            delete p;
      _parts.clear();

      sigmap->clear();
      sigmap->add(0, 4, 4);
      tempomap->clear();
      _layout->systems()->clear();
      _layout->pages().clear();
      sel->clear();
      _showInvisible = true;
      }

//---------------------------------------------------------
//   renumberMeasures
//---------------------------------------------------------

void Score::renumberMeasures()
      {
      int measureNo = 0;
      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* measure = (Measure*)mb;
            measure->setNo(measureNo);
            if (!measure->irregular())
                  ++measureNo;
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

/**
 Import file \a name.
 */

void Score::read(QString name)
      {
      _mscVersion = MSCVERSION;
      _saved = false;
      info.setFile(name);

      QString cs = info.completeSuffix();

      if (cs == "xml") {
            importMusicXml(name);
            connectSlurs();
            }
      else if (cs == "mxl")
            importCompressedMusicXml(name);
      else if (cs.toLower() == "mid" || cs.toLower() == "kar") {
            if (!importMidi(name))
                  return;
            }
      else if (cs == "md") {
            if (!importMuseData(name))
                  return;
            }
      else if (cs == "ly") {
            if (!importLilypond(name))
                  return;
            }
      else if (cs.toLower() == "mgu" || cs.toLower() == "sgu") {
            if (!importBB(name))
                  return;
            }
      else {
            loadMsc(name);
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
      _layout->doLayout();
      layoutAll = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml)
      {
      xml.tag("Spatium", _spatium / DPMM);
      xml.tag("Division", division);
      xml.curTrack = -1;
      if (editObject) {                          // in edit mode?
            endCmd();
            canvas()->setState(Canvas::NORMAL);  //calls endEdit()
            }
      _style->save(xml);
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (*_textStyles[i] != defaultTextStyleArray[i])
                  _textStyles[i]->write(xml);
            }
      xml.tag("showInvisible", _showInvisible);
      pageFormat()->write(xml);
      if (rights) {
            xml.stag("rights");
            xml << rights->toHtml("UTF-8") << '\n';
            xml.etag();
            }
      if (!movementNumber.isEmpty())
            xml.tag("movement-number", movementNumber);
      if (!movementTitle.isEmpty())
            xml.tag("movement-title", movementTitle);

      sigmap->write(xml);
      tempomap->write(xml);
      foreach(const Part* part, _parts)
            part->write(xml);
      foreach(const Excerpt* excerpt, _excerpts)
            excerpt->write(xml);

      // to serialize slurs, the get an id; this id is referenced
      // in begin-end elements
      int slurId = 0;
      foreach(Element* el, _gel) {
            if (el->type() == SLUR)
                  ((Slur*)el)->setId(slurId++);
            }
      foreach(Element* el, _gel)
            el->write(xml);
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1));
            xml.curTick  = 0;
            xml.curTrack = staffIdx * VOICES;
            for (MeasureBase* m = _layout->first(); m; m = m->next()) {
                  if (m->type() == MEASURE || staffIdx == 0)
                        m->write(xml, staffIdx, staffIdx == 0);
                  if (m->type() == MEASURE)
                        xml.curTick = m->tick() + sigmap->ticksMeasure(m->tick());
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
      int tick    = m->tick();
      MeasureBase* im = tick2measureBase(tick);
      m->setNext(im);
      _layout->add(m);
      }

//---------------------------------------------------------
//   removeMeasure
//---------------------------------------------------------

void Score::removeMeasure(MeasureBase* im)
      {
      _layout->remove(im);
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
            tempomap->removeTime(tick, len);
            foreach(Staff* staff, _staves) {
                  staff->clef()->removeTime(tick, len);
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
            tempomap->insertTime(tick, len);
            foreach(Staff* staff, _staves) {
                  staff->clef()->insertTime(tick, len);
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
      int bar  = 0;
      int tick = 0;
      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE) {
                  mb->setTick(tick);
                  continue;
                  }
            Measure* m = (Measure*)mb;
            if (m->no() != bar)
                  m->setNo(bar);
            if (!m->irregular())
                  ++bar;
            int mtick = m->tick();
            int diff  = tick - mtick;
// printf("move %d  -  soll %d  ist %d  len %d\n", bar, tick, mtick, sigmap->ticksMeasure(tick));
            tick     += sigmap->ticksMeasure(tick);
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
      int voice = 0;
      foreach (const Page* page, _layout->pages()) {
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
                        Measure* m = (Measure*)mb;
                        double sy1 = 0;
                        if (rst && *rst == -1) {
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
                                    for (Segment* segment = m->first(); segment; segment = segment->next()) {
                                          if (segment->subtype() != Segment::SegChordRest || (segment->element(track) == 0 && voice == 0)) {
                                                continue;
                                                }
                                          Segment* ns = segment->next();
                                          for (; ns; ns = ns->next()) {
                                                if (ns->subtype() == Segment::SegChordRest && (ns->element(track) || voice))
                                                      break;
                                                }
                                          if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                                                i -= 1;
                                                *rst = i;
                                                if (tick)
                                                      *tick = segment->tick();
                                                if (pitch) {
                                                      Staff* s = _staves[i];
                                                      int clef = s->clef()->clef(*tick);
                                                      *pitch = y2pitch(pppp.y()-staff->bbox().y(), clef);
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
                                    if (segment->subtype() != Segment::SegChordRest)
                                          continue;
                                    Segment* ns = segment->next();
                                    while (ns && ns->subtype() != Segment::SegChordRest)
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
                                          int clef = staff(*rst)->clef()->clef(*tick);
                                          *pitch = y2pitch(pppp.y(), clef);
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
      int voice = _padState.voice;

      foreach(const Page* page, _layout->pages()) {
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
                        Measure* m = (Measure*)mb;
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
                                    if (segment->subtype() != Segment::SegChordRest || (segment->element(track) == 0 && voice == 0)) {
                                          segment = segment->next();
                                          continue;
                                          }
                                    Segment* ns = segment->next();
                                    for (; ns; ns = ns->next()) {
                                          if (ns->subtype() == Segment::SegChordRest && (ns->element(track) || voice))
                                                break;
                                          }
                                    if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                                          i     -= 1;
                                          *rst   = i;
                                          *tick  = segment->tick();
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
      foreach(const Page* page, _layout->pages()) {
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
      MeasureBase* mb = _layout->first();
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
                        _layout->add(measure);
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
                              _layout->add(measure);
                              }
                        }
                  measure->read(e, staff);
                  curTick = measure->tick() + measure->tickLen();
                  }
            else if (tag == "HBox") {
                  HBox* hbox = new HBox(this);
                  hbox->setTick(curTick);
                  hbox->read(e);
                  _layout->add(hbox);
                  }
            else if (tag == "VBox") {
                  VBox* vbox = new VBox(this);
                  vbox->setTick(curTick);
                  vbox->read(e);
                  _layout->add(vbox);
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
                  x += dx / 2.0;
            if (p.x() < x)
                  break;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Score::startEdit(Element* element)
      {
      origEditObject = element;
      editObject     = element->clone();
      editObject->setSelected(false);
      origEditObject->resetMode();
      undoChangeElement(origEditObject, editObject);
      select(editObject, 0, 0);
      updateAll = true;
      end();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Score::endEdit()
      {
      refresh |= editObject->bbox();
      editObject->endEdit();
      refresh |= editObject->bbox();

      if (editObject->type() == TEXT) {
            Text* in = (Text*)editObject;
            Part* p  = part(in->staffIdx());
            if (editObject->subtype() == TEXT_INSTRUMENT_SHORT) {
                  p->setShortName(*(in->doc()));
                  _layout->setInstrumentNames();
                  }
            else if (editObject->subtype() == TEXT_INSTRUMENT_LONG) {
                  p->setLongName(*(in->doc()));
                  _layout->setInstrumentNames();
                  }
            }
      else if (editObject->type() == LYRICS) {
            //
            //  special handling of Lyrics
            //
            Lyrics* lyrics = (Lyrics*)editObject;
            Lyrics* origL  = (Lyrics*)origEditObject;

            // search previous lyric:
            int verse    = lyrics->no();
            Segment* seg = (Segment*)lyrics->parent();
            int staffIdx = lyrics->staffIdx();
            Lyrics* nl = 0;
            while (seg->prev1()) {
                  seg = seg->prev1();
                  LyricsList* nll = seg->lyricsList(staffIdx);
                  if (!nll)
                        continue;
                  nl = nll->value(verse);
                  if (nl)
                        break;
                  }

            if (lyrics->isEmpty() && origL->isEmpty()) {
                  Measure* measure = (Measure*)(lyrics->parent());
                  measure->remove(lyrics);
                  }
            else {
                  if (nl && nl->syllabic() == Lyrics::END) {
                        if (nl->endTick() >= lyrics->tick())
                              nl->setEndTick(0);
                        }
                  }
            }
      layoutAll = true;
      mscore->setState(STATE_NORMAL);
      editObject = 0;
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void Score::startDrag()
      {
      origDragObject  = _dragObject;
      _dragObject     = _dragObject->clone();
      undoChangeElement(origDragObject, _dragObject);
      sel->clear();
      sel->add(_dragObject);
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

void Score::drag(const QPointF& delta)
      {
      foreach(Element* e, *sel->elements())
            refresh |= e->drag(delta);
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void Score::endDrag()
      {
      _dragObject->endDrag();
      if (origDragObject) {
            sel->clear();
            sel->add(_dragObject);
            origDragObject = 0;
            }
      layoutAll = true;
      _dragObject = 0;
      }

//---------------------------------------------------------
//   setNoteEntry
//---------------------------------------------------------

/**
 Switch note entry mode
*/

ChordRest* Score::setNoteEntry(bool val, bool step)
      {
      ChordRest* cr = 0;
      if (val) {
            Element* el = sel->element();
            if (noteEntryMode()) {     // already in note entry mode
                  if (el) {
                        if (el->type() == NOTE)
                              el = el->parent();
                        if (!el->isChordRest())
                              return 0;
                        cr = (ChordRest*)el;
                        if (cr->tick() == _is.pos) {
                              // int len = cr->tuplet() ? cr->tuplet()->noteLen() : cr->tickLen();
                              _is.pos += cr->tickLen();
                              }
                        }
                  return cr;
                  }
            if (sel->state() == SEL_NONE || (el && el->type() != NOTE && !el->isChordRest())) {
                  QMessageBox::information(0, "MuseScore: Note Entry",
                        tr("No note or rest selected:\n"
                           "please select a note or rest were you want to\n"
                           "start note entry"));
                  return 0;
                  }
            if (el == 0) {
                  // int tick = sel->tickStart;
                  Segment* seg = tick2segment(sel->tickStart);
                  if (seg == 0) {
                        printf("no note or rest selected 1\n");
                        return 0;
                        }
                  int staffIdx = sel->staffStart;
                  for (int track = staffIdx * VOICES; track < (staffIdx+1)*VOICES; ++track) {
                        el = seg->element(track);
                        if (el)
                              break;
                        }
                  if (el == 0) {
                        printf("no note or rest selected 2\n");
                        return 0;
                        }
                  select(el, 0, 0);
                  }
            if (el->type() == NOTE)
                  el = el->parent();
            cr = (ChordRest*)el;
            _is.pos = cr->tick();
            if (step) {
                  if (cr->tuplet())
                        _is.pos += cr->tuplet()->noteLen();
                  else
                        _is.pos += cr->tickLen();
                  }
            _is.noteEntryMode = true;
            canvas()->moveCursor();
            }
      else {
            _padState.len     = 0;
            _is.pos           = -1;
            _is.noteEntryMode = false;
            if (_is.slur) {
                  QList<SlurSegment*>* el = _is.slur->slurSegments();
                  if (!el->isEmpty())
                        el->front()->setSelected(false);
                  ((ChordRest*)_is.slur->startElement())->addSlurFor(_is.slur);
                  ((ChordRest*)_is.slur->endElement())->addSlurBack(_is.slur);
                  _is.slur = 0;
                  }
            setPadState();
            }
      canvas()->setState(_is.noteEntryMode ? Canvas::NOTE_ENTRY : Canvas::NORMAL);
      mscore->setState(_is.noteEntryMode ? STATE_NOTE_ENTRY : STATE_NORMAL);
      return cr;
      }

//---------------------------------------------------------
//   midiReceived
//---------------------------------------------------------

void Score::midiReceived()
      {
#ifdef USE_ALSA
      midiDriver->read();
#endif
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void Score::midiNoteReceived(int pitch, bool chord)
      {
      startCmd();
      if (!noteEntryMode())
            setNoteEntry(true, false);
      if (noteEntryMode()) {
            int len = _padState.tickLen;
            if (chord) {
                  Note* on = getSelectedNote();
                  Note* n = addNote(on->chord(), pitch);
                  select(n, 0, 0);
                  }
            else {
                  setNote(_is.pos, _is.track, pitch, len);
                  _is.pos += len;
                  }
            }
      layoutAll = true;
      endCmd();
      }

#if 0
//---------------------------------------------------------
//   snapNote
//    p - absolute position
//---------------------------------------------------------

int Score::snapNote(int tick, const QPointF p, int staff) const
      {
      foreach(const Page* page, _layout->pages()) {
            if (!page->contains(p))
                  continue;
            QPointF rp = p - page->pos();  // transform to page relative
            const QList<System*>* sl = page->systems();
            double y = 0.0;
            for (ciSystem is = sl->begin(); is != sl->end(); ++is) {
                  System* system = *is;
                  ciSystem nis   = is;
                  ++nis;
                  if (nis == sl->end()) {
                        return system->snapNote(tick, rp - system->pos(), staff);
                        }
                  System* nsystem = *nis;
                  double nexty = nsystem->y();
                  double gap = nexty - (system->y() + system->height());
                  nexty -= gap/2.0;
                  if (p.y() >= y && p.y() < nexty) {
                        return system->snapNote(tick, rp - system->pos(), staff);
                        }
                  y = nexty;
                  }
            }
      printf("snapNote: nothing found\n");
      return tick;
      }
#endif

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
            double nx = x + (ns->x() - x) / 2;
#if 0
            if (s->tick() == tick)
                  x += dx / 3.0 * 2.0;
            else  if (ns->tick() == tick)
                  x += dx / 3.0;
            else
                  x += dx / 2.0;
#endif
            if (p.x() < nx)
                  break;
            s = ns;
            }
      return s->tick();
      }

//---------------------------------------------------------
//   undoEmpty
//---------------------------------------------------------

bool Score::undoEmpty() const
      {
      return undoList.empty();
      }

//---------------------------------------------------------
//   redoEmpty
//---------------------------------------------------------

bool Score::redoEmpty() const
      {
      return redoList.empty();
      }

//---------------------------------------------------------
//   setShowInvisible
//---------------------------------------------------------

void Score::setShowInvisible(bool v)
      {
      start();
      _showInvisible = v;
      updateAll      = true;
      layoutAll      = false;
      end();
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void Score::setDirty(bool val)
      {
      _dirty = val;
      if (val)
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
//   adjustTime
//    change all time positions starting with measure
//    according to new start time
//---------------------------------------------------------

void Score::adjustTime(int tick, MeasureBase* m)
      {
      int delta = tick - m->tick();
      if (delta == 0)
            return;
      while (m) {
            m->moveTicks(delta);
            tick += m->tickLen();
            m = m->next();
            }
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
      Segment* seg;
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
            for(MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;
                  for (Segment* s = m->first(); s; s = s->next()) {
                        int strack = i * VOICES;
                        int etrack = strack + VOICES;
                        for (int track = strack; track < etrack; ++track) {
                              Element* e = s->element(track);
                              if (e && e->type() == CHORD) {
                                    Chord* chord = (Chord*) e;
                                    const NoteList* nl = chord->noteList();
                                    for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                                          Note* note = in->second;
                                          notes.append(note);
                                          }
                                    }
                              }
                        }
                  }
            ::spell(notes);
            }
      }

//---------------------------------------------------------
//   prevNote
//---------------------------------------------------------

Note* prevNote(Note* n)
      {
      Chord* chord = n->chord();
      Segment* seg = chord->segment();
      NoteList* nl = chord->noteList();
      ciNote i = nl->std::multimap<const int, Note*>::find(n->pitch());
      if (i != nl->begin()) {
            --i;
            return i->second;
            }
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() - 1;
      int endTrack   = 0;
      while (seg) {
            if (seg->subtype() == Segment::SegChordRest) {
                  for (int track = startTrack; track >= endTrack; --track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == CHORD)
                              return ((Chord*)e)->upNote();
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
      NoteList* nl = chord->noteList();
      ciNote i = nl->std::multimap<const int, Note*>::find(n->pitch());
      ++i;
      if (i != nl->end())
            return i->second;
      Segment* seg   = chord->segment();
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() + 1;
      int endTrack   = staff * VOICES + VOICES;
      while (seg) {
            if (seg->subtype() == Segment::SegChordRest) {
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

void Score::setTextStyles(QVector<TextStyle*>& s)
      {
      foreach(TextStyle* ts, _textStyles)
            delete ts;
      _textStyles.clear();
      foreach(TextStyle* ts, s)
            _textStyles.append(new TextStyle(*ts));
      }

//---------------------------------------------------------
//   setCopyright
//---------------------------------------------------------

void Score::setCopyright(QTextDocument* doc)
      {
      if (rights) {
            delete rights;
            rights = 0;
            }
      if (doc)
            rights = doc->clone();
      }

void Score::setCopyright(const QString& s)
      {
      if (rights == 0) {
            rights = new QTextDocument(0);
            rights->setUseDesignMetrics(true);
            }
      rights->setPlainText(s);
      }

//---------------------------------------------------------
//   setCopyrightHtml
//---------------------------------------------------------

void Score::setCopyrightHtml(const QString& s)
      {
      if (rights == 0)
            rights = new QTextDocument(0);
      rights->setHtml(s);
      }

//---------------------------------------------------------
//   isVolta
//---------------------------------------------------------

bool Score::isVolta(int tick, int repeat) const
      {
      foreach(const Element* el, *gel()) {
            if (el->type() == VOLTA) {
                  const Volta* volta = (Volta*)el;
                  if (tick >= volta->tick() && tick < volta->tick2()) {
                        foreach(int ending, volta->endings()) {
                              if (ending == repeat)
                                    return true;
                              }
                        return false;
                        }
                  }
            }
      return true;      // return true if no volta found
      }

//---------------------------------------------------------
//   collectChord
//---------------------------------------------------------

void Score::collectChord(EventMap* events, int channel, int port,
   int pitchOffset, Chord* chord, int tick, int len)
      {
      NoteList* nl = chord->noteList();
      for (iNote in = nl->begin(); in != nl->end(); ++in) {
            Note* note = in->second;
            if (note->hidden())       // do not play overlapping notes
                  continue;
            if (note->tieBack())
                  continue;
            NoteOn* ev = new NoteOn();
            int pitch = note->pitch() + pitchOffset;
            if (pitch > 127)
                  pitch = 127;
            ev->setPitch(pitch);
            ev->setVelo(60);
            ev->setNote(note);
            ev->setChannel(channel);
            ev->setPort(port);
            events->insertMulti(tick, ev);

            ev = new NoteOn();
            ev->setPitch(pitch);
            ev->setVelo(0);
            ev->setNote(note);
            ev->setChannel(channel);
            ev->setPort(port);
            events->insertMulti(tick + len, ev);
            }
      }

//---------------------------------------------------------
//   collectMeasureEvents
//---------------------------------------------------------

void Score::collectMeasureEvents(EventMap* events, Measure* m, int staffIdx, int tickOffset)
      {
      Part* prt       = part(staffIdx);
      int pitchOffset = prt->pitchOffset();
      int channel     = prt->midiChannel();
      int port        = prt->midiPort();

      QList<Chord*> lv;       // appoggiatura
      QList<Chord*> sv;       // acciaccatura

      for (int voice = 0; voice < VOICES; ++voice) {
            int track = staffIdx * VOICES + voice;
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  Element* el = seg->element(track);
                  if (!el || el->type() != CHORD)
                        continue;
                  Chord* chord = (Chord*)el;
                  if (chord->noteType() != NOTE_NORMAL) {
                        if (chord->noteType() == NOTE_ACCIACCATURA)
                              sv.append(chord);
                        else if (chord->noteType() == NOTE_APPOGGIATURA)
                              lv.append(chord);
                        continue;
                        }

                  int gateTime = _style->gateTime;  // 100 - legato (100%)

                  int tick        = chord->tick();
                  int ottavaShift = 0;
                  foreach(Element* e, *m->score()->gel()) {
                        if (e->type() == OTTAVA) {
                              Ottava* ottava = (Ottava*)e;
                              int tick1 = ottava->tick();
                              int tick2 = ottava->tick2();
                              if (tick >= tick1 && tick < tick2) {
                                    ottavaShift = ottava->pitchShift();
                                    }
                              }
                        else if (e->type() == SLUR) {
                              Slur* slur = (Slur*)e;
                              int tick1 = slur->tick();
                              int tick2 = slur->tick2();
                              if (tick >= tick1 && tick < tick2 && slur->track() == track) {
                                    gateTime = _style->slurGateTime;
                                    }

                              }
                        }
                  foreach(NoteAttribute* a, *chord->getAttributes()) {
                        switch(a->subtype()) {
                              case TenutoSym:
                                    gateTime = _style->tenutoGateTime;
                                    break;
                              case StaccatoSym:
                                    gateTime = _style->staccatoGateTime;
                                    break;
                              default:
                                    break;
                              }
                        }

                  // compute len of chord

                  int len = chord->tickLen();
                  Note* note = chord->noteList()->front();
                  if (note->tieBack())
                        continue;
                  while (note->tieFor()) {
                        if (note->tieFor()->endNote() == 0)
                              break;
                        len += note->chord()->tickLen();
                        note = note->tieFor()->endNote();
                        }

                  if (!sv.isEmpty()) {
                        //
                        // move acciaccatura's in front of
                        // main note
                        //
                        int sl = len / 4;
                        int ssl = len / (2 * sv.size());
                        foreach(Chord* c, sv) {
                              collectChord(events, channel, port,
                                 pitchOffset + ottavaShift,
                                 c,
                                 tick + tickOffset - sl,
                                 ssl * gateTime / 100
                                 );
                              sl -= ssl;
                              }
                        }
                  else if (!lv.isEmpty()) {
                        //
                        // appoggiatura's use time from main note
                        //
                        int sl = 0;
                        foreach(Chord* c, lv) {
                              int ssl = c->tickLen();
                              collectChord(events, channel, port,
                                 pitchOffset + ottavaShift,
                                 c,
                                 tick + tickOffset + sl,
                                 ssl * gateTime / 100
                                 );
                              sl += ssl;
                              len -= ssl;
                              }
                        }
                  collectChord(events,
                     channel,
                     port,
                     pitchOffset + ottavaShift,
                     chord, tick + tickOffset,
                     (len * gateTime) / 100
                     );
                  lv.clear();
                  sv.clear();
                  }
            }
      //
      // collect program changes and controller
      //
      foreach(const Element* e, *m->el()) {
            if (e->type() != STAFF_TEXT || e->staffIdx() != staffIdx)
                  continue;
            StaffText* st = (StaffText*)e;
            MidiAction a;
            if (!st->instrumentActionName().isEmpty()) {
                  a = prt->instrument()->midiAction(st->instrumentActionName());
                  }
            else
                  a = st->midiAction();
            int hb, lb, pr, c, v;
            int tick = st->tick();
            if (a.programChange(&hb, &lb, &pr)) {
                  int v = ((hb & 0xff) << 16) + ((lb & 0xff) << 8) + (pr & 0xff);
                  ControllerEvent* ev = new ControllerEvent(tick, channel, CTRL_PROGRAM, v);
                  ev->setPort(port);
                  events->insertMulti(tick, ev);
                  }
            else if (a.controller(&c, &v)) {
                  ControllerEvent* ev = new ControllerEvent(tick, channel, c, v);
                  events->insertMulti(tick, ev);
                  }
            }
      }

//---------------------------------------------------------
//   toEList
//    export score to event list
//---------------------------------------------------------

void Score::toEList(EventMap* events, int offset)
      {
      bool expandRepeats = getAction("repeat")->isChecked();
      int staffIdx   = 0;
      foreach(Part* part, _parts) {
            for (int i = 0; i < part->staves()->size(); ++i) {
                  toEList(events, expandRepeats, offset, staffIdx);
                  ++staffIdx;
                  }
            }
      }

//---------------------------------------------------------
//   RepeatLoop
//---------------------------------------------------------

struct RepeatLoop {
      enum LoopType { LOOP_REPEAT, LOOP_JUMP };

      LoopType type;
      MeasureBase* m;   // start measure of LOOP_REPEAT
      int count;
      QString stop, cont;

      RepeatLoop() {}
      RepeatLoop(MeasureBase* _m)  {
            m     = _m;
            count = 0;
            type  = LOOP_REPEAT;
            }
      RepeatLoop(const QString s, const QString c)
         : stop(s), cont(c)
            {
            type = LOOP_JUMP;
            }
      };

//---------------------------------------------------------
//   searchLabel
//---------------------------------------------------------

MeasureBase* Score::searchLabel(const QString& s, MeasureBase* start)
      {
      if (s == "start")
            return layout()->first();
      else if (s == "end")
            ;
      if (start == 0)
            start = layout()->first();
      for (MeasureBase* m = start; m; m = m->next()) {
            foreach(const Element* e, *m->el()) {
                  if (e->type() == MARKER) {
                        Marker* marker = (Marker*)e;
                        if (marker->label() == s)
                              return m;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   toEList
//    implements:
//          - repeats
//          - volta
//          - d.c. al fine
//          - d.s. al fine
//          - d.s. al coda
//---------------------------------------------------------

void Score::toEList(EventMap* events, bool expandRepeats, int offset, int staffIdx)
      {
      if (!expandRepeats) {
            for (MeasureBase* mb = layout()->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  collectMeasureEvents(events, (Measure*)mb, staffIdx, offset);
                  }
            return;
            }
      QStack<RepeatLoop> rstack;
      int tickOffset = 0;
      int overallRepeatCount = 0;

      MeasureBase* fm = layout()->first();
      for (MeasureBase* mb = fm; mb;) {
            if (mb->type() != MEASURE) {
                  mb = mb->next();
                  continue;
                  }
            Measure* m = (Measure*)mb;
            if (
                  (m->repeatFlags() & RepeatStart)
               && (rstack.isEmpty() || (rstack.top().m != m))
               && (rstack.isEmpty() || (rstack.top().type != RepeatLoop::LOOP_JUMP))
               )
                  rstack.push(RepeatLoop(m));

            if (!rstack.isEmpty() && !isVolta(m->tick(), rstack.top().count + 1))
                  tickOffset -= m->tickLen();   // skip this measure
            else
                  collectMeasureEvents(events, m, staffIdx, tickOffset + offset);

            if (rstack.isEmpty()) {
                  // Jumps are only accepted outside of other repeats
                  if (m->repeatFlags() & RepeatJump) {
                        Jump* s = 0;
                        foreach(Element* e, *m->el()) {
                              if (e->type() == JUMP) {
                                    s = (Jump*)e;
                                    break;
                                    }
                              }
                        if (s) {
                              const QString& jumpTo = s->jumpTo();
                              MeasureBase* nmb = searchLabel(jumpTo);
                              if (nmb) {
                                    rstack.push(RepeatLoop(s->playUntil(), s->continueAt()));
                                    tickOffset += m->tick() + m->tickLen() - nmb->tick();
                                    mb = nmb;
                                    continue;
                                    }
                              else
                                    printf("JUMP: label <%s> not found\n",
                                       qPrintable(jumpTo));
                              }
                        else
                              printf("Jump not found\n");
                        }
                  else if (m->repeatFlags() & RepeatEnd) {
                        // this is a end repeat without start repeat:
                        //    repeat from beginning
                        ++overallRepeatCount;
                        if (overallRepeatCount < m->repeatCount()) {
                              mb = layout()->first();
                              tickOffset += m->tick() + m->tickLen() - mb->tick();
                              continue;
                              }
                        else
                              overallRepeatCount = 0;
                        }
                  }
            else if (rstack.top().type == RepeatLoop::LOOP_REPEAT) {
                  if (m->repeatFlags() & RepeatEnd) {
                        //
                        // increment repeat count
                        //
                        if (++rstack.top().count < m->repeatCount()) {
                              //
                              // goto start of loop, fix tickOffset
                              //
                              mb = rstack.top().m;
                              tickOffset += m->tick() + m->tickLen() - mb->tick();
                              continue;
                              }
                        rstack.pop();     // end this loop
                        }
                  }
            else if (rstack.top().type == RepeatLoop::LOOP_JUMP) {
                  MeasureBase* m = searchLabel(rstack.top().stop);
                  if (m == 0)
                        printf("LOOP_JUMP: label not found\n");
                  if (m == mb) {
                        if (m->next() == 0)
                              break;
                        MeasureBase* nmb = searchLabel(rstack.top().cont, m->next());
                        if (nmb)
                              tickOffset += m->tick() + m->tickLen() - nmb->tick();
                        else if (!rstack.top().cont.isEmpty())
                              printf("Cont label not found: <%s>\n", qPrintable(rstack.top().cont));

                        mb = nmb;
                        rstack.pop();     // end this loop
                        continue;
                        }
                  }
            mb = mb->next();
            }
      if (!rstack.isEmpty()) {
            if (rstack.top().type == RepeatLoop::LOOP_JUMP
               && rstack.top().stop == "end")
                  ;
            else
                  printf("repeat stack not empty!\n");
            }
      }

//---------------------------------------------------------
//   setInputTrack
//---------------------------------------------------------

void Score::setInputTrack(int v)
      {
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
      write(xml);
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
      Score* score = new Score;
      docName = "--";
      score->read(doc.documentElement());
      return score;
      }

