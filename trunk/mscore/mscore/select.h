//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: select.h,v 1.8 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __SELECT_H__
#define __SELECT_H__

class Score;

//---------------------------------------------------------
//   SelState
//---------------------------------------------------------

enum SelState {
      SEL_NONE,         // nothing is selected
      SEL_SINGLE,       // a single object is selected
      SEL_MULT,         // more than one object is selected
      SEL_STAFF,        // a range in one or more staffs is selected
      SEL_SYSTEM,       // a system range ("passage") is selected
      };

class Page;
class System;
class ChordRest;
class Element;

//---------------------------------------------------------
//   Selection
//---------------------------------------------------------

class Selection {
      Score* _score;
      SelState _state;
      QList<Element*> _el;        // valid in mode SEL_SINGLE and SEL_MULT

      QByteArray staffMimeData() const;

   public:
      Selection(Score* s)       { _score = s;    }
      SelState state() const    { return _state; }
      void setState(SelState s) { _state = s;    }

      int tickStart;          // selection start time tick
      int tickEnd;            // selection end time tick
      int staffStart;         // valid if selState is SEL_STAFF
      int staffEnd;           // valid if selState is SEL_STAFF
      QRectF lasso;

      QList<Element*>* elements()      { return &_el; }
      void add(Element*);
      void append(Element* el)         { _el.append(el); }
      QRectF deselectAll(Score*);
      void remove(Element*);
      QRectF clear();
      Element* element() const;
      ChordRest* firstChordRest() const;
      ChordRest* lastChordRest() const;
      void update();
      void updateState();
      void dump();
      QString mimeType() const;
      QByteArray mimeData() const;
      };

#endif

