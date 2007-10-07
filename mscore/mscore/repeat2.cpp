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
#include "element.h"
#include "symbol.h"
#include "repeat.h"
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


//int RepeatStack::push(Measure* m)
Measure* RepeatStack::push(Measure* m)
      {
      RepeatStack* p = 0x00;
      RepeatStack* pp = 0x00;
      Measure* mm = 0x00;

      int type = 0;
      Measure* ret = 0x00;

      if ( firstStack == 0 ) // No Stack!!!!
            abort ();
      if (!playRepeats)
            return m;

      ret = m;
      type = checkType(m);
      while (type) {
            if (type&NORMAL) {
                  if ((p = getSlot(m,NORMAL)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(NORMAL);
                        }
                  type &= ~(NORMAL);
                  }

            if (type&FINE) {
                  p = getSlotByType(FINE);
                  if (!p) {
                        p = setNewSlot(m);
                        p->setRepeatType(FINE);
                        p->setLoopCount(2);
                        }
                  if ((type&(DACAPO|DALSEGNO))) {
                        if (p) {
                              p->setActive(SECONDTIME);
                              p->setLoopCount(p->getLoopCount()-1);
                              }
                        }
                  else {
                        if (m != p->getStartMeasure()) {
                        // only one fine allowed, the last is the winner
                              p->setStartMeasure(m);
                              }
                        }
                  type &= ~(FINE);
                  }

            if (type&SEGNO) {
                  if ((p = getSlotByType(SEGNO)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(SEGNO);
                        }
                  if (p->getActive() == FIRSTTIME) {
                        p->setTicks2Add(m->tick());
                        p->setStartMeasure(m);
                        if ((pp = getSlotByType(DALSEGNO))) {
                              p->setEndMeasure(pp->getStartMeasure());
                              }
                        }
                  type &= ~(SEGNO);
                  }

            if (type&DALSEGNO) {
            // D.S. and RepeatEnd does not make sense, ignore D.S.
                  if (!(type&END)) {
                        if ((p = getSlot(m,DALSEGNO)) == 0x00) {
                              p = setNewSlot(m);
                              p->setRepeatType(DALSEGNO);
                              }
                        if (p->getActive() == FIRSTTIME) {
                              p->setStartMeasure(m);
                              p->setTicks2Add(m->tick());
                              if ((pp = getSlotByType(SEGNO))) {
                                    p->setEndMeasure(pp->getStartMeasure());
                                    pp->setEndMeasure(m);
                                    }
                              p->setLoopCount(2);
                              }
                        }
                  type &= ~(DALSEGNO);
                  }

            if (type&DACAPO) {
            // D.C. and RepeatEnd does not make sense, ignore D.C.
                  if (!(type&END)) {
                        if ((p = getSlot(m,DACAPO)) == 0x00) {
                              p = setNewSlot(m);
                              p->setRepeatType(DACAPO);
                              }
                        if (p->getActive() == FIRSTTIME) {
                              p->setStartMeasure(m);
                              p->setTicks2Add(m->tick());
                              if ((pp = getSlotByType(CAPO))) {
                                    p->setEndMeasure(pp->getStartMeasure());
                                    pp->setEndMeasure(m);
                                    }
                              p->setLoopCount(2);
                              }
                        }
                  type &= ~(DACAPO);
                  }

            if (type&CODA) {
                  if ((p = getSlot(m,CODA)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(CODA);
                        }
                  if (p->getActive() == FIRSTTIME) {
                        p->setTicks2Add(m->tick());
                        p->setStartMeasure(m);
                        }
                  type &= ~(CODA);
                  }

            if (type&ALCODA) {
                  if ((p = getSlot(m,ALCODA)) == 0x00) {
                        mm = searchCoda(m,-1);
                        if (mm) {
                              p = setNewSlot(m);
                              p->setRepeatType(ALCODA);
                              p->setEndMeasure(mm);
                              }
                        }
                  type &= ~(ALCODA);
                  }

            if (type&CODETTA) {
                  if ((p = getSlot(m,CODETTA)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(CODETTA);
                        }
                  p->setTicks2Add(m->tick());
                  type &= ~(CODETTA);
                  }

            if (type&CAPO) {
                  if ((p = getSlot(m,CAPO)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(CAPO);
                        }
                        if ((pp = getSlotByType(DACAPO))) {
                              pp->setEndMeasure(m);
                              }
                  p->setTicks2Add(m->tick());
                  type &= ~(CAPO);
                  }

            if (type&S_VOLTA) {
                  if ((p = getSlot(m,S_VOLTA)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(S_VOLTA);
                        p->setLoopCount(2);
                        }
                  if (repeatEachTime && p->getActive() == THIRDTIME) {
                        pp = p->getNext();
                        while (pp) {
                              if (pp->getRepeatType() == S_VOLTA) {
                                    pp->setActive(FIRSTTIME);
                                    if (pp->getStartMeasure()->endRepeat()) {
                                          getSlot(pp->getStartMeasure(),END)->setActive(FIRSTTIME);
                                          getSlot(pp->getStartMeasure(),END)->setLoopCount(2);
                                          }
                                    pp = pp->getNext();
                                    }
                              else
                                    break;
                              }
                        p->setActive(FIRSTTIME);
                        p->setLoopCount(2);
                        }
                  if (p->getActive() == FIRSTTIME) {
                        // check end of seconda volta
                        int tl = 0;
                        for (mm = m; mm;) {
//WS:TODO                              if (mm->ending() != 2) {
//                                    p->setEndMeasure(mm);
//                                    break;
//                                    }
                              tl += m->tickLen();
                              mm = mm->next();
                              }
                        p->setTicks2Add(tl);
                        if (!mm)
                              p->setEndMeasure(m);
                        p->setLoopCount(p->getLoopCount()-1);
                        p->setActive(SECONDTIME);
                        mm = m->prev();
                        // if the measure before was end of repeat and prima volta,
                        // accept this as a part of the repeat-loop and in this case
                        // the first time is to take as second time, play the part
#if 0 //WS:TODO
                        if ((!mm) ||
                              (!((mm && mm->repeatFlags() & RepeatEnd)
                               && mm->ending() == 1))) {
                              ret = p->getEndMeasure();
                              rtickOffSet = rtickOffSet - p->getTicks2Add();
                              push(ret);
                              }
#endif
                        }
                  else {
                        p->setLoopCount(p->getLoopCount()-1);
                        p->setActive(THIRDTIME);
                        }
                  type &= ~(S_VOLTA);
                  }

            if (type&P_VOLTA) {
                  if ((p = getSlot(m,P_VOLTA)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(P_VOLTA);
                        p->setLoopCount(2);
                        }
                  if (repeatEachTime && p->getActive() == THIRDTIME) {
                        if (type&END) {
                              pp = getSlot(m,END);
                              pp->setActive(FIRSTTIME);
                              pp->setLoopCount(1);
                              }
                        pp = p->getNext();
                        while (pp) {
                              if (pp->getRepeatType() == P_VOLTA) {
                                    pp->setActive(FIRSTTIME);
                                    if (pp->getStartMeasure()->endRepeat()) {
                                          getSlot(pp->getStartMeasure(),END)->setActive(FIRSTTIME);
                                          getSlot(pp->getStartMeasure(),END)->setLoopCount(1);
                                          }
                                    pp = pp->getNext();
                                    }
                              else
                                    break;
                              }
                        p->setActive(FIRSTTIME);
                        }
                  if (p->getActive() == FIRSTTIME) {
                        // check end of prima volta
                        int tl = 0;
                        for (mm = m; mm;) {
//WS:TODO                              if (mm->ending() != 1) {
//                                    p->setEndMeasure(mm);
//                                    break;
//                                    }
                              tl += m->tickLen();
                              mm = mm->next();
                              }
                        p->setTicks2Add(tl);
                        if (!mm)
                              p->setEndMeasure(m);
                        p->setLoopCount(2);
                        }
                 else {
                        p->setLoopCount(p->getLoopCount()-1);
                        ret = p->getEndMeasure();
                        rtickOffSet = rtickOffSet - p->getTicks2Add();
                        p->setActive(THIRDTIME);
                        }
                  type &= ~(P_VOLTA);
                  }

            if (type&START) {
                  if ((p = getSlot(m,START)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(START);
                        }
                  if (repeatEachTime)
                        p->setActive(FIRSTTIME);
                  if (p->getActive() == FIRSTTIME) {
                        pp = getLastSlotByType(END);
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
                  type &= ~(START);
                  }

            if (type&END) {
                  if ((p = getSlot(m,END)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(END);
                        }
                        if (p->getActive() == FIRSTTIME) {
                              pp = getSlot(p->getEndMeasure(),START);
                              if (!pp)
                                    pp = getLastActiveSlotByType(START);
                              if (!pp)
                                    pp = getSlotByType(CAPO);
                              if (pp) {
                                    p->setEndMeasure(pp->getStartMeasure());
                                    pp->setEndMeasure(m);
                                    p->setTicks2Add(m->tick());
                                    p->setLoopCount(m->endRepeat());
                                    }
                              }
                  type &= ~(END);
                  }


            if (type&T_VOLTA) {
                  type &= ~(T_VOLTA);
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
      n->setActive(FIRSTTIME);
      return n;
      }

//******************************************************
// function pop, get measure and other infos from stack
//******************************************************

Measure* RepeatStack::pop(Measure* m)
      {
      Measure* ret = 0x00;
      Measure* mm = 0x00;
      RepeatStack* p = 0x00;
      RepeatStack* pp = 0x00;
      int type = 0;

      if (!firstStack) // No Stack!!!!
            abort ();

      if (!playRepeats)
            return 0;

      ret = m;

      type = checkType(m);

      while (type) {
            if (type&CAPO) { //MUST be the first, don't change position at top
                  ret = 0x00;
                  type &= ~(CAPO);
                  }
            if (type&NORMAL) {
                  ret = 0x00;
                  type &= ~(NORMAL);
                  }
            if (type&CODA) {
                  p = getSlot(m,CODA);
                  ret = 0x00;
                  if (p && p->getActive() == FIRSTTIME) {
                        p->setActive(SECONDTIME);
                        }
                  else {
                        pp = getSlotByType(ALCODA);
                        if (pp) {
                              mm = searchCodetta(m->next(),1);
                              if (mm) {
                                    mm = push(mm);
                                    pp = getSlot(mm,CODETTA);
                                    p->setEndMeasure(pp->getStartMeasure());
                                    pp->setEndMeasure(p->getStartMeasure());
                                    p->setActive(THIRDTIME);
                                    p->setTicks2Add(p->getTicks2Add()+m->tickLen());
                                    rtickOffSet -= (pp->getTicks2Add()-p->getTicks2Add());
                                    ret = p->getEndMeasure();
                                    }
                              }
                        }
                  type &= ~(CODA);
                  }
            if (type&CODETTA) {
                  ret = 0x00;
                  type &= ~(CODETTA);
                  }
            if (type&ALCODA) {
                  ret = 0x00;
                  type &= ~(ALCODA);
                  }
            if (type&SEGNO) {
                  ret = 0x00;
                  type &= ~(SEGNO);
                  }
            if (type&FINE) {
                  ret = 0x00;
                  if ((p = getSlotByType(FINE)) != 0x00) {
                        if (!(type&(DACAPO|DALSEGNO))) {
                              if (p->getLoopCount() <= 1) {
                                    for (;m->next() != 0x00; m = m->next()) {}
                                    ret = m;
                                    type = 0;
                                    }
                              }
                        }
                  type &= ~(FINE);
                  }
            if (type&P_VOLTA) {
                  p = getSlot(m,P_VOLTA);
                  if (p && p->getActive() == FIRSTTIME) {
                        p->setActive(SECONDTIME);
                        }
                  ret = 0x00;
                  type &= ~(P_VOLTA);
                  }
            if (type&S_VOLTA) {
                  p = getSlot(m,S_VOLTA);
                  if (p && p->getActive() == FIRSTTIME) {
                        p->setActive(SECONDTIME);
                        }
                  ret = 0x00;
                  type &= ~(S_VOLTA);
                  }
            if (type&T_VOLTA) {
                  ret = 0x00;
                  type &= ~(T_VOLTA);
                  }
            if (type&DACAPO) {
            // D.C. and RepeatEnd does not make sense, ignore D.C.
                  if (!(type&END)) {
                        p =getSlot(m,DACAPO);
                        pp = getSlot(p->getEndMeasure(),CAPO);
                        if (pp) {
                              if (p->getActive() == FIRSTTIME) {
                                    p->setTickOffset((p->getTicks2Add() -
                                                      pp->getTicks2Add()) +
                                                      m->tickLen());
                                    ret = p->getEndMeasure();
                                    p->setActive(SECONDTIME);
                                    }
                              if (p->getLoopCount() <= 1) {
                                    if (repeatEachTime)
                                          p->setActive(FIRSTTIME);
                                    else
                                          p->setActive(THIRDTIME);
                                    ret = 0x00;
                                    }
                              else {
                                    p->setLoopCount(p->getLoopCount()-1);
                                    rtickOffSet += p->getTickOffset();
                                    ret = p->getEndMeasure();
                                    }
                              }
                        }
                  type &= ~(DACAPO);
                  }
            if (type&DALSEGNO) {
            // D.S. and RepeatEnd does not make sense, ignore D.S.
                  if (!(type&END)) {
                        p = getSlot(m,DALSEGNO);
                        pp = getSlot(p->getEndMeasure(),SEGNO);
                        if (pp) {
                              if (p->getActive() == FIRSTTIME) {
                                    p->setTickOffset((p->getTicks2Add() -
                                                      pp->getTicks2Add()) +
                                                      m->tickLen());
                                    ret = p->getEndMeasure();
                                    p->setActive(SECONDTIME);
                                    }
                              if (p->getLoopCount() <= 1) {
                                    if (repeatEachTime)
                                          p->setActive(FIRSTTIME);
                                   else
                                          p->setActive(THIRDTIME);
                                    ret = 0x00;
                                    }
                              else {
                                    p->setLoopCount(p->getLoopCount()-1);
                                    rtickOffSet += p->getTickOffset();
                                    ret = p->getEndMeasure();
                                    }
                              }
                        else { // here if no "segno" before "dal segno"
                              if (p->getPrev())
                                    p->getPrev()->setNext(p->getNext());
                              if (p->getNext())
                                    p->getNext()->setPrev(p->getPrev());
                              delStackElement(p);
                              ret = 0x00;
                              }
                        }
                  type &= ~(DALSEGNO);
                  }
            if (type&START) {
                  p = getSlot(m,START);
                  if (p->getActive() == THIRDTIME && repeatEachTime) {
                        p->setActive(FIRSTTIME);
                        pp = getSlot(p->getEndMeasure(),END);
                        pp->setActive(FIRSTTIME);
                        }
                  ret = 0x00;
                  type &= ~(START);
                  }
            if (type&END) {
                  p = getSlot(m,END);
                  pp = getSlot(p->getEndMeasure(),START);
                  if (!pp)
                        pp = getSlotByType(CAPO);
                  if (p->getActive() == FIRSTTIME) {
                        p->setTickOffset((p->getTicks2Add() -
                                          pp->getTicks2Add()) +
                                          m->tickLen());
                        p->setActive(SECONDTIME);
                        }
                  if (p->getLoopCount() <= 1) {
                        if (repeatEachTime)
                              p->setActive(FIRSTTIME);
                        else
                              p->setActive(THIRDTIME);
                        ret = 0x00;
                        }
                  else {
                        p->setLoopCount(p->getLoopCount()-1);
                        rtickOffSet += p->getTickOffset();
                        ret = p->getEndMeasure();
                        }
                  type &= ~(END);
                  }
            }
      return ret;
      }

Measure* RepeatStack::searchCoda(Measure* m, int dir) {
      Measure* mm;

      mm = m;
      while (mm) {
            if ((mm->repeatFlags()&RepeatCoda))
                  break;
            if (dir == 1)
                  mm = mm->next();
            if (dir == -1)
                  mm = mm->prev();
            }
      return mm;
      }

Measure* RepeatStack::searchCodetta(Measure* m, int dir) {
      Measure* mm;

      mm = m;
      while (mm) {
            if ((mm->repeatFlags()&RepeatCodetta))
                  break;
            if (dir == 1)
                  mm = mm->next();
            if (dir == -1)
                  mm = mm->prev();
            }
      return mm;
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

RepeatStack* RepeatStack::getSlotByType(int type)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p != 0) {
           if (p->getRepeatType() == type)
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
            if (p->getActive() == FIRSTTIME && p->getRepeatType() != 0)
                  break;
            p = p->getPrev();
            }
      return p;
      }

RepeatStack* RepeatStack::getSlotByActiveAndType(int a, int type)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p) {
            if (p->getActive() == a && p->getRepeatType() == type)
                  break;
            p = p->getNext();
            }
      return p;
      }


RepeatStack* RepeatStack::getLastActiveSlotByType(int type)
      {
      RepeatStack* p;

      for (p = firstStack; p->getNext() != 0; p = p->getNext())
            ;
      while (p) {
            if (p->getActive() == FIRSTTIME && p->getRepeatType() == type)
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
            if (p->getActive() == SECONDTIME && p->getRepeatType() == type)
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
            if (p->getActive() == SECONDTIME && p->getRepeatType() != 0)
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
            if (p->getStartMeasure() == m)
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
            if (p->getEndMeasure() == m)
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
      if (m->endRepeat() > 0)
            type |= END;
#if 0 // WS:TODO
      if (m->ending() == 1)
            type |= P_VOLTA;
      if (m->ending() == 2)
            type |= S_VOLTA;
      if (m->ending() == 3)
            type |= T_VOLTA;
#endif
      if (m->repeatFlags() & RepeatStart)
            type |= START;
      if (m->repeatFlags() & RepeatSegno)
            type |= SEGNO;
      if (m->repeatFlags() & RepeatCoda)
            type |= CODA;
      if (m->repeatFlags() & RepeatVarcoda)
            type |= VARCODA;
      if (m->repeatFlags() & RepeatCodetta)
            type |= CODETTA;
      if (m->repeatFlags() & RepeatDacapo)
            type |= DACAPO;
      if (m->repeatFlags() & RepeatDacapoAlFine)
            type |= (DACAPO | FINE);
      if (m->repeatFlags() & RepeatDacapoAlCoda)
            type |= (DACAPO | ALCODA);
      if (m->repeatFlags() & RepeatDalSegno)
            type |= DALSEGNO;
      if (m->repeatFlags() & RepeatDalSegnoAlFine)
            type |= (DALSEGNO | FINE);
      if (m->repeatFlags() & RepeatDalSegnoAlCoda)
            type |= (DALSEGNO | ALCODA);
      if (m->repeatFlags() & RepeatAlSegno)
            type |= ALSEGNO;
      if (m->repeatFlags() & RepeatFine)
            type |= FINE;
      if (!type)
            type |= NORMAL;
      return type;
      }
