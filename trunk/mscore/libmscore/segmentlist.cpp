//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "segmentlist.h"
#include "segment.h"

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

SegmentList SegmentList::clone() const
      {
      SegmentList dl;
      Segment* s = _first;
      for (int i = 0; i < _size; ++i) {
            Segment* ns = s->clone();
            dl.push_back(ns);
            s = s->next();
            }
      dl.check();
      return dl;
      }

//---------------------------------------------------------
//   check
//---------------------------------------------------------

#ifndef NDEBUG
void SegmentList::check()
      {
      int n = 0;
      Segment* f = 0;
      Segment* l = 0;
      for (Segment* s = _first; s; s = s->next()) {
            if (f == 0)
                  f = s;
            l = s;
            ++n;
            }
      if (f != _first) {
            printf("SegmentList::check: bad first\n");
            abort();
            }
      if (l != _last) {
            printf("SegmentList::check: bad last\n");
            abort();
            }
      if (n != _size) {
            printf("SegmentList::check: wrong segment segments %d count %d\n", n, _size);
            _size = n;
            abort();
            }
      }
#endif

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

/**
 Insert Segment \a e before Segment \a el.
*/

void SegmentList::insert(Segment* e, Segment* el)
      {
      if (el == 0) {
            push_back(e);
            return;
            }
      if (el == first()) {
            push_front(e);
            return;
            }
      ++_size;
      e->setNext(el);
      e->setPrev(el->prev());
      el->prev()->setNext(e);
      el->setPrev(e);
      check();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SegmentList::remove(Segment* el)
      {
      --_size;
      if (el == _first) {
            _first = _first->next();
            if (_first)
                  _first->setPrev(0);
            if (el == _last)
                  _last = 0;
            }
      else if (el == _last) {
            _last = _last->prev();
            if (_last)
                  _last->setNext(0);
            }
      else {
            el->prev()->setNext(el->next());
            el->next()->setPrev(el->prev());
            }
      check();
      }

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void SegmentList::push_back(Segment* e)
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
      check();
      }

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void SegmentList::push_front(Segment* e)
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
      check();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void SegmentList::insert(Segment* seg)
      {
#ifndef NDEBUG
//      printf("insertSeg <%s> %p %p %p\n", seg->subTypeName(), seg->prev(), seg, seg->next());
      check();
      for (Segment* s = _first; s; s = s->next()) {
            if (s == seg) {
                  printf("SegmentList::insert: already in list\n");
                  abort();
                  }
            }
      if (seg->prev()) {
            Segment* s;
            for (s = _first; s; s = s->next()) {
                  if (s == seg->prev())
                        break;
                  }
            if (s != seg->prev()) {
                  printf("SegmentList::insert: seg->prev() not in list\n");
                  abort();
                  }
            }

      if (seg->next()) {
            Segment* s;
            for (s = _first; s; s = s->next()) {
                  if (s == seg->next())
                        break;
                  }
            if (s != seg->next()) {
                  printf("SegmentList::insert: seg->next() not in list\n");
                  abort();
                  }
            }
#endif
      if (seg->prev())
            seg->prev()->setNext(seg);
      else
            _first = seg;
      if (seg->next())
            seg->next()->setPrev(seg);
      else
            _last = seg;
      ++_size;
      check();
      }

//---------------------------------------------------------
//   firstCRSegment
//---------------------------------------------------------

Segment* SegmentList::firstCRSegment() const
      {
      return first(SegChordRest);
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* SegmentList::first(SegmentTypes types) const
      {
      for (Segment* s = _first; s; s = s->next()) {
            if (s->subtype() & types)
                  return s;
            }
      return 0;
      }


