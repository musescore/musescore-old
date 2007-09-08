//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  $Id: repeat2.cpp,v 1.00 2007/08/23 14:00:00 dikrau Exp $
//
//  Copyright (C) 2007- Dieter Krause (dikrau@users.sourceforge.net)
//
// repeat2: contains function to handel single and multiple repeats/loops 
//          in a partiture
//          Depands on repeat2.h --> class and function definition
//                  and measure.h
//  
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
//
// TODO :
// Add Volta more comfortable, da capo, dal segno, fine, coda
//
#include "repeat2.h"
#include "measure.h"
#include "mscore.h"

RepeatStack* firstStack;
int   rloopCounter;
int   rtickOffSet;
bool  repeatEachTime;
bool  playRepeats;



RepeatStack::RepeatStack() 
      {
      firstStack = init();
      setNoOffElements(0);
      rtickOffSet = 0;
      }

void RepeatStack::setNoOffElements(int n) 
      {
      firstStack->_noOffElements = n;
      }

int RepeatStack::getNoOffElements() 
      {
      return firstStack->_noOffElements;
      }

RepeatStack* RepeatStack::init() 
      {
      RepeatStack* n = new RepeatStack(*this);
      if ( n <= 0 )
            abort();      
      n->setPrev(0);
      n->setNext(0);
      n->setLoopCount(0);
      n->setStartMeasure(0x00);
      n->setEndMeasure(0x00);
      n->setActive(false);
      n->setRepeatType(NORMAL);
      n->setTicks2Add(0);
      n->setTickOffset(0);
      repeatEachTime = playRepeats;
      return (n);
      }

//******************************************************
// function push, put measure and other infos on stack
//******************************************************


int RepeatStack::push(Measure* m) 
      {
      RepeatStack* p = 0x00;
      RepeatStack* pp = 0x00;

      int type = 0;
      int ret = false;
      int loop = true;

      if ( firstStack == 0 ) // No Stack!!!!
            abort ();
      if (!playRepeats)
            return 0;

      type = checkType(m);
      if (type&NORMAL) {
            if ((p = getSlot(m,NORMAL)) == 0x00) {
                  p = setNewSlot(m);
                  p->setRepeatType(NORMAL);
                  }
            }

      if (type&CAPO) { 
            if ((p = getSlot(m,CAPO)) == 0x00) {
                  p = setNewSlot(m);
                  p->setRepeatType(CAPO);
                  }
            }  
 
      if (type&START_REPEAT) { 
            if ((p = getSlot(m,START_REPEAT)) == 0x00) {
                  p = setNewSlot(m);
                  p->setRepeatType(START_REPEAT);
                  }
            if (p->getActive() == true) {
                  pp = getLastSlotByType(END_REPEAT);
                  if (pp && pp->getEndMeasure() == p->getStartMeasure()) {
                        p->setEndMeasure(pp->getStartMeasure());
                        }
                  else {
                        p->setTicks2Add(m->tick());
                        p->setEndMeasure(0);
                        }
                  }
            else
                  p->setLoopCount(p->getLoopCount()+1);
            } 
  
      if (type&END_REPEAT) {
            if ((p = getSlot(m,END_REPEAT)) == 0x00) {
                  p = setNewSlot(m);
                  p->setRepeatType(END_REPEAT);
                  }
            if (p->getActive() == true) {
                  pp = getLastActiveSlotByType(START_REPEAT);
                  if (!pp)
                        pp = getLastActiveSlotByType(CAPO);
                  if (pp) {
                        p->setEndMeasure(pp->getStartMeasure());
                        pp->setEndMeasure(m);
                        p->setTicks2Add(m->tick());
                        p->setLoopCount(m->endRepeat());
                        }
                  }
            else {
                  if (p->getLoopCount() > 0)
                        p->setLoopCount(p->getLoopCount()-1);
                  }
            }

      if (type&P_VOLTA) {
            if ((p = getSlot(m,P_VOLTA)) == 0x00) {
                  p = setNewSlot(m);
                  p->setRepeatType(P_VOLTA);
                  }
            if (p->getActive() == true) { // first time
                  p->setTicks2Add(m->tickLen());
                  p->setActive(false);
                  }
            else { // second time
                  p->setActive(true);
                  ret = 1;
                  p->setTickOffset(p->getTicks2Add()+
                                   m->tickLen());
                                   p->setLoopCount(m->endRepeat());
                  rtickOffSet = rtickOffSet - p->getTicks2Add();
                  }
            }
      return ret;
      }

RepeatStack* RepeatStack::setNewSlot(Measure* m)
      {
      RepeatStack* p = 0x00;
      RepeatStack* n = 0x00;

      if (getNoOffElements() > 0) {
            n = init();
            for (p = firstStack; p->getNext() != 0; p = p->getNext())
                  ;  //search last entry
            n->setPrev(p);
            p->setNext(n);
            int x = getNoOffElements();
            setNoOffElements(++x);
            }
      else {
            n = firstStack;
            n->setNext(0);
            n->setPrev(0);
            rloopCounter = 1;
            setNoOffElements(1);
            }                        
      n->setLoopCount(1);
      n->setStartMeasure(m);
      n->setEndMeasure(0);
      n->setActive(true);
      return n;
      }

//******************************************************
// function pop, get measure and other infos from stack
//******************************************************

Measure* RepeatStack::pop(Measure* m) 
      {
      Measure* ret = 0x00;
      RepeatStack* p = 0x00;
      RepeatStack* pp = 0x00;
      int type = 0;
      bool lc = 0;


      if (!firstStack) // No Stack!!!!
            abort ();

      if (!playRepeats)
            return 0;

      ret = m;

      type = checkType(m);

      if (type&NORMAL) {
            ret = 0x00;
            }

      if (type&P_VOLTA) {
            ret = 0x00;
            }

      if (type&S_VOLTA) {
            ret = 0x00;
            }

      if (type&T_VOLTA) {
            ret = 0x00;
            }

      if (type&CAPO) {
            ret = 0x00;
            }

      if (type&START_REPEAT) {
            ret = 0x00;
            }

      if (type&END_REPEAT) {
            p = getSlot(m,END_REPEAT);
            pp = getLastActiveSlotByType(START_REPEAT);
            if (!pp)
                  pp = getLastActiveSlotByType(CAPO);
            if (p->getActive() == true) {
                  p->setTickOffset((p->getTicks2Add() -
                                    pp->getTicks2Add()) + 
                                    m->tickLen());
                  ret = p->getEndMeasure();
                  rtickOffSet += p->getTickOffset();
                  p->setLoopCount(p->getLoopCount()-1);
                  p->setActive(false);
                  pp->setActive(false);
                  }                              
            if (p->getLoopCount() <= 0) {
                  p->setActive(0x04);
                  ret = 0x00;
                  }
            }
      return ret;
      }

RepeatStack* RepeatStack::getStartMeasure(Measure* m)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p != 0) {
           if (p->getStartMeasure() == m) 
                  break;
                  p = p->getNext();
            }
      return p;
      }

RepeatStack* RepeatStack::getSlot(Measure* m, int type)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p != 0) {
           if (p->getStartMeasure() == m && p->getRepeatType() == type) 
                  break;
                  p = p->getNext();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastSlotByType(int type)
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getRepeatType() == type)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastActiveSlot()
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() == true && p->getRepeatType() != 0)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastActiveSlotByType(int type)
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() == true && p->getRepeatType() == type)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastInActiveSlotByType(int type)
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() == false && p->getRepeatType() == type)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastInactiveSlot()
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() == false && p->getRepeatType() != 0)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastSpecialSlot()
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() >= 0x02 && p->getRepeatType() != 0)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastStartMeasure(Measure* m)
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getStartMeasure() == m) // && p == getLastActiveSlot())
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastEndMeasure(Measure* m)
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getEndMeasure() == m) // && p == getLastInactiveSlot())
                  break;
            p = p->getPrev();
            }
      return p;
      }
         
RepeatStack* RepeatStack::getLastSlot()
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      return p;
      }

bool RepeatStack::delStackElement(RepeatStack* de) 
      {
      delete de;
      return true;
      }

RepeatStack::~RepeatStack() 
      {
      }

int RepeatStack::checkType(Measure* m)
      {
      int type = 0;

      if (m->prev() == 0)
            type |= CAPO;
      if (m->startRepeat())
            type |= START_REPEAT;      
      if (m->endRepeat() > 0)
            type |= END_REPEAT;
      if (m->ending() == 1)
            type |= P_VOLTA;
      if (m->ending() == 2)
            type |= S_VOLTA;
      if (m->ending() == 3)
            type |= T_VOLTA;
      if (!type)
            type |= NORMAL;
      return type;
      }
            


      
      


