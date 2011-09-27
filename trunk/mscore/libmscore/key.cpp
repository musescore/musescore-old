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

#include "key.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "pitchspelling.h"

//---------------------------------------------------------
//   KeySigEvent
//---------------------------------------------------------

KeySigEvent::KeySigEvent()
      {
      _accidentalType = 0;
      _naturalType    = 0;
      _customType     = 0;
      _custom         = false;
      _invalid        = true;
      }

KeySigEvent::KeySigEvent(int n)
      {
      _accidentalType = n;
      _naturalType    = 0;
      _customType     = 0;
      _custom         = false;
      _invalid        = false;
      enforceLimits();
      }

//---------------------------------------------------------
//   enforceLimits - ensure _accidentalType and _naturalType
//   are within acceptable limits (-7 .. +7).
//   see KeySig::layout()
//---------------------------------------------------------

void KeySigEvent::enforceLimits()
      {
      const char* msg = 0;
      if (_accidentalType < -7) {
            _accidentalType = -7;
            msg = "accidentalType < -7";
            }
      else if (_accidentalType > 7) {
            _accidentalType = 7;
            msg = "accidentalType > 7";
            }
      if (_naturalType < -7) {
            _naturalType = -7;
            msg = "naturalType < -7";
            }
      else if (_naturalType > 7) {
            _naturalType = 7;
            msg = "naturalType > 7";
            }
      if (msg)
            printf("KeySigEvent: %s\n", msg);
      }

//---------------------------------------------------------
//   setCustomType
//---------------------------------------------------------

void KeySigEvent::setCustomType(int v)
      {
      _accidentalType = 0;
      _customType     = v;
      _custom         = true;
      _invalid        = false;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void KeySigEvent::print() const
      {
      printf("<KeySigEvent: ");
      if (_invalid)
            printf("invalid>");
      else {
            if (_custom)
                  printf("nat %d custom %d>", _naturalType, _customType);
            else
                  printf("nat %d accidental %d>", _naturalType, _accidentalType);
            }
      }

//---------------------------------------------------------
//   setAccidentalType
//---------------------------------------------------------

void KeySigEvent::setAccidentalType(int v)
      {
      _accidentalType = v;
      _custom         = false;
      _invalid        = false;
      enforceLimits();
      }

//---------------------------------------------------------
//   KeySigEvent::operator==
//---------------------------------------------------------

bool KeySigEvent::operator==(const KeySigEvent& e) const
      {
      if ((e._invalid != _invalid) || (e._custom != _custom))
            return false;
      if (_custom)
            return e._customType == _customType;
      else
            return e._accidentalType == _accidentalType;
      }

//---------------------------------------------------------
//   KeySigEvent::operator!=
//---------------------------------------------------------

bool KeySigEvent::operator!=(const KeySigEvent& e) const
      {
      if ((e._invalid != _invalid) || (e._custom != _custom))
            return true;
      if (_custom)
            return e._customType != _customType;
      else
            return e._accidentalType != _accidentalType;
      }

//---------------------------------------------------------
//   initLineList
//    preset lines list with accidentals for given key
//---------------------------------------------------------

void AccidentalState::init(const KeySigEvent& ks)
      {
      int type = ks.accidentalType();
      memset(state, 0, 74);
      for (int octave = 0; octave < 11; ++octave) {
            if (type > 0) {
                  for (int i = 0; i < type; ++i) {
                        int idx = tpc2step(20 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = 1;
                        }
                  }
            else {
                  for (int i = 0; i > type; --i) {
                        int idx = tpc2step(12 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = -1;
                        }
                  }
            }
      }


//---------------------------------------------------------
//   key
//---------------------------------------------------------

KeySigEvent KeyList::key(int tick) const
      {
      if (empty())
            return KeySigEvent();
      ciKeyList i = upper_bound(tick);
      if (i == begin())
            return KeySigEvent();
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   KeyList::write
//---------------------------------------------------------

void KeyList::write(Xml& xml, const char* name) const
      {
      xml.stag(name);
      for (ciKeyList i = begin(); i != end(); ++i) {
            if (i->second.custom())
                  xml.tagE("key tick=\"%d\" custom=\"%d\"", i->first, i->second.customType());
            else
                  xml.tagE("key tick=\"%d\" idx=\"%d\"", i->first, i->second.accidentalType());
            }
      xml.etag();
      }

//---------------------------------------------------------
//   KeyList::read
//---------------------------------------------------------

void KeyList::read(QDomElement e, Score* cs)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "key") {
                  KeySigEvent ke;
                  int tick = e.attribute("tick", "0").toInt();
                  if (e.hasAttribute("custom"))
                        ke.setCustomType(e.attribute("custom").toInt());
                  else
                        ke.setAccidentalType(e.attribute("idx").toInt());
                  (*this)[cs->fileDivision(tick)] = ke;
                  }
            else
                  domError(e);
            }
      }

#if 0
//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void KeyList::removeTime(int tick, int len)
      {
      KeyList tmp;
      for (ciKeyList i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0)) {
                  if (i->first >= tick + len)
                        tmp[i->first - len] = i->second;
                  else
                        printf("remove key event\n");
                  }
            else
                  tmp[i->first] = i->second;
            }
      clear();
      insert(tmp.begin(), tmp.end());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void KeyList::insertTime(int tick, int len)
      {
      KeyList tmp;
      for (ciKeyList i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0))
                  tmp[i->first + len] = i->second;
            else
                  tmp[i->first] = i->second;
            }
      clear();
      insert(tmp.begin(), tmp.end());
      }
#endif

//---------------------------------------------------------
//   transposeKey
//    -  F# changes to Gb
//    -  Cb changes to enharmonic B
//    -  C# changes to enharmonic Db
//
//     0    1    2    3     4    5    6     7    8     9    10    11
//    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
//    "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
//---------------------------------------------------------

int transposeKey(int key, int semitones)
      {
      //                         Gb Db Ab Eb  Bb   F  C  G  D  A  E  B   F#
      //                         -6 -5 -4 -3  -2  -1  0  1  2  3  4  5   6
      //                          0  1  2  3   4  5   6  7  8  9 10  11  12
      static const int kp[] =  {  6, 1, 8, 3, 10, 5,  0, 7, 2, 9, 4, 11, 6  };

//                                C  Db D  Eb  E   F  Gb  G Ab  A  Bb  B
      static const int kp1[] = {  6, 1, 8,  3, 10, 5,  0, 7, 2, 9,  4, 11 };

      while (semitones < 0)
            semitones += 12;
      semitones %= 12;

printf("transposeKey key %d semitones %d\n", key, semitones);

      // switch to enharmonic key:
      if (key == -7)
            key = 5;
      else if (key == 7)
            key = -5;
//      printf("  transposeKey key %d semitones %d\n", key, semitones);

      key += 6;         // normalize key to 0 - 13

      int kpitch = kp[key];
      kpitch = (kpitch + semitones) % 12;
      key = kp1[kpitch] - 6;
printf("  key %d\n", key);
      return key;
      }

//---------------------------------------------------------
//   initFromSubtype
//    for backward compatibility
//---------------------------------------------------------

void KeySigEvent::initFromSubtype(int st)
      {
      union U {
            int subtype;
            struct {
                  int _accidentalType:4;
                  int _naturalType:4;
                  unsigned _customType:16;
                  bool _custom : 1;
                  bool _invalid : 1;
                  };
            };
      U a;
      a.subtype       = st;
      _accidentalType = a._accidentalType;
      _naturalType    = a._naturalType;
      _customType     = a._customType;
      _custom         = a._custom;
      _invalid        = a._invalid;
      enforceLimits();
      }

