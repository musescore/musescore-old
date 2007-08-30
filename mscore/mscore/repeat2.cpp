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
// Add Volta, da capo, dal segno, fine, coda
//
#include "repeat2.h"
#include "measure.h"

RepeatStack* firstStack;
int   rloopCounter;
int   rtickOffSet;



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
      n->setStartMeasure(0);
      n->setEndMeasure(0);
      n->setActive(false);
      n->setRepeatType(UNKNOWN);
      return (n);
      }

//******************************************************
// function push, put measure and other infos on stack
//******************************************************
int RepeatStack::push(Measure* m) 
      {
      RepeatStack* p = 0;

      int type = 0;
      int ret = false;

      if ( firstStack == 0 ) // No Stack!!!!
            abort ();

      p = getStartMeasure(m);
      if (!p || (!getNoOffElements()&&!p->getStartMeasure())) {
            if (!m->prev())
                  type = CAPO;            
            else if (m->startRepeat())
                  type = NORMAL;
            else if (m->ending() == 1)
                  type = VOLTA1;
            //Add more types here, may be segno or something else
            if (type == 0)            
                  return ret; 

            p = setNewSlot(m);
            p->setRepeatType(type);
            }
      else
            type = p->getRepeatType();


      int a = p->getActive();      
      switch (type) {
            case NORMAL:
            case CAPO:
                  {
                  if (a == true) {
                        p->setTicks2Add(m->tick());
                        p->setTickOffset(0);
                        rloopCounter = p->getLoopCount();
                        }
                  if (a == false) {
                        if (rloopCounter < p->getLoopCount()) 
                              // increment LoopCount
                              rloopCounter++;
                        }                      
                        break;
                  }
            case VOLTA1:
                  {
                        if (a == true) { // first time
                              p->setTicks2Add(m->tickLen());
                              p->setActive(false);
                              }
                        else { // second time
                              p->setActive(0x04);
                              if (m->endRepeat()) {
                                    p->setEndMeasure(m);
                                    p->setTickOffset(p->getTicks2Add()+
                                                      m->tickLen());
                                    rtickOffSet = rtickOffSet - p->getTicks2Add();
                                    p->setLoopCount(m->endRepeat());
                                    }
                              ret = true;                                    
                              }
                        break;
                  }
            default:
                  break;
            }
      return ret;
      }

RepeatStack* RepeatStack::setNewSlot(Measure* m)
      {
      RepeatStack* p;
      RepeatStack* n;

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
      Measure* ret = 0;
      RepeatStack* p;
      int type;
      bool lc = 0;


      if (firstStack == 0) // No Stack!!!!
            abort ();

      if (m->endRepeat())
            type = NORMAL;
      else
            return 0;
      //Add more types here, may be segno or something else
      
      ret = m;
      p = getLastStartMeasure(m);
      if (!p)
            p = getLastEndMeasure(m);
      if (!p)
            p = getLastSlotByType(type);
      if (!p)
            p = firstStack;      
      if (!p)
            return 0;
      if (!type)      
            type = p->getRepeatType();
      while (!lc) {  
            switch (type) {
                  case CAPO:
                  case NORMAL:
                        {
                        switch (p->getActive()) {
                              case false:            
                                    {
                                    if (rloopCounter == p->getLoopCount()) {
                                          rloopCounter = 1;
                                          p->setActive(0x04);
                                          }
                                    ret = 0;  
                                    break;
                                    }
                              case true:
                                    {
                                    p->setActive(false);
                                    p->setEndMeasure(m);
                                    p->setTickOffset((m->tick() - 
                                                      p->getTicks2Add()) +
                                                      m->tickLen());
                                    p->setTicks2Add(p->getTickOffset());
                                    rtickOffSet += p->getTickOffset();
                                    p->setLoopCount(m->endRepeat());
                                    ret = p->getStartMeasure();
                                    break;
                                    }
                              default:
                                    {
                                    ret = 0;
                                    break;
                                    }
                              }
                        lc = 1;
                        break;
                        }
                  case VOLTA1:
                        {
                        if (p->getActive() == true) {
                              p->setActive(false);
                              lc = 1;
                              }
                        else if (p->getActive() == false) {
                                    p = getLastStartMeasure(m);
                                    if (p) {
                                          type = p->getRepeatType();
                                          lc = 0;
                                          }
                                    else
                                          lc = 1;
                              ret = 0;
                              lc = 1;
                              }
                        else {
                              ret = 0;
                              lc = 1;
                              }      
                        break;
                        }
                  default :
                        {
                        lc = 1;
                        break;
                        }
                  }
            }
      return ret;
      }

RepeatStack* RepeatStack::getStartMeasure(Measure* m)
      {
      RepeatStack* p;

      p = firstStack;
      while (p != 0) {
           if (p->getStartMeasure() == m) 
                  break;
                  p = p->getNext();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastSlotByType(int type)
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getRepeatType() == type && p->getActive() == true)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastActiveSlot()
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() == true)
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
            if (p->getActive() == false)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastSpecialSlot()
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() >= 0x02)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastStartMeasure(Measure* m)
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getStartMeasure() == m && p == getLastActiveSlot())
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getLastEndMeasure(Measure* m)
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getEndMeasure() == m && p == getLastInactiveSlot())
                  break;
            p = p->getPrev();
            }
      return p;
      }
         
RepeatStack* RepeatStack::getLastSlot()
      {
      RepeatStack* p;

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

            


      
      


