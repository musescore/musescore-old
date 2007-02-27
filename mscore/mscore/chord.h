//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chord.h,v 1.3 2006/03/02 17:08:33 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __CHORD_H__
#define __CHORD_H__

/**
 \file
 Definition of classes Chord, HelpLine, NoteList and Stem.
*/

#include "globals.h"
#include "chordrest.h"

class Note;
class Hook;

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

/**
 Graphic representation of a note stem.
*/

class Stem : public Element {
      Spatium _len;

   public:
      Stem(Score*);
      Stem &operator=(const Stem&);

      virtual Stem* clone() const { return new Stem(*this); }
      virtual ElementType type() const { return STEM; }
      virtual void draw(QPainter& p);
      void setLen(const Spatium&);
      virtual QRectF bbox() const;
      };

//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of notes.

 Used by Chord to store its notes.
*/

class NoteList : public std::multimap <const int, Note*> {
   public:
      NoteList::iterator add(Note* n);
      Note* front() const { return (begin() != end()) ? begin()->second : 0;  }
      Note* back()  const { return (begin() != end()) ? rbegin()->second : 0; }
      Note* find(int pitch) const;
      };

typedef NoteList::iterator iNote;
typedef NoteList::reverse_iterator riNote;
typedef NoteList::const_iterator ciNote;
typedef NoteList::const_reverse_iterator criNote;

//---------------------------------------------------------
//   HelpLine
//---------------------------------------------------------

/**
 Graphic representation of a helpline.
*/

class HelpLine : public Line {
   public:
      HelpLine(Score*);
      HelpLine &operator=(const HelpLine&);
      virtual HelpLine* clone() const { return new HelpLine(*this); }
      virtual ElementType type() const { return HELP_LINE; }
      };

typedef QList<HelpLine*>::iterator iHelpLine;
typedef QList<HelpLine*>::const_iterator ciHelpLine;

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

/**
 Graphic representation of a chord.

 Single notes are handled as degenerated chords.
*/

class Chord : public ChordRest {
      NoteList notes;
      QList<HelpLine*> helpLines;
      Stem* _stem;
      Hook* _hook;
      Direction _stemDirection;
      bool _grace;

      void computeUp();
      void init();
      void readSlur(QDomNode, int staff);
      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void addHelpLine(double x, double y, int i);

   public:
      Chord(Score*);
      Chord(Score*, int tick);
      Chord &operator=(const Chord&);

      virtual Chord* clone() const { return new Chord(*this); }
      virtual ElementType type() const { return CHORD; }

      virtual void draw(QPainter&);
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode, int staff);
      virtual void setSelected(bool f);
      virtual void dump() const;

      virtual QRectF bbox() const;
      void setStemDirection(Direction d)     { _stemDirection = d; }
      Direction stemDirection() const        { return _stemDirection; }
      bool grace() const                     { return _grace; }
      void setGrace(bool g)                  { _grace = g; }

      QList<HelpLine*>* getHelpLines() { return &helpLines; }

      virtual void layoutStem();
      NoteList* noteList()                   { return &notes; }
      const NoteList* noteList() const       { return &notes; }
      const Note* upNote() const             { return notes.back(); }
      const Note* downNote() const           { return notes.front(); }
      Note* upNote()                         { return notes.back(); }
      Note* downNote()                       { return notes.front(); }
      virtual int move() const;

      QList<HelpLine*>* helpLineList() { return &helpLines; }

      Stem* stem()                           { return _stem; }
      void setStem(Stem* s);
      virtual QPointF stemPos(bool, bool) const;

      Hook* hook()                           { return _hook; }
      void setHook(Hook* f);

      virtual void add(Element*);
      virtual void remove(Element*);

      Note* selectedNote() const;
      virtual void layout();

      virtual int upLine() const;
      virtual int downLine() const;
      virtual void space(double& min, double& extra) const;

      };

#endif



