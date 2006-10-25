//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: segment.cpp,v 1.28 2006/04/12 14:58:10 wschweer Exp $
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

#include "globals.h"
#include "segment.h"
#include "element.h"
#include "chord.h"
#include "score.h"
#include "beam.h"
#include "tuplet.h"
#include "textelement.h"
#include "measure.h"
#include "painter.h"
#include "barline.h"

const char* Segment::segmentTypeNames[] = {
   "Clef", "Key Signature", "Time Signature", "Begin Repeat", "ChordRest"
   };

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Segment::setElement(int track, Element* el)
      {
      if (el)
            el->setParent(this);
      _elist[track] = el;
      }

//---------------------------------------------------------
//   setLyrics
//---------------------------------------------------------

void Segment::setLyrics(int staff, Lyrics* l)
      {
      int idx = l->no();
      int n = _lyrics[staff].size();
      for (int i = n; i <= idx; ++i)
            _lyrics[staff].push_back(0);
      _lyrics[staff][idx] = l;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::removeElement(int track)
      {
      Element* el = element(track);
      if (el->isChordRest()) {
            ChordRest* cr = (ChordRest*)el;
            Beam* beam = cr->beam();
            if (beam)
                  beam->remove(cr);
            Tuplet* tuplet = cr->tuplet();
            if (tuplet)
                  tuplet->remove(cr);
            }
      }

//---------------------------------------------------------
//   Segment
//---------------------------------------------------------

Segment::Segment(Measure* m)
   : Element(m->score())
      {
      setParent(m);
      init();
      }

Segment::Segment(Measure* m, int t)
   : Element(m->score())
      {
      setParent(m);
      setTick(t);
      init();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Segment::init()
      {
      int staves = measure()->score()->nstaves();
      int tracks = staves * VOICES;
      for (int i = 0; i < tracks; ++i)
            _elist.push_back(0);
      for (int i = 0; i < staves; ++i)
            _lyrics.push_back(LyricsList());
      }

//---------------------------------------------------------
//   next1
//---------------------------------------------------------

Segment* Segment::next1() const
      {
      if (next())
            return next();
      Measure* m = measure()->next();
      if (m)
            return m->first();
      return 0;
      }

//---------------------------------------------------------
//   prev1
//---------------------------------------------------------

Segment* Segment::prev1() const
      {
      if (prev())
            return prev();
      Measure* m = measure()->prev();
      if (m)
            return m->last();
      return 0;
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Segment::insertStaff(int staff)
      {
      int track = staff * VOICES;
      for (int voice = 0; voice < VOICES; ++voice)
            _elist.insert(_elist.begin() + track, 0);
      _lyrics.insert(_lyrics.begin() + staff, LyricsList());
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Segment::removeStaff(int staff)
      {
      int track = staff * VOICES;
      _elist.erase(_elist.begin() + track, _elist.begin() + track + VOICES);
      _lyrics.erase(_lyrics.begin() + staff);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Segment::draw(Painter& p)
      {
      p.translate(pos());

      int staves = measure()->score()->nstaves();
      int tracks = staves * VOICES;
      for (int track = 0; track < tracks; ++track) {
            Element* e = _elist[track];
            if (e)
                  e->draw(p);
            }
      for (int staff = 0; staff < staves; ++staff) {
            const LyricsList* ll = lyricsList(staff);
            for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
                  if (*i)
                        (*i)->draw(p);
                  }
            }
      p.translate(-pos());
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Segment::add(Element* el)
      {
// printf("Segment::add %s\n", el->name());
      el->setParent(this);
      int staffIdx = el->staffIdx();
      if (el->type() == LYRICS) {
            LyricsList* ll = &_lyrics[staffIdx];
            ll->push_back((Lyrics*)el);
            el->layout();     //DEBUG
            return;
            }
      if (el->type() == BAR_LINE && el->subtype() == START_REPEAT) {
            measure()->add(el);
            }
      else
            _elist[staffIdx * VOICES + el->voice()] = el;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Segment::remove(Element* el)
      {
// printf("Segment::remove %s\n", el->name());
      int staffIdx = el->staffIdx();
      if (el->type() == LYRICS) {
            LyricsList* ll = &_lyrics[staffIdx];
            if (ll) {
                  for (iLyrics il = ll->begin(); il != ll->end(); ++il) {
                        if ((*il) == el) {
                              ll->erase(il);
                              return;
                              }
                        }
                  }
            printf("Measure::remove: %s %p not found\n", el->name(), el);
            return;
            }
      if (el->type() == BAR_LINE)
            measure()->setStartRepeat(false);
      _elist[staffIdx * VOICES + el->voice()] = 0;
      }

