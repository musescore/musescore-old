//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chord.h,v 1.3 2006/03/02 17:08:33 wschweer Exp $
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
class Arpeggio;
class Tremolo;
class Chord;
class Glissando;

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

/**
 Graphic representation of a note stem.
*/

class Stem : public Element {
      Spatium _len;
      Spatium _userLen;

   public:
      Stem(Score*);
      Stem &operator=(const Stem&);

      virtual Stem* clone() const { return new Stem(*this); }
      virtual ElementType type() const { return STEM; }
      virtual void draw(QPainter& p) const;
      void setLen(const Spatium&);
      Spatium stemLen() const { return _len + _userLen; }
      virtual QRectF bbox() const;
      virtual bool isMovable() const  { return true; }
      virtual bool startEdit(Viewer*, const QPointF&);
      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement e);
      virtual void resetUserOffsets() {  _userLen = Spatium(0.0); }
      Spatium userLen() const { return _userLen; }
      };

//---------------------------------------------------------
//   StemSlash
//    used for grace notes of type acciaccatura
//---------------------------------------------------------

class StemSlash : public Element {
      QLineF line;

   public:
      StemSlash(Score*);
      StemSlash &operator=(const Stem&);

      void setLine(const QLineF& l)    { line = l; }

      virtual StemSlash* clone() const { return new StemSlash(*this); }
      virtual ElementType type() const { return STEM_SLASH; }
      virtual void draw(QPainter& p) const;
      virtual QRectF bbox() const;
      };

//---------------------------------------------------------
///   Graphic representation of a ledger line.
///
///    parent:     Chord
///    x-origin:   Chord
///    y-origin:   SStaff
//---------------------------------------------------------

class LedgerLine : public Line {
   public:
      LedgerLine(Score*);
      LedgerLine &operator=(const LedgerLine&);
      virtual LedgerLine* clone() const { return new LedgerLine(*this); }
      virtual ElementType type() const { return LEDGER_LINE; }
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      Chord* chord() const { return (Chord*)parent(); }
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
//   Chord
//---------------------------------------------------------

/**
 Graphic representation of a chord.

 Single notes are handled as degenerated chords.
*/

class Chord : public ChordRest {
      NoteList notes;
      QList<LedgerLine*> _ledgerLines;
      Stem* _stem;
      Hook* _hook;
      StemSlash* _stemSlash;
      Direction _stemDirection;
      Arpeggio* _arpeggio;
      Tremolo* _tremolo;
      Glissando* _glissando;
      NoteType _noteType;     ///< mark grace notes: acciaccatura and appoggiatura

      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void addLedgerLine(double x, int staffIdx, int line);

   public:
      Chord(Score*);
      ~Chord();
      Chord &operator=(const Chord&);

      virtual Chord* clone() const     { return new Chord(*this); }
      virtual ElementType type() const { return CHORD; }

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement, int staff);
      virtual void setSelected(bool f);
      virtual void dump() const;

      virtual void setDuration(Duration t);

      virtual QRectF bbox() const;
      void setStemDirection(Direction d)     { _stemDirection = d; }
      Direction stemDirection() const        { return _stemDirection; }

      QList<LedgerLine*>* ledgerLines()      { return &_ledgerLines; }

      virtual void layoutStem1(ScoreLayout*);
      virtual void layoutStem(ScoreLayout*);
      NoteList* noteList()                   { return &notes; }
      const NoteList* noteList() const       { return &notes; }
      const Note* upNote() const             { return notes.back(); }
      const Note* downNote() const           { return notes.front(); }
      Note* upNote()                         { return notes.back(); }
      Note* downNote()                       { return notes.front(); }

      Stem* stem() const                     { return _stem; }
      void setStem(Stem* s);
      Arpeggio* arpeggio() const             { return _arpeggio;  }
      Tremolo* tremolo() const               { return _tremolo;   }
      Glissando* glissando() const           { return _glissando; }

      virtual QPointF stemPos(bool, bool) const;

      Hook* hook() const                     { return _hook; }
      void setHook(Hook* f);

      virtual void add(Element*);
      virtual void remove(Element*);

      Note* selectedNote() const;
      virtual void layout(ScoreLayout*);

      virtual int upLine() const;
      virtual int downLine() const;
      virtual void space(double& min, double& extra) const;
      void readNote(QDomElement node, int staffIdx);

      NoteType noteType() const         { return _noteType; }
      void setNoteType(NoteType t)      { _noteType = t; }

      virtual void collectElements(QList<const Element*>& el) const;

      virtual int staffMove() const;
      virtual void setTrack(int val);

      void computeUp();
      };

#endif

