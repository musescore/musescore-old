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

#ifndef __CHORD_H__
#define __CHORD_H__

/**
 \file
 Definition of classes Chord, HelpLine and NoteList.
*/

#include "chordrest.h"

class Note;
class Hook;
class Arpeggio;
class Tremolo;
class Chord;
class Glissando;
class Stem;
class QPainter;
class Chord;

enum TremoloChordType { TremoloSingle, TremoloFirstNote, TremoloSecondNote };

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
      virtual void draw(QPainter*) const;
      virtual void layout();
      Chord* chord() const { return (Chord*)parent(); }
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
      virtual QPointF pagePos() const;      ///< position in page coordinates
      Chord* chord() const { return (Chord*)parent(); }
      virtual void layout();
      qreal measureXPos() const;
      };

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

/**
 Graphic representation of a chord.

 Single notes are handled as degenerated chords.
*/

class Chord : public ChordRest {
      QList<Note*> _notes;                // sorted to increasing pitch
      QList<LedgerLine*> _ledgerLines;

      Stem*      _stem;
      Hook*      _hook;
      StemSlash* _stemSlash;
      Direction  _stemDirection;
      Arpeggio*  _arpeggio;
      Tremolo*   _tremolo;
      TremoloChordType _tremoloChordType;
      Glissando* _glissando;
      ElementList _el;              ///< chordline

      NoteType   _noteType;         ///< mark grace notes: acciaccatura and appoggiatura
      bool       _noStem;

      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void addLedgerLine(qreal x, int staffIdx, int line, int extend, bool visible);
      void addLedgerLines(qreal x, int move);
      void renderArticulation(ArticulationType);

   public:
      Chord(Score* s = 0);
      Chord(const Chord&);
      ~Chord();
      Chord &operator=(const Chord&);

      virtual Chord* clone() const     { return new Chord(*this); }
      virtual Chord* linkedClone();

      virtual void setScore(Score* s);
      virtual ElementType type() const { return CHORD; }

      virtual void write(Xml& xml) const;
      void read(const QDomElement&, QList<Tuplet*>*, QList<Spanner*>*);
      virtual void read(const QDomElement&);
      virtual void setSelected(bool f);
      virtual Element* drop(const DropData&);

      void setStemDirection(Direction d)     { _stemDirection = d; }
      Direction stemDirection() const        { return _stemDirection; }

      QList<LedgerLine*>* ledgerLines()      { return &_ledgerLines; }

      virtual void layoutStem1();
      virtual void layoutStem();
      void layoutArpeggio2();

      QList<Note*>& notes()                  { return _notes; }
      const QList<Note*>& notes() const      { return _notes; }

      Note* upNote() const                   { return _notes.back(); }
      Note* downNote() const                 { return _notes.front(); }
      virtual int upLine() const;
      virtual int downLine() const;

      Note* findNote(int pitch) const;

      Stem* stem() const                     { return _stem; }
      void setStem(Stem* s);
      Arpeggio* arpeggio() const             { return _arpeggio;  }
      Tremolo* tremolo() const               { return _tremolo;   }
      void setTremolo(Tremolo* t)            { _tremolo = t;      }
      Glissando* glissando() const           { return _glissando; }
      StemSlash* stemSlash() const           { return _stemSlash; }
      void setStemSlash(StemSlash* s)        { delete _stemSlash; _stemSlash = s; }

      virtual QPointF stemPos() const;
      QPointF stemPosBeam() const;

      Hook* hook() const                     { return _hook; }

      virtual void add(Element*);
      virtual void remove(Element*);

      Note* selectedNote() const;
      virtual void layout();
      void layout2();

      void readNote(const QDomElement& node, QList<Tuplet*>*, QList<Spanner*>*);

      NoteType noteType() const         { return _noteType; }
      void setNoteType(NoteType t)      { _noteType = t; }
      bool isGrace() const              { return _noteType != NOTE_NORMAL; }

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      virtual void setTrack(int val);

      void computeUp();
      qreal dotPosX() const;
      void setDotPosX(qreal val);
      bool noStem() const                 { return _noStem;  }
      void setNoStem(bool val)            { _noStem = val;   }
      virtual void setMag(qreal val);
      void pitchChanged();
      void renderPlayback();
      TremoloChordType tremoloChordType() const      { return _tremoloChordType; }
      void setTremoloChordType(TremoloChordType t)   { _tremoloChordType = t; }

      ElementList& el()                { return _el; }
      const ElementList& el() const    { return _el; }

      QPointF layoutArticulation(Articulation*);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      };

#endif

