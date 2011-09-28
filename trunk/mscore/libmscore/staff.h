//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STAFF_H__
#define __STAFF_H__

/**
 \file
 Definition of class Staff.
*/

#include "mscore.h"
#include "key.h"
#include "velo.h"
#include "pitch.h"
#include "cleflist.h"

class Instrument;
struct InstrumentTemplate;
class Xml;
class Part;
class Score;
class KeyList;
class StaffType;
class StaffTypeTablature;
class Staff;
class Tablature;
class ClefList;
struct ClefTypeList;
class Segment;
class Clef;
class TimeSig;

//---------------------------------------------------------
//   LinkedStaves
//---------------------------------------------------------

class LinkedStaves {
      QList<Staff*> _staves;

   public:
      LinkedStaves() {}
      QList<Staff*>& staves()             { return _staves; }
      const QList<Staff*>& staves() const { return _staves; }
      void add(Staff*);
      void remove(Staff*);
      bool isEmpty() const { return _staves.isEmpty(); }
      };

//---------------------------------------------------------
//   BracketItem
//---------------------------------------------------------

struct BracketItem {
      int _bracket;
      int _bracketSpan;

      BracketItem() {
            _bracket = -1;
            _bracketSpan = 0;
            }
      BracketItem(int a, int b) {
            _bracket = a;
            _bracketSpan = b;
            }
      };

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

/**
 Global staff data not directly related to drawing.
*/

class Staff {
      Score* _score;
      Part* _part;
      int _rstaff;            ///< Index in Part.

      ClefList _clefList;           // for backward compatibility
      ClefTypeList _initialClef;    // used by new score wizard

      QList<Clef*> clefs;
      QList<TimeSig*> timesigs;

      KeyList* _keymap;
      QList <BracketItem> _brackets;
      int _barLineSpan;       ///< 0 - no bar line, 1 - span this staff, ...
      bool _show;             ///< derived from part->show()
      bool _small;
      bool _invisible;

      bool _updateKeymap;

      StaffType* _staffType;

      LinkedStaves* _linkedStaves;

      QMap<int,int> _channelList[VOICES];

      VeloList _velocities;         ///< cached value
      PitchList _pitchOffsets;      ///< cached value

   public:
      Staff(Score*, Part*, int);
      ~Staff();
      void init(const InstrumentTemplate*, int);

      bool isTop() const             { return _rstaff == 0; }
      QString trackName() const;
      int rstaff() const             { return _rstaff; }
      int idx() const;
      void setRstaff(int n)          { _rstaff = n;    }
      void read(QDomElement);
      void write(Xml& xml) const;
      Part* part() const             { return _part;        }
      void setPart(Part* p)          { _part = p;           }

      int bracket(int idx) const;
      int bracketSpan(int idx) const;
      void setBracket(int idx, int val);
      void setBracketSpan(int idx, int val);
      int bracketLevels() const      { return _brackets.size(); }
      void addBracket(BracketItem);
      QList <BracketItem> brackets() const { return _brackets; }
      void cleanupBrackets();

      KeyList* keymap() const        { return _keymap;      }

      ClefType clef(int tick) const;
      ClefType clef(Segment*) const;
      void addClef(Clef*);
      void removeClef(Clef*);
      void setInitialClef(const ClefTypeList& cl) { _initialClef = cl; }
      void setInitialClef(ClefType ct)            { _initialClef = ClefTypeList(ct, ct); }
      ClefTypeList initialClef() const            { return _initialClef; }

      void addTimeSig(TimeSig*);
      void removeTimeSig(TimeSig*);
      Fraction timeStretch(int tick) const;
      TimeSig* timeSig(int tick) const;

      ClefTypeList clefTypeList(int tick) const;
      KeySigEvent key(int tick) const;
      void setKey(int tick, int st);
      void setKey(int tick, const KeySigEvent& st);
      void removeKey(int tick);

      bool show() const              { return _show;        }
      bool slashStyle() const;
      void setShow(bool val)         { _show = val;         }
      bool small() const             { return _small;       }
      void setSmall(bool val)        { _small = val;        }
      bool invisible() const         { return _invisible;   }
      void setInvisible(bool val)    { _invisible = val;    }
      void setSlashStyle(bool val);
      int lines() const;
      void setLines(int);
      int barLineSpan() const        { return _barLineSpan; }
      void setBarLineSpan(int val)   { _barLineSpan = val;  }
      Score* score() const           { return _score;       }
      qreal mag() const;
      qreal height() const;
      qreal spatium() const;
      int channel(int tick, int voice) const;
      QMap<int,int>* channelList(int voice) { return  &_channelList[voice]; }

      StaffType* staffType() const     { return _staffType;      }
      void setStaffType(StaffType* st);

      bool useTablature() const;
      bool updateKeymap() const        { return _updateKeymap;   }
      void setUpdateKeymap(bool v)     { _updateKeymap = v;      }
      VeloList& velocities()           { return _velocities;     }
      PitchList& pitchOffsets()        { return _pitchOffsets;   }

      LinkedStaves* linkedStaves() const    { return _linkedStaves; }
      void setLinkedStaves(LinkedStaves* l) { _linkedStaves = l;    }
      void linkTo(Staff* staff);
      bool primaryStaff() const;
      ClefList* clefList()          { return &_clefList; }   // for backward compatibility
      };
#endif

