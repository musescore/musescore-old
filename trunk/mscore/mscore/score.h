//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: score.h,v 1.21 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __SCORE_H__
#define __SCORE_H__

/**
 \file
 Definition of ElemList and Score classes.
*/

#include "system.h"
#include "undo.h"

class Page;
class PageList;
class SigList;
class KeyList;
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
class Dynamic;
class Slur;
class Hairpin;
class Undo;
class Synth;
class Audio;
class SymbolPalette;
class EditTempo;
class Part;
class StaffList;
class MidiFile;
class ScoreLayout;
class TimeSig;
class MidiTrack;
class Pedal;
class Symbol;
class Image;

struct SigEvent;

extern QPoint scorePos;
extern QSize scoreSize;
extern bool debugMode;
extern bool layoutDebug;
extern bool noSeq;            ///< Dont use sequencer; cmd line option.
extern bool noMidi;           ///< Dont use midi; cmd line option.
extern bool dumpMidi;         ///< debug option: dump midi input
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
//   Score
//---------------------------------------------------------

class Score : public QObject {
      Q_OBJECT

      QFileInfo info;
      bool _created;          ///< file is never saved, has generated name

      QRectF refresh;
      bool updateAll;
      Qt::KeyboardModifiers keyState;

      QList<Viewer*> viewer;

      ScoreView scoreView;
      ScoreLayout* _layout;   ///< Main layout.

      bool _showInvisible;

      Element* _paletteObject;

      EditTempo* editTempo;
      Element* origDragObject;
      Element* _dragObject;
      ElementList startDragSelected;

      QList<Part*> _parts;
      StaffList* _staves;

      UndoList undoList;
      UndoList redoList;

      bool _playlistDirty;
      bool _dirty;      ///< Score data was modified.
      bool _saved;      ///< True if project was already saved; only on first
                        ///< save a backup file will be created, subsequent
                        ///< saves will not overwrite the backup file.

      int _fileDivision; ///< division of current loading *.msc file

      ChordRest* nextChordRest(ChordRest*);
      ChordRest* prevChordRest(ChordRest*);
      ChordRest* nextMeasure(ChordRest*);
      ChordRest* prevMeasure(ChordRest*);
      void cmdSetBeamMode(int);
      void cmdFlipStemDirection();
      void moveCursor();
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
      void convertTrack(MidiTrack*, int);
      void preprocessTrack(MidiTrack*);
      void checkUndoOp();
      void pasteStaff(const QMimeData* ms);
      void cmdReplaceElements(Measure* sm, Measure* dm, int staffIdx);
      void move(const QString& cmd);

   public:
      bool undoActive;

      //---------------------------------------------------
      //    input data for layout():
      //---------------------------------------------------

      QString movementNumber;
      QString movementTitle;
      QString rights;

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

      void cmdAddPitch(int, bool);
      void cmdAddInterval(int);

   public slots:
      void printFile();
      void cmdAddTitle();
      void cmdAddSubTitle();
      void cmdAddComposer();
      void cmdAddPoet();
      void cmdAppendMeasure();
      void cmdAppendMeasures(int);
      void appendMeasures(int);
      void padToggle(int n);
      void doUndo();
      void doRedo();
      void processUndoOp(UndoOp*, bool);

      void addLyrics();
      void addTempo();
      void addTechnik();
      void addMetronome();
      void addExpression();

      void cmdTuplet(int);
      void midiReceived();
      void resetUserOffsets();
      void resetUserStretch();

      void pageBreak();
      void systemBreak();

   signals:
      void selectionChanged(int);

   public:
      Score();
      ~Score();

      void clear();
      void write(Xml&);
      void readStaff(QDomNode);

      StaffList* staves() const     { return _staves; }
      int nstaves() const;
      int staff(const Part*) const;
      int staff(const Staff*) const;
      Staff* staff(int n) const;
      bool noStaves() const;

      void cmdInsertPart(Part*, int);
      void insertPart(Part*, int);
      void cmdRemovePart(Part*);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void removeStaff(Staff*);

      Part* part(int staff);

      void addMeasure(Measure*);
      void removeMeasure(int tick);

      Measure* pos2measure(const QPointF&, int* tick, Staff** staff, int* pitch,
         Segment**, QPointF* offset) const;
      Measure* pos2measure2(const QPointF&, int* tick, Staff** staff, int* pitch, Segment**) const;
      void readGeometry(QDomNode);
      int snap(int tick, const QPointF) const;
      int snapNote(int tick, const QPointF, int staff) const;

      void addViewer(Viewer* v);
      void clearViewer();

      void startUndo();
      void endUndo();
      void undoOp(QList<int> si, QList<int> di);
      void undoOp(UndoOp::UndoType type, Measure* m);
      void undoOp(UndoOp::UndoType type, Measure*, int, int);
      void undoOp(UndoOp::UndoType type, Measure* m, MStaff s, int staff);
      void undoOp(UndoOp::UndoType type, Staff* staff, int idx);
      void undoOp(UndoOp::UndoType type, Staff* staff, int tick, int oval, int nval);
      void undoOp(UndoOp::UndoType type, Part* part, int idx);
      void undoOp(UndoOp::UndoType type, Segment* seg, int staff);
      void undoOp(UndoOp::UndoType type, Element* object);
      void undoOp(UndoOp::UndoType type, Element*, const QColor&);
      void undoOp(UndoOp::UndoType type, Element*, int idx);
      void undoOp(UndoOp::UndoType type, int a, int b);
      void undoChangeSig(int tick, const SigEvent& o, const SigEvent& n);
      void undoAddElement(Element* element);
      void undoRemoveElement(Element* element);
      void undoChangeMeasureLen(Measure* m, int tick);
      void undoChangeElement(Element* oldElement, Element* newElement);

      void setNote(int tick, Staff* staff, int voice, int pitch, int len);
      int clefOffset(int tick, Staff*) const;
      Rest* setRest(int tick, int len, Staff*, int voice, Measure*);
      void setRest(int tick, Staff*, int voice, int len);
      Canvas* canvas() const;

      void select(Element* obj, int state, int staff);
      void deselect(Element* obj);

      void searchSelectedElements();

      void layout();
      bool needLayout() const;
      void doLayout();

      void cmdAddText(int style);
      void upDown(bool up, bool octave);
      Element* searchNote(int tick, int staffIdx) const;

      // undo/redo ops
      void endUndoRedo(Undo*);
      void addAttribute(int);
      void addAccidental(int);
      void addAccidental(Note* oNote, int prefix);

      void addElement(Element*);
      void removeElement(Element*);

      Element* addClef(Clef*);
      void addTimeSig(int tick, int keySigSubtype);
      Element* addKeySig(KeySig*, const QPointF&);

      void cmdAdd(Element* e, const QPointF& pos, const QPointF& dragOffset);
      void cmdAddSymbol(Symbol*, const QPointF&, const QPointF&);
      void cmdAddImage(Image* s, const QPointF& pos, const QPointF& off);

      Element* cmdAddHairpin(Hairpin* atr, const QPointF& pos);
      Element* addSlur(Slur* atr, const QPointF& pos);
      Note* addNote(Chord*, int pitch);

      // menu entry:
      void addBar(BarLine*, Measure*);

      void cmdAddSlur();
      void cmdAddTie();
      void cmdAddHairpin(bool);
      void cmdAddStretch(double);

      void removeClef(Clef*);
      void deleteItem(Element*);
      void cmdDeleteItem(Element*);
      void cmdDeleteSelection();
      void toggleInvisible(Element* obj);

      void changeRest(Rest* rest, int tick, int len);
      void changeStaff(Staff* staff, int idx);

      void putNote(const QPointF& pos, bool addToChord);
      void setPadState();

      void start();
      void end();
      void end1();
      void startCmd();
      void endCmd(bool undo);
      void cmdAdd(Element*);
      void cmdRemove(Element*);
      void update(const QRectF&);

      void setUpdateAll()              { updateAll = true; }
      void addRefresh(const QRectF& r) { refresh |= r; }
      void lyricsTab(bool back);
      void lyricsReturn();
      void lyricsMinus();

      void startEdit(Element* element);
      bool edit(QKeyEvent* ev);
      void endEdit();

      void startDrag();
      void drag(const QPointF&);
      void endDrag();

      void dragEdit(Viewer*, QPointF* startMove, const QPointF& delta);
      void layoutPageHeader(Page*);

      void changeVoice(int);
      ChordRest* setNoteEntry(bool on, bool step);

      Element* paletteObject() const { return _paletteObject; }
      void colorItem(Element*);
      void adjustCanvasPosition(Element*);
      Element* dragObject() const    { return _dragObject; }
      void setDragObject(Element* e) { _dragObject = e; }
      void midiNoteReceived(int pitch, bool);
      QList<Part*>* parts()       { return &_parts; }
      void updateStaffIndex();
      void sortStaves(QList<int> src, QList<int> dst);
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
      bool loadFile(QFile* f);
      void importMusicXml(const QString&);
      void convertMidi(MidiFile*);
      void importMidi(const QString& name);
      ChordRest* getSelectedChordRest();
      int pos();
      Measure* tick2measure(int tick) const;
      Segment* tick2segment(int tick) const;
      QPointF tick2pos(int tick, int staff);
      void fixTicks();
      bool undoEmpty() const;
      bool redoEmpty() const;
      PageFormat* pageFormat() const;
      ScoreLayout* mainLayout() const { return _layout; }
      QString filePath() const    { return info.filePath(); }
      QFileInfo* fileInfo()       { return &info; }
      QString projectName() const { return info.baseName(); }
      void addAttribute(Element*, NoteAttribute* atr);

      bool playlistDirty();
      void changeTimeSig(int tick, int st);

      void cmd(const QString&);
      Element* editElement() const { return editObject; }
      int fileDivision(int t) const { return (t * division + _fileDivision/2) / _fileDivision; }
      bool saveFile();
      void adjustTime(int tick, Measure*);

      void setDirty(bool val = true);
      bool dirty() const        { return _dirty; }
      void setCreated(bool val) { _created = val; }
      bool created() const      { return _created; }
      bool saved() const        { return _saved; }
      void setSaved(bool v)     { _saved = v; }

      QPointF tick2Anchor(int tick, int staffIdx) const;
      bool pos2TickAnchor(QPointF&, Staff*, int* tick, QPointF* anchor) const;
      };

extern void setPadState(Element*);
extern void fixTicks();
extern int y2pitch(double y, int clef);
extern int line2pitch(int line, int clef);

#endif

