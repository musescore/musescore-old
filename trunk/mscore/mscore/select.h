//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: select.h,v 1.8 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __SELECT_H__
#define __SELECT_H__

class Score;

#include "element.h"

//---------------------------------------------------------
//   SelState
//---------------------------------------------------------

enum SelState {
      SEL_NONE,         // nothing is selected
      SEL_SINGLE,       // a single object is selected
      SEL_MULT,         // more than one object is selected
      SEL_STAFF,        // a range in one or more staffs is selected
      SEL_SYSTEM        // a system range ("passage") is selected
      };

class Page;
class System;
class ChordRest;

//---------------------------------------------------------
//   Selection
//---------------------------------------------------------

class Selection {
      ElementList _el;        // valid when SEL_SINGLE or SEL_MULT

   public:
      SelState state;
      int tickStart;          // selection start time tick
      int tickEnd;            // selection end time tick
      int staffStart;         // valid if selState is SEL_STAFF
      int staffEnd;           // valid if selState is SEL_STAFF

      ElementList* elements() { return &_el; }
      void add(Element*);
      void add(ElementList& ns);
      QRectF deselectAll(Score*);
      void remove(Element*);
      QRectF clear();
      Element* element() const;
      ChordRest* firstChordRest() const;
      ChordRest* lastChordRest() const;
      void update();
      void updateState();
      void dump();
      const char* mimeType() const;
      QByteArray mimeData() const;
      };

#endif

