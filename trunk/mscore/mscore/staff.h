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

class Instrument;
class ClefList;
class Xml;
class Part;
class Score;

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

class Staff {
      Score* _score;
      Part* _part;
      int _rstaff;                  // index in Part
      ClefList* _clef;
      int _bracket;
      int _bracketSpan;             // bracket this number of staves

   public:
      Staff(Score*, Part*, int);
      ~Staff();
      ClefList* clef() const         { return _clef; }
      int key(int tick) const;
      bool isTop() const             { return _rstaff == 0; }
      bool isTopSplit() const;
      QString trackName() const;
      QString longName() const;
      QString shortName() const;
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
      int bracket() const            { return _bracket;     }
      int bracketSpan() const        { return _bracketSpan; }
      void setBracket(int val)       { _bracket = val;      }
      void setBracketSpan(int val)   { _bracketSpan = val;  }
      };

//---------------------------------------------------------
//   StaffList
//---------------------------------------------------------

class StaffList : public std::vector<Staff*> {
   public:
      void remove(Staff*);
      int idx(const Staff* p) const;
      };

typedef StaffList::iterator iStaff;
typedef StaffList::const_iterator ciStaff;
typedef StaffList::reverse_iterator riStaff;
#endif

