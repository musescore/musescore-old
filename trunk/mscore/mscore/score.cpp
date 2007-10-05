//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: score.cpp,v 1.24 2006/04/12 14:58:10 wschweer Exp $
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

/**
 \file
 Implementation of classes ElemList (complete) and Score (partial).
*/

// #include "alsa.h"
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

Score* gscore;                 ///< system score, used for palettes etc.

QPoint scorePos(0,0);
QSize  scoreSize(950, 500);

MuseScore* mscore;
bool debugMode      = false;
bool layoutDebug    = false;
bool noSeq          = false;
bool noMidi         = false;
bool dumpMidi       = false;
bool showRubberBand = true;

//---------------------------------------------------------
//   doLayout
//---------------------------------------------------------

void Score::doLayout()
      {
      _layout->doLayout();
      }

//---------------------------------------------------------
//   reLayout
//---------------------------------------------------------

void Score::reLayout(Measure* m)
      {
      _layout->reLayout(m);
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
      _noteEntryMode    = false;
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

      if (info.completeSuffix() == "xml")
            importMusicXml(name);
      else if (info.completeSuffix().toLower() == "mid") {
            if (!importMidi(name))
                  return;
            }
      else if (info.completeSuffix() == "md") {
            if (!importMuseData(name))
                  return;
            }
      else if (info.completeSuffix() == "ly") {
            if (!importLilypond(name))
                  return;
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
      layoutAll = true;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Score::write(Xml& xml)
      {
      if (editObject) {                          // in edit mode?
            endCmd();
            canvas()->setState(Canvas::NORMAL);  //calls endEdit()
            }
      _style->saveStyle(xml);
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (*_textStyles[i] != defaultTextStyleArray[i])
                  _textStyles[i]->write(xml);
            }
      xml.tag("showInvisible", _showInvisible);
      pageFormat()->write(xml);
      if (rights)
            xml.tag("rights", rights->toHtml());
      if (!movementNumber.isEmpty())
            xml.tag("movement-number", movementNumber);
      if (!movementTitle.isEmpty())
            xml.tag("movement-title", movementTitle);

      sigmap->write(xml);
      tempomap->write(xml);
      foreach(const Part* part, _parts)
            part->write(xml);
      foreach(Element* el, _layout->_gel)
            el->write(xml);
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1));
            int measureNumber = 1;
            xml.curTick = 0;
            for (Measure* m = _layout->first(); m; m = m->next()) {
                  m->write(xml, measureNumber++, staffIdx);
                  xml.curTick = m->tick() + sigmap->ticksMeasure(m->tick());
                  }
            xml.etag();
            }
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

void Score::addMeasure(Measure* m)
      {
      int tick    = m->tick();
      Measure* im = tick2measure(tick);
      _layout->insert(m, im);
      fixTicks();
      }

//---------------------------------------------------------
//   removeMeasure
//---------------------------------------------------------

void Score::removeMeasure(Measure* im)
      {
      _layout->erase(im);
      fixTicks();
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Score::insertTime(int tick, int len)
      {
      if (len < 0) {
            len = -len;
            sigmap->removeTime(tick, len);
            tempomap->removeTime(tick, len);
            foreach(Staff* staff, _staves) {
                  staff->clef()->removeTime(tick, len);
                  staff->keymap()->removeTime(tick, len);
                  }
            }
      else {
            sigmap->insertTime(tick, len);
            tempomap->insertTime(tick, len);
            foreach(Staff* staff, _staves) {
                  staff->clef()->insertTime(tick, len);
                  staff->keymap()->insertTime(tick, len);
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
      for (Measure* m = _layout->first(); m; m = m->next()) {
            if (m->no() != bar) {
                  m->setNo(bar);
                  if (_style->showMeasureNumber) {
                        m->setNoText("");
                        if (bar != 0 || _style->showMeasureNumberOne) {
                              if (_style->measureNumberSystem
                                 && (((bar+1) % _style->measureNumberInterval) == 0)) {
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
            int diff  = tick - mtick;
// printf("fixTicks %d  %5d + %3d  len %3d\n", m->no(), mtick, diff, sigmap->ticksMeasure(tick));
            tick += sigmap->ticksMeasure(tick);
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
//      int voice = _padState.voice;            ?!?
      int voice = 0; // _padState.voice;

      for (ciPage ip = _layout->pages()->begin(); ip != _layout->pages()->end(); ++ip) {
            const Page* page = *ip;

            // if (!page->contains(p))
            if (!page->abbox().contains(p))
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
                  foreach(Measure* m, s->measures()) {
                        if (ppp.x() >= (m->x() + m->bbox().width()))
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
                                                *rst = _staves[i];
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
                                          int clef = (*rst)->clef()->clef(*tick);
                                          *pitch = y2pitch(pppp.y(), clef);
                                          }
                                    if (offset) {
                                          //??
                                          int staffIdx = _staves.indexOf(*rst);
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
      int voice = _padState.voice;

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
                  foreach(Measure* m, s->measures()) {
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
                                          *rst   = _staves[i];
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
                  foreach(Measure* m, s->measures()) {
                        if (ppp.x() > (m->x() + m->bbox().width()))
                              continue;
                        if (ppp.x() < (m->x() + m->bbox().width()*.5)) {
                              *tick = m->tick();
                              return m;
                              }
                        else if (m->next()) {
                              *tick = m->next()->tick();
                              return m->next();
                              }
                        else {
                              *tick = m->tick() + m->tickLen();
                              return m;
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
//   readStaff
//---------------------------------------------------------

void Score::readStaff(QDomElement e)
      {
      Measure* im = _layout->first();
      int staff   = e.attribute("id", "1").toInt() - 1;

      int curTick = 0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
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
                  measure->read(e, staff);
                  curTick = measure->tick() + sigmap->ticksMeasure(measure->tick());
                  }
            else
                  domError(e);
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
            _noteEntryMode = true;
            foreach(Viewer* v, viewer)
                  v->setCursorOn(true);
            canvas()->setState(Canvas::NOTE_ENTRY);
            mscore->setState(STATE_NOTE_ENTRY);
            }
      else {
            _padState.len   = 0;
            _is.pos        = -1;
            _noteEntryMode = false;
            setPadState();
            foreach(Viewer* v, viewer)
                  v->setCursorOn(false);
            canvas()->setState(Canvas::NORMAL);
            mscore->setState(STATE_NORMAL);
            }
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

void Score::adjustTime(int tick, Measure* m)
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

bool Score::pos2TickAnchor(const QPointF& pos, Staff* staff, int* tick, QPointF* anchor) const
      {
      Segment* seg;
      Measure* m = pos2measure(pos, tick, &staff, 0, &seg, 0);
      if (!m) {
            printf("pos2TickAnchor: no measure found\n");
            return false;
            }
      System* system = m->system();
      qreal y = system->staff(staff->idx())->bbox().y();
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
            for(Measure* m = _layout->first(); m; m = m->next()) {
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

//---------------------------------------------------------
//   gel
//---------------------------------------------------------

QList<Element*>* Score::gel()
      {
      return _layout->gel();
      }

