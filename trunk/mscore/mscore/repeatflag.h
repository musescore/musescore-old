
//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  $Id: repeatflag.h,v 1.00 2007/09/26 23:00:00 dikrau Exp $
//
//  Copyright (C) 2007- Dieter Krause (dikrau@users.sourceforge.net)
//
//  repeatflag.h: contains class, function definition and detailed Information
//  about a type of repeat
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

#ifndef __REPEATFLAG_H__
#define __REPEATFLAG_H__

#include "repeat2.h"
#include "repeatflagdialog.h"

class Measure;

class RepeatFlag {


      int _repeatFlag;              // type of repeat
      int _no;                      // numeration of Flag
      int _cycle;                   // how often the part is to play
      int _destno;                  // destination for coda/jumps
      QString _cycleList;           // list of cycles, seperated by ','
      Measure* _measure;            // Measure repeat belongs to

public:
      RepeatFlag();
      ~RepeatFlag();


      bool genPropertyMenu(QMenu* menu) const;
      void propertyAction(const QString&,Element*);

      void setRepeatFlag(int f) { _repeatFlag = f; }
      int repeatFlag() { return _repeatFlag; }
      void setNo(int n) { _no = n; }
      int no() { return _no; }
      void setDestNo(int i) { _destno = i; }
      int destNo() { return _destno; }
      void setCycle(int v) { _cycle = v; }
      int cycle() { return _cycle; }
      void setCycleList(QString s) { _cycleList = s; };
      QString cycleList() { return _cycleList; }
      void setMeasure(Measure* m) { _measure = m; }
      Measure* measure() { return _measure; }
      void accept();
      void setRepeatFlagProps(Element*,RepeatFlag*);
      void setDefaults(RepeatFlag*);
      void setMeasureRepeatFlag(Element*,int);
      RepeatFlag* findRepElement(Measure*,int);
      RepeatFlag* findCodaElement(int);
      RepeatFlag* findCodettaElement(int);
      };

extern Element* actElement;
#endif

