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

#ifndef __MEASURE_H__
#define __MEASURE_H__

/**
 \file
 Definition of classes MStaff, Measure and MeasureList.
*/

#include "segment.h"
#include "measurebase.h"
#include "al/fraction.h"

class Xml;
class Beam;
class Tuplet;
class Staff;
class Chord;
class Text;
class ChordRest;
class Score;
class ScoreView;
class System;
class Note;
class Spacer;
class AccidentalState;

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

/**
 Per staff values of measure.
*/

struct MStaff {
      double distance;
      StaffLines*  lines;
      Spacer* _vspacer;
      bool hasVoices;         ///< indicates that MStaff contains more than one voice,
                              ///< this changes some layout rules
      bool _visible;
      bool _slashStyle;

      MStaff();
      ~MStaff();
      bool visible() const         { return _visible;    }
      void setVisible(bool val)    { _visible = val;     }
      bool slashStyle() const      { return _slashStyle; }
      void setSlashStyle(bool val) { _slashStyle = val;  }
      };

enum {
      RepeatEnd         = 1,
      RepeatStart       = 2,
      RepeatMeasureFlag = 4,
      RepeatJump        = 8
      };


/**
      One measure in a system.
*/

class Measure : public MeasureBase {
      Q_DECLARE_TR_FUNCTIONS(Measure)

      Segment* _first;        ///< First item of segment list
      Segment* _last;         ///< Last item of segment list
      int _size;              ///< Number of items on segment list

      int _repeatCount;       ///< end repeat marker und repeat count
      int _repeatFlags;       ///< or'd RepeatType's

      QList<MStaff*>  staves;
      QList<Tuplet*>  _tuplets;

      int    _no;             ///< Measure number, counting from zero
      int    _noOffset;       ///< Offset to measure number
      Text* _noText;          ///< Measure number text object

      double _userStretch;

      bool _irregular;              ///< Irregular measure, do not count
      bool _breakMultiMeasureRest;  ///< set by user
      bool _breakMMRest;            ///< set by layout

      bool _endBarLineGenerated;
      bool _endBarLineVisible;
      int _endBarLineType;

      int _mmEndBarLineType;       ///< bar line type if this measure is presented
                                    ///< as multi measure rest

      int _multiMeasure;      // set from layout();
                              //   0 - normal measure
                              // > 0 - multi measure rest;
                              // < 0 - skipped measure

      QColor _endBarLineColor;

      mutable int _tickLen;   // cached value
      mutable unsigned sigSerial;

      void push_back(Segment* e);
      void push_front(Segment* e);

   public:
      Measure(Score*);
      ~Measure();
      virtual Measure* clone() const   { return new Measure(*this); }
      virtual ElementType type() const { return MEASURE; }

      virtual void read(QDomElement, int idx);
      virtual void write(Xml&, int, bool writeSystemElements) const;
      virtual void write(Xml&) const;
      void writeBox(Xml&) const;
      void readBox(QDomElement);
      virtual bool isEditable() { return false; }

      virtual bool isMovable() const { return false; }
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      System* system() const               { return (System*)parent(); }
      QList<MStaff*>* staffList()          { return &staves;      }
      MStaff* mstaff(int staffIdx)         { return staves[staffIdx]; }
      StaffLines* staffLines(int staffIdx) { return staves[staffIdx]->lines; }
      QList<Tuplet*>* tuplets()            { return &_tuplets;    }
      int no() const                       { return _no;          }
      bool irregular() const               { return _irregular;   }
      void setIrregular(bool val)          { _irregular = val;    }
      int noOffset() const                 { return _noOffset;    }
      Text* noText() const                 { return _noText;      }
      void setNo(int n)                    { _no = n;             }
      void setNoOffset(int n)              { _noOffset = n;       }
      virtual double distance(int i) const { return staves[i]->distance; }
      virtual Spatium userDistance(int i) const;

      int size() const                 { return _size;       }
      virtual int tickLen() const;
      virtual void setTick(int t);

      Segment* first() const           { return _first;      }
      Segment* first(SegmentTypes) const;

      Segment* last() const            { return _last;       }
      Segment* firstCRSegment() const;
      void remove(Segment*);

      double userStretch() const       { return _userStretch; }
      void setUserStretch(double v)    { _userStretch = v;  }

      void layoutX(double stretch);
      void layout(double width);
      void layout2();

      Chord* findChord(int tick, int track, int gl);
      ChordRest* findChordRest(int tick, int track);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      void insertStaff(Staff*, int staff);
      void insertMStaff(MStaff* staff, int idx);
      void removeMStaff(MStaff* staff, int idx);

      virtual void moveTicks(int diff);
      void insert(Segment* ns, Segment* s);

      void cmdRemoveStaves(int s, int e);
      void cmdAddStaves(int s, int e);
      void removeStaves(int s, int e);
      void insertStaves(int s, int e);

      double tick2pos(int) const;
      Segment* tick2segment(int, bool grace = false) const;

      void sortStaves(QList<int>& dst);

      void dump() const;
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);

      int repeatCount() const         { return _repeatCount; }
      void setRepeatCount(int val)    { _repeatCount = val; }

      Segment* getSegment(Element* el);
      Segment* getSegment(SegmentType st, int tick);
      Segment* getSegment(SegmentType st, int tick, int gl);
      Segment* findSegment(SegmentType st, int t);
      Segment* createSegment(SegmentType st, int t);

      bool createEndBarLines();
      void setEndBarLineType(int val, bool g, bool visible = true, QColor color = Qt::black);
      int endBarLineType() const          { return _endBarLineType; }
      int mmEndBarLineType() const        { return _mmEndBarLineType; }
      void setMmEndBarLineType(int v)     { _mmEndBarLineType = v;    }
      bool setStartRepeatBarLine(bool);
      bool endBarLineGenerated() const    { return _endBarLineGenerated; }
      void setEndBarLineGenerated(bool v) { _endBarLineGenerated = v; }
      bool endBarLineVisible() const      { return _endBarLineVisible;   }

      void cmdRemoveEmptySegment(Segment* s);
      virtual void scanElements(void* data, void (*func)(void*, Element*));
      void createVoice(int track);
      void adjustToLen(int, int);
      int repeatFlags() const      { return _repeatFlags; }
      void setRepeatFlags(int val);
      int findAccidental(Note*) const;
      int findAccidental2(Note*) const;
      void exchangeVoice(int, int, int, int);
      void checkMultiVoices(int staffIdx);
      bool hasVoice(int track) const;
      bool isMeasureRest(int staffIdx);
      bool visible(int staffIdx) const;
      bool slashStyle(int staffIdx) const;

      bool breakMultiMeasureRest() const      { return _breakMultiMeasureRest | _breakMMRest; }
      bool breakMMRest() const                { return _breakMMRest; }
      void setBreakMMRest(bool v)             { _breakMMRest = v;    }
      bool getBreakMultiMeasureRest() const   { return _breakMultiMeasureRest; }
      void setBreakMultiMeasureRest(bool val) { _breakMultiMeasureRest = val;  }

      bool isEmpty() const;

      int multiMeasure() const                { return _multiMeasure; }
      void setMultiMeasure(int val)           { _multiMeasure = val;  }
      Fraction fraction() const;
//      void layoutChords1(Segment* segment, int startTrack);
      void layoutChords0(Segment* segment, int startTrack, AccidentalState*);
      void writeTuplets(Xml&, int staff) const;
      };

extern void initLineList(char* ll, int key);

#endif

