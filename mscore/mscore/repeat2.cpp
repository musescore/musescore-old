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

#if 0       // TODO: MeasureBase

#include "repeat2.h"
#include "element.h"
#include "symbol.h"
#include "repeat.h"
#include "measure.h"
#include "mscore.h"
#include "repeatflag.h"

RepeatStack* firstStack;
RepList* rList;
int   rCycles;
int   rtickOffSet;
bool  repeatEachTime;
bool  playRepeats;

RepeatStack::RepeatStack()
      {
      firstStack = init();
      rList = new RepList();
      setNoOffElements(0);
      rtickOffSet = 0;
      rCycles = 1;
      }

void RepeatStack::setNoOffElements(int n)
      {
      firstStack->_noOffElements = n;
      }

int RepeatStack::noOffElements()
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
      n->setGlLoopCount(0);
      for (int i = 0; i < MAXCYCLE; i++)
            _cycleList[i] = 0;
      repeatEachTime = playRepeats;
      return (n);
      }

//**************************************************************************************
// function push : input parameter -> measure, output parameter -> measure
//                return : the same measure as input or the new measure to
//                         proceed with, return vale 0x00 means end of partiture
//
// Depending on repeatType the function build loops and jump points
//
// DON'T CHANGE ORDER OF APPEARENCE OF THE IF STATEMENTS
// Result will be a very confused replay or bring the program
// in a loop or crash
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//**************************************************************************************


Measure* RepeatStack::push(Measure* m)
      {
      RepeatStack* p = 0x00;
      RepeatStack* pp = 0x00;
      Measure* mm = 0x00;
      Measure* ret = 0x00;
      RepeatFlag* rp;

      int type = 0;

      if ( firstStack == 0x00 ) // No Stack!!!!
            abort ();
      if (!playRepeats)
            return m;

      ret = m;
      type = checkType(m);
      while (type) {
            if (type&CAPO) {
                  if ((p = slot(m,CAPO)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(CAPO);
                        }
                        if ((pp = slotByType(DACAPO))) {
                              pp->setEndMeasure(m);
                              }
                  p->setGlLoopCount((p->glLoopCount()+1));
                  p->setTickOffset(rtickOffSet);
                  p->setTicks2Add(m->tick());
                  type &= ~(CAPO);
                  }

            if (type&NORMAL) {
                  if ((p = slot(m,NORMAL)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(NORMAL);
                        }
                  p->setTickOffset(rtickOffSet);
                  type &= ~(NORMAL);
                  }

            if (type&REPEATMES) {
                  if ((rp = rp->findRepElement(m,RepeatMeasureFlag)) != 0x00) {
                        p = slot(m,REPEATMES);
                        if (!p) {
                              p = setNewSlot(m);
                              p->setRepeatType(REPEATMES);
                              p->setLoopCount((rp->cycle()));
                              p->buildCycleList(p,(rp->cycleList()));
                              p->setNo((rp->no()));
                              p->setDestNo(rp->destNo());
                              }
                        p->setTickOffset(rtickOffSet);
                        if (isInList(p,p->active())) {
                              if (p->loopCount() > 1) {
                                    p->setLoopCount(p->loopCount()-1);
                                    }
                              else {
                                    p->setActive(p->active()+1);
                                    p->setLoopCount((rp->cycle()));
                                    }
                              ret = m;
                              for (int i = 0; i < p->destNo(); i++)
                                    ret = ret->prev();
                              p->setTicks2Add(m->tick());
                              rtickOffSet += (p->ticks2Add() - ret->tick());
                              }
                        else {
                              p->setActive(p->active()+1);
                              rtickOffSet += (p->ticks2Add() - ret->tick());
                              ret = m->next();
                              if (ret) {
                                    mm = push(ret);
                                    pp = slot(ret,NO);
                                    rtickOffSet += (m->tickLen())*-1;
                                    }
                              }
                        }
                  type = 0;
                  }

            if (type&FINE) {
                  if ((rp = rp->findRepElement(m,RepeatFine)) != 0x00) {
                        p = slotByType(FINE);
                        if (!p) {
                              p = setNewSlot(m);
                              p->setRepeatType(FINE);
                              p->setLoopCount((rp->cycle()));
                              p->buildCycleList(p,rp->cycleList());
                              p->setNo((rp->no()));
                              p->setDestNo(rp->destNo());
                              }
                        if (((pp = slotByType(DACAPO)) != 0x00) ||
                             (pp = slotByType(DALSEGNO)) != 0x00 ) {
                              if (pp->endMeasure() == m) {
                                    p->setActive(SECONDTIME);
                                    p->setLoopCount(p->loopCount()-1);
                                    }
                              }
                        else {
                              if (m != p->startMeasure()) {
                              // only one fine allowed, the last is the winner
                                    p->setStartMeasure(m);
                                    }
                              }
                        p->setTickOffset(rtickOffSet);
                        }
                  type &= ~(FINE);
                  }

            if (type&SEGNO) {
                  if ((rp = rp->findRepElement(m,RepeatSegno)) != 0x00) {
                        if ((p = slotByType(SEGNO)) == 0x00) {
                              p = setNewSlot(m);
                              p->setRepeatType(SEGNO);
                              p->setNo((rp->no()));
                              p->buildCycleList(p,rp->cycleList());
                              p->setDestNo(rp->destNo());
                              }
                        if (isInList(p,p->active())) {
                              p->setTicks2Add(m->tick());
                              p->setStartMeasure(m);
                              p->setLoopCount((rp->cycle()));
                              if ((pp = slotByType(DALSEGNO))) {
                                    p->setEndMeasure(pp->startMeasure());
                                    }
                              p->setTickOffset(rtickOffSet);
                              }
                        }
                  type &= ~(SEGNO);
                  }

            if (type&DALSEGNO) {
            // D.S. and RepeatEnd does not make sense, ignore D.S.
                  if (!(type&END)) {
                        rp = rp->findRepElement(m,RepeatDalSegno);
                        if (!rp)
                              rp = rp->findRepElement(m,RepeatDalSegnoAlCoda);
                        if (!rp)
                              rp = rp->findRepElement(m,RepeatDalSegnoAlFine);
                        if (rp) {
                              if ((p = slot(m,DALSEGNO)) == 0x00) {
                                    p = setNewSlot(m);
                                    p->setRepeatType(DALSEGNO);
                                    p->setNo((rp->no()));
                                    p->setDestNo(rp->destNo());
                                    p->buildCycleList(p,rp->cycleList());
                                    }
                              if (isInList(p,p->active())) {
                                    p->setStartMeasure(m);
                                    p->setTicks2Add(m->tick());
                                    if ((pp = slotByType(SEGNO))) {
                                          p->setEndMeasure(pp->startMeasure());
                                          pp->setEndMeasure(m);
                                          }
                                    p->setLoopCount((rp->cycle()));
                                    p->setTickOffset(rtickOffSet);
                                    }
                              }
                        }
                  type &= ~(DALSEGNO);
                  }

            if (type&DACAPO) {
            // D.C. and RepeatEnd does not make sense, ignore D.C.
                  if (!(type&END)) {
                        rp = rp->findRepElement(m,RepeatDacapo);
                        if (!rp)
                              rp = rp->findRepElement(m,RepeatDacapoAlCoda);
                        if (!rp)
                              rp = rp->findRepElement(m,RepeatDacapoAlFine);
                        if (rp) {
                              if ((p = slot(m,DACAPO)) == 0x00) {
                                    p = setNewSlot(m);
                                    p->setRepeatType(DACAPO);
                                    p->setNo((rp->no()));
                                    p->setDestNo(rp->destNo());
                                    p->buildCycleList(p,rp->cycleList());
                                    }
                              if (isInList(p,p->active())) {
                                    p->setTickOffset(rtickOffSet);
                                    p->setLoopCount((rp->cycle()));
                                    p->setStartMeasure(m);
                                    p->setTicks2Add(m->tick());
                                    if ((pp = slotByType(CAPO))) {
                                          p->setEndMeasure(pp->startMeasure());
                                          }
                                    }
                              }
                        }
                  type &= ~(DACAPO);
                  }

            if (type&CODA) {
                  if ((rp = rp->findRepElement(m,RepeatCoda)) != 0x00) {
                        if ((p = slot(m,CODA)) == 0x00) {
                              p = setNewSlot(m);
                              p->setRepeatType(CODA);
                              p->setNo((rp->no()));
                              p->setDestNo(rp->destNo());
                              p->buildCycleList(p,rp->cycleList());
                              }
                        if (!isInList(p,p->active())) {
                              p->setTickOffset(rtickOffSet);
                              p->setLoopCount((rp->cycle()));
                              p->setTicks2Add(m->tick());
                              p->setStartMeasure(m);
                              }
                        }
                  type &= ~(CODA);
                  }

            if (type&ALCODA) {
                  if ((p = slot(m,ALCODA)) == 0x00) {
                        int d;
                        rp = rp->findRepElement(m,RepeatDacapoAlCoda);
                        if (!rp)
                              rp = rp->findRepElement(m,RepeatDalSegnoAlCoda);
                              d = rp->destNo();
                        if ((rp = rp->findCodaElement(rp->destNo()))) {
                              p = setNewSlot(m);
                              p->setRepeatType(ALCODA);
                              p->setEndMeasure(rp->measure());
                              p->setTickOffset(rtickOffSet);
                              p->setDestNo(d);
                              }
                        }
                  type &= ~(ALCODA);
                  }

            if (type&CODETTA) {
                  if ((rp = rp->findRepElement(m,RepeatCodetta)) != 0x00) {
                        if ((p = slot(m,CODETTA)) == 0x00) {
                              p = setNewSlot(m);
                              p->setRepeatType(CODETTA);
                              p->setNo((rp->destNo()));
                              }
                        p->setTickOffset(rtickOffSet);
                        p->setTicks2Add(m->tick());
                        type &= ~(CODETTA);
                        }
                  }

            if (type&S_VOLTA) {
                  if ((p = slot(m,S_VOLTA)) == 0x00) {
                        p = setNewSlot(m);
                        rp = rp->findRepElement(m,RepeatDalSegno);
                        p->setRepeatType(S_VOLTA);
                        p->setLoopCount(rp->cycle());
                        p->setNo(rp->no());
                        p->setDestNo(rp->destNo());
                        }
                  if (repeatEachTime && p->active() == THIRDTIME) {
                        pp = p->next();
                        while (pp) {
                              if (pp->repeatType() == S_VOLTA) {
                                    pp->setActive(FIRSTTIME);
                                    if (pp->startMeasure()->repeatCount() ||
                                       (pp->startMeasure()->repeatFlags() & RepeatEnd)) {
                                          slot(pp->startMeasure(),END)->setActive(FIRSTTIME);
                                          slot(pp->startMeasure(),END)->setLoopCount(rp->cycle());
                                          }
                                    pp = pp->next();
                                    }
                              else
                                    break;
                              }
                        p->setActive(FIRSTTIME);
                        p->setLoopCount(rp->cycle());
                        }
                  if (p->active() == FIRSTTIME) {
                        // check end of seconda volta
                        int tl = 0;
                        for (mm = m; mm;) {
//                              if (mm->ending() != 2) {
//                                    p->setEndMeasure(mm);
//                                    break;
//                                    }
                              tl += m->tickLen();
                              mm = mm->next();
                              }
                        p->setTicks2Add(tl);
                        if (!mm)
                              p->setEndMeasure(m);
                        p->setLoopCount(p->loopCount()-1);
                        p->setTickOffset(rtickOffSet);
                        mm = m->prev();
                        // if measure before was end of repeat and prima volta,
                        // accept this as a part of the repeat-loop and in this case
                        // the first time is to take as second time, play the part
//                        if ((!mm) ||
//                              (!((mm && mm->repeatFlags() & RepeatEnd)
//                               && mm->ending() == 1))) {
//                              ret = p->endMeasure();
//                              rtickOffSet = rtickOffSet - p->ticks2Add();
//                              push(ret);
//                              }
                        }
                  else {
                        p->setLoopCount(p->loopCount()-1);
                        p->setActive(THIRDTIME);
                        }
                  type &= ~(S_VOLTA);
                  }

            if (type&P_VOLTA) {
                  if ((p = slot(m,P_VOLTA)) == 0x00) {
                        p = setNewSlot(m);
                        p->setRepeatType(P_VOLTA);
                        p->setLoopCount(rp->cycle());
                        p->buildCycleList(p,rp->cycleList());
                        p->setNo(rp->no());
                        p->setDestNo(rp->destNo());
                        }
                  if (repeatEachTime && p->active() == THIRDTIME) {
                        if (type&END) {
                              pp = slot(m,END);
                              pp->setActive(FIRSTTIME);
                              pp->setLoopCount(1);
                              }
                        pp = p->next();
                        while (pp) {
                              if (pp->repeatType() == P_VOLTA) {
                                    pp->setActive(FIRSTTIME);
                                    if (pp->startMeasure()->repeatCount() ||
                                       (pp->startMeasure()->repeatFlags() & RepeatEnd)) {
                                          slot(pp->startMeasure(),END)->setActive(FIRSTTIME);
                                          slot(pp->startMeasure(),END)->setLoopCount(1);
                                          }
                                    pp = pp->next();
                                    }
                              else
                                    break;
                              }
                        p->setActive(FIRSTTIME);
                        }
                  if (p->active() == FIRSTTIME) {
                        // check end of prima volta
                        int tl = 0;
                        for (mm = m; mm;) {
//                              if (mm->ending() != 1) {
//                                    p->setEndMeasure(mm);
//                                    break;
//                                    }
                              tl += m->tickLen();
                              mm = mm->next();
                              }
                        p->setTicks2Add(tl);
                        if (!mm)
                              p->setEndMeasure(m);
                        p->setLoopCount(rp->cycle());
                        p->setTickOffset(rtickOffSet);
                        }
                 else {
                        p->setLoopCount(p->loopCount()-1);
                        ret = p->endMeasure();
                        p->setTickOffset(rtickOffSet);
                        rtickOffSet = rtickOffSet - p->ticks2Add();
                        p->setActive(THIRDTIME);
                        }
                  type &= ~(P_VOLTA);
                  }

            if (type&START) {
                  if ((rp = rp->findRepElement(m,RepeatStart)) != 0x00) {
                        if ((p = slot(m,START)) == 0x00) {
                              p = setNewSlot(m);
                              p->setRepeatType(START);
                              p->setNo((rp->no()));
                              p->setDestNo(rp->destNo());
                              p->setLoopCount(rp->cycle());
                              p->buildCycleList(p,rp->cycleList());
                              }
                        if (repeatEachTime)
                              p->setActive(FIRSTTIME);
                        if (p->active() == FIRSTTIME) {
                              pp = lastSlotByType(END);
                              if (pp && pp->endMeasure() == p->startMeasure()) {
                                    p->setEndMeasure(pp->startMeasure());
                                    }
                              else {
                                    p->setTicks2Add(m->tick());
                                    p->setEndMeasure(0);
                                    }
                              }
                        else
                              p->setLoopCount(p->loopCount()+1);
                        p->setTickOffset(rtickOffSet);
                        }
                  type &= ~(START);
                  }

            if (type&END) {
                  if ((rp = rp->findRepElement(m,RepeatEnd)) != 0x00) {
                        if ((p = slot(m,END)) == 0x00) {
                              p = setNewSlot(m);
                              p->setRepeatType(END);
                              p->setNo((rp->no()));
                              p->setDestNo(rp->destNo());
                              }
                        if (p->active() == FIRSTTIME) {
                              pp = slot(p->endMeasure(),START);
                              if (!pp)
                                    pp = lastActiveSlotByType(START);
                              if (pp)
                                    p->setEndMeasure(pp->startMeasure());
                              if (!pp) {
                                    pp = slotByType(CAPO);
                                    p->setEndMeasure(pp->startMeasure());
                                    }
                              if (pp) {
                                    pp->setEndMeasure(m);
                                    p->setTicks2Add(m->tick());
                                    p->setLoopCount((rp->cycle()));
                                    }
                              }
                        p->setTickOffset(rtickOffSet);
                        }
                  type &= ~(END);
                  }

            if (type&T_VOLTA) {
                  type &= ~(T_VOLTA);
                  }
            }
      return ret;
      }

//*****************************************************************************************
//
// function setNewSlot : input parameter -> measure, output -> new RepeatStack pointer
//
// Build a new RepeatStack and preset some elements. Elements which not set here are set
// in other fuctions in repeat2.cpp
//
//*****************************************************************************************

RepeatStack* RepeatStack::setNewSlot(Measure* m)
      {
      RepeatStack* p = 0x00;
      RepeatStack* n = 0x00;

      if (noOffElements() > 0) {
            n = init();
            for (p = firstStack; p->next() != 0; p = p->next())
                  ;  //search last entry
            n->setPrev(p);
            p->setNext(n);
            int x = noOffElements();
            setNoOffElements(++x);
            }
      else {
            n = firstStack;
            n->setNext(0);
            n->setPrev(0);
            setNoOffElements(1);
            }
      n->setLoopCount(1);
      n->setStartMeasure(m);
      n->setEndMeasure(0);
      n->setActive(FIRSTTIME);
      return n;
      }

//****************************************************************************************
//
// function pop : input parameter -> actuelle Measure, output -> Mesaure to process with
//                return : 0 --> caller has to dicide which measure to use
//                         (see seq.cpp collect events()
//                         measure --> new measure to proceed with
// The function build the destination measure depending on repeatType
//
//****************************************************************************************

Measure* RepeatStack::pop(Measure* m)
      {
      Measure* ret = 0x00;
      Measure* mm = 0x00;
      RepeatStack* p = 0x00;
      RepeatStack* pp = 0x00;
      RepeatFlag* rp;
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
            if (type&REPEATMES) { // don't change position from here
                  ret = 0x00;
                  type = 0;
                  }
            if (type&NORMAL) {
                  ret = 0x00;
                  type &= ~(NORMAL);
                  }
            if (type&CODETTA) { // only at begin of measure
                  ret = 0x00;
                  type &= ~(CODETTA);
                  }
            if (type&ALCODA) { // only at begin of measure
                  ret = 0x00;
                  type &= ~(ALCODA);
                  }
            if (type&SEGNO) { // only at begin of measure
                  ret = 0x00;
                  type &= ~(SEGNO);
                  }
            if (type&CODA) {
                  if ((p = slot(m,CODA)) != 0x00) {
                        ret = 0x00;
                        pp = slotEndMeasureByType(m,ALCODA);
                        if (pp) {
                              if (isInList(p,p->active())) {
                                    if (p->destNo() == 0) {
                                          rp = rp->findCodettaElement(1);
                                          if (rp)
                                                mm = push(rp->measure());
                                          if (pp)
                                                pp = slot(mm,CODETTA);
                                          }
                                    else {
                                          rp = rp->findCodaElement(p->destNo());
                                          if (rp)
                                                mm = push(rp->measure());
                                          if (mm)
                                                pp = slot(mm,CODA);
                                          }
                                    if (mm) {
                                          p->setEndMeasure(pp->startMeasure());
                                          pp->setEndMeasure(p->startMeasure());
                                          p->setTicks2Add(p->ticks2Add()+m->tickLen());
                                          rtickOffSet -= (pp->ticks2Add()-p->ticks2Add());
                                          ret = p->endMeasure();
                                          }
                                    }
                              }
                        p->setActive(p->active()+1);
                        }
                  type &= ~(CODA);
                  }
            if (type&FINE) {
                  if ((p = slotByType(FINE)) != 0x00) {
                        ret = 0x00;
                        if (!(type&(DACAPO|DALSEGNO))) {
                              if (p->loopCount() <= 1) {
                                    for (;m->next() != 0x00; m = m->next()) {}
                                    ret = m;
                                    type = 0;
                                    }
                              }
                        }
                  type &= ~(FINE);
                  }
            if (type&P_VOLTA) {
                  p = slot(m,P_VOLTA);
                  if (p && p->active() == FIRSTTIME) {
                        p->setActive(SECONDTIME);
                        }
                  ret = 0x00;
                  type &= ~(P_VOLTA);
                  }
            if (type&S_VOLTA) {
                  p = slot(m,S_VOLTA);
                  if (p && p->active() == FIRSTTIME) {
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
                        if ((p = slot(m,DACAPO)) != 0x00) {
                              pp = slotByType(CAPO);
                              if (pp) {
                                    if (isInList(p,p->active())) {
                                          p->setTickOffset(p->ticks2Add() +
                                                            m->tickLen());
                                          ret = p->endMeasure();

                                          }
                                    if (p->loopCount() <= 1) {
                                          ret = 0x00;
                                          }
                                    else {
                                          p->setLoopCount(p->loopCount()-1);
                                          rtickOffSet += p->tickOffset();
                                          ret = p->endMeasure();
                                          }
                                    }
                              p->setActive(p->active()+1);
                              }
                        }
                  type &= ~(DACAPO);
                  }
            if (type&DALSEGNO) {
            // D.S. and RepeatEnd does not make sense, ignore D.S.
                  if (!(type&END)) {
                        if ((p = slot(m,DALSEGNO)) != 0x00) {
                              pp = slot(p->endMeasure(),SEGNO);
                              if (pp) {
                                    if (isInList(p,p->active())) {
                                          p->setTickOffset((p->ticks2Add() -
                                                            pp->ticks2Add()) +
                                                            m->tickLen());
                                          ret = p->endMeasure();

                                          }
                                    if (p->loopCount() <= 1) {
                                          ret = 0x00;
                                          }
                                    else {
                                          p->setLoopCount(p->loopCount()-1);
                                          rtickOffSet += p->tickOffset();
                                          ret = p->endMeasure();
                                          }
                                          p->setActive(p->active()+1);
                                    }
                              else { // here if no "segno" before "dal segno"
                                    if (p->prev())
                                          p->prev()->setNext(p->next());
                                    if (p->next())
                                          p->next()->setPrev(p->prev());
                                    delStackElement(p);
                                    ret = 0x00;
                                    }
                              }
                        }
                  type &= ~(DALSEGNO);
                  }
            if (type&START) {
                  if ((p = slot(m,START)) != 0x00) {
                        if (p->active() == THIRDTIME && repeatEachTime) {
                              p->setActive(FIRSTTIME);
                              pp = slot(p->endMeasure(),END);
                              pp->setActive(FIRSTTIME);
                              }
                        ret = 0x00;
                        }
                  type &= ~(START);
                  }
            if (type&END) {
                  if ((p = slot(m,END)) != 0x00) {
                        pp = slot(p->endMeasure(),START);
                        if (!pp)
                              pp = slotByType(CAPO);
                        if (p->active() == FIRSTTIME) {
                              p->setTickOffset((p->ticks2Add() -
                                                pp->ticks2Add()) +
                                                m->tickLen());
                              p->setActive(SECONDTIME);
                              }
                        if (p->loopCount() <= 1) {
                              if (repeatEachTime)
                                    p->setActive(FIRSTTIME);
                              else
                                    p->setActive(THIRDTIME);
                              ret = 0x00;
                              }
                        else {
                              p->setLoopCount(p->loopCount()-1);
                              rtickOffSet += p->tickOffset();
                              ret = p->endMeasure();
                              }
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

Measure* RepeatStack::searchVarcoda(Measure* m, int dir) {
      Measure* mm;

      mm = m;
      while (mm) {
            if ((mm->repeatFlags()&RepeatVarcoda))
                  break;
            if (dir == 1)
                  mm = mm->next();
            if (dir == -1)
                  mm = mm->prev();
            }
      return mm;
      }

Measure* RepeatStack::searchSegno(Measure* m, int dir) {
      Measure* mm;

      mm = m;
      while (mm) {
            if ((mm->repeatFlags()&RepeatSegno))
                  break;
            if (dir == 1)
                  mm = mm->next();
            if (dir == -1)
                  mm = mm->prev();
            }
      return mm;
      }

RepeatStack* RepeatStack::startMeasure(Measure* m)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p != 0) {
           if (p->startMeasure() == m)
                  break;
                  p = p->next();
            }
      return p;
      }

RepeatStack* RepeatStack::slot(Measure* m, int type)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p != 0) {
            if (type > 0) {
                  if (p->startMeasure() == m && p->repeatType() == type)
                  break;
                  }
            else {
                  if (p->startMeasure() == m)
                        break;
                  }
                  p = p->next();
            }
      return p;
      }

RepeatStack* RepeatStack::slotByType(int type)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p != 0) {
           if (p->repeatType() == type)
                  break;
                  p = p->next();
            }
      return p;
      }


RepeatStack* RepeatStack::lastSlotByType(int type)
      {
      RepeatStack* p = 0x00;

      for (p = firstStack; p->next() != 0; p = p->next())
            ;
      while (p) {
            if (p->repeatType() == type)
                  break;
            p = p->prev();
            }
      return p;
      }


RepeatStack* RepeatStack::lastActiveSlotByType(int type)
      {
      RepeatStack* p;

      for (p = firstStack; p->next() != 0; p = p->next())
            ;
      while (p) {
            if (p->active() == FIRSTTIME && p->repeatType() == type)
                  break;
            p = p->prev();
            }
      return p;
      }

RepeatStack* RepeatStack::slotEndMeasureByType(Measure* m, int type)
      {
      RepeatStack* p = 0x00;

      p = firstStack;
      while (p != 0) {
            if (type > 0) {
                  if (p->endMeasure() == m && p->repeatType() == type)
                  break;
                  }
            else {
                  if (p->endMeasure() == m)
                        break;
                  }
                  p = p->next();
            }
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

//***********************************************************************************
//
// function checkType : input parameter -> measure*, output -> int
// check the type of repeats represented by repeatFlags in measure
//
// The type and repeatFlags values differ!!
//
//***********************************************************************************

int RepeatStack::checkType(Measure* m)
      {
      RepList* rl;

      int type = 0;
      bool found = false;

      for (rl = rList; rl; rl = rl->next()) {
            if (m == rl->measure()) {
                  found = true;
                  break;
                  }
            }

      if (m->prev() == 0)
            type |= CAPO;

#if 0
      if (m->endRepeat() > 0)
            type |= END;
      if (m->ending() == 1)
            type |= P_VOLTA;
      if (m->ending() == 2)
            type |= S_VOLTA;
      if (m->ending() == 3)
            type |= T_VOLTA;
#endif
      if (found) {
            if (rl->repeatFlag() & RepeatStart)
                  type |= START;
            if (rl->repeatFlag() & RepeatEnd)
                  type |= END;
            if (rl->repeatFlag() & RepeatSegno)
                  type |= SEGNO;
            if (rl->repeatFlag() & RepeatCoda)
                  type |= CODA;
            if (rl->repeatFlag() & RepeatVarcoda)
                  type |= VARCODA;
            if (rl->repeatFlag() & RepeatCodetta)
                  type |= CODETTA;
            if (rl->repeatFlag() & RepeatDacapo)
                  type |= DACAPO;
            if (rl->repeatFlag() & RepeatDacapoAlFine)
                  type |= (DACAPO | FINE);
            if (rl->repeatFlag() & RepeatDacapoAlCoda)
                  type |= (DACAPO | ALCODA);
            if (rl->repeatFlag() & RepeatDalSegno)
                  type |= DALSEGNO;
            if (rl->repeatFlag() & RepeatDalSegnoAlFine)
                  type |= (DALSEGNO | FINE);
            if (rl->repeatFlag() & RepeatDalSegnoAlCoda)
                  type |= (DALSEGNO | ALCODA);
            if (rl->repeatFlag() & RepeatAlSegno)
                  type |= ALSEGNO;
            if (rl->repeatFlag() & RepeatFine)
                  type |= FINE;
            if (rl->repeatFlag() & RepeatMeasureFlag)
                  type |= REPEATMES;
            }
      if (!type)
            type |= NORMAL;
      return type;
      }

//**********************************************************************************
//
// split  string s into integer values, each value is represented bei a substring
// seperated by ',' or string end '\0'
//
//*********************************************************************************
void RepeatStack::buildCycleList(RepeatStack* p,QString s)
      {
      QString a;
      int y, z, x;

      x = y = z = 0;
      if (!s.isEmpty()) {
            while (x < s.size()) {
                  a[y++] = s.at(x++);
                  if ((x == s.size() && y > 0) || s.at(x) == ',') {
                        p->setCycleList(z++, a.toInt());
                        p->setCycleList(z, -1);
                        y = 0;
                        x++;
                        }
                  }
            }
      }

bool RepeatStack::collectRepeats(Element* e,Measure* m)
      {
#if 0       // WS
      RepList* rl;
      RepList* rlf;
      bool found;

      RepeatFlag* rf = e->repeatFlag();
      if (!rf)
            return false;
      found = false;
      if (rList == 0x00)
            rList = new RepList();
      rlf = 0x00;
      rl = rList;
      while (!found) {
            if (m == rl->measure()) {
                  if (!rlf)
                        rlf = rl;
                  found = true;
                  }
            if (found) {
                  if (!(rl->getRf() == e->repeatFlag()))
                        found = false;
                  }
            if (!found) {
                  if (rl->next())
                        rl = rl->next();
                  else
                        break;
                  }
            }
      if (!found) {
            if (!rlf)
                  rlf = rl;
            if (rl->measure()) {
                  rl->setNext(new RepList());
                  if (rl->measure() != m)
                        rlf = rl->next();
                  rl = rl->next();
                  }
            rl->setMeasure(m);
            rl->setRf(rf);
            rf->setMeasure(rl->measure());
            found = true;
            }
      if (found)
            rlf->setRepeatFlag(rf->repeatFlag());
#endif
      return true;
      }

bool RepeatStack::isInList(RepeatStack* r, int val)
      {

      for (int i = 0; i < MAXCYCLE; i++) {
            if (r->cycleList(i) == val)
                  return true;
            }
      return false;
      }

RepList::RepList()
      {
      _next = 0x00;
      _measure = 0x00;
      _rf = 0x00;
      _repeatFlag = 0;
      }

RepList::~RepList()
      {
      }



#endif

