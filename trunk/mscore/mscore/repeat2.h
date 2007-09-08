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

#include "measure.h"


enum // types for repeat
{ 
      START_REPEAT = 1,
      END_REPEAT = 2,
      SEGNO = 4,
      CODA = 8,
      CAPO = 16,
      LOOP = 32,
      P_VOLTA = 64,
      S_VOLTA = 128,      
      T_VOLTA = 256,
      NORMAL = 4096
};



class RepeatStack {

      RepeatStack* _prev; // Previouse Element
      RepeatStack* _next; // Next Element
      int _noOffElements; // Number off All Elements on Stack, correct value only in firstStack*
      int _active;        // Indicates wether the Element is active (only pushed) or inactive
                          // inactive means the Element was called by pop(),
                          // or elements with the same measure but differend types
      int _loopCount;     
      int _repeatType;    // type of repeat is, "unknown", "normal", "segno", "coda", "capo", "volta" 
      Measure* _startm;   // Measure to start repeating with
      Measure* _endm;     // Measure to repeat to
      int _ticks2Add;     // added ticks while this measure is reached
      int _tickOffs;      // previouse Offset
      

    public:
      RepeatStack();
      ~RepeatStack();

      int push(Measure*);
      Measure* pop(Measure*);

      RepeatStack* init();
      bool delStackElement(RepeatStack *);
      void setNext(RepeatStack* rn) { _next = rn; }
      RepeatStack* getNext() { return _next; }
      void setPrev(RepeatStack* rn) { _prev = rn; }
      RepeatStack* getPrev() { return _prev; } 
      void setActive(int s) { _active = s; }
      int getActive() { return _active; }
      void setLoopCount(int n) { _loopCount = n; }
      int getLoopCount() { return _loopCount; }
      void setRepeatType(int n) { _repeatType = n; }
      int getRepeatType() { return _repeatType; }
      void setNoOffElements(int);
      int getNoOffElements();
      int checkType(Measure*);
      void setStartMeasure(Measure* m) { _startm = m; }
      Measure* getStartMeasure () { return _startm; } 
      void setEndMeasure(Measure* m) { _endm = m; }
      Measure* getEndMeasure () { return _endm; }
      RepeatStack* setNewSlot(Measure*);         
      void setTickOffset(int t) { _tickOffs = t; }
      int getTickOffset() { return _tickOffs; }
      void setTicks2Add(int t) { _ticks2Add = t; }
      int getTicks2Add() { return _ticks2Add; }
      RepeatStack* getLastStartMeasure(Measure*);
      RepeatStack* getLastEndMeasure(Measure*);
      RepeatStack* getStartMeasure(Measure*);
      RepeatStack* getSlot(Measure*,int);
      RepeatStack* getLastSlot();   
      RepeatStack* getLastActiveSlot();   
      RepeatStack* getLastInactiveSlot();   
      RepeatStack* getLastSpecialSlot();   
      RepeatStack* getLastSlotByType(int); 
      RepeatStack* getLastActiveSlotByType(int);
      RepeatStack* getLastInActiveSlotByType(int);     
};

extern RepeatStack* firstStack;
extern int rtickOffSet;
extern int rloopCounter;
extern int raddTickLen;
extern bool repeatEachTime;
//extern bool playRepeats;

#endif
