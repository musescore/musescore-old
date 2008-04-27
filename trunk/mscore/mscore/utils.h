//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: utils.h,v 1.12 2006/03/02 17:08:43 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "globals.h"

//---------------------------------------------------------
//   cycles
//---------------------------------------------------------

static inline unsigned long long cycles()
      {
      unsigned long long rv;
      __asm__ __volatile__("rdtsc" : "=A" (rv));
      return rv;
      }

class Measure;
class Segment;
class System;
class Element;

extern QRectF drawHandle(QPainter& p, const QPointF& pos, bool active);
extern QRectF handleRect(const QPointF& pos);

extern int getStaff(System* system, const QPointF& p);
extern int headType(int tickLen, DurationType* type, int* dots);
extern int pitchKeyAdjust(int note, int key);
extern int line2pitch(int line, int clef, int key);
extern int y2pitch(double y, int clef);
extern int quantizeLen(int, int, int);
extern double curTime();
extern void selectNoteMessage();
extern void selectNoteRestMessage();
extern void selectNoteSlurMessage();


#endif

