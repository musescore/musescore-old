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
#include "lyrics.h"

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
      info.setFile("");
      _dirty          = false;
      _saved          = false;
      _created        = false;
      rights          = "";
      movementNumber  = "";
      movementTitle   = "";
      _pageOffset     = 0;

      for (iStaff i = _staves->begin(); i != _staves->end(); ++i)
            delete *i;
      _staves->clear();
      _layout->clear();
      foreach(Part* p, _parts)
            delete p;
      _parts.clear();

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
      foreach(const Part* part, _parts)
            part->write(xml);

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

      for (ciPage ip = _layout->pages()->begin(); ip != _layout->pages()->end(); ++ip) {
            const Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF pp = p - page->pos();  // transform to page relative

            QList<System*>* sl = page->systems();
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
                              //
                              // staff is fixed
                              //
                              // search for segment + offset
                              QPointF pppp = ppp - m->pos();
                              for (Segment* segment = m->first(); segment; segment = segment->next()) {
                                    if (segment->subtype() != Segment::SegChordRest)
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

      for (ciPage ip = _layout->pages()->begin(); ip != _layout->pages()->end(); ++ip) {
            const Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF pp = p - page->pos();  // transform to page relative

            QList<System*>* sl = page->systems();
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
      foreach(Part* p, _parts) {
            if (p == part)
                  break;
            staff += p->nstaves();
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
      for (iPage ip = _layout->pages()->begin(); ip != _layout->pages()->end(); ++ip) {
            Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF rp = p - page->pos();  // transform to page relative
            QList<System*>* sl = page->systems();
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
            undoAddElement(newSlur);
            }
      else {
            origEditObject = element;
            editObject     = element->clone();
            origEditObject->resetMode();
            select(editObject, 0, 0);
            undoChangeElement(origEditObject, editObject);
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
                  _layout->setInstrumentNames();
                  }
            else if (editObject->subtype() == TEXT_INSTRUMENT_LONG) {
                  p->setLongName(*(in->getDoc()));
                  _layout->setInstrumentNames();
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
      layout();
      }

//---------------------------------------------------------
//   dragEdit
//---------------------------------------------------------

void Score::dragEdit(Viewer* viewer, QPointF* startMove, const QPointF& fdelta)
      {
      refresh |= editObject->abbox();
      editObject->editDrag(viewer, startMove, fdelta);
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
            if (sel->state == SEL_NONE || (el && el->type() != NOTE && !el->isChordRest())) {
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
            cis->pos = cr->tick();
            if (step) {
                  if (cr->tuplet())
                        cis->pos += cr->tuplet()->noteLen();
                  else
                        cis->pos += cr->tickLen();
                  }
            }
      else {
            padState.len = 0;
            cis->pos = -1;
            setPadState();
            }
      if (cis->pos == -1) {
            canvas()->setState(Canvas::NORMAL);
            mscore->setState(STATE_NORMAL);
            }
      else {
            canvas()->setState(Canvas::NOTE_ENTRY);
            mscore->setState(STATE_NOTE_ENTRY);
            }
      moveCursor();
      return cr;
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
//   snapNote
//    p - absolute position
//---------------------------------------------------------

int Score::snapNote(int tick, const QPointF p, int staff) const
      {
      for (iPage ip = _layout->pages()->begin(); ip != _layout->pages()->end(); ++ip) {
            Page* page = *ip;
            if (!page->contains(p))
                  continue;
            QPointF rp = p - page->pos();  // transform to page relative
            QList<System*>* sl = page->systems();
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

//---------------------------------------------------------
//   adjustTime
//    change all time positions starting with measure
//    according to new start time
//---------------------------------------------------------

void Score::adjustTime(int tick, Measure* m)
      {
      while (m) {
            m->setTick(tick);
            int delta = m->first()->tick() - tick;
            for (Segment* s = m->first(); s; s = s->next()) {
                  s->setTime(s->tick() - delta);
                  }
            tick += m->tickLen();
            m = m->next();
            }
      }

//---------------------------------------------------------
//   tick2Anchor
//    return anchor position for tick in global
//    coordinates
//---------------------------------------------------------

QPointF Score::tick2Anchor(int tick, int staffIdx) const
      {
      Segment* seg = tick2segment(tick);
      qreal x = seg->abbox().x();
      System* system = seg->measure()->system();
      qreal y = system->staff(staffIdx)->bbox().y() + system->canvasPos().y();
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   pos2TickAnchor
//    Calculates anchor position and tick for a
//    given position+staff in global coordinates.
//
//    return false if no anchor found
//---------------------------------------------------------

bool Score::pos2TickAnchor(QPointF& pos, Staff* staff, int* tick, QPointF* anchor) const
      {
      Segment* seg;
      Measure* m = pos2measure(pos, tick, &staff, 0, &seg, 0);
      if (!m)
            return false;
      System* system = m->system();
      qreal y = system->staff(staff->idx())->bbox().y();
      *anchor = QPointF(seg->abbox().x(), y + system->canvasPos().y());
      return true;
      }

