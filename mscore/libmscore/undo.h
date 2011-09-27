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

#ifndef __UNDO_H__
#define __UNDO_H__

/**
 \file
 Definition of undo-releated classes and structs.
*/

#include "spatium.h"
#include "mscore.h"
#include "sig.h"
#include "tempo.h"
#include "input.h"
#include "style.h"
#include "key.h"
#include "select.h"
#include "instrument.h"
#include "msynth/synti.h"
#include "pitchvalue.h"

class ElementList;
class Element;
class Instrument;
class System;
class Measure;
class Segment;
class Staff;
class Part;
class Volta;
class Score;
class Note;
class Chord;
class ChordRest;
class Harmony;
class SlurTie;
class Beam;
struct MStaff;
class MeasureBase;
class Dynamic;
class Selection;
class Text;
struct Channel;
struct PageFormat;
class TextStyle;
class Tuplet;
class KeySig;
class StaffType;
class TimeSig;
class Clef;
class Image;
class Hairpin;
class Bend;
class TremoloBar;
class NoteEvent;
class SlurSegment;
class InstrumentChange;
class Box;
class Accidental;
class Spanner;

#define DEBUG_UNDO

#ifdef DEBUG_UNDO
#define UNDO_NAME(a)  virtual const char* name() const { return a; }
#else
#define UNDO_NAME(a)
#endif

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

class UndoCommand {
      QList<UndoCommand*> childList;

   public:
      virtual ~UndoCommand();
      virtual void undo();
      virtual void redo();
      void appendChild(UndoCommand* cmd) { childList.append(cmd);       }
      UndoCommand* removeChild()         { return childList.takeLast(); }
      int childCount() const             { return childList.size();     }
      void unwind();
#ifdef DEBUG_UNDO
      virtual const char* name() const  { return "UndoCommand"; }
#endif
      };

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

class UndoStack {
      UndoCommand* curCmd;
      QList<UndoCommand*> list;
      int curIdx;
      int cleanIdx;

   public:
      UndoStack();
      ~UndoStack();

      bool active() const           { return curCmd != 0; }
      void beginMacro();
      void endMacro(bool rollback);
      void push(UndoCommand*);
      void pop();
      void setClean();
      bool canUndo() const          { return curIdx > 0;           }
      bool canRedo() const          { return curIdx < list.size(); }
      bool isClean() const          { return cleanIdx == curIdx;   }
      UndoCommand* current() const  { return curCmd;               }
      void undo();
      void redo();
      };

//---------------------------------------------------------
//   SaveState
//---------------------------------------------------------

class SaveState : public UndoCommand {
      InputState undoInputState;
      InputState redoInputState;
      Selection  undoSelection;
      Selection  redoSelection;
      Score* score;

   public:
      SaveState(Score*);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("SaveState");
      };

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

class InsertPart : public UndoCommand {
      Part* part;
      int idx;

   public:
      InsertPart(Part* p, int i);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertPart");
      };

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

class RemovePart : public UndoCommand {
      Part* part;
      int idx;

   public:
      RemovePart(Part*, int idx);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemovePart");
      };

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

class InsertStaff : public UndoCommand {
      Staff* staff;
      int idx;

   public:
      InsertStaff(Staff*, int idx);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertStaff");
      };

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

class RemoveStaff : public UndoCommand {
      Staff* staff;
      int idx;

   public:
      RemoveStaff(Staff*, int idx);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveStaff");
      };

//---------------------------------------------------------
//   InsertMStaff
//---------------------------------------------------------

class InsertMStaff : public UndoCommand {
      Measure* measure;
      MStaff* mstaff;
      int idx;

   public:
      InsertMStaff(Measure*, MStaff*, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertMStaff");
      };

//---------------------------------------------------------
//   RemoveMStaff
//---------------------------------------------------------

class RemoveMStaff : public UndoCommand {
      Measure* measure;
      MStaff* mstaff;
      int idx;

   public:
      RemoveMStaff(Measure*, MStaff*, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveMStaff");
      };

//---------------------------------------------------------
//   InsertMeasure
//---------------------------------------------------------

class InsertMeasure : public UndoCommand {
      MeasureBase* measure;
      MeasureBase* pos;

   public:
      InsertMeasure(MeasureBase* nm, MeasureBase* p) : measure(nm), pos(p) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertMeasure");
      };

//---------------------------------------------------------
//   InsertStaves
//---------------------------------------------------------

class InsertStaves : public UndoCommand {
      Measure* measure;
      int a;
      int b;

   public:
      InsertStaves(Measure*, int, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertStaves");
      };

//---------------------------------------------------------
//   RemoveStaves
//---------------------------------------------------------

class RemoveStaves : public UndoCommand {
      Measure* measure;
      int a;
      int b;

   public:
      RemoveStaves(Measure*, int, int);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveStaves");
      };

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

class SortStaves : public UndoCommand {
      Score* score;
      QList<int> list;
      QList<int> rlist;

   public:
      SortStaves(Score*, QList<int>);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("SortStaves");
      };

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

class ChangePitch : public UndoCommand {
      Note* note;
      int pitch;
      int tpc;
      int line;
      int fret;
      int string;
      void flip();

   public:
      ChangePitch(Note* note, int pitch, int tpc, int l, int f, int string);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangePitch");
      };

//---------------------------------------------------------
//   FlipSlurDirection
//---------------------------------------------------------

class FlipSlurDirection : public UndoCommand {
      SlurTie* slur;
      void flip();

   public:
      FlipSlurDirection(SlurTie*);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("FlipSlurDirection");
      };

//---------------------------------------------------------
//   FlipBeamDirection
//---------------------------------------------------------

class FlipBeamDirection : public UndoCommand {
      Beam* beam;
      void flip();

   public:
      FlipBeamDirection(Beam*);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("FlipBeamDirection");
      };

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

class ChangeKeySig : public UndoCommand {
      KeySig* keysig;
      KeySigEvent ks;
      bool showCourtesy;
      bool showNaturals;

      void flip();

   public:
      ChangeKeySig(KeySig*, KeySigEvent newKeySig, bool sc, bool sn);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeKeySig");
      };

//---------------------------------------------------------
//   FlipTupletDirection
//---------------------------------------------------------

class FlipTupletDirection : public UndoCommand {
      Tuplet* tuplet;
      void flip();

   public:
      FlipTupletDirection(Tuplet* t) : tuplet(t) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("FlipTupletDirection");
      };

//---------------------------------------------------------
//   FlipNoteDotDirection
//---------------------------------------------------------

class FlipNoteDotDirection : public UndoCommand {
      Note* note;
      void flip();

   public:
      FlipNoteDotDirection(Note* n) : note(n) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("FlipNoteDotDirection");
      };

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

class ChangeMeasureLen : public UndoCommand {
      Measure* measure;
      int oldTicks;
      int newTicks;
      void flip();

   public:
      ChangeMeasureLen(Measure*, int oldTicks, int newTicks);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeMeasureLen");
      };

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

class ChangeElement : public UndoCommand {
      Element* oldElement;
      Element* newElement;
      void flip();

   public:
      ChangeElement(Element* oldElement, Element* newElement);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeElement");
      };

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

class InsertTime : public UndoCommand {
      Score* score;
      int tick;
      int len;
      void flip();

   public:
      InsertTime(Score*, int tick, int len);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("InsertTime");
      };

//---------------------------------------------------------
//   ChangeRepeatFlags
//---------------------------------------------------------

class ChangeRepeatFlags : public UndoCommand {
      Measure* measure;
      int flags;
      void flip();

   public:
      ChangeRepeatFlags(Measure*, int flags);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeRepeatFlags");
      };

//---------------------------------------------------------
//   ChangeVoltaEnding
//---------------------------------------------------------

class ChangeVoltaEnding : public UndoCommand {
      Volta* volta;
      QList<int> list;
      void flip();

   public:
      ChangeVoltaEnding(Volta*, const QList<int>&);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeVoltaEnding");
      };

//---------------------------------------------------------
//   ChangeVoltaText
//---------------------------------------------------------

class ChangeVoltaText : public UndoCommand {
      Volta* volta;
      QString text;
      void flip();

   public:
      ChangeVoltaText(Volta*, const QString&);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeVoltaText");
      };

//---------------------------------------------------------
//   ChangeChordRestSize
//---------------------------------------------------------

class ChangeChordRestSize : public UndoCommand {
      ChordRest* cr;
      bool small;
      void flip();

   public:
      ChangeChordRestSize(ChordRest*, bool small);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeChordRestSize");
      };

//---------------------------------------------------------
//   ChangeChordNoStem
//---------------------------------------------------------

class ChangeChordNoStem : public UndoCommand {
      Chord* chord;
      bool noStem;
      void flip();

   public:
      ChangeChordNoStem(Chord*, bool noStem);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeChordNoStem");
      };

//---------------------------------------------------------
//   ChangeChordRestSpace
//---------------------------------------------------------

class ChangeChordRestSpace : public UndoCommand {
      ChordRest* cr;
      Spatium l;
      Spatium t;
      void flip();

   public:
      ChangeChordRestSpace(ChordRest*, Spatium l, Spatium t);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeChordRestSpace");
      };

//---------------------------------------------------------
//   ChangeEndBarLineType
//---------------------------------------------------------

class ChangeEndBarLineType : public UndoCommand {
      Measure* measure;
      BarLineType subtype;
      void flip();

   public:
      ChangeEndBarLineType(Measure*, BarLineType subtype);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeEndBarLineType");
      };

//---------------------------------------------------------
//   ChangeBarLineSpan
//---------------------------------------------------------

class ChangeBarLineSpan : public UndoCommand {
      Staff* staff;
      int span;
      void flip();

   public:
      ChangeBarLineSpan(Staff*, int span);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeBarLineSpan");
      };

//---------------------------------------------------------
//   ChangeUserOffset
//---------------------------------------------------------

class ChangeUserOffset : public UndoCommand {
      Element* element;
      QPointF offset;
      void flip();

   public:
      ChangeUserOffset(Element*, const QPointF& offset);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeUserOffset");
      };

//---------------------------------------------------------
//   ChangeSlurOffsets
//---------------------------------------------------------

class ChangeSlurOffsets : public UndoCommand {
      SlurSegment* slur;
      QPointF off[4];
      void flip();

   public:
      ChangeSlurOffsets(SlurSegment* s, const QPointF& o1, const QPointF& o2,
         const QPointF& o3, const QPointF& o4) : slur(s) {
            off[0] = o1;
            off[1] = o2;
            off[2] = o3;
            off[3] = o4;
            }
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeSlurOffsets");
      };

//---------------------------------------------------------
//   ChangeDynamic
//---------------------------------------------------------

class ChangeDynamic : public UndoCommand {
      Dynamic* dynamic;
      int velocity;
      DynamicType dynType;
      void flip();

   public:
      ChangeDynamic(Dynamic*, int velocity, DynamicType dt);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeDynamic");
      };

//---------------------------------------------------------
//   SigInsertTime
//---------------------------------------------------------

class SigInsertTime : public UndoCommand {
      Score* score;
      int tick;
      int len;
      void flip();

   public:
      SigInsertTime(Score*, int tick, int len);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("SigInsertTime");
      };

//---------------------------------------------------------
//   ChangeBeamMode
//---------------------------------------------------------

class ChangeBeamMode : public UndoCommand {
      ChordRest* cr;
      BeamMode mode;
      void flip();

   public:
      ChangeBeamMode(ChordRest*, BeamMode mode);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeBeamMode");
      };

#if 0
//---------------------------------------------------------
//   ChangeCopyright
//---------------------------------------------------------

class ChangeCopyright : public UndoCommand {
      Score* score;
      QString text;
      void flip();

   public:
      ChangeCopyright(Score*, const QString&);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeCopyright");
      };
#endif

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

class TransposeHarmony : public UndoCommand {
      Harmony* harmony;
      int rootTpc, baseTpc;
      void flip();

   public:
      TransposeHarmony(Harmony*, int rootTpc, int baseTpc);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("TransposeHarmony");
      };

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

class ExchangeVoice : public UndoCommand {
      Measure* measure;
      int val1, val2;
      int staff1, staff2;

   public:
      ExchangeVoice(Measure*, int val1, int val2, int staff1, int staff2);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("ExchangeVoice");
      };

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

class ChangeInstrumentShort : public UndoCommand {
      Part* part;
      int tick;
      QList<StaffNameDoc> text;
      void flip();

   public:
      ChangeInstrumentShort(int, Part*, QList<StaffNameDoc>);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeInstrumentShort");
      };

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

class ChangeInstrumentLong : public UndoCommand {
      Part* part;
      int tick;
      QList<StaffNameDoc> text;
      void flip();

   public:
      const QList<StaffNameDoc>& longNames() const;
      ChangeInstrumentLong(int, Part*, QList<StaffNameDoc>);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeInstrumentLong");
      };

//---------------------------------------------------------
//   ChangeChordRestLen
//---------------------------------------------------------

class ChangeChordRestLen : public UndoCommand {
      ChordRest* cr;
      Duration d;
      void flip();

   public:
      ChangeChordRestLen(ChordRest*, const Duration& d);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeChordRestLen");
      };

//---------------------------------------------------------
//   MoveElement
//---------------------------------------------------------

class MoveElement : public UndoCommand {
      Element* element;
      QPointF offset;
      void flip();

   public:
      MoveElement(Element*, const QPointF&);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("MoveElement");
      };

//---------------------------------------------------------
//   ChangeBracketSpan
//---------------------------------------------------------

class ChangeBracketSpan : public UndoCommand {
      Staff* staff;
      int column;
      int span;
      void flip();

   public:
      ChangeBracketSpan(Staff*, int column, int span);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeBracketSpan");
      };

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

class AddElement : public UndoCommand {
      Element* element;

   public:
      AddElement(Element*);
      virtual void undo();
      virtual void redo();
#ifdef DEBUG_UNDO
      virtual const char* name() const;
#endif
      };

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

class RemoveElement : public UndoCommand {
      Element* element;

   public:
      RemoveElement(Element*);
      virtual void undo();
      virtual void redo();
#ifdef DEBUG_UNDO
      virtual const char* name() const;
#endif
      };

//---------------------------------------------------------
//   ChangeNoteHead
//---------------------------------------------------------

class ChangeNoteHead : public UndoCommand {
      Note* note;
      int group;
      NoteHeadType type;
      void flip();

   public:
      ChangeNoteHead(Note* note, int group, NoteHeadType type);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeNoteHead");
      };

//---------------------------------------------------------
//   ChangeConcertPitch
//---------------------------------------------------------

class ChangeConcertPitch : public UndoCommand {
      Score* score;
      bool val;
      void flip();

   public:
      ChangeConcertPitch(Score* s, bool val);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeConcertPitch");
      };

//---------------------------------------------------------
//   EditText
//---------------------------------------------------------

class EditText : public UndoCommand {
      Text* text;
      int undoLevel;

   public:
      EditText(Text* t, int l) : text(t), undoLevel(l) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("EditText");
      };

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

class ChangePatch : public UndoCommand {
      Part* part;
      Channel* channel;
      MidiPatch patch;

      void flip();

   public:
      ChangePatch(Part* p, Channel* c, const MidiPatch* pt)
         : part(p), channel(c), patch(*pt) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangePitch");
      };

//---------------------------------------------------------
//   ChangeTuning
//---------------------------------------------------------

class ChangeTuning : public UndoCommand {
      Note* note;
      qreal tuning;

      void flip();

   public:
      ChangeTuning(Note* n, qreal t) : note(n), tuning(t) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeTuning");
      };

//---------------------------------------------------------
//   ChangeUserMirror
//---------------------------------------------------------

class ChangeUserMirror : public UndoCommand {
      Note* note;
      DirectionH dir;

      void flip();

   public:
      ChangeUserMirror(Note* n, DirectionH d) : note(n), dir(d) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeUserMirror");
      };

//---------------------------------------------------------
//   ChangePageFormat
//---------------------------------------------------------

class ChangePageFormat : public UndoCommand {
      Score* score;
      PageFormat* pf;
      qreal spatium;
      int pageOffset;

      void flip();

   public:
      ChangePageFormat(Score*, PageFormat*, qreal sp, int po);
      ~ChangePageFormat();
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangePageFormat");
      };

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

class ChangeStaff : public UndoCommand {
      Staff* staff;
      bool small;
      bool invisible;
      bool show;
      StaffType* staffType;

      void flip();

   public:
      ChangeStaff(Staff*, bool small, bool invisible, bool show, StaffType*);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeStaff");
      };

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

class ChangePart : public UndoCommand {
      Part* part;
      Instrument instrument;

      void flip();

   public:
      ChangePart(Part*, const Instrument&);

      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangePart");
      };

//---------------------------------------------------------
//   ChangeTextStyle
//---------------------------------------------------------

class ChangeTextStyle : public UndoCommand {
      Score* score;
      TextStyle style;
      void flip();

   public:
      ChangeTextStyle(Score*, const TextStyle& style);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeTextStyle");
      };

//---------------------------------------------------------
//   AddTextStyle
//---------------------------------------------------------

class AddTextStyle : public UndoCommand {
      Score* score;
      TextStyle style;

   public:
      AddTextStyle(Score* s, const TextStyle& st) : score(s), style(st) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("AddTextStyle");
      };

//---------------------------------------------------------
//   ChangeStretch
//---------------------------------------------------------

class ChangeStretch : public UndoCommand {
      Measure* measure;
      qreal stretch;
      void flip();

   public:
      ChangeStretch(Measure*, qreal);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeStretch");
      };

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

class ChangeStyle : public UndoCommand {
      Score* score;
      Style style;
      void flip();

   public:
      ChangeStyle(Score*, const Style&);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeStyle");
      };

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

class ChangeChordStaffMove : public UndoCommand {
      Chord* chord;
      int staffMove;
      void flip();

   public:
      ChangeChordStaffMove(Chord*, int);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeChordStaffMove");
      };

//---------------------------------------------------------
//   ChangeTupletProperties
//---------------------------------------------------------

class ChangeTupletProperties : public UndoCommand {
      Tuplet* tuplet;
      int numberType;
      int bracketType;
      void flip();

   public:
      ChangeTupletProperties(Tuplet*, int numberType, int bracketType);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeTupletProperties");
      };

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

class ChangeVelocity : public UndoCommand {
      Note* note;
      ValueType veloType;
      int veloOffset;
      void flip();

   public:
      ChangeVelocity(Note*, ValueType, int);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeVelocity");
      };

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

class ChangeMStaffProperties : public UndoCommand {
      MStaff* mstaff;
      bool visible;
      bool slashStyle;
      void flip();

   public:
      ChangeMStaffProperties(MStaff*, bool visible, bool slashStyle);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeMStaffProperties");
      };

//---------------------------------------------------------
//   ChangeMeasureProperties
//---------------------------------------------------------

class ChangeMeasureProperties : public UndoCommand {
      Measure* measure;
      Fraction sig;
      Fraction len;
      bool breakMM;
      int repeatCount;
      qreal stretch;
      int noOffset;
      bool irregular;

      void flip();

   public:
      ChangeMeasureProperties(Measure*, const Fraction& sig, const Fraction& len,
         bool breakMM,
         int repeatCount, qreal stretch, int noOffset, bool irregular);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeMeasureProperties");
      };

//---------------------------------------------------------
//   ChangeNoteProperties
//---------------------------------------------------------

class ChangeNoteProperties : public UndoCommand {
      Note* note;

      ValueType _veloType;
      int _veloOffset;        ///< velocity user offset in promille

      int _onTimeUserOffset;  ///< start note user offset
      int _offTimeUserOffset; ///< stop note user offset

      void flip();

   public:
      ChangeNoteProperties(Note*, ValueType, int, int, int);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeNoteProperties");
      };

//---------------------------------------------------------
//   ChangeMeasureTimesig
//---------------------------------------------------------

class ChangeMeasureTimesig : public UndoCommand {
      Measure* m;
      Fraction ts;

      void flip();

   public:
      ChangeMeasureTimesig(Measure*, const Fraction&);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeMeasureTimesig");
      };

//---------------------------------------------------------
//   ChangeTimesig
//---------------------------------------------------------

class ChangeTimesig : public UndoCommand {
      TimeSig* timesig;
      bool showCourtesy;
      Fraction actual;
      Fraction stretch;
      QString sz;
      QString sn;
      int subtype;

      void flip();

   public:
      ChangeTimesig(TimeSig * _timesig, bool sc, const Fraction&,
         const Fraction&, int subtype, const QString&, const QString&);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeTimesig");
      };

//---------------------------------------------------------
//   RemoveMeasures
//---------------------------------------------------------

class RemoveMeasures : public UndoCommand {
      Measure* fm;
      Measure* lm;

   public:
      RemoveMeasures(Measure*, Measure*);
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveMeasures");
      };

//---------------------------------------------------------
//   InsertMeasures
//---------------------------------------------------------

class InsertMeasures : public UndoCommand {
      Measure* fm;
      Measure* lm;

   public:
      InsertMeasures(Measure* m1, Measure* m2) : fm(m1), lm(m2) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("InsertMeasures");
      };

//---------------------------------------------------------
//   ChangeImage
//---------------------------------------------------------

class ChangeImage : public UndoCommand {
      Image* image;
      bool lockAspectRatio;
      bool autoScale;
      int z;

      void flip();

   public:
      ChangeImage(Image* i, bool l, bool a, int _z)
         : image(i), lockAspectRatio(l), autoScale(a), z(_z) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeImage");
      };

//---------------------------------------------------------
//   ChangeHairpin
//---------------------------------------------------------

class ChangeHairpin : public UndoCommand {
      Hairpin* hairpin;
      int veloChange;
      DynamicType dynType;
      bool diagonal;

      void flip();

   public:
      ChangeHairpin(Hairpin* h, int c, DynamicType t, bool dg)
         : hairpin(h), veloChange(c), dynType(t), diagonal(dg) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeHairpin");
      };

//---------------------------------------------------------
//   ChangeDuration
//---------------------------------------------------------

class ChangeDuration : public UndoCommand {
      ChordRest* cr;
      Fraction d;

      void flip();

   public:
      ChangeDuration(ChordRest* _cr, Fraction _d) : cr(_cr), d(_d) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeDuration");
      };

//---------------------------------------------------------
//   AddExcerpt
//---------------------------------------------------------

class AddExcerpt : public UndoCommand {
      Score* score;

   public:
      AddExcerpt(Score* s) : score(s) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("AddExcerpt");
      };

//---------------------------------------------------------
//   RemoveExcerpt
//---------------------------------------------------------

class RemoveExcerpt : public UndoCommand {
      Score* score;

   public:
      RemoveExcerpt(Score* s) : score(s) {}
      virtual void undo();
      virtual void redo();
      UNDO_NAME("RemoveExcerpt");
      };

//---------------------------------------------------------
//   ChangeBend
//---------------------------------------------------------

class ChangeBend : public UndoCommand {
      Bend* bend;
      QList<PitchValue> points;

      void flip();

   public:
      ChangeBend(Bend* b, QList<PitchValue> p) : bend(b), points(p) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeBend");
      };

//---------------------------------------------------------
//   ChangeTremoloBar
//---------------------------------------------------------

class ChangeTremoloBar : public UndoCommand {
      TremoloBar* bend;
      QList<PitchValue> points;

      void flip();

   public:
      ChangeTremoloBar(TremoloBar* b, QList<PitchValue> p) : bend(b), points(p) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeTremoloBar");
      };

//---------------------------------------------------------
//   ChangeNoteEvents
//---------------------------------------------------------

class ChangeNoteEvents : public UndoCommand {
      Chord* chord;
      QList<NoteEvent*> events;

      void flip();

   public:
      ChangeNoteEvents(Chord* n, const QList<NoteEvent*>& l) : chord(n), events(l) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeNoteEvents");
      };

//---------------------------------------------------------
//   ChangeBeamProperties
//---------------------------------------------------------

class ChangeBeamProperties : public UndoCommand {
      Beam* beam;
      qreal grow1, grow2;

      void flip();

   public:
      ChangeBeamProperties(Beam* b, qreal g1, qreal g2) : beam(b), grow1(g1), grow2(g2) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeBeamProperties");
      };

//---------------------------------------------------------
//   ChangeInstrument
//---------------------------------------------------------

class ChangeInstrument : public UndoCommand {
      InstrumentChange* is;
      Instrument instrument;

      void flip();

   public:
      ChangeInstrument(InstrumentChange* _is, const Instrument& i) : is(_is), instrument(i) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeInstrument");
      };

extern void updateNoteLines(Segment* segment, int track);

//---------------------------------------------------------
//   ChangeBoxProperties
//---------------------------------------------------------

class ChangeBoxProperties : public UndoCommand {
      Box* _box;

      qreal _marginLeft, _marginTop, _marginRight, _marginBottom;
      Spatium _height, _width;
      qreal _topGap, _bottomGap;

      void flip();

   public:
      ChangeBoxProperties(Box *, qreal, qreal, qreal, qreal,
         Spatium, Spatium,
         qreal, qreal);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeBoxProperties");
      };

//---------------------------------------------------------
//   SwapCR
//---------------------------------------------------------

class SwapCR : public UndoCommand {
      ChordRest* cr1;
      ChordRest* cr2;

      void flip();

   public:
      SwapCR(ChordRest* a, ChordRest* b) : cr1(a), cr2(b) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("SwapCR");
      };

//---------------------------------------------------------
//   ChangeClefType
//---------------------------------------------------------

class ChangeClefType : public UndoCommand {
      Clef* clef;
      ClefType concertClef;
      ClefType transposingClef;
      void flip();

   public:
      ChangeClefType(Clef*, ClefType cl, ClefType tc);
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeClef");
      };

//---------------------------------------------------------
//   MoveStaff
//---------------------------------------------------------

class MoveStaff : public UndoCommand {
      Staff* staff;
      Part* part;
      int rstaff;

      void flip();

   public:
      MoveStaff(Staff* s, Part* p, int idx) : staff(s), part(p), rstaff(idx) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("MoveStaff");
      };

//---------------------------------------------------------
//   ChangeDurationType
//---------------------------------------------------------

class ChangeDurationType : public UndoCommand {
      ChordRest* cr;
      Duration t;

      void flip();

   public:
      ChangeDurationType(ChordRest* _cr, Duration _t)
         : cr(_cr), t(_t) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeDurationType");
      };

//---------------------------------------------------------
//   ChangeSpannerAnchor
//---------------------------------------------------------

class ChangeSpannerAnchor : public UndoCommand {
      Spanner* spanner;
      Element* startElement;
      Element* endElement;

      void flip();

   public:
      ChangeSpannerAnchor(Spanner* s, Element* se, Element* ee)
         : spanner(s), startElement(se), endElement(ee) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeSpannerAnchor");
      };

//---------------------------------------------------------
//   ChangeProperty
//---------------------------------------------------------

class ChangeProperty : public UndoCommand {
      Element* element;
      int id;
      QVariant property;

      void flip();

   public:
      ChangeProperty(Element* e, int i, QVariant v) : element(e), id(i), property(v) {}
      virtual void undo() { flip(); }
      virtual void redo() { flip(); }
      UNDO_NAME("ChangeProperty");
      };

#endif

