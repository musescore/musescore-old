//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __KEY_H__
#define __KEY_H__

class Xml;
class Score;

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

class KeySigEvent {
      int _accidentalType;
      int _naturalType;
      int _customType;
      bool _custom;
      bool _invalid;
      void enforceLimits();

   public:
      KeySigEvent();
      KeySigEvent(int);

      bool isValid() const { return !_invalid; }
      bool operator==(const KeySigEvent& e) const;
      bool operator!=(const KeySigEvent& e) const;
      void setCustomType(int v);
      void setAccidentalType(int v);
      void print() const;

      int accidentalType() const { return _accidentalType; }
      int naturalType() const    { return _naturalType;    }
      void setNaturalType(int v) { _naturalType = v;       }
      int customType() const     { return _customType;     }
      bool custom() const        { return _custom;         }
      bool invalid() const       { return _invalid;        }
      void initFromSubtype(int);    // for backward compatibility
      };

//---------------------------------------------------------
//   KeyList
//    this list is instantiated for every staff
//    to keep track of key signature changes
//---------------------------------------------------------

typedef std::map<const int, KeySigEvent>::iterator iKeyList;
typedef std::map<const int, KeySigEvent>::const_iterator ciKeyList;

class KeyList : public std::map<const int, KeySigEvent> {
   public:
      KeyList() {}
      KeySigEvent key(int tick) const;
      void read(QDomElement, Score*);
      void write(Xml&, const char* name) const;

      void removeTime(int start, int len);
      void insertTime(int start, int len);
      };

extern int transposeKey(int oldKey, int semitones);

#endif

