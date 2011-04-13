//=============================================================================
//  MusE Score
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

#include "measurebase.h"
#include "measure.h"
#include "staff.h"
#include "lyrics.h"
#include "score.h"
#include "chord.h"
#include "note.h"
#include "layoutbreak.h"
#include "image.h"
#include "segment.h"

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::MeasureBase(Score* score)
   : Element(score)
      {
      _prev = 0;
      _next = 0;
      _lineBreak    = false;
      _pageBreak    = false;
      _sectionBreak = 0;
      _dirty        = true;
      }

MeasureBase::MeasureBase(const MeasureBase& m)
   : Element(m)
      {
      _next         = m._next;
      _prev         = m._prev;
      _mw           = m._mw;
      _dirty        = m._dirty;
      _lineBreak    = m._lineBreak;
      _pageBreak    = m._pageBreak;
      _sectionBreak = m._sectionBreak;

      foreach(Element* e, m._el)
            add(e->clone());
      }

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::~MeasureBase()
      {
      foreach(Element* e, _el)
            delete e;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void MeasureBase::scanElements(void* data, void (*func)(void*, Element*))
      {
      if (type() == MEASURE) {
            foreach(Element* e, _el) {
                  if ((e->track() == -1) || ((Measure*)this)->visible(e->staffIdx()))
                        e->scanElements(data, func);
                  }
            }
      else {
            foreach(Element* e, _el)
                  e->scanElements(data, func);
            }
      func(data, this);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

/**
 Add new Element \a el to MeasureBase
*/

void MeasureBase::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == LAYOUT_BREAK) {
            for (iElement i = _el.begin(); i != _el.end(); ++i) {
                  if ((*i)->type() == LAYOUT_BREAK && (*i)->subtype() == e->subtype()) {
                        if (debugMode)
                              printf("warning: layout break already set\n");
                        return;
                        }
                  }
            switch(e->subtype()) {
                  case LAYOUT_BREAK_PAGE:
                        _pageBreak = true;
                        break;
                  case LAYOUT_BREAK_LINE:
                        _lineBreak = true;
                        break;
                  case LAYOUT_BREAK_SECTION:
                        _sectionBreak = static_cast<LayoutBreak*>(e);
                        break;
                  }
            }
      _el.append(e);
      if (e->type() == IMAGE)
            static_cast<Image*>(e)->reference();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

/**
 Remove Element \a el from MeasureBase.
*/

void MeasureBase::remove(Element* el)
      {
      if (el->type() == LAYOUT_BREAK) {
            switch(el->subtype()) {
                  case LAYOUT_BREAK_PAGE:
                        _pageBreak = false;
                        break;
                  case LAYOUT_BREAK_LINE:
                        _lineBreak = false;
                        break;
                  case LAYOUT_BREAK_SECTION:
                        _sectionBreak = 0;
                        break;
                  }
            }
      if (!_el.remove(el))
            printf("MeasureBase(%p)::remove(%s,%p) not found\n", this, el->name(), el);
      if (el->type() == IMAGE)
            static_cast<Image*>(el)->dereference();
      }


//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

Measure* MeasureBase::nextMeasure() const
      {
      MeasureBase* m = next();
      while (m) {
            if (m->type() == MEASURE)
                  return static_cast<Measure*>(m);
            m = m->next();
            }
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

Measure* MeasureBase::prevMeasure() const
      {
      MeasureBase* m = prev();
      while (m) {
            if (m->type() == MEASURE)
                  return static_cast<Measure*>(m);
            m = m->prev();
            }
      return 0;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void MeasureBase::spatiumChanged(double oldValue, double newValue)
      {
      foreach(Element* e, _el)
            e->spatiumChanged(oldValue, newValue);
      if (type() == MEASURE) {
            Measure* m = static_cast<Measure*>(this);
            if (m->noText())
                  m->noText()->spatiumChanged(oldValue, newValue);
            for (Segment* s = m->first(); s; s = s->next()) {
                  for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
                        const QList<Lyrics*>* ll = s->lyricsList(staffIdx);
                        if (ll) {
                              foreach(Lyrics* l, *ll) {
                                    if (l)
                                          l->spatiumChanged(oldValue, newValue);
                                    }
                              }
                        }
                  for (int track = 0; track < score()->nstaves()*VOICES; ++track) {
                        Element* e = s->element(track);
                        if ((e == 0) || (e->type() != CHORD))
                              continue;
                        Chord* ch = static_cast<Chord*>(e);
                        foreach(Note* n, ch->notes()) {
                              ElementList* el = n->el();
                              foreach(Element* e, *el)
                                    e->spatiumChanged(oldValue, newValue);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   pause
//---------------------------------------------------------

double MeasureBase::pause() const
      {
      return _sectionBreak ? _sectionBreak->pause() : 0.0;
      }

