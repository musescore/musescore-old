//=============================================================================
//  MuseScore
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
      subtype = 0;
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            _invalidR        = true;
            }
      else {
            _invalid        = true;
            }
#else
      _invalid        = true;
#endif
      }

KeySigEvent::KeySigEvent(int v)
      {
      subtype = v;
      }

//---------------------------------------------------------
//   accidentalType
//---------------------------------------------------------

int KeySigEvent::accidentalType()  const
      {
#ifdef Q_WS_MAC
      return QSysInfo::ByteOrder == QSysInfo::BigEndian ? _accidentalTypeR : _accidentalType;
#else
      return _accidentalType;
#endif
      }

//---------------------------------------------------------
//   naturalType
//---------------------------------------------------------

int KeySigEvent::naturalType() const
      {
#ifdef Q_WS_MAC
      return QSysInfo::ByteOrder == QSysInfo::BigEndian ? _naturalTypeR : _naturalType;
#else
      return _naturalType;
#endif
      }

//---------------------------------------------------------
//   customType
//---------------------------------------------------------

unsigned KeySigEvent::customType() const
      {
#ifdef Q_WS_MAC
      return QSysInfo::ByteOrder == QSysInfo::BigEndian ? _customTypeR : _customType;
#else
      return _customType;
#endif
      }

//---------------------------------------------------------
//   custom
//---------------------------------------------------------

bool KeySigEvent::custom() const
      {
#ifdef Q_WS_MAC
      return QSysInfo::ByteOrder == QSysInfo::BigEndian ? _customR : _custom;
#else
      return _custom;
#endif
      }

//---------------------------------------------------------
//   isValid
//---------------------------------------------------------

bool KeySigEvent::isValid() const
      {
#ifdef Q_WS_MAC
      return QSysInfo::ByteOrder == QSysInfo::BigEndian ? !_invalidR : !_invalid;
#else
      return !_invalid;
#endif
      }



//---------------------------------------------------------
//   setCustomType
//---------------------------------------------------------

void KeySigEvent::setCustomType(int v)
      {
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            _accidentalTypeR = 0;
            _customTypeR     = v;
            _customR         = true;
            _invalidR        = false;
            }
      else {
            _accidentalType = 0;
            _customType     = v;
            _custom         = true;
            _invalid        = false;
            }
#else
      _accidentalType = 0;
      _customType     = v;
      _custom         = true;
      _invalid        = false;
#endif
      }

//---------------------------------------------------------
//   setAccidentalType
//---------------------------------------------------------

void KeySigEvent::setAccidentalType(int v)
      {
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            _accidentalTypeR = v;
            _customR         = false;
            _invalidR        = false;
            }
      else {
            _accidentalType = v;
            _custom         = false;
            _invalid        = false;
            }
#else
      _accidentalType = v;
      _custom         = false;
      _invalid        = false;
#endif
      }

//---------------------------------------------------------
//   setNaturalType
//---------------------------------------------------------

void KeySigEvent::setNaturalType(int v)
      {
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            _naturalTypeR = v;
            }
      else {
            _naturalType = v;
            }
#else
      _naturalType = v;
#endif
      }

//---------------------------------------------------------
//   setCustom
//---------------------------------------------------------

void KeySigEvent::setCustom(bool v)
      {
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            _customR = v;
            }
      else {
            _custom = v;
            }
#else
      _custom = v;
#endif
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void KeySigEvent::print() const
      {
      printf("<KeySigEvent: ");
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            if (_invalidR)
                  printf("invalid>");
            else {
                  if (_customR)
                        printf("nat %d custom %d>", _naturalTypeR, _customTypeR);
                  else
                        printf("nat %d accidental %d>", _naturalTypeR, _accidentalTypeR);
                  }
            }
      else {
            if (_invalid)
                  printf("invalid>");
            else {
                  if (_custom)
                        printf("nat %d custom %d>", _naturalType, _customType);
                  else
                        printf("nat %d accidental %d>", _naturalType, _accidentalType);
                  }
            }
#else
      if (_invalid)
            printf("invalid>");
      else {
            if (_custom)
                  printf("nat %d custom %d>",_naturalType, _customType);
            else
                  printf("nat %d accidental %d>", _naturalType, _accidentalType);
            }
#endif
      }

//---------------------------------------------------------
//   KeySigEvent::operator==
//---------------------------------------------------------

bool KeySigEvent::operator==(const KeySigEvent& e) const
      {
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            if (e._invalidR || _invalidR || (e._customR != _customR))
                  return false;
            if (_customR)
                  return e._customTypeR== _customTypeR;
            else
                  return e._accidentalTypeR == _accidentalTypeR;
            }
      else {
            if (e._invalid || _invalid || (e._custom != _custom))
                  return false;
            if (_custom)
                  return e._customType == _customType;
            else
                  return e._accidentalType == _accidentalType;
            }
#else
      if (e._invalid || _invalid || (e._custom != _custom))
            return false;
      if (_custom)
            return e._customType == _customType;
      else
            return e._accidentalType == _accidentalType;
#endif

      }

//---------------------------------------------------------
//   KeySigEvent::operator!=
//---------------------------------------------------------

bool KeySigEvent::operator!=(const KeySigEvent& e) const
      {
#ifdef Q_WS_MAC
      if(QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            if (e._invalidR || _invalidR || (e._customR != _customR))
                  return true;
            if (_customR)
                  return e._customTypeR != _customTypeR;
            else
                  return e._accidentalTypeR != _accidentalTypeR;
            }
      else {
            if (e._invalid || _invalid || (e._custom != _custom))
                  return true;
            if (_custom)
                  return e._customType != _customType;
            else
                  return e._accidentalType != _accidentalType;
            }
#else
      if (e._invalid || _invalid || (e._custom != _custom))
            return true;
      if (_custom)
            return e._customType != _customType;
      else
            return e._accidentalType != _accidentalType;
#endif

      }

//---------------------------------------------------------
//   key
//---------------------------------------------------------

KeySigEvent KeyList::key(int tick) const
      {
      if (empty())
            return 0;
      ciKeyList i = upper_bound(tick);
      if (i == begin())
            return 0;
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
//   initLineList
//    preset lines list with accidentals for given key
//---------------------------------------------------------

void AccidentalState::init(int type)
      {
      memset(state, 2, 74);
      for (int octave = 0; octave < 11; ++octave) {
            if (type > 0) {
                  for (int i = 0; i < type; ++i) {
                        int idx = tpc2step(20 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = 1 + 2;
                        }
                  }
            else {
                  for (int i = 0; i > type; --i) {
                        int idx = tpc2step(12 + i) + octave * 7;
                        if (idx < 74)
                              state[idx] = -1 + 2;
                        }
                  }
            }
      }


