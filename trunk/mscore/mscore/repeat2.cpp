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
      n->setActive(FALSE);
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
      int ret = FALSE;

      if ( firstStack == 0 ) // No Stack!!!!
            abort ();

      p = searchSlot(m);
      if (!p || (!getNoOffElements()&&!p->getStartMeasure())) {
            if (m->prev() == 0)
                  type = CAPO;            
            else if (m->startRepeat())
                  type = NORMAL;
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
                  if (a == TRUE) {
                        p->setTicks2Add(m->tick());
                        p->setTickOffset(0);
                        rloopCounter = p->getLoopCount();
                        }
                  if (a == FALSE) {
                        if (rloopCounter < p->getLoopCount()) 
                              // increment LoopCount
                              rloopCounter++;
                        }                      
                        break;
                  }
            default:
                  break;
            }
      return TRUE;
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
      n->setActive(TRUE);
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


      if (firstStack == 0) // No Stack!!!!
            abort ();
      if (!m->endRepeat())
            return 0;

      ret = m;
      p = searchSlot(0);
      if (!p)
            return 0;
      type = p->getRepeatType();  

      switch (p->getActive()) {
            case FALSE:            
                  {
                  if (rloopCounter == p->getLoopCount()) {
                        rloopCounter = 1;
                        p->setActive(0x04);
                        }
                        ret = 0;  
                        break;
                  }
            case TRUE:
                  {
                  p->setActive(FALSE);
                  p->setEndMeasure(m);
                  p->setTickOffset((m->tick() - p->getTicks2Add()) + m->tickLen());
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
      return ret;
      }

RepeatStack* RepeatStack::searchSlot(Measure* m)
      {
      RepeatStack* p;
      RepeatStack* ret;

      ret = 0;
      p = firstStack;
      if ( m == 0 ) {
            for (; p->getNext() != 0; p = p->getNext())
                  ;
            while (p != 0) {
                  if ( p->getActive() == TRUE || p->getActive() == FALSE) {
                        ret = p;
                        break;
                        }
                  p = p->getPrev();
                  }
            }
      else {
            while (p != 0) {
                  if (p->getStartMeasure() == m) { 
                        ret = p;
                        break;
                        }
                  if (m == 0 && p->getNext() == 0) {
                        ret = p;
                        break;
                        }
                  p = p->getNext();
                  }
            }
      return ret;
      }

bool RepeatStack::delStackElement(RepeatStack* de) 
      {
      delete de;
      return TRUE;
      }

RepeatStack::~RepeatStack() 
      {
      }

            


      
      


