//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
      double _len;
      Spatium _userLen;

   public:
      Stem(Score*);
      Stem &operator=(const Stem&);

      virtual Stem* clone() const      { return new Stem(*this); }
      virtual ElementType type() const { return STEM; }
      virtual void draw(QPainter& p) const;
      void setLen(double v)            { _len = v; }
      double stemLen() const           { return _len + point(_userLen); }
      virtual QRectF bbox() const;
      virtual bool isMovable() const   { return true; }
      virtual bool isEditable() { return true; }

      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement e);
      virtual void toDefault();
      Spatium userLen() const         { return _userLen; }
      virtual void setVisible(bool f);
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      Chord* chord() const            { return (Chord*)parent(); }
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

      void setLine(const QLineF& l);

      virtual StemSlash* clone() const { return new StemSlash(*this); }
      virtual ElementType type() const { return STEM_SLASH; }
      virtual void draw(QPainter& p) const;
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
      virtual ElementType type() const  { return LEDGER_LINE; }
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      Chord* chord() const { return (Chord*)parent(); }
      virtual void layout();
      };

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

/**
 Graphic representation of a chord.

 Single notes are handled as degenerated chords.
*/

class Chord : public ChordRest {
      QList<Note*> _notes;
      QList<LedgerLine*> _ledgerLines;
      Stem* _stem;
      Hook* _hook;
      StemSlash* _stemSlash;
      Direction _stemDirection;
      Arpeggio* _arpeggio;
      Tremolo* _tremolo;
      Glissando* _glissando;
      NoteType _noteType;           ///< mark grace notes: acciaccatura and appoggiatura
      bool _noStem;
      double _dotPosX;
      double minSpace, extraSpace;  ///< cached values

      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void addLedgerLine(double x, int staffIdx, int line, int extend);
      void addLedgerLines(double x, int move);

   public:
      Chord(Score* s = 0);
      Chord(const Chord&);
      ~Chord();
      Chord &operator=(const Chord&);

      virtual Chord* clone() const     { return new Chord(*this); }
      virtual ElementType type() const { return CHORD; }

      void write(Xml& xml, int start, int end) const;
      virtual void write(Xml& xml) const { write(xml, 0, 0); }
      void read(QDomElement, const QList<Tuplet*>&);
      virtual void read(QDomElement);
      virtual void setSelected(bool f);
      virtual void dump() const;

      virtual QRectF bbox() const;
      void setStemDirection(Direction d)     { _stemDirection = d; }
      Direction stemDirection() const        { return _stemDirection; }

      QList<LedgerLine*>* ledgerLines()      { return &_ledgerLines; }

      virtual void layoutStem1();
      virtual void layoutStem();

      QList<Note*>& notes()                  { return _notes; }
      const QList<Note*>& notes() const      { return _notes; }
      Note* upNote() const                   { return _notes.back(); }
      Note* downNote() const                 { return _notes.front(); }
      Note* findNote(int pitch) const;

      Stem* stem() const                     { return _stem; }
      void setStem(Stem* s);
      Arpeggio* arpeggio() const             { return _arpeggio;  }
      Tremolo* tremolo() const               { return _tremolo;   }
      Glissando* glissando() const           { return _glissando; }
      StemSlash* stemSlash() const           { return _stemSlash; }

      virtual QPointF stemPos(bool, bool) const;

      Hook* hook() const                     { return _hook; }
      void setHook(Hook* f);

      virtual void add(Element*);
      virtual void remove(Element*);

      Note* selectedNote() const;
      virtual void layout();
      void layout2();

      virtual int upLine() const;
      virtual int downLine() const;
      virtual Space space() const;
      void readNote(QDomElement node, const QList<Tuplet*>&);

      NoteType noteType() const         { return _noteType; }
      void setNoteType(NoteType t)      { _noteType = t; }

      virtual void scanElements(void* data, void (*func)(void*, Element*));

      virtual void setTrack(int val);

      void computeUp();
      double dotPosX() const              { return _dotPosX; }
      void setDotPosX(double val)         { _dotPosX = val;  }
      bool noStem() const                 { return _noStem;  }
      void setNoStem(bool val)            { _noStem = val;   }
      virtual void setMag(double val);
      void pitchChanged();
      };

#endif

