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

#ifndef __STAFF_H__
#define __STAFF_H__

/**
 \file
 Definition of class Staff.
*/

#include "globals.h"
#include "key.h"

class Instrument;
class ClefList;
class Xml;
class Part;
class Score;
class KeyList;
class StaffType;

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
      ClefList* _clefList;
      KeyList* _keymap;
      QList <BracketItem> _brackets;
      int _barLineSpan;       ///< 0 - no bar line, 1 - span this staff, ...
      bool _show;             ///< derived from part->show()
      bool _small;
      bool _invisible;

      bool _updateClefList;
      bool _updateKeymap;

      StaffType* _staffType;

      QMap<int,int> _channelList[VOICES];

   public:
      Staff(Score*, Part*, int);
      ~Staff();
      bool isTop() const             { return _rstaff == 0; }
      QString trackName() const;
      int rstaff() const             { return _rstaff; }
      int idx() const;
      void setRstaff(int n)          { _rstaff = n;    }
      void read(QDomElement);
      void write(Xml& xml) const;
      Instrument* instrument() const;
      Part* part() const             { return _part;        }

      int bracket(int idx) const;
      int bracketSpan(int idx) const;
      void setBracket(int idx, int val);
      void setBracketSpan(int idx, int val);
      int bracketLevels() const      { return _brackets.size(); }
      void addBracket(BracketItem);
      QList <BracketItem> brackets() const { return _brackets; }
      void cleanupBrackets();

      KeyList* keymap() const        { return _keymap;      }
      ClefList* clefList() const     { return _clefList;    }
      void changeKeySig(int tick, KeySigEvent st);
      void changeClef(int tick, int st);
      int clef(int tick) const;
      void setClef(int tick, int clef);
      KeySigEvent key(int tick) const;
      void setKey(int tick, int st);
      void setKey(int tick, const KeySigEvent& st);

      bool show() const              { return _show;        }
      bool slashStyle() const;
      void setShow(bool val)         { _show = val;         }
      bool small() const             { return _small;       }
      void setSmall(bool val)        { _small = val;        }
      bool invisible() const         { return _invisible;   }
      void setInvisible(bool val)    { _invisible = val;    }
      void setSlashStyle(bool val);
      int lines() const;
      void setLines(int val);
      int barLineSpan() const        { return _barLineSpan; }
      void setBarLineSpan(int val)   { _barLineSpan = val;  }
      Score* score() const           { return _score;       }
      double mag() const;
      double height() const;
      double spatium() const;
      int channel(int tick, int voice) const;
      QMap<int,int>* channelList(int voice) { return  &_channelList[voice]; }

      bool useTablature() const;
      void setUseTablature(bool val);
      StaffType* staffType() const     { return _staffType;      }
      void setStaffType(StaffType* st) { _staffType = st;        }
      bool updateClefList() const      { return _updateClefList; }
      bool updateKeymap() const        { return _updateKeymap;   }
      void setUpdateClefList(bool v)   { _updateClefList = v;    }
      void setUpdateKeymap(bool v)     { _updateKeymap = v;      }
      };
#endif

