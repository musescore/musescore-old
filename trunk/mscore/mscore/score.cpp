//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: score.cpp,v 1.24 2006/04/12 14:58:10 wschweer Exp $
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

/**
 \file
 Implementation of classes ElemList (complete) and Score (partial).
*/

#include "alsa.h"
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
#include "input.h"
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

InputState inputState;
InputState* cis;              ///<  = &inputState;

QPoint scorePos(0,0);
QSize  scoreSize(950, 500);

MuseScore* mscore;
bool debugMode      = false;
bool layoutDebug    = false;
bool noSeq          = false;
bool noMidi         = false;
bool showRubberBand = true;

//---------------------------------------------------------
//   doLayout
//---------------------------------------------------------

void Score::doLayout()
      {
      _layout->doLayout();
      }

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
      ::_spatium = v;
      }

//---------------------------------------------------------
//   pages
//---------------------------------------------------------

PageList* Score::pages() const
      {
      return _layout->pages();
      }

//---------------------------------------------------------
//   systems
//---------------------------------------------------------

SystemList* Score::systems() const
      {
      return _layout->systems();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Score::layout()
      {
      updateAll = true;
      _layout->layout();
      }

//---------------------------------------------------------
//   needLayout
//---------------------------------------------------------

bool Score::needLayout() const
      {
      return _layout->needLayout();
      }

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void ElemList::push_back(Element* e)
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

void ElemList::push_front(Element* e)
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
//   insert
//---------------------------------------------------------

void ElemList::insert(Element* e, Element* el)
      {
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

void ElemList::erase(Element* el)
      {
      --_size;
      if (el->prev())
            el->prev()->setNext(el->next());
      else {
            _first = el->next();
            }
      if (el->next())
            el->next()->setPrev(el->prev());
      else
            _last = el->prev();
      }

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

Score::Score()
      {
      _layout       = new ScoreLayout();
      _layout->setScore(this);
      tempomap          = new TempoList;
      sigmap            = new SigList;
      sel               = new Selection(this);
      _staves           = new StaffList;
      _parts            = new PartList;
      _dirty            = false;
      _saved            = false;
      sel->state        = SEL_NONE;
      editObject        = 0;
      origDragObject    = 0;
      _dragObject       = 0;
      cis               = &inputState;
      cis->pos          = -1;
      cis->staff        = 0;
      cis->voice        = 0;
      keyState          = 0;
      editTempo         = 0;
      updateAll         = false;
      _pageOffset       = 0;
      undoActive        = false;
      _fileDivision     = division;
      clear();
      }

//---------------------------------------------------------
//   ~Score
//---------------------------------------------------------

Score::~Score()
      {
      delete tempomap;
      delete sigmap;
      delete sel;
      delete _staves;
      delete _parts;
      delete _layout;
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
//   moveCursor
//---------------------------------------------------------

void Score::moveCursor()
      {
      for (QList<Viewer*>::iterator i = viewer.begin(); i != viewer.end(); ++i)
            refresh |= (*i)->moveCursor();
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
      info.setFile(tr("untitled.msc"));
      _dirty          = false;
      _saved          = false;
      rights          = "";
      movementNumber  = "";
      movementTitle   = "";
      _pageOffset     = 0;

      for (iStaff i = _staves->begin(); i != _staves->end(); ++i)
            delete *i;
      _staves->clear();
      _layout->clear();
      for (iPart i = _parts->begin(); i != _parts->end(); ++i)
            delete *i;
      _parts->clear();

      sigmap->clear();
      sigmap->add(0, 4, 4);
      tempomap->clear();
      _layout->systems()->clear();
      _layout->pages()->clear();
      sel->clear();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Score::read(QString name)
      {
      _saved = false;
      info.setFile(name);

      if (info.completeSuffix() == "xml") {
            importMusicXml(name);
            }
      else if (info.completeSuffix() == "mid") {
            importMidi(name);
            }
      else {
            loadMsc(name);
            }
      int measureNo = 0;
      for (Element* m = _layout->first(); m; m = m->next()) {
            Measure* measure = (Measure*)m;
            measure->setNo(measureNo);
            if (!measure->irregular())
                  ++measureNo;
            }
      layout();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml)
      {
      if (editObject) {                          // in edit mode?
            endUndo();
            canvas()->setState(Canvas::NORMAL);  //calls endEdit()
            }
      xml.tag("showInvisible", _showInvisible);
      pageFormat()->write(xml);
      if (!rights.isEmpty())
            xml.tag("rights", rights);
      if (!movementNumber.isEmpty())
            xml.tag("movement-number", movementNumber);
      if (!movementTitle.isEmpty())
            xml.tag("movement-title", movementTitle);

      sigmap->write(xml);
      tempomap->write(xml);
      for (iPart i = _parts->begin(); i != _parts->end(); ++i)
            (*i)->write(xml);

      int staff = 0;
      for (iStaff ip = _staves->begin(); ip != _staves->end(); ++ip, ++staff) {
            xml.stag(QString("Staff id=\"%1\"").arg(staff+1));
            int measureNumber = 1;
            xml.curTick = 0;
            for (Measure* m = _layout->first(); m; m = m->next()) {
                  m->write(xml, measureNumber++, staff);
                  xml.curTick = m->tick() + sigmap->ticksMeasure(m->tick());
                  }
            xml.etag("Staff");
            }
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Part* Score::part(int n)
      {
      int idx = 0;
      for (iPart i = _parts->begin(); i != _parts->end(); ++i) {
            idx += (*i)->nstaves();
            if (n < idx)
                  return *i;
            }
      return 0;
      }

//---------------------------------------------------------
//   addMeasure
//---------------------------------------------------------

void Score::addMeasure(Measure* m)
      {
      int tick = m->tick();
      Measure* im;
      for (im = _layout->first(); im; im = im->next()) {
            int mtick = im->tick();
            if (mtick == tick)
                  break;
            }
      if (im) {
            int mtick = im->tick();
            int len   = im->tickLen();
            sigmap->insertTime(mtick, len);
            for (iStaff i = _staves->begin(); i != _staves->end(); ++i) {
                  (*i)->clef()->insertTime(mtick, len);
                  (*i)->keymap()->insertTime(mtick, len);
                  }
            }
      _layout->insert(im, m);
      fixTicks();
      }

//---------------------------------------------------------
//   removeMeasure
//---------------------------------------------------------

void Score::removeMeasure(int tick)
      {
      for (Measure* im = _layout->first(); im; im = im->next()) {
            int mtick = im->tick();
            if (mtick == tick) {
                  int len = im->tickLen();
                  sigmap->removeTime(mtick, len);
                  for (iStaff i = _staves->begin(); i != _staves->end(); ++i) {
                        (*i)->clef()->removeTime(mtick, len);
                        (*i)->keymap()->removeTime(mtick, len);
                        }
                  _layout->erase(im);
                  fixTicks();
                  return;
                  }
            }
      printf("no measure found at tick %d\n", tick);
      }

//---------------------------------------------------------
//   fixTicks
//---------------------------------------------------------

/**
 Recalculate all ticks and measure numbers.

 This is needed after inserting or removing a
 measure.
*/

void Score::fixTicks()
      {
      int bar  = 0;
      int tick = 0;
      for (Measure* m = _layout->first(); m; m = m->next()) {
            if (m->no() != bar) {
                  m->setNo(bar);
                  if (::style->showMeasureNumber) {
                        m->setNoText("");
                        if (bar != 0 || ::style->showMeasureNumberOne) {
                              if (::style->measureNumberSystem
                                 && (((bar+1) % ::style->measureNumberInterval) == 0)) {
                                    QString s("%1");
                                    s.arg(bar+1);
                                    m->setNoText(s);
                                    }
                              }
                        }
                  }
            if (!m->irregular())
                  ++bar;
            int mtick = m->tick();
            int diff = tick - mtick;
            tick += sigmap->ticksMeasure(tick);
            if (diff == 0)
                  continue;
            m->moveTicks(diff);
            }
      }

//---------------------------------------------------------
//   pos2measure
//---------------------------------------------------------

/**
 Return measure for canvas relative position \a p.

 If *rst != 0, then staff is fixed.
*/

Measure* Score::pos2measure(const QPointF& p, int* tick, Staff** rst, int* pitch,
   Segment** seg, QPointF* offset) const
      {
      int voice = padState.voice;

      for (ciPage ip = pages()->begin(); ip != pages()->end(); ++ip) {
            const Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF pp = p - page->pos();  // transform to page relative

            SystemList* sl = page->systems();
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
                  for (ciMeasure im = s->measures()->begin(); im != s->measures()->end(); ++im) {
                        Measure* m = *im;
                        if (ppp.x() > (m->x() + m->bbox().width()))
                              continue;
                        double sy1 = 0;
                        if (rst && *rst == 0) {
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
                                          if (segment->segmentType() != Segment::SegChordRest || (segment->element(track) == 0 && voice == 0)) {
                                                continue;
                                                }
                                          Segment* ns = segment->next();
                                          for (; ns; ns = ns->next()) {
                                                if (ns->segmentType() == Segment::SegChordRest && (ns->element(track) || voice))
                                                      break;
                                                }
                                          if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                                                i -= 1;
                                                if (rst)
                                                      *rst = (*_staves)[i];
                                                if (tick)
                                                      *tick = segment->tick();
                                                if (pitch) {
                                                      Staff* s = (*_staves)[i];
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
                              // search for segment + offset
                              QPointF pppp = ppp - m->pos();
                              for (Segment* segment = m->first(); segment; segment = segment->next()) {
                                    if (segment->segmentType() != Segment::SegChordRest)
                                          continue;
                                    bool found = segment->next() == 0;
                                    if (!found) {
                                          double x1 = segment->x();
                                          double x2 = segment->next()->x();
                                          found = pppp.x() < (x1 + (x2 - x1) / 2);
                                          }
                                    if (!found)
                                          continue;
                                    if (tick)
                                          *tick = segment->tick();
                                    if (pitch) {
                                          int clef = (*rst)->clef()->clef(*tick);
                                          *pitch = y2pitch(pppp.y(), clef);
                                          }
                                    if (offset) {
                                          //??
                                          int staffIdx = _staves->indexOf(*rst);
                                          SysStaff* staff = s->staff(staffIdx);
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
//    p - canvas relative position
//---------------------------------------------------------

/**
 Return measure for canvas relative position \a p.

 Sets \a *tick to the nearest notes tick,
 \a *rst to the nearest staff,
 \a *line to the nearest staff line.
*/

Measure* Score::pos2measure2(const QPointF& p, int* tick, Staff** rst, int* line,
   Segment** seg) const
      {
      int voice = padState.voice;

      for (ciPage ip = pages()->begin(); ip != pages()->end(); ++ip) {
            const Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF pp = p - page->pos();  // transform to page relative

            SystemList* sl = page->systems();
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
                  for (ciMeasure im = s->measures()->begin(); im != s->measures()->end(); ++im) {
                        Measure* m = *im;
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
                                    if (segment->segmentType() != Segment::SegChordRest || (segment->element(track) == 0 && voice == 0)) {
                                          segment = segment->next();
                                          continue;
                                          }
                                    Segment* ns = segment->next();
                                    for (; ns; ns = ns->next()) {
                                          if (ns->segmentType() == Segment::SegChordRest && (ns->element(track) || voice))
                                                break;
                                          }
                                    if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                                          i     -= 1;
                                          *rst   = (*_staves)[i];
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
//   staff
//---------------------------------------------------------

/**
 Return index for the first staff of \a part.
*/

int Score::staff(const Part* part) const
      {
      int staff = 0;
      for (ciPart i = _parts->begin(); i != _parts->end(); ++i) {
            if (*i == part)
                  break;
            staff += (*i)->nstaves();
            }
      return staff;
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

/**
 Return index for staff \a p in the staff list.
*/

int Score::staff(const Staff* p) const
      {
      return _staves->indexOf((Staff*)p);
      }

//---------------------------------------------------------
//   setInstrumentNames
//---------------------------------------------------------

void Score::setInstrumentNames()
      {
      for (iSystem is = systems()->begin(); is != systems()->end(); ++is)
            (*is)->setInstrumentNames();
      }

//---------------------------------------------------------
//   part
//---------------------------------------------------------

/**
 Return staff \a n in the staff list.
*/

Staff* Score::staff(int n) const
      {
      if (n < int(_staves->size()))
            return (*_staves)[n];
      printf("staff %d out of range; %zd\n", n, _staves->size());
      return 0;
      }

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

void Score::readStaff(QDomNode node)
      {
      Measure* im = _layout->first();

      QDomElement e = node.toElement();
      int staff = e.attribute("id", "1").toInt() - 1;

      int curTick = 0;
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());

            if (tag == "Measure") {
                  Measure* measure;
                  if (staff == 0) {
                        measure = new Measure(this);
                        _layout->push_back(measure);
                        }
                  else {
                        if (im) {
                              measure = im;
                              im = im->next();
                              }
                        else {
                              printf("Score::readStaff(): missing measure!\n");
                              measure = new Measure(this);
                              _layout->push_back(measure);
                              }
                        }
                  measure->setTick(curTick);
                  measure->read(node, staff);
                  curTick = measure->tick() + sigmap->ticksMeasure(measure->tick());
                  }
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   snap
//---------------------------------------------------------

int Score::snap(int tick, const QPointF p) const
      {
      for (iPage ip = pages()->begin(); ip != pages()->end(); ++ip) {
            Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF rp = p - page->pos();  // transform to page relative
            SystemList* sl = page->systems();
            double y = 0.0;
            for (ciSystem is = sl->begin(); is != sl->end(); ++is) {
                  System* system = *is;
                  ciSystem nis = is;
                  ++nis;
                  if (nis == sl->end()) {
                        return system->snap(tick, rp - system->pos());
                        }
                  System* nsystem = *nis;
                  double nexty = nsystem->y();
                  double gap = nexty - y - system->height();
                  nexty -= gap/2.0;
                  if (p.y() >= y && p.y() < nexty) {
                        return system->snap(tick, rp - system->pos());
                        }
                  }
            }
      printf("snapSegment: nothing found\n");
      return tick;
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
      foreach (Shortcut* s, shortcuts) {
            if (s->action)
                  s->action->setShortcut(0);
            }
      if (element->type() == SLUR_SEGMENT) {
            //
            // we must clone the whole slur with all segments
            //
            SlurSegment* segment = (SlurSegment*)element;
            SlurTie* slur        = segment->slurTie();
            SlurTie* newSlur     = (SlurTie*)slur->clone();
            segment->resetMode();

            removeElement(slur);
            origEditObject = element;

            //
            // find editObject (the right segment) in the
            // cloned slur
            //
            int idx = slur->elements()->indexOf(segment);
            if (idx == -1)
                  abort();
            editObject = newSlur->elements()->at(idx);


            select(editObject, 0, 0);
            undoOp(UndoOp::RemoveElement, slur);
            undoOp(UndoOp::AddElement, newSlur);
            }
      else {
            origEditObject = element;
            editObject     = element->clone();

            removeElement(element);
            element->resetMode();

            select(editObject, 0, 0);
            undoOp(UndoOp::RemoveElement, origEditObject);
            undoOp(UndoOp::AddElement, editObject);
            }
      updateAll = true;
      endCmd(false);
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

/**
 Return true if end edit.
*/

bool Score::edit(QKeyEvent* ev)
      {
      if ((ev->key() == Qt::Key_Escape) || editObject->edit(ev)) {
            endEdit();
            endCmd(true);
            return true;
            }
      endCmd(false);
      layout();
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Score::endEdit()
      {
      foreach (Shortcut* s, shortcuts) {
            if (s->action)
                  s->action->setShortcut(s->key);
            }
      refresh |= editObject->bbox();
      editObject->endEdit();
      refresh |= editObject->bbox();

      if (editObject->type() == TEXT) {
            Text* in = (Text*)editObject;
            Part* p  = part(in->staffIdx());
            if (editObject->subtype() == TEXT_INSTRUMENT_SHORT) {
                  p->setShortName(*(in->getDoc()));
                  setInstrumentNames();
                  }
            else if (editObject->subtype() == TEXT_INSTRUMENT_LONG) {
                  p->setLongName(*(in->getDoc()));
                  setInstrumentNames();
                  }
            }
      else if (editObject->type() == LYRICS) {
            Lyrics* lyrics = (Lyrics*)editObject;
            Lyrics* origL = (Lyrics*)origEditObject;
            if (lyrics->isEmpty() && origL->isEmpty()) {
                  Measure* measure = (Measure*)(lyrics->parent());
                  measure->remove(lyrics);
                  }
            }
      layout();
      editObject = 0;
      mscore->setState(STATE_NORMAL);
      }

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

/**
 Paste element \a el at position \a pos.
*/

void Score::paste(const Element* /*e*/, const QPointF& /*pos*/)
      {
      printf("TODO: Score::paste()\n");
#if 0       //TODO: paste arbitrary element
      Element* obj = e->clone();

      Staff* staff = 0;
      int tick;
      Measure* measure = pos2measure(pos, &tick, &staff, 0, 0, 0);

      bool tickFound = measure != 0;
      Element* el = findSelectableElement(pos);

      Element* element = 0;
      switch(obj->type()) {
            case SYMBOL:
            case COMPOUND:
            case TEXT:
                  {
                  if (measure == 0) {
                        printf("cmdAddElement(): measure not found\n");
                        break;
                        }
                  iPage ip;
                  for (ip = pages()->begin(); ip != pages()->end(); ++ip) {
                        if ((*ip)->contains(pos)) {
                              obj->setPos(pos - (*ip)->apos());
                              measure->add(obj);
                              undoOp(UndoOp::AddObject, obj);
                              (*ip)->add(obj);
                              element = obj;
                              break;
                              }
                        }
                  if (ip == pages()->end())
                        printf("cmdAddElement(): page not found\n");
                  }
                  break;
            case CLEF:
                  {
                  if (!tickFound) {
                        printf("cannot put clef at this position\n");
                        delete obj;
                        break;
                        }
                  Clef* clef = (Clef*)obj;
                  clef->setTick(tick);
                  clef->setStaff(staff);
                  element = addClef(clef);
                  if (element)
                        undoOp(UndoOp::AddObject, element);
                  else
                        delete obj;
                  }
                  break;

            case TIMESIG:
                  element = addTimeSig((TimeSig*) obj, pos);
                  break;
            case KEYSIG:
                  element = addKeySig((KeySig*) obj, pos);
                  break;
            case BAR_LINE:
                  addBar((BarLine*)obj, measure);
                  break;
            case ATTRIBUTE:
                  {
                  NoteAttribute* a = (NoteAttribute*)obj;
                  int st = a->subtype();
                  if ((el->type() == NOTE)
                     || (el->type() == REST && (st == UfermataSym || st == DfermataSym))) {
                        addAttribute(el, a);
                        element = obj;
                        }
                  }
                  break;
            case DYNAMIC:
                  element = addDynamic((Dynamic*) obj, pos);
                  break;
            case HAIRPIN:
                  element = cmdAddHairpin((Hairpin*) obj, pos);
                  break;
            case SLUR:
                  element = addSlur((Slur*) obj, pos);
                  break;
            case FINGERING:
                  {
                  if (el && el->type() == NOTE) {
                        el->add(obj);
                        select(obj, 0, 0);
                        layout();
                        undoOp(UndoOp::AddObject, obj);
                        element = obj;
                        }
                  }
                  break;
            case ACCIDENTAL:
                  if (el && el->type() == NOTE)
                        addAccidental((Note*)el, ((Accidental*)obj)->idx());
                  delete obj;
                  break;
            case BRACKET:
            	if (el == 0)
                  	break;
			measure->drop(pos, obj->type(), obj->subtype());
            	break;
            case NOTE:
            case CHORD:
            default:
                  printf("cannot add element %s to %s\n",
                     obj->name(), el ? el->name() :"??");
                  delete obj;
                  break;
            }
      _dragObject = element;

      if (_dragObject)
            select(_dragObject, 0, 0);
#endif
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void Score::startDrag()
      {
      origDragObject  = _dragObject;
      _dragObject     = _dragObject->clone();
      undoOp(UndoOp::RemoveElement, origDragObject);
      removeElement(origDragObject);

      undoOp(UndoOp::AddElement, _dragObject);
      sel->clear();
      sel->add(_dragObject);
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

void Score::drag(const QPointF& delta)
      {
      for (iElement i = sel->elements()->begin(); i != sel->elements()->end(); ++i)
            refresh |= (*i)->drag(delta);
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
      layout();
      }

//---------------------------------------------------------
//   dragEdit
//---------------------------------------------------------

void Score::dragEdit(QMatrix& matrix, QPointF* startMove, const QPointF& fdelta)
      {
      refresh |= editObject->abbox();
      editObject->editDrag(matrix, startMove, fdelta);
      refresh |= editObject->abbox();
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
            if (cis->pos != -1) {     // already in note entry mode
                  if (el) {
                        if (el->type() == NOTE)
                              el = el->parent();
                        if (!el->isChordRest())
                              return 0;
                        cr = (ChordRest*)el;
                        if (cr->tick() == cis->pos) {
                              // int len = cr->tuplet() ? cr->tuplet()->noteLen() : cr->tickLen();
                              cis->pos += cr->tickLen();
                              }
                        }
                  return cr;
                  }
            if (sel->state != SEL_SINGLE || !el || (el->type() != NOTE &&
               !el->isChordRest())) {
                  QMessageBox::information(0, "MuseScore: Note Entry",
                        tr("No note or rest selected:\n"
                           "please select a note or rest were you want to\n"
                           "start note entry"));
                  return 0;
                  }
            if (el->type() == NOTE)
                  el = el->parent();
            cr = (ChordRest*)el;
            cis->pos = cr->tick();
            if (step) {
                  if (cr->tuplet())
                        cis->pos += cr->tuplet()->noteLen();
                  else
                        cis->pos += cr->tickLen();
                  }
            canvas()->setState(Canvas::NOTE_ENTRY);
            }
      else {
            padState.len = 0;
            cis->pos = -1;
            setPadState();
            canvas()->setState(Canvas::NORMAL);
            }
      mscore->setState(val ? STATE_NOTE_ENTRY : STATE_NORMAL);
      moveCursor();
      return cr;
      }

//---------------------------------------------------------
//   startNoteEntry
//---------------------------------------------------------

void Score::startNoteEntry()
      {
      setNoteEntry(true, false);
      padState.rest = false;
      endCmd(false);
      }

//---------------------------------------------------------
//   midiReceived
//---------------------------------------------------------

void Score::midiReceived()
      {
      readMidiEvent();
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void Score::midiNoteReceived(int pitch, bool chord)
      {
      startCmd();
      if (cis->pos == -1) {
            setNoteEntry(true, false);
            }
      if (cis->pos != -1) {
            int len = padState.tickLen;
            if (chord) {
                  Note* on = getSelectedNote();
                  Note* n = addNote(on->chord(), pitch);
                  select(n, 0, 0);
                  }
            else {
                  setNote(cis->pos, staff(cis->staff), cis->voice, pitch, len);
                  cis->pos += len;
                  moveCursor();
                  }
            }
      endCmd(true);
      }

//---------------------------------------------------------
//   searchTieNote
//---------------------------------------------------------

Note* Score::searchTieNote(Note* note, Segment* segment, int track)
      {
      int pitch = note->pitch();

      while ((segment = segment->next1())) {
            Element* element = segment->element(track);
            if (element == 0 || element->type() != CHORD)
                  continue;
            const NoteList* nl = ((Chord*)element)->noteList();
            for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                  if (in->second->pitch() == pitch)
                        return in->second;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   connectTies
//---------------------------------------------------------

/**
 Rebuild tie connections.
*/

void Score::connectTies()
      {
      int tracks = nstaves() * VOICES;
      for (Measure* m = _layout->first(); m; m = m->next()) {
            for (Segment* s = m->first(); s; s = s->next()) {
                  for (int i = 0; i < tracks; ++i) {
                        Element* el = s->element(i);
                        if (el == 0 || el->type() != CHORD)
                              continue;
                        const NoteList* nl = ((Chord*)el)->noteList();
                        for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                              Tie* tie = in->second->tieFor();
                              if (!tie)
                                    continue;
                              Note* nnote = searchTieNote(in->second, s, i);
                              if (nnote == 0)
                                    printf("next note at %d(measure %d) for tie not found\n",
                                       in->second->chord()->tick(),
                                       m->no());
                              else {
                                    tie->setEndNote(nnote);
                                    nnote->setTieBack(tie);
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   snapNote
//    p - absolute position
//---------------------------------------------------------

int Score::snapNote(int tick, const QPointF p, int staff) const
      {
      for (iPage ip = pages()->begin(); ip != pages()->end(); ++ip) {
            Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF rp = p - page->pos();  // transform to page relative
            SystemList* sl = page->systems();
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

//---------------------------------------------------------
//   pos2sel
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
//   nstaves
//---------------------------------------------------------

/**
 Return number of staves in the staff list.
*/

int Score::nstaves() const
      {
      return _staves->size();
      }

//---------------------------------------------------------
//   noStaves
//---------------------------------------------------------

/**
 Return true if the staff list is empty.
*/

bool Score::noStaves() const
      {
      return _staves->empty();
      }

//---------------------------------------------------------
//   textStyleChanged
//---------------------------------------------------------

void Score::textStyleChanged()
      {
      PageList* pl = pages();
      for (iPage ip = pl->begin(); ip != pl->end(); ++ip) {
            Page* page = *ip;
            SystemList* sl = page->systems();

            ElementList* el = page->pel();
            for (iElement i = el->begin(); i != el->end(); ++i) {
                  if ((*i)->type() == TEXT)
                        (*i)->layout();
                  }
            for (iSystem s = sl->begin(); s != sl->end(); ++s) {
                  MeasureList* ml = (*s)->measures();
                  for (iMeasure im = ml->begin(); im != ml->end(); ++im) {
                        Measure* measure = *im;
                        el = measure->el();
                        for (iElement i = el->begin(); i != el->end(); ++i) {
                              if ((*i)->type() == TEXT)
                                    (*i)->layout();
                              }
                        for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                              // TODO: Lyrics
                              }
                        }
                  }
            }
      layout();
      endCmd(false);
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
      _showInvisible = v;
      updateAll = true;
      endCmd(false);
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

