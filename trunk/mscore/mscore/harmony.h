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

      QString name(int key, bool flat);
      };

//---------------------------------------------------------
//   class HDegree
//---------------------------------------------------------

enum HDegreeType {
      ADD, ALTER, SUBTRACT
      };

class HDegree {
      int _value;
      int _alter;
      int _type;

   public:
      HDegree(int v, int a, int t) { _value = v; _alter = a; _type = t; }
      int value() const { return _value; }
      int alter() const { return _alter; }
      int type() const  { return _type; }
};

//---------------------------------------------------------
//   class Harmony
//---------------------------------------------------------

class Harmony : public Text {
      unsigned char _base;
      int _extension;
      unsigned char _root;
      QList<HDegree> _degreeList;

   public:
      Harmony(Score*);
      ~Harmony();
      virtual Harmony* clone() const   { return new Harmony(*this); }
      virtual ElementType type() const { return HARMONY; }

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      virtual void endEdit();

      unsigned char base() const           { return _base;      }
      void setBase(unsigned char val)      { _base = val;       }
      unsigned int extension() const       { return _extension; }
      void setExtension(unsigned char val) { _extension = val;  }
      unsigned char root() const           { return _root;      }
      void setRoot(unsigned char val)      { _root = val;       }
      void addDegree(HDegree d)            { _degreeList << d;  }

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      QString harmonyName() const;
      QString extensionName() const { return getExtensionName(extension()); }
      void buildText();

      static const char* getExtensionName(int i);
      static QString harmonyName(int root, int extension, int base);
      static int parseHarmony(const QString& s, int* root, int* base);
      static QString rootName(int root);
      };

#endif

