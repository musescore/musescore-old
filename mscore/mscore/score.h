//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: score.h,v 1.21 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __SCORE_H__
#define __SCORE_H__

/**
 \file
 Definition of Score class.
*/

#include "undo.h"
#include "input.h"
#include "padstate.h"

class System;
class TextStyle;
class Page;
class SigList;
class PageFormat;
class ElementList;
class TempoList;
class Selection;
class Segment;
class Rest;
class Xml;
class Canvas;
class Viewer;
class NoteAttribute;
class Note;
class Chord;
class ChordRest;
class Slur;
class Hairpin;
class Undo;
class EditTempo;
class Part;
class MidiFile;
class ScoreLayout;
class MidiTrack;
class BSymbol;
class KeySig;
class Volta;
class BBTrack;
class MidiEvent;
struct MNote;
class Excerpt;
class EventMap;
class Harmony;
struct Articulation;

struct Style;
struct SigEvent;

extern bool showRubberBand;

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

class ScoreView {
   public:
      qreal _mag, _xoffset, _yoffset;
      ScoreView() {
            _mag     = 1.0;
            _xoffset = 0.0;
            _yoffset = 0.0;
            }
      };

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

class MeasureBaseList {
      int _size;
      MeasureBase* _first;
      MeasureBase* _last;

      void push_back(MeasureBase* e);
      void push_front(MeasureBase* e);

   public:
      MeasureBaseList();
      MeasureBase* first() const { return _first; }
      MeasureBase* last()  const { return _last; }
      void clear()               { _first = _last = 0; }
      void add(MeasureBase*);
      void remove(MeasureBase*);
      void change(MeasureBase* o, MeasureBase* n);
      };

//---------------------------------------------------------
//   MidiMapping
//---------------------------------------------------------

struct MidiMapping {
      char port;
      char channel;
      Part* part;
      Articulation* articulation;
      };

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

class Score : public QObject {
      Q_OBJECT
      Q_PROPERTY (int nstaves READ nstaves)
      Q_PROPERTY (QString name READ name)

      QList<MidiMapping> _midiMapping;
      MeasureBaseList _measures;          // here are the notes
      QList<Element*> _gel;               // global elements: Slur, SLine

      PadState   _padState;
      InputState _is;

      QList<Excerpt*> _excerpts;

      Style* _style;
      QVector<TextStyle*> _textStyles;

      QFileInfo info;
      bool _created;          ///< file is never saved, has generated name

      // the following variables are reset on startCmd()
      //   modified during cmd processing and used in endCmd() to
      //   determine what to layout and what to repaint:

      QRectF refresh;
      bool updateAll;
      Measure* layoutStart;   ///< start a relayout at this measure
      bool layoutAll;         ///< do a complete relayout

      Qt::KeyboardModifiers keyState;

      QList<Viewer*> viewer;

      ScoreView scoreView;
      ScoreLayout* _layout;

      bool _showInvisible;

      EditTempo* editTempo;
      Element* origDragObject;
      Element* _dragObject;

      QList<Part*> _parts;
      QList<Staff*> _staves;

      UndoList undoList;
      UndoList redoList;

      bool _printing;   ///< True if we are drawing to a printer
      bool _playlistDirty;
      bool _dirty;      ///< Score data was modified.
      bool _saved;      ///< True if project was already saved; only on first
                        ///< save a backup file will be created, subsequent
                        ///< saves will not overwrite the backup file.
      int _playPos;     ///< sequencer seek position

      int _fileDivision; ///< division of current loading *.msc file
      int _mscVersion;   ///< version of current loading *.msc file

      ChordRest* nextMeasure(ChordRest*);
      ChordRest* prevMeasure(ChordRest*);
      void cmdSetBeamMode(int);
      void cmdFlipStemDirection();
      Note* getSelectedNote();
      void pageNext();
      void pagePrev();
      void pageTop();
      void pageEnd();
      Note* upAlt(Element*);
      Note* upAltCtrl(Note*) const;
      Note* downAlt(Element*);
      Note* downAltCtrl(Note*) const;
      void moveUp(Note*);
      void moveDown(Note*);
      void convertTrack(MidiTrack*);
      void convertTrack(BBTrack*, int);
      void checkUndoOp();
      void pasteStaff(const QMimeData* ms);
      void move(const QString& cmd);

      void collectChord(EventMap*, Instrument*,
         int pitchOffset, Chord*, int tick, int gateTime);
      void collectMeasureEvents(EventMap*, Measure*, int staffIdx, int tickOffset);

      void padToggle(int n);
      void insertMeasures(int, int);

      void cmdAddPitch(int, bool);
      void cmdAddInterval(int);

      void printFile();
      void addLyrics();
      void addTempo();
      void addMetronome();

      MeasureBase* searchLabel(const QString& s, MeasureBase* start = 0);
      void undoSigInsertTime(int, int);
      void undoFixTicks();

      void cmdAddText(int style);
      void cmdAddChordName();
      int processPendingNotes(QList<MNote*>* notes, int, int);
      void writeExcerpt(Excerpt*, Xml&);
      void renumberMeasures();
      void cmdResetBeamMode();
      void connectSlurs();
      void checkSlurs();
      void checkTuplets();
      void tupletDialog();

      void cmdInsertClef(int type);
      void cmdCreateTuplet(ChordRest*, Tuplet*);
      void cmdExchangeVoice(int, int);

   public slots:
      void doUndo();
      void doRedo();
      void processUndoOp(UndoOp*, bool);

      void resetUserOffsets();
      void resetUserStretch();

   signals:
      void selectionChanged(int);

   public:
      bool cmdActive;
      int curTick;            // for read optimizations
      int curTrack;

      //---------------------------------------------------
      //    input data for layout():
      //---------------------------------------------------

      QString movementNumber;
      QString movementTitle;
      QTextDocument* rights;

      int _pageOffset;              ///< Offset for page numbers.

      SigList*   sigmap;
      TempoList* tempomap;

      //---------------------------------------------------
      //    state information
      //---------------------------------------------------

      Selection* sel;

      Element* origEditObject;
      Element* editObject;

      System* dragSystem;     ///< Valid if DRAG_STAFF.
      int dragStaff;

      void cmdAppendMeasures(int);
      void cmdInsertMeasures(int);
      bool noStaves() const         { return _staves.empty(); }
      void insertPart(Part*, int);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void cmdRemoveStaff(int staffIdx);
      void removeStaff(Staff*);
      void addMeasure(MeasureBase*);
      void removeMeasure(MeasureBase*);
      void appendMeasures(int, int);
      void readStaff(QDomElement);

      void cmdTuplet(int);
      void cmdInsertPart(Part*, int);
      void cmdRemovePart(Part*);
      void cmdReplaceElements(Measure* sm, Measure* dm, int srcStaff, int dstStaff);
      void cmdAddSlur();
      void cmdAddTie();
      void cmdAddHairpin(bool);
      void cmdAddStretch(double);
      void transpose();

      void cmdEnterRest();

      Rest* addRest(int tick, int len, int track);
      void lyricsEndEdit();
      void harmonyEndEdit();

   public:
      Score();
      ~Score();

      Score* clone();

      void clear();
      void write(Xml&, bool autosave);
      bool read(QDomElement);

      QList<Staff*>& staves()                { return _staves; }
      Q_INVOKABLE int nstaves() const        { return _staves.size(); }

      int staffIdx(const Part*) const;
      int staffIdx(const Staff* staff) const { return _staves.indexOf((Staff*)staff, 0); }
      Staff* staff(int n) const              { return _staves[n]; }

      Part* part(int staffIdx);

      MeasureBase* pos2measure(const QPointF&, int* tick, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;
      Measure* pos2measure2(const QPointF&, int* tick, int* staffIdx, int* pitch, Segment**) const;
      Measure* pos2measure3(const QPointF& p, int* tick) const;

      void addViewer(Viewer* v);
      void clearViewer();

      void undoOp(QList<int>);
      void undoOp(UndoOp::UndoType type, MeasureBase* m);
      void undoOp(UndoOp::UndoType type, Measure*, int, int);
      void undoOp(UndoOp::UndoType type, Measure* m, MStaff* s, int staff);
//      void undoOp(UndoOp::UndoType type, Staff* staff, int idx);
      void undoOp(UndoOp::UndoType type, Staff* staff, int tick, int oval, int nval);
      void undoOp(UndoOp::UndoType type, Segment* seg, int staff);
      void undoOp(UndoOp::UndoType type, Element* object);
      void undoOp(UndoOp::UndoType type, Element*, const QColor&);
      void undoOp(UndoOp::UndoType type, Element*, int idx);
      void undoOp(UndoOp::UndoType type, int a, int b);

      void undoSetPitchSpellNeeded();
      void undoChangeSig(int tick, const SigEvent& o, const SigEvent& n);
      void undoChangeTempo(int tick, const TEvent& o, const TEvent& n);
      void undoChangeKey(Staff* staff, int tick, int o, int n);
      void undoChangeClef(Staff* staff, int tick, int o, int n);
      void undoAddElement(Element* element);
      void undoRemoveElement(Element* element);
      void undoChangeMeasureLen(Measure* m, int tick);
      void undoChangeElement(Element* oldElement, Element* newElement);
      void undoInsertTime(int tick, int len);
      void undoChangeRepeatFlags(Measure*, int);
      void undoChangeVoltaEnding(Volta* volta, const QList<int>& l);
      void undoChangeVoltaText(Volta* volta, const QString& s);
      void undoChangeChordRestSize(ChordRest* cr, bool small);
      void undoChangeSubtype(Element* element, int st);
      void undoChangeNoteHead(Note*, int group);
      void undoChangePitch(Note* note, int pitch);
      void undoChangeBeamMode(ChordRest* cr, int mode);
      void undoChangeEndBarLineType(Measure*, int);
      void undoChangeBarLineSpan(Staff*, int);
      void undoChangeCopyright(const QString&);
      void undoTransposeHarmony(Harmony*, int);
      void undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2);
      void undoRemovePart(Part* part, int idx);
      void undoInsertPart(Part* part, int idx);
      void undoRemoveStaff(Staff* staff, int idx);
      void undoInsertStaff(Staff* staff, int idx);

      void setNote(int tick, int track, int pitch, int len);
      void setTupletChordRest(ChordRest* cr, int pitch, int len);

      void setGraceNote(Chord*,  int pitch, NoteType type, int len);
      int clefOffset(int tick, Staff*) const;
      Rest* setRest(int tick, int len, int track);
      bool setRest(int tick, int track, int len, bool useDots);
      Canvas* canvas() const;

      void select(Element* obj, int state, int staff);
      void deselect(Element* obj);

      void searchSelectedElements();

      bool needLayout() const;
//      void doLayout();

      void upDown(bool up, bool octave);
      Element* searchNote(int tick, int track) const;

      // undo/redo ops
      void endUndoRedo(Undo*);
      void addAttribute(int);
      void addAccidental(int);
      void addAccidental(Note* oNote, int prefix);

      void addElement(Element*);
      void removeElement(Element*);

      void addTimeSig(int tick, int keySigSubtype);

      void cmdAdd1(Element* e, const QPointF& pos, const QPointF& dragOffset);
      void cmdAddBSymbol(BSymbol*, const QPointF&, const QPointF&);

      Note* addNote(Chord*, int pitch);

      void deleteItem(Element*);
      void cmdDeleteSelection();
      void toggleInvisible(Element* obj);

      void changeRest(Rest* rest, int tick, int len);

      void putNote(const QPointF& pos, bool replace);
      void setPadState();
      void setPadState(Element* obj);

      void startCmd();        // start undoable command
      void endCmd();          // end undoable command
      void end();             // layout & update canvas

      void cmdAdd(Element*);
      void cmdRemove(Element*);

      void setUpdateAll()              { updateAll = true; }
      void setLayoutAll(bool val)      { layoutAll = val;  }
      void setLayoutStart(Measure* m)  { layoutStart = m;  }
      void addRefresh(const QRectF& r) { refresh |= r;     }

      void chordTab(bool back);
      void lyricsTab(bool back, bool end);
      void lyricsReturn();
      void lyricsMinus();
      void lyricsUnderscore();
      void changeLineSegment(bool);

      void startEdit(Element* element);
      void endEdit();

      void startDrag(Element*);
      void drag(const QPointF&);
      void endDrag();

      void changeVoice(int);
      void setNoteEntry(bool on);

      void colorItem(Element*);
      void adjustCanvasPosition(Element*);
      Element* dragObject() const    { return _dragObject; }
      void setDragObject(Element* e) { _dragObject = e; }
      void midiNoteReceived(int pitch, bool);
      const QList<Part*>* parts() const  { return &_parts; }
      void appendPart(Part* p);
      void updateStaffIndex();
      void sortStaves(QList<int> dst);
      void read(QString name);

      void setSpatium(double v);

      double mag() const         { return scoreView._mag;     }
      void setMag(double val)    { scoreView._mag = val;      }
      qreal xoffset() const      { return scoreView._xoffset; }
      qreal yoffset() const      { return scoreView._yoffset; }
      void setXoffset(qreal val) { scoreView._xoffset = val;  }
      void setYoffset(qreal val) { scoreView._yoffset = val;  }

      bool showInvisible() const { return _showInvisible; }
      void setShowInvisible(bool v);

      bool loadMsc(QString name);
      bool loadCompressedMsc(QString name);
      void importMusicXml(const QString&);
      void importCompressedMusicXml(const QString&);
      void convertMidi(MidiFile*, int);
      bool importMidi(const QString& name);
      bool importMuseData(const QString& name);
      bool importLilypond(const QString& name);
      bool importBB(const QString& name);

      void print(QPrinter* printer);
      Q_INVOKABLE bool saveXml(const QString& name);
      Q_INVOKABLE bool saveMxl(const QString& name);
      Q_INVOKABLE bool saveMidi(const QString& name);
      bool savePsPdf(const QString& saveName, QPrinter::OutputFormat format);
      Q_INVOKABLE bool savePng(const QString& name);
      Q_INVOKABLE bool saveSvg(const QString& name);
      Q_INVOKABLE bool saveLilypond(const QString& name);

      ChordRest* getSelectedChordRest() const;
      Element* getSelectedElement() const { return sel->element(); }
      Selection* selection() const        { return sel; }
      int pos();
      Measure* tick2measure(int tick) const;
      MeasureBase* tick2measureBase(int tick) const;
      Segment* tick2segment(int tick) const;
      void fixTicks();
      bool undoEmpty() const;
      bool redoEmpty() const;
      PageFormat* pageFormat() const;
      ScoreLayout* layout() const { return _layout; }

      void addAttribute(Element*, NoteAttribute* atr);

      bool playlistDirty();
      void changeTimeSig(int tick, int st);

      void cmd(const QString&);
      int fileDivision(int t) const { return (t * division + _fileDivision/2) / _fileDivision; }
      bool saveFile(bool autosave);
      void adjustTime(int tick, MeasureBase*);

      QString filePath() const    { return info.filePath(); }
      QFileInfo* fileInfo()       { return &info; }

      Q_INVOKABLE QString name() const { return info.baseName(); }

      bool isSavable() const;
      void setDirty(bool val = true);
      bool dirty() const        { return _dirty;    }
      void setCreated(bool val) { _created = val;   }
      bool created() const      { return _created;  }
      bool saved() const        { return _saved;    }
      void setSaved(bool v)     { _saved = v;       }
      bool printing() const     { return _printing; }

      bool pos2TickAnchor(const QPointF&, int staffIdx, int* tick, QPointF* anchor) const;
      void spell();
      void spell(Note*);
      int nextSeg(int tick, int track);
      int nextSeg1(int tick, int& track);
      int prevSeg1(int tick, int& track);

      Style* style() const          { return _style; }
      void setStyle(const Style& s);

      void insertTime(int tick, int len);
      void cmdRemoveTime(int tick, int len);
      QList<Viewer*> getViewer()    { return viewer;    }
      int playPos() const           { return _playPos;  }
      void setPlayPos(int val)      { _playPos = val;   }

      bool noteEntryMode() const    { return _is.noteEntryMode; }
      int inputPos() const          { return _is.pos;   }
      int inputTrack() const        { return _is.track; }
      void setInputTrack(int);

      PadState* padState()          { return &_padState; }
      TextStyle* textStyle(int idx) { return _textStyles[idx]; }
      const QVector<TextStyle*>& textStyles() const { return _textStyles; }
      void setTextStyles(QVector<TextStyle*>&s);
      bool loadStyle(QFile* qf);
      void loadStyle();
      void saveStyle();

      void setCopyright(QTextDocument*);
      void setCopyright(const QString& s);
      void setCopyrightHtml(const QString& s);
      void pasteStaff(QDomElement e, Measure* measure, int staffIdx);
      bool isVolta(int tick, int repeat) const;
      void toEList(EventMap* events, int tickOffset);
      void toEList(EventMap* events, bool expandRepeats, int tickOffset, int staffIdx);
      int mscVersion() const { return _mscVersion; }

      SigList*   getSigmap()  { return sigmap; }
      MeasureBase* appendMeasure(int type);
      UndoList* getUndoList() { return &undoList; }
      void addLyrics(int tick, int staffIdx, const QString&);

      QList<Excerpt*>* excerpts() { return &_excerpts; }
      Score* createExcerpt(Excerpt*);
      MeasureBaseList* measures()  { return &_measures; }

      QList<Element*>* gel()                  { return &_gel; }
      const QList<Element*>* gel() const      { return &_gel; }
      void setLayout(Measure* m);
      int midiPort(int idx) const;
      int midiChannel(int idx) const;
      QList<MidiMapping>* midiMapping()       { return &_midiMapping; }
      void rebuildMidiMapping();
      void updateArticulation();
      void cmdTransposeStaff(int staffIdx, int offset);
      void cmdConcertPitchChanged(bool);
      };

extern Score* gscore;
extern void fixTicks();

#endif

