//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  Some Code inspired by "The JAZZ++ Midi Sequencer"
//  Copyright (C) 1994-2000 Andreas Voss and Per Sigmond, all rights reserved.
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

#ifndef __HARMONY_H__
#define __HARMONY_H__

#include "text.h"

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

class HChord {
      static const char* const scaleNames[2][12];
      static const HChord C0;

   protected:
      int keys;

   public:
      HChord()      { keys = 0; }
      HChord(int k) { keys = k; }
      HChord(int a, int b, int c=-1, int d=-1, int e=-1, int f=-1, int g=-1,
            int h=-1, int i=-1, int k=-1, int l=-1);
      HChord(const char*);
      void rotate(int semiTones);
      static const char* scaleName(int key, bool flat = false) {
            return scaleNames[flat][key % 12];
            }
      bool contains(int key) const {       // key in chord?
            return (1 << (key % 12)) & keys;
            }
      HChord& operator+= (int key) {
            keys |= (1 << (key % 12));
            return *this;
            }
      HChord& operator-= (int key) {
            keys &= ~(1 << (key % 12));
            return *this;
            }
      bool operator==(const HChord& o) const { return (keys == o.keys); }
      bool operator!=(const HChord& o) const { return (keys != o.keys); }

      QString name(int tpc);
      };

//---------------------------------------------------------
//   class HDegree
//---------------------------------------------------------

enum HDegreeType {
      UNDEF, ADD, ALTER, SUBTRACT
      };

class HDegree {
      int _value;
      int _alter;
      int _type;

   public:
      HDegree() { _value = 0; _alter = 0; _type = UNDEF; }
      HDegree(int v, int a, int t) { _value = v; _alter = a; _type = t; }
      int value() const { return _value; }
      int alter() const { return _alter; }
      int type() const  { return _type; }
};

//---------------------------------------------------------
//   class Harmony
//
//    root note and bass note are notatated as
//    "tonal pitch class":
//
//           bb  b   -   #  ##
//            0,  7, 14, 21, 28,  // C
//            2,  9, 16, 23, 30,  // D
//            4, 11, 18, 25, 32,  // E
//           -1,  6, 13, 20, 27,  // F
//            1,  8, 15, 22, 29,  // G
//            3, 10, 17, 24, 31,  // A
//            5, 12, 19, 26, 33,  // B
//---------------------------------------------------------

class Harmony : public Text {
      int _baseTpc;     // bass note, chord base; used for "slash" chords
      int _extension;   // chord number, index in table "extensionNames"
      int _rootTpc;     // root note for chord
      QList<HDegree> _degreeList;

   public:
      Harmony(Score*);
      ~Harmony();
      virtual Harmony* clone() const   { return new Harmony(*this); }
      virtual ElementType type() const { return HARMONY; }

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      virtual void endEdit();

      int baseTpc() const                  { return _baseTpc;      }
      void setBaseTpc(int val)             { _baseTpc = val;       }
      int rootTpc() const                  { return _rootTpc;      }
      void setRootTpc(int val)             { _rootTpc = val;       }
      int extension() const                { return _extension;    }
      void setExtension(int val)           { _extension = val;     }
      void addDegree(HDegree d)            { _degreeList << d;     }
      int numberOfDegrees()                { return _degreeList.size();   }
      HDegree degree(int i)                { return _degreeList.value(i); }

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      QString harmonyName() const;
      QString extensionName() const { return getExtensionName(extension()); }
      void buildText();

      static const char* getExtensionName(int i);
      static QString harmonyName(int root, int extension, int base);
      static int parseHarmony(const QString& s, int* root, int* base);
      };

#endif

