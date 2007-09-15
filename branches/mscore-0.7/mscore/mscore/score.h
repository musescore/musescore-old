//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: score.h,v 1.21 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
#include "input.h"
#include "padstate.h"

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
//   Score
//---------------------------------------------------------

class Score : public QObject {
      Q_OBJECT

      PadState _padState;
      InputState _is;
      bool _noteEntryMode;

      Style* _style;
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
      ScoreLayout* _layout;   ///< Main layout.

      bool _showInvisible;

      Element* _paletteObject;

      EditTempo* editTempo;
      Element* origDragObject;
      Element* _dragObject;
      ElementList startDragSelected;

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

      bool cmdActive;
      int _fileDivision; ///< division of current loading *.msc file

      ChordRest* nextChordRest(ChordRest*);
      ChordRest* prevChordRest(ChordRest*);
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
      void convertTrack(MidiTrack*, int);
      void checkUndoOp();
      void pasteStaff(const QMimeData* ms);
      void cmdReplaceElements(Measure* sm, Measure* dm, int staffIdx);
      void move(const QString& cmd);

   public:
      int curTick;      // used for read()/write() optimization

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
      void cmdAppendMeasures(int);
      void appendMeasures(int);
      void insertMeasures(int);
// Added by DK, 06.08.07
      void cmdInsertMeasures(int);
      void padToggle(int n);
      void doUndo();
      void doRedo();
      void processUndoOp(UndoOp*, bool);

      void addLyrics();
      void addTempo();
      void addMetronome();

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
      void readStaff(QDomElement);

      QList<Staff*>& staves()       { return _staves; }
      int nstaves() const           { return _staves.size(); }
      bool noStaves() const         { return _staves.empty(); }

      int staff(const Part*) const;
      Staff* staff(int n) const     { return _staves[n]; }

      void cmdInsertPart(Part*, int);
      void insertPart(Part*, int);
      void cmdRemovePart(Part*);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void removeStaff(Staff*);

      Part* part(int staff);

      void addMeasure(Measure*);
      void removeMeasure(Measure*);

      Measure* pos2measure(const QPointF&, int* tick, Staff** staff, int* pitch,
         Segment**, QPointF* offset) const;
      Measure* pos2measure2(const QPointF&, int* tick, Staff** staff, int* pitch, Segment**) const;
      int snap(int tick, const QPointF) const;
      int snapNote(int tick, const QPointF, int staff) const;

      void addViewer(Viewer* v);
      void clearViewer();

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
      void undoChangeTempo(int tick, const TEvent& o, const TEvent& n);
      void undoChangeKey(Staff* staff, int tick, int o, int n);
      void undoChangeClef(Staff* staff, int tick, int o, int n);
      void undoAddElement(Element* element);
      void undoRemoveElement(Element* element);
      void undoChangeMeasureLen(Measure* m, int tick);
      void undoChangeElement(Element* oldElement, Element* newElement);
      void undoInsertTime(int tick, int len);

      void setNote(int tick, int track, int pitch, int len);
      int clefOffset(int tick, Staff*) const;
      Rest* setRest(int tick, int len, int track, Measure*);
      void setRest(int tick, int track, int len);
      Canvas* canvas() const;

      void select(Element* obj, int state, int staff);
      void deselect(Element* obj);

      void searchSelectedElements();

      bool needLayout() const;
      void doLayout();
      void reLayout(Measure*);

      void cmdAddText(int style);
      void upDown(bool up, bool octave);
      Element* searchNote(int tick, int track) const;

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
      void cmdAddBSymbol(BSymbol*, const QPointF&, const QPointF&);

      Element* cmdAddHairpin(Hairpin* atr, const QPointF& pos);
      Element* addSlur(Slur* atr, const QPointF& pos);
      Note* addNote(Chord*, int pitch);

      // menu entry:
      void addBar(BarLine*, Measure*);

      Slur* cmdAddSlur();
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
      void setPadState(Element* obj);

      void startCmd();
      void start();
      void end();
      void endCmd();

      void cmdAdd(Element*);
      void cmdRemove(Element*);

      void setUpdateAll()              { updateAll = true; }
      void setLayoutAll(bool val)      { layoutAll = val;  }
      void setLayoutStart(Measure* m)  { layoutStart = m;  }
      void addRefresh(const QRectF& r) { refresh |= r;     }

      void chordTab(bool back);
      void lyricsTab(bool back);
      void lyricsReturn();
      void lyricsMinus();

      void startEdit(Element* element);
      void endEdit();

      void startDrag();
      void drag(const QPointF&);
      void endDrag();

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
      bool importMidi(const QString& name);
      bool importMuseData(const QString& name);
      bool importLilypond(const QString& name);

      void print(QPrinter* printer);
      bool saveXml(const QString& name);
      bool saveMidi(const QString& name);
      bool savePdf(const QString& name);
      bool savePs(const QString& name);
      bool savePng(const QString& name);
      bool saveSvg(const QString& name);
      bool saveLilypond(const QString& name);

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

      void addAttribute(Element*, NoteAttribute* atr);

      bool playlistDirty();
      void changeTimeSig(int tick, int st);

      void cmd(const QString&);
      Element* editElement() const { return editObject; }
      int fileDivision(int t) const { return (t * division + _fileDivision/2) / _fileDivision; }
      bool saveFile();
      void adjustTime(int tick, Measure*);

      QString filePath() const    { return info.filePath(); }
      QFileInfo* fileInfo()       { return &info; }
      QString projectName() const { return info.baseName(); }
      bool isSavable() const;
      void setDirty(bool val = true);
      bool dirty() const        { return _dirty; }
      void setCreated(bool val) { _created = val; }
      bool created() const      { return _created; }
      bool saved() const        { return _saved; }
      void setSaved(bool v)     { _saved = v; }
      bool printing() const     { return _printing; }

      QPointF tick2Anchor(int tick, int staffIdx) const;
      bool pos2TickAnchor(QPointF&, Staff*, int* tick, QPointF* anchor) const;
      void spell();
      void spell(Note*);
      int nextSeg(int tick, int track);
      int prevSeg(int tick, int track);
      int nextSeg1(int tick, int& track);
      int prevSeg1(int tick, int& track);
      Style* style() const { return _style; }
      bool noteEntryMode() const { return _noteEntryMode; }
      void insertTime(int tick, int len);
      void cmdRemoveTime(int tick, int len);
      QList<Viewer*> getViewer() { return viewer;    }
      int playPos() const        { return _playPos;  }
      void setPlayPos(int val)   { _playPos = val;   }
      int inputPos() const       { return _is.pos;   }
      int inputTrack() const     { return _is.track; }
      void setInputTrack(int v)  { _is.track = v;    }
      PadState* padState()       { return &_padState; }
      };

extern Score* gscore;
extern void fixTicks();
extern int y2pitch(double y, int clef);
extern int line2pitch(int line, int clef);

#endif

