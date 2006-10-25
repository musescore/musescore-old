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

#include "plist.h"
#include "element.h"

class Segment;
class Xml;
class System;
class Beam;
class Tuplet;
class Staff;
class Chord;
class BarLine;
class TextElement;
class ChordRest;
class Score;
class Painter;

typedef pstl::plist<Beam*>::iterator iBeam;
typedef pstl::plist<Beam*>::const_iterator ciBeam;

typedef pstl::plist<Beam*> BeamList;
typedef BeamList::iterator iBeam;
typedef BeamList::const_iterator ciBeam;

typedef QList<Tuplet*> TupletList;

//---------------------------------------------------------
//   MeasureWidth
//    return value for layoutX()
//---------------------------------------------------------

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
//    per staff values of measure
//---------------------------------------------------------

struct MStaff {
      BarLine* endBarLine;
      double distance;
      double userDistance;

      MStaff() {
            endBarLine = 0;
            }
      };

typedef QList<MStaff> MStaffList;
typedef MStaffList::iterator iMStaff;
typedef MStaffList::const_iterator ciMStaff;

//---------------------------------------------------------
//   Measure
//    one measure in a system
//---------------------------------------------------------

class Measure : public Element {
      Segment* _first;        // segment list
      Segment* _last;
      int _size;              // number of segments

      bool _startRepeat;
      int _endRepeat;
      int _ending;

      MStaffList  staves;
      BeamList    _beamList;
      TupletList  _tuplets;
      ElementList _pel;       // page relative elements (text)
      ElementList _sel;       // measure(/tick) relative elements

      int    _no;             // measure number, counting from zero
      int    _noOffset;       // offset to measure number
      TextElement* _noText;   // measure number text object

      double _userStretch;
      bool _lineBreak;        // forced line break
      bool _pageBreak;        // forced page break;
      bool _irregular;        // irregular measure, do not count

      void push_back(Segment* e);
      void push_front(Segment* e);

   public:
      Measure(Score*);
      ~Measure();
      virtual ElementType type() const { return MEASURE; }

      virtual void draw(Painter&);
      virtual void read(QDomNode, int idx);
      virtual void write(Xml&, int, int) const;
      virtual bool isMovable() const { return true; }
      virtual void add(Element*);
      virtual void remove(Element*);

      MStaffList* staffList()          { return &staves;      }
      BeamList*   beamList()           { return &_beamList;   }
      TupletList* tuplets()            { return &_tuplets;    }

      ElementList* pel()               { return &_pel;        }
      const ElementList* pel() const   { return &_pel;        }

      int    no() const                { return _no;          }
      bool   irregular() const         { return _irregular;   }
      void   setIrregular(bool val)    { _irregular = val;    }
      int    noOffset() const          { return _noOffset;    }
      TextElement* noText() const            { return _noText;      }
      void   setNoText(const QString& s);
      void   setNo(int n)              { _no = n;             }
      void   setNoOffset(int n)        { _noOffset = n;       }
      double distance(int i) const     { return staves[i].distance; }
      double userDistance(int i) const { return staves[i].userDistance; }

      int size() const            { return _size;       }
      virtual int tickLen() const;
      void moveAll(double, double);
      Segment* first() const      { return _first;      }
      Segment* last() const       { return _last;       }
      void remove(Segment*);
      bool empty()                { return _first == 0; }

      double userStretch() const  { return _userStretch; }
      bool lineBreak() const      { return _lineBreak; }
      bool pageBreak() const      { return _pageBreak; }
      void setUserStretch(double v)  { _userStretch = v;  }
      void setLineBreak(bool v)   { _lineBreak = v;    }
      void setPageBreak(bool v)   { _pageBreak = v;    }

      void addBeam(Beam* b);
      void addTuplet(Tuplet* b);

      System* system() const        { return (System*)parent(); }
      void setSystem(System* s)     { setParent((Element*)s);   }

      ElementList* el()             { return &_sel; }
      const ElementList* el() const { return &_sel; }
      BarLine* barLine(int staff) const     { return staves[staff].endBarLine;   }

      MeasureWidth layoutX(double stretch);
      void layout(double width);
      void moveY(int, double);
      void layout2();

      Chord* findChord(int tick, int staff, int voice, bool grace);
      ChordRest* findChordRest(int tick, Staff* staff, int voice, bool grace);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      void insertStaff(Staff*, int staff);
      void insertStaff1(Staff*, int);
      void insertMStaff(MStaff staff, int idx);
      void removeMStaff(MStaff staff, int idx);

      Element* findSelectableElement(QPointF p) const;
      void layoutBeams();
      void layoutNoteHeads(int staff);

      void moveTicks(int diff);
      void insert(Segment* ns, Segment* s);

      void cmdRemoveStaves(int s, int e);
      void cmdAddStaves(int s, int e);

      Measure* next() const { return (Measure*)Element::next(); }
      Measure* prev() const { return (Measure*)Element::prev(); }

      double tick2pos(int) const;
      Segment* tick2segment(int) const;

      void sortStaves(std::list<int>& src, std::list<int>& dst);

      void dump() const;
      virtual bool acceptDrop(int, int) const;
      virtual void drop(const QPointF&, int, int);

      bool startRepeat() const      { return _startRepeat; }
      void setStartRepeat(bool val) { _startRepeat = val; }
      int endRepeat() const         { return _endRepeat; }
      void setEndRepeat(bool val)   { _endRepeat = val; }

      int ending() const         { return _ending; }
      void setEnding(int r)      { _ending = r;    }
      };

//---------------------------------------------------------
//   MeasureList
//    list of measures
//---------------------------------------------------------

class MeasureList : public pstl::plist<Measure*> {
   public:
      MeasureList() {}
      };

typedef MeasureList::iterator iMeasure;
typedef MeasureList::reverse_iterator riMeasure;
typedef MeasureList::const_iterator ciMeasure;
typedef MeasureList::const_reverse_iterator criMeasure;

#endif

