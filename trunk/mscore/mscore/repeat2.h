//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  $Id: repeat2.h,v 1.00 2007/08/23 14:00:00 dikrau Exp $
//
//  Copyright (C) 2007- Dieter Krause (dikrau@users.sourceforge.net)
//
//  repeat2.h: contains class and function definition
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

#ifndef __REPEAT2_H__
#define __REPEAT2_H__

// #include "repeatflag.h"

class Measure;
class RepeatStack;
class RepeatFlag;
class Element;

#define MAXCYCLE 15

enum RepTypes// types for repeat
{
      NO          = 0,
      START       = 1<<1,
      END         = 1<<2,
      SEGNO       = 1<<3,
      DALSEGNO    = 1<<4,
      CODA        = 1<<5,
      CODETTA     = 1<<6,
      VARCODA     = 1<<6,
      CAPO        = 1<<7,
      DACAPO      = 1<<8,
      ALCODA      = 1<<9,
      P_VOLTA     = 1<<10,
      S_VOLTA     = 1<<11,
      T_VOLTA     = 1<<12,
      FINE        = 1<<13,
      ALSEGNO     = 1<<14,
      REPEATMES   = 1<<15,
      NORMAL      = 1<<16
};

enum LoopCounts
{
      FIRSTTIME = 1,
      SECONDTIME = 2,
      THIRDTIME = 3,
      FOURTHTIME = 4
};

class RepList {
      RepList* _next;
      Measure* _measure;
      RepeatFlag* _rf;
      int _repeatFlag;

public:
      RepList();
      ~RepList();

      void setNext(RepList* n) { _next = n; }
      RepList* next() { return _next; }
      void setMeasure(Measure* m) { _measure = m; }
      Measure* measure() { return _measure; }
      void setRepeatFlag(int f) { _repeatFlag |= f; }
      void resetRepeatFlag(int f) { _repeatFlag &= ~f; }
      int repeatFlag() { return _repeatFlag; }
      void setRf(RepeatFlag* r) { _rf = r; }
      RepeatFlag* getRf() { return _rf; }
};

class RepeatStack {

      RepeatStack* _prev; // Previouse Element
      RepeatStack* _next; // Next Element
      int _noOffElements; // Number off All Elements on Stack
      int _active;        // Indicates the Element is active at loopcount #, is incr.
      int _glLoopCount;   // Global loop counter
      int _loopCount;     // count number of loops, build from RepeatFlag::_Cycles, is decr.
      int _repeatType;    // type of repeat
      Measure* _startm;   // Measure to start with
      Measure* _endm;     // Measure to end with
      int _ticks2Add;     // add ticks to
      int _tickOffs;      // previouse Offset
      int _cycleList[MAXCYCLE]; // list of cycles, build from RepeatFlag::_cycleList
      int _no;            // element no., get from RepeatFlag::_no
      int _destno;        // destination number from RepeatFlag::_destno


    public:
      RepeatStack();
      ~RepeatStack();

      Measure* push(Measure*);
      Measure* pop(Measure*);

      RepeatStack* init();
      bool delStackElement(RepeatStack *);
      void setNext(RepeatStack* rn) { _next = rn; }
      RepeatStack* next() { return _next; }
      void setPrev(RepeatStack* rn) { _prev = rn; }
      RepeatStack* prev() { return _prev; }
      void setActive(int s) { _active = s; }
      int active() { return _active; }
      void setLoopCount(int n) { _loopCount = n; }
      int loopCount() { return _loopCount; }
      void setGlLoopCount(int n) { _glLoopCount = n; }
      int glLoopCount() { return _glLoopCount; }
      void setRepeatType(int n) { _repeatType = n; }
      int repeatType() { return _repeatType; }
      void setNoOffElements(int);
      int noOffElements();
      int checkType(Measure*);
      void setStartMeasure(Measure* m) { _startm = m; }
      Measure* startMeasure () { return _startm; }
      void setEndMeasure(Measure* m) { _endm = m; }
      Measure* endMeasure () { return _endm; }
      RepeatStack* setNewSlot(Measure*);
      void setTickOffset(int t) { _tickOffs = t; }
      int tickOffset() { return _tickOffs; }
      void setTicks2Add(int t) { _ticks2Add = t; }
      int ticks2Add() { return _ticks2Add; }
      void buildCycleList(RepeatStack*,QString);
      void setCycleList(int i,int v) { _cycleList[i] = v; }
      int cycleList(int idx) { return _cycleList[idx]; }
      void setNo(int n) { _no = n; };
      int no() { return _no; }
      void setDestNo(int n) { _destno = n; };
      int destNo() { return _destno; }
      bool isInList(RepeatStack*,int);
      bool collectRepeats(Element*,Measure*);

      Measure* searchCoda(Measure*,int);
      Measure* searchCodetta(Measure*,int);
      Measure* searchVarcoda(Measure*,int);
      Measure* searchSegno(Measure*,int);

      RepeatStack* lastStartMeasure(Measure*);
      RepeatStack* slotEndMeasureByType(Measure*,int);
      RepeatStack* startMeasure(Measure*);
      RepeatStack* slot(Measure*,int);
      RepeatStack* lastSlot();
      RepeatStack* lastActiveSlot();
      RepeatStack* lastInactiveSlot();
      RepeatStack* lastSpecialSlot();
      RepeatStack* lastSlotByType(int);
      RepeatStack* slotByType(int);
      RepeatStack* lastActiveSlotByType(int);
      RepeatStack* lastInActiveSlotByType(int);
      RepeatStack* slotByActiveAndType(int,int);
};

extern RepList* rList;
extern RepeatStack* firstStack;
extern int rtickOffSet;
extern bool repeatEachTime;
extern int rCycles;

#endif
