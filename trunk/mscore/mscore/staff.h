//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: staff.h,v 1.10 2006/03/13 21:35:59 wschweer Exp $
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

#ifndef __STAFF_H__
#define __STAFF_H__

/**
 \file
 Definition of classes Staff and StaffList.
*/

class Instrument;
class ClefList;
class Xml;
class Part;
class Score;
class KeyList;

//---------------------------------------------------------
//   BracketItem
//---------------------------------------------------------

struct BracketItem {
      int _bracket;
      int _bracketSpan;

      BracketItem() {
            _bracket = -1;
            _bracketSpan = 0;
            }
      BracketItem(int a, int b) {
            _bracket = a;
            _bracketSpan = b;
            }
      };

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

/**
 Global staff data not directly related to drawing.

 Most functions actually return data of the part the staff is
 associated with.
*/

class Staff {
      Score* _score;
      Part* _part;
      int _rstaff;            ///< Index in Part.
      ClefList* _clef;
      KeyList* _keymap;
      QList <BracketItem> _brackets;

   public:
      Staff(Score*, Part*, int);
      ~Staff();
      int key(int tick) const;
      bool isTop() const             { return _rstaff == 0; }
      bool isTopSplit() const;
      QString trackName() const;
      const QTextDocument& longName() const;
      const QTextDocument& shortName() const;
      int midiChannel() const;
      int midiProgram() const;
      int volume() const;
      int reverb() const;
      int chorus() const;
      int rstaff() const             { return _rstaff; }
      int idx() const;
      void setRstaff(int n)          { _rstaff = n; }
      void read(QDomNode);
      void write(Xml& xml) const;
      Instrument* instrument() const;
      Part* part() const             { return _part;        }
      int bracket(int idx) const;
      int bracketSpan(int idx) const;
      void setBracket(int idx, int val);
      void setBracketSpan(int idx, int val);
      int bracketLevels() const      { return _brackets.size(); }
      void addBracket(BracketItem);
      KeyList* keymap() const        { return _keymap;      }
      ClefList* clef() const         { return _clef; }
      void changeKeySig(int tick, int st);
      void changeClef(int tick, int st);
      };

//---------------------------------------------------------
//   StaffList
//---------------------------------------------------------

/**
 List of staves.
*/

class StaffList : public QList<Staff*> {
   public:
      void remove(Staff*);
      };

typedef StaffList::iterator iStaff;
typedef StaffList::const_iterator ciStaff;
#endif

