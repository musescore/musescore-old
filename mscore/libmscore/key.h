//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __KEY_H__
#define __KEY_H__

class Xml;
class Score;

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

class KeySigEvent {
      int _accidentalType;          // -7 -> +7
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
      void initLineList(char*);
      };

//---------------------------------------------------------
//   AccidentalState
//---------------------------------------------------------

static const int TIE_CONTEXT = 0x10;

class AccidentalState {
      char state[75];    // -7 --- +7   | TIE_CONTEXT

   public:
      AccidentalState() {}
      void init(const KeySigEvent&);
      int accidentalVal(int line) const {
            return state[line] & ~TIE_CONTEXT;
            }
      bool tieContext(int line) const {
            return state[line] & TIE_CONTEXT;
            }
      void setAccidentalVal(int line, int val, bool tieContext = false) {
            state[line] = val | (tieContext ? TIE_CONTEXT : 0);
            }
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
      };

extern int transposeKey(int oldKey, int semitones);

#endif

