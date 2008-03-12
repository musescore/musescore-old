//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: measure.h,v 1.40 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "element.h"
#include "segment.h"
#include "measurebase.h"

class Segment;
class Xml;
class Beam;
class Tuplet;
class Staff;
class Chord;
class Text;
class ChordRest;
class Score;
class Viewer;
class System;
class Note;

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

/**
 Per staff values of measure.
*/

struct MStaff {
      double distance;
      double userDistance;
      StaffLines*  lines;

      MStaff();
      ~MStaff();
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
      QList<Beam*>    _beamList;
      QList<Tuplet*>  _tuplets;

      int    _no;             ///< Measure number, counting from zero
      int    _noOffset;       ///< Offset to measure number
      Text* _noText;          ///< Measure number text object

      double _userStretch;
      bool _irregular;        ///< Irregular measure, do not count

      int _endBarLineType;
      bool _endBarLineGenerated;

      void push_back(Segment* e);
      void push_front(Segment* e);
      void layoutBeams(ScoreLayout*);

   public:
      Measure(Score*);
      ~Measure();
      virtual Measure* clone() const   { return new Measure(*this); }
      virtual ElementType type() const { return MEASURE; }

      virtual void read(QDomElement, int idx);
      virtual void write(Xml&, int, bool writeSystemElements) const;
      virtual void write(Xml&) const;
      void writeBox(Xml&) const;
      virtual void read(QDomElement);
      void readBox(QDomElement);
      virtual bool startEdit(const QPointF&) { return false; }

      virtual bool isMovable() const { return false; }
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      System* system() const               { return (System*)parent(); }
      QList<MStaff*>* staffList()          { return &staves;      }
      StaffLines* staffLines(int staffIdx) { return staves[staffIdx]->lines; }
      QList<Beam*>* beamList()             { return &_beamList;   }
      QList<Tuplet*>* tuplets()            { return &_tuplets;    }

      int    no() const                { return _no;          }
      bool   irregular() const         { return _irregular;   }
      void   setIrregular(bool val)    { _irregular = val;    }
      int    noOffset() const          { return _noOffset;    }
      Text* noText() const             { return _noText;      }
      void   setNoText(const QString& s);
      void   setNo(int n)              { _no = n;             }
      void   setNoOffset(int n)        { _noOffset = n;       }
      virtual double distance(int i) const     { return staves[i]->distance; }
      virtual double userDistance(int i) const { return staves[i]->userDistance; }

      int size() const                 { return _size;       }
      virtual int tickLen() const;
      Segment* first() const           { return _first;      }
      Segment* last() const            { return _last;       }
      void remove(Segment*);
      bool empty()                     { return _first == 0; }

      double userStretch() const       { return _userStretch; }
      void setUserStretch(double v)    { _userStretch = v;  }

      void layoutX(ScoreLayout*, double stretch);
      void layout(ScoreLayout*, double width);
      void layout2(ScoreLayout*);

      Chord* findChord(int tick, int track, bool grace);
      ChordRest* findChordRest(int tick, int track);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      void insertStaff(Staff*, int staff);
      void insertStaff1(Staff*, int);
      void insertMStaff(MStaff* staff, int idx);
      void removeMStaff(MStaff* staff, int idx);

      void layoutBeams1(ScoreLayout*);
      void layoutChord(Chord* chord, char*);
      void layout0(int staff);

      virtual void moveTicks(int diff);
      void insert(Segment* ns, Segment* s);

      void cmdRemoveStaves(int s, int e);
      void cmdAddStaves(int s, int e);
      void removeStaves(int s, int e);
      void insertStaves(int s, int e);

      double tick2pos(int) const;
      Segment* tick2segment(int) const;

      void sortStaves(QList<int>& dst);

      void dump() const;
      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);

      int repeatCount() const         { return _repeatCount; }
      void setRepeatCount(int val)    { _repeatCount = val; }

      Segment* getSegment(Element* el);
      Segment* getSegment(Segment::SegmentType st, int tick);
      Segment* findSegment(Segment::SegmentType st, int t);
      Segment* createSegment(Segment::SegmentType st, int t);

      bool createEndBarLines();
      void setEndBarLineType(int val, bool g);
      int endBarLineType() const { return _endBarLineType; }
      bool setStartRepeatBarLine(bool);
      bool endBarLineGenerated() const { return _endBarLineGenerated; }

      void cmdRemoveEmptySegment(Segment* s);
      void collectElements(QList<const Element*>& el) const;
      void createVoice(int track);
      void adjustToLen(int, int);
      int repeatFlags() const      { return _repeatFlags; }
      void setRepeatFlags(int val);
      int findAccidental(Note*);
      };

#endif

