//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: measure.h,v 1.40 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

class Segment;
class Xml;
class System;
class Beam;
class Tuplet;
class Staff;
class Chord;
class BarLine;
class Text;
class ChordRest;
class Score;
class Viewer;

//---------------------------------------------------------
//   MeasureWidth
//---------------------------------------------------------

/**
 Return value for layoutX().
*/

struct MeasureWidth {
      double stretchable;
      double nonStretchable;

      MeasureWidth() {}
      MeasureWidth(double a, double b) {
            stretchable = a;
            nonStretchable = b;
            }
      };

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

/**
 Per staff values of measure.
*/

struct MStaff {
      BarLine* endBarLine;
      double distance;
      double userDistance;

      MStaff() {
            endBarLine = 0;
            distance = .0;
            userDistance = .0;
            }
      };

typedef QList<MStaff> MStaffList;
typedef MStaffList::iterator iMStaff;
typedef MStaffList::const_iterator ciMStaff;

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

/**
 One measure in a system.
*/

class Measure : public Element {
      Segment* _first;        ///< First item of segment list
      Segment* _last;         ///< Last item of segment list
      int _size;              ///< Number of items on segment list

      bool _startRepeat;
      int _endRepeat;
      int _ending;

      MStaffList  staves;
      QList<Beam*>    _beamList;
      QList<Tuplet*>  _tuplets;
      ElementList _pel;       ///< Page relative elements (i.e. text)
      ElementList _sel;       ///< Measure(/tick) relative elements: with defined start time
                              ///< but outside the staff

      int    _no;             ///< Measure number, counting from zero
      int    _noOffset;       ///< Offset to measure number
      Text* _noText;          ///< Measure number text object

      double _userStretch;
      bool _lineBreak;        ///< Forced line break
      bool _pageBreak;        ///< Forced page break
      bool _irregular;        ///< Irregular measure, do not count

      void push_back(Segment* e);
      void push_front(Segment* e);
      void adjustToLen(int, int);

   public:
      Measure(Score*);
      ~Measure();
      virtual Measure* clone() const { return new Measure(*this); }
      virtual ElementType type() const { return MEASURE; }

      virtual void draw(QPainter&);

      virtual void read(QDomElement, int idx);
      virtual void write(Xml&, int, int) const;
      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      virtual bool isMovable() const { return false; }
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      MStaffList* staffList()          { return &staves;      }
      QList<Beam*>* beamList()         { return &_beamList;   }
      QList<Tuplet*>* tuplets()        { return &_tuplets;    }

      ElementList* pel()               { return &_pel;        }
      const ElementList* pel() const   { return &_pel;        }

      int    no() const                { return _no;          }
      bool   irregular() const         { return _irregular;   }
      void   setIrregular(bool val)    { _irregular = val;    }
      int    noOffset() const          { return _noOffset;    }
      Text* noText() const             { return _noText;      }
      void   setNoText(const QString& s);
      void   setNo(int n)              { _no = n;             }
      void   setNoOffset(int n)        { _noOffset = n;       }
      double distance(int i) const     { return staves[i].distance; }
      double userDistance(int i) const { return staves[i].userDistance; }

      int size() const                 { return _size;       }
      virtual int tickLen() const;
      void moveAll(double, double);
      Segment* first() const           { return _first;      }
      Segment* last() const            { return _last;       }
      void remove(Segment*);
      bool empty()                     { return _first == 0; }

      double userStretch() const       { return _userStretch; }
      bool lineBreak() const           { return _lineBreak; }
      bool pageBreak() const           { return _pageBreak; }
      void setUserStretch(double v)    { _userStretch = v;  }
      void setLineBreak(bool v)        { _lineBreak = v;    }
      void setPageBreak(bool v)        { _pageBreak = v;    }

      void addBeam(Beam* b);

      System* system() const        { return (System*)parent(); }
      void setSystem(System* s)     { setParent((Element*)s);   }

      ElementList* el()             { return &_sel; }
      const ElementList* el() const { return &_sel; }
      BarLine* barLine(int staff) const     { return staves[staff].endBarLine;   }

      MeasureWidth layoutX(ScoreLayout*, double stretch);
      void layout(ScoreLayout*, double width);
      void moveY(int, double);
      void layout2(ScoreLayout*);

      Chord* findChord(int tick, int staff, int voice, bool grace);
      ChordRest* findChordRest(int tick, Staff* staff, int voice, bool grace);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      void insertStaff(Staff*, int staff);
      void insertStaff1(Staff*, int);
      void insertMStaff(MStaff staff, int idx);
      void removeMStaff(MStaff staff, int idx);

      void layoutBeams(ScoreLayout*);
      void layoutNoteHeads(int staff);

      void moveTicks(int diff);
      void insert(Segment* ns, Segment* s);

      void cmdRemoveStaves(int s, int e);
      void cmdAddStaves(int s, int e);
      void removeStaves(int s, int e);
      void insertStaves(int s, int e);

      Measure* next() const { return (Measure*)Element::next(); }
      Measure* prev() const { return (Measure*)Element::prev(); }

      double tick2pos(int) const;
      Segment* tick2segment(int) const;

      void sortStaves(QList<int>& src, QList<int>& dst);

      void dump() const;
      virtual bool acceptDrop(Viewer*, const QPointF&, int, const QDomElement&) const;
      virtual Element* drop(const QPointF&, const QPointF&, int, const QDomElement&);

      bool startRepeat() const      { return _startRepeat; }
      void setStartRepeat(bool val) { _startRepeat = val; }
      int endRepeat() const         { return _endRepeat; }
      void setEndRepeat(bool val)   { _endRepeat = val; }

      int ending() const            { return _ending; }
      void setEnding(int r)         { _ending = r;    }
      Segment* getSegment(Element* el);
      Segment* findSegment(Segment::SegmentType st, int t);
      Segment* createSegment(Segment::SegmentType st, int t);
      void setEndBarLine(BarLine* barLine);
      void cmdRemoveEmptySegment(Segment* s);
      void collectElements(QList<Element*>& el);
      };

//---------------------------------------------------------
//   MeasureList
//---------------------------------------------------------

/**
 List of measures.
*/

class MeasureList : public QList<Measure*> {
   public:
      MeasureList() {}
      };

typedef MeasureList::iterator iMeasure;
typedef MeasureList::const_iterator ciMeasure;

#endif

