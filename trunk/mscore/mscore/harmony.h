//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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
//   class HDegree
//---------------------------------------------------------

enum HDegreeType {
      UNDEF, ADD, ALTER, SUBTRACT
      };

class HDegree {
      int _value;
      int _alter;       // -1, 0, 1  (b - - #)
      int _type;

   public:
      HDegree() { _value = 0; _alter = 0; _type = UNDEF; }
      HDegree(int v, int a, int t) { _value = v; _alter = a; _type = t; }
      int value() const { return _value; }
      int alter() const { return _alter; }
      int type() const  { return _type; }
      QString text() const;
      };

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

class HChord {
      QString str;

   protected:
      int keys;

   public:
      HChord()      { keys = 0; }
      HChord(int k) { keys = k; }
      HChord(int a, int b, int c=-1, int d=-1, int e=-1, int f=-1, int g=-1,
            int h=-1, int i=-1, int k=-1, int l=-1);
      HChord(const QString&);

      void rotate(int semiTones);

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

      int getKeys() const { return keys; }
      void print() const;

      QString name(int tpc);
      void add(const QList<HDegree>& degreeList);
      };


//---------------------------------------------------------
//   RenderAction
//---------------------------------------------------------

struct RenderAction {
      enum RenderActionType {
            RENDER_SET, RENDER_MOVE, RENDER_PUSH, RENDER_POP,
            RENDER_NOTE, RENDER_ACCIDENTAL
            };

      RenderActionType type;
      double movex, movey;          // RENDER_MOVE
      QString text;                 // RENDER_SET

      RenderAction() {}
      RenderAction(RenderActionType t) : type(t) {}
      };

//---------------------------------------------------------
//   ChordDescription
//---------------------------------------------------------

struct ChordDescription {
      int id;                 // Chord id number (Band In A Box Chord Number)
      QString name;           // chord name as entered from the keyboard (without root/base)
      QString xmlKind;        // MusicXml description: kind
      QStringList xmlDegrees; // MusicXml description: list of degrees (if any)
      HChord chord;           // C based chord
      QList<RenderAction> renderList;

   public:
      void read(QDomElement);
      void write(Xml&);
      };

//---------------------------------------------------------
//   ChordSymbol
//---------------------------------------------------------

struct ChordSymbol {
      int fontIdx;
      QString name;
      QChar code;

      ChordSymbol() { fontIdx = -1; }
      bool isValid() const { return fontIdx != -1; }
      };

//---------------------------------------------------------
//   ChordFont
//---------------------------------------------------------

struct ChordFont {
      QString family;
      double mag;
      };

//---------------------------------------------------------
//   ChordList
//---------------------------------------------------------

class ChordList : public QMap<int, ChordDescription*> {
      QHash<QString, ChordSymbol> symbols;

   public:
      QList<ChordFont> fonts;
      QList<RenderAction> renderListRoot;
      QList<RenderAction> renderListBase;

      ChordList() {}
      virtual ~ChordList();
      bool write(const QString& path);
      bool read(const QString& path);
      void read(QDomElement);
      ChordSymbol symbol(const QString& s) const { return symbols.value(s); }
      };

typedef QMap<int, ChordDescription*>::iterator iChordDescription;
typedef QMap<int, ChordDescription*>::const_iterator ciChordDescription;

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

struct TextSegment {
      QFont font;
      QString text;
      double x, y;
      bool select;

      double width() const;
      QRectF boundingRect() const;

      TextSegment()                { select = false; x = y = 0.0; }
      TextSegment(const QFont& f, double _x, double _y) : font(f), x(_x), y(_y), select(false) {}
      TextSegment(const QString&, const QFont&, double x, double y);
      void set(const QString&, const QFont&, double x, double y);
      void setText(const QString& t)      { text = t; }
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
      Q_DECLARE_TR_FUNCTIONS(Harmony)
      int _rootTpc;                       // root note for chord
      int _baseTpc;                       // bass note or chord base; used for "slash" chords
                                          // or notation of base note in chord
      int _id;
      QString _userName;

      QList<HDegree> _degreeList;
      QList<QFont> fontList;              // temp values used in render()
      QList<TextSegment*> textList;       // rendered chord

      virtual void draw(QPainter&, ScoreView*) const;
      void render(const QList<RenderAction>& renderList, double&, double&, int tpc);

   public:
      Harmony(Score*);
      Harmony(const Harmony&);
      ~Harmony();
      virtual Harmony* clone() const           { return new Harmony(*this); }
      virtual ElementType type() const         { return HARMONY; }
      Segment* segment() const                 { return (Segment*)parent(); }
      Measure* measure() const                 { return (Measure*)(parent()->parent()); }

      void setId(int d)                        { _id = d; }
      int id() const                           { return _id;           }

      const ChordDescription* descr() const;

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      virtual void layout();

      virtual bool isEditable() const { return true; }
      virtual void startEdit(ScoreView*, const QPointF&);
      virtual void endEdit();

      int baseTpc() const                      { return _baseTpc;      }
      void setBaseTpc(int val)                 { _baseTpc = val;       }
      int rootTpc() const                      { return _rootTpc;      }
      void setRootTpc(int val)                 { _rootTpc = val;       }
      void addDegree(const HDegree& d)         { _degreeList << d;            }
      int numberOfDegrees() const              { return _degreeList.size();   }
      HDegree degree(int i) const              { return _degreeList.value(i); }
      void clearDegrees()                      { _degreeList.clear();         }
      const QList<HDegree>& degreeList() const { return _degreeList;          }

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      QString harmonyName() const;
      void render(const TextStyle* ts = 0);

      void parseHarmony(const QString& s, int* root, int* base);

      QString extensionName() const    { return _id != -1 ? descr()->name       : _userName;     }
      QString xmlKind() const          { return _id != -1 ? descr()->xmlKind    : QString();     }
      QStringList xmlDegrees() const   { return _id != -1 ? descr()->xmlDegrees : QStringList(); }

      void resolveDegreeList();

      virtual bool isEmpty() const;
      virtual qreal baseLine() const;

      const ChordDescription* fromXml(const QString& s,  const QList<HDegree>&);
      const ChordDescription* fromXml(const QString& s);
      virtual void spatiumChanged(double oldValue, double newValue);
      virtual QLineF dragAnchor() const;
      void setHarmony(const QString& s);
      };

#endif

