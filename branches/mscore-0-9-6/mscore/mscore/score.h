//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "input.h"
#include "globals.h"
#include "style.h"
#include "durationtype.h"
#include "select.h"
#include "config.h"
#include "element.h"
#include "bsp.h"
#include "al/fraction.h"
#include "al/al.h"
#include "scoreview.h"
#include "key.h"
#include "interval.h"
#include "segment.h"

namespace AL {
      class TempoMap;
      struct TEvent;
      struct SigEvent;
      class TimeSigMap;
      };

class System;
class TextStyle;
class Page;
class PageFormat;
class ElementList;
class Selection;
class Segment;
class Rest;
class Xml;
class Articulation;
class Note;
class Chord;
class ChordRest;
class Slur;
class Hairpin;
class Undo;
class EditTempo;
class Part;
class MidiFile;
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
struct Channel;
class Tuplet;
class Capella;
class CapVoice;
class TextC;
class Dynamic;
class Measure;
class MeasureBase;
class Staff;
class Part;
class Instrument;
class UndoStack;
class RepeatList;
class MusicXmlCreator;
class TimeSig;
class Clef;
class TextB;
class Beam;
class Lyrics;
class Text;

extern bool showRubberBand;

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
      void clear()               { _first = _last = 0; _size = 0; }
      void add(MeasureBase*);
      void remove(MeasureBase*);
      void change(MeasureBase* o, MeasureBase* n);
      int size() const { return _size; }
      };

//---------------------------------------------------------
//   MidiMapping
//---------------------------------------------------------

struct MidiMapping {
      char port;
      char channel;
      Part* part;
      Channel* articulation;
      };

//---------------------------------------------------------
//   MidiInputEvent
//---------------------------------------------------------

struct MidiInputEvent {
      int pitch;
      bool chord;
      };

//---------------------------------------------------------
//   Position
//---------------------------------------------------------

struct Position {
      Measure* measure;
      int tick;
      int staffIdx;
      int line;
      QPointF pos;
      };

//---------------------------------------------------------
//   ImagePath
//---------------------------------------------------------

class ImagePath {
      QString _path;
      int _references;  // > 0 if image is used in score
      QBuffer _buffer;
      bool _loaded;     // true if buffer contains image data

   public:
      ImagePath(const QString& p);
      void dereference();
      void reference();
      const QString& path() const      { return _path;     }
      QBuffer& buffer()                { return _buffer;   }
      void setLoaded(bool val)         { _loaded = val;    }
      bool loaded() const              { return _loaded;   }
      void setPath(const QString& val) { _path = val;      }
      bool isUsed() const              { return _references > 0;  }
      };

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

class Score : public QObject {
      Q_OBJECT

      double _spatium;
      PageFormat* _pageFormat;
      QPaintDevice* _paintDevice;
      BspTree bspTree;

      //
      // generated objects during layout:
      //
      QList<Page*> _pages;          // pages are build from systems
      QList<System*> _systems;      // measures are akkumulated to systems


      // values used during doLayout:
      int curPage;
      int curSystem;
      bool firstSystem;
      MeasureBase* curMeasure;

      UndoStack* _undo;
      QList<ImagePath*> imagePathList;

      QQueue<MidiInputEvent> midiInputQueue;
      QList<MidiMapping> _midiMapping;

      MeasureBaseList _measures;          // here are the notes
      QList<Element*> _gel;               // global elements: Slur, SLine
      QList<Beam*>    _beams;
      RepeatList* _repeatList;
      AL::TimeSigMap* _sigmap;
      AL::TempoMap* _tempomap;

      InputState _is;

      QList<Excerpt*> _excerpts;

      Style _style;
      QVector<TextStyle*> _textStyles;

      QFileInfo info;
      bool _created;          ///< file is never saved, has generated name
      QString _tmpName;       ///< auto saved with this name if not empty

      // the following variables are reset on startCmd()
      //   modified during cmd processing and used in endCmd() to
      //   determine what to layout and what to repaint:

      bool _needLayout;
      QRectF refresh;
      bool _updateAll;
      Measure* startLayout;   ///< start a relayout at this measure
      bool layoutAll;         ///< do a complete relayout

      Qt::KeyboardModifiers keyState;

      bool _showInvisible;
      bool _showFrames;

      EditTempo* editTempo;

      QList<Part*> _parts;
      QList<Staff*> _staves;

      bool _printing;   ///< True if we are drawing to a printer
      bool _playlistDirty;
      bool _autosaveDirty;
      bool _dirty;      ///< Score data was modified.
      bool _saved;      ///< True if project was already saved; only on first
                        ///< save a backup file will be created, subsequent
                        ///< saves will not overwrite the backup file.
      int _playPos;     ///< sequencer seek position

      double _swingRatio; ///< Swing ratio

      bool _foundPlayPosAfterRepeats; ///< Temporary used during playback rendering
                                      ///< indicating if playPos after expanded repeats
                                      ///< has been calculated.

      int _fileDivision; ///< division of current loading *.msc file
      int _mscVersion;   ///< version of current loading *.msc file

      QString _movementNumber;       // some meta data; used for musicXML
      QString _movementTitle;
      QString _workNumber;
      QString _workTitle;
      QString _source;
      QString _rights;
      QList<MusicXmlCreator*> _creators;
      bool _creditsRead;             ///< credits were read at MusicXML import
      bool _defaultsRead;            ///< defaults were read at MusicXML import, allow export of defaults in convertermode

      Selection _selection;
      QList<KeySig*> customKeysigs;

      //------------------

      ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false);
      ChordRest* prevMeasure(ChordRest* element);
      void cmdSetBeamMode(int);
      void cmdFlip();
      Note* getSelectedNote();
      Note* upAlt(Element*);
      Note* upAltCtrl(Note*) const;
      Note* downAlt(Element*);
      Note* downAltCtrl(Note*) const;
      ChordRest* upStaff(ChordRest* cr);
      ChordRest* downStaff(ChordRest* cr);
      void moveUp(Chord*);
      void moveDown(Chord*);

      void convertMidi(MidiFile*);
      void convertCapella(Capella* cap);
      int readCapVoice(CapVoice* cvoice, int staffIdx, int tick);
      void convertTrack(MidiTrack*);
      void convertTrack(BBTrack*, int);

      void moveInputPos(Segment* s);
      void moveToNextInputPos();

      void collectNote(EventMap* events, int channel, Note* note, int onTime, int len);
      void collectChord(EventMap*, Instrument*, Chord*, int tick, int gateTime);
      void collectMeasureEvents(EventMap*, Measure*, int staffIdx, int tickOffset);

      void padToggle(int n);

      void cmdAddPitch(int note, bool addFlag);

      void printFile();
      void addTempo();
      void addMetronome();

     

      void cmdAddChordName2();
      int processPendingNotes(QList<MNote*>* notes, int, int);
      void writeExcerpt(Excerpt*, Xml&);
      void cmdResetBeamMode();
      void connectSlurs();
      void checkSlurs();
      void checkTuplets();
      void tupletDialog();

      void cmdInsertClef(int type);
      void cmdExchangeVoice(int, int);

      void updateSelectedElements();
      void removeChordRest(ChordRest* cr, bool clearSegment);
      void cmdMove(Element* e, QPointF delta);

      void cmdHalfDuration();
      void cmdDoubleDuration();

      void resetUserStretch();
      void toDefault();

      Page* addPage();
      bool layoutPage();
      bool layoutSystem1(double& minWidth, double w, bool);
      QList<System*> layoutSystemRow(qreal x, qreal y, qreal w, bool, double*);
      void processSystemHeader(Measure* m, bool);
      System* getNextSystem(bool, bool);
      void getCurPage();
      bool doReLayout();
      void rebuildBspTree();
      Measure* skipEmptyMeasures(Measure*, System*);

      void layoutStage1();
      void layoutStage2();
      void layoutStage3();
      void transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, int semitones);
      void reLayout(Measure*);

   signals:
      void selectionChanged(int);
      void dirtyChanged(Score*);
      void posChanged(int);
      void updateAll();
      void dataChanged(const QRectF&);
      void layoutChanged();

   public slots:
      void setClean(bool val);
      void setDirty(bool val = true) { setClean(!val); }

   public:
      int curTick;                  // for read optimizations
      int curTrack;

      TextC* rights;                ///< Copyright printed at bottom of page
      int _pageOffset;              ///< Offset for page numbers.

      void cmdAppendMeasures(int);
      void cmdInsertMeasures(int);
      bool noStaves() const         { return _staves.empty(); }
      void insertPart(Part*, int);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void cmdRemoveStaff(int staffIdx);
      void removeStaff(Staff*);
      void addMeasure(MeasureBase*);
      void appendMeasures(int, int);
      void readStaff(QDomElement);

      void undoFixTicks();

      void cmdInsertPart(Part*, int);
      void cmdRemovePart(Part*);
      void cmdAddTie();
      void cmdAddHairpin(bool);
      void cmdAddStretch(double);
      void transpose();
      void transpose(Note* n, Interval, bool useSharpsFlats);

      Score(const Style&);
      ~Score();

      Score* clone();

      void write(Xml&, bool autosave);
      bool read(QDomElement);

      QList<Staff*>& staves()                { return _staves; }
      Q_INVOKABLE int nstaves() const        { return _staves.size(); }

      int staffIdx(const Part*) const;
      int staffIdx(const Staff* staff) const { return _staves.indexOf((Staff*)staff, 0); }
      Staff* staff(int n) const              { return _staves.value(n); }

      Part* part(int staffIdx);

      MeasureBase* pos2measure(const QPointF&, int* tick, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;
      Measure* pos2measure2(const QPointF&, int* tick, int* staffIdx, int* pitch, Segment**) const;
      Measure* pos2measure3(const QPointF& p, int* tick) const;

      void undoChangeSig(int tick, const AL::SigEvent& o, const AL::SigEvent& n);
      void undoChangeKey(Staff* staff, int tick, KeySigEvent o, KeySigEvent n);
      void undoChangeTempo(int tick, const AL::TEvent& o, const AL::TEvent& n);
      void undoChangeClef(Staff* staff, int tick, int o, int n);
      void undoAddElement(Element* element);
      void undoRemoveElement(Element* element);
      void undoChangeMeasureLen(Measure* m, int oldTicks, int newTicks);
      void undoChangeElement(Element* oldElement, Element* newElement);
      void undoInsertTime(int tick, int len);
      void undoSigInsertTime(int, int);
      void undoChangeRepeatFlags(Measure*, int);
      void undoChangeVoltaEnding(Volta* volta, const QList<int>& l);
      void undoChangeVoltaText(Volta* volta, const QString& s);
      void undoChangeChordRestSize(ChordRest* cr, bool small);
      void undoChangeChordNoStem(Chord* cr, bool noStem);
      void undoChangeChordRestSpace(ChordRest* cr, Spatium l, Spatium t);
      void undoChangeSubtype(Element* element, int st);
      void undoChangePitch(Note* note, int pitch, int tpc, int userAccidental);
      void spellNotelist(QList<Note*>& notes);
      void undoChangeTpc(Note* note, int tpc);
      void undoChangeBeamMode(ChordRest* cr, BeamMode mode);
      void undoChangeChordRestLen(ChordRest* cr, const Duration&);
      void undoChangeEndBarLineType(Measure*, int);
      void undoChangeBarLineSpan(Staff*, int);
      void undoChangeUserOffset(Element* e, const QPointF& offset);
      void undoChangeDynamic(Dynamic* e, int velocity, int type);
      void undoChangeCopyright(const QString&);
      void undoTransposeHarmony(Harmony*, int, int);
      void undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2);
      void undoRemovePart(Part* part, int idx);
      void undoInsertPart(Part* part, int idx);
      void undoRemoveStaff(Staff* staff, int idx);
      void undoInsertStaff(Staff* staff, int idx);
      void undoInsertMeasure(MeasureBase*);
      void undoToggleInvisible(Element*);
      void undoMove(Element* e, const QPointF& pt);
      void undoChangeBracketSpan(Staff* staff, int column, int span);
      void undoChangeTuning(Note*, double);
      void undoChangePageFormat(PageFormat*, double spatium);
      void undoChangeUserMirror(Note*, DirectionH);

      void setGraceNote(Chord*,  int pitch, NoteType type, int len);
      int clefOffset(int tick, Staff*) const;

      Segment* setNoteRest(ChordRest*, int track, int pitch, Fraction, int headGroup = 0,
         Direction stemDirection = AUTO);
      void changeCRlen(ChordRest* cr, const Duration&);

      Fraction makeGap(ChordRest*, const Fraction&, Tuplet*);
      bool makeGap1(int tick, int staffIdx, Fraction len);

      Rest* addRest(int tick, int track, Duration, Tuplet*);
      Rest* addRest(Segment* seg, int track, Duration d, Tuplet*);
      Chord* addChord(int tick, Duration d, Chord* oc, bool genTie, Tuplet* tuplet);

      ChordRest* addClone(ChordRest* cr, int tick, const Duration& d);
      Rest* setRest(int tick,  int track, Fraction, bool useDots, Tuplet* tuplet);

      void select(Element* obj, SelectType = SELECT_SINGLE, int staff = 0);
      void deselect(Element* obj);
      void deselectAll() { _selection.deselectAll(); }
      void updateSelection() { _selection.update(); }

      void searchSelectedElements();

      void upDown(bool up, bool octave);
      Element* searchNote(int tick, int track) const;

      // undo/redo ops
      void endUndoRedo(Undo*);
      void addArticulation(int);
      void changeAccidental(int);
      void changeAccidental(Note* oNote, int prefix);

      void addElement(Element*);
      void removeElement(Element*);

      void addTimeSig(int tick, int keySigSubtype);

      void cmdAdd1(Element* e, const QPointF& pos, const QPointF& dragOffset);
      void cmdAddBSymbol(BSymbol*, const QPointF&, const QPointF&);

      Note* addNote(Chord*, int pitch);

      void deleteItem(Element*);
      void cmdDeleteSelectedMeasures();
      void cmdDeleteSelection();
      void toggleInvisible(Element* obj);

      void putNote(const QPointF& pos, bool replace);
      void setPadState();
      void setPadState(Element* obj);

      void startCmd();        // start undoable command
      void endCmd();          // end undoable command
      void end();             // layout & update canvas

      void cmdAdd(Element*);
      void cmdRemove(Element*);
      void cmdRemoveClef(Clef*);
      void cmdRemoveKeySig(KeySig*);
      void cmdRemoveTimeSig(TimeSig*);

      void setUpdateAll(bool v = true) { _updateAll = v;   }
      void setLayoutAll(bool val)      { layoutAll = val;  }
      void addRefresh(const QRectF& r) { refresh |= r;     }

      void changeLineSegment(bool);

      void changeVoice(int);

      void colorItem(Element*);
      const QList<Part*>* parts() const  { return &_parts; }
      void appendPart(Part* p);
      void updateStaffIndex();
      void sortStaves(QList<int>& dst);
      bool read(QString name);

      void setSpatium(double v);

      bool showInvisible() const { return _showInvisible; }
      bool showFrames() const { return _showFrames; }
      void setShowInvisible(bool v);
      void setShowFrames(bool v);

      bool loadMsc(QString name);
      bool loadCompressedMsc(QString name);
      void importMusicXml(const QString&);
      void importCompressedMusicXml(const QString&);
      bool importMidi(const QString& name);
      bool importMuseData(const QString& name);
      bool importLilypond(const QString& name);
      bool importBB(const QString& name);
      bool importCapella(const QString& name);
      bool importOve(const QString& name);
      void saveFile(QFileInfo& info, bool autosave);
      void saveFile(QIODevice* f, bool autosave);
      void saveCompressedFile(QFileInfo&, bool autosave);
      void saveCompressedFile(QIODevice*, QFileInfo&, bool autosave);
      bool saveAs(bool saveCopy = false);
      bool saveAs(bool saveCopy, const QString& name, const QString& ext);

      void print(QPrinter* printer);
      bool saveXml(const QString& name);
      bool saveMxl(const QString& name);
      bool saveMidi(const QString& name);
      bool savePsPdf(const QString& saveName, QPrinter::OutputFormat format);
      bool savePng(const QString& name);
      bool savePng(const QString& name, bool screenshot, bool transparent, double convDpi, QImage::Format format);
      bool saveSvg(const QString& name);
      bool saveLilypond(const QString& name);
#ifdef HAS_AUDIOFILE
      bool saveAudio(const QString& name, const QString& ext, QString soundFont = QString());
#endif
      ChordRest* getSelectedChordRest() const;
      void getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const;

      Element* getSelectedElement() const   { return _selection.element(); }
      const Selection& selection() const    { return _selection; }
      void setSelection(const Selection& s);

      int pos();
      Measure* tick2measure(int tick) const;
      MeasureBase* tick2measureBase(int tick) const;
      Segment* tick2segment(int tick, bool first = false, SegmentTypes st = SegAll) const;
      void fixTicks();
      void addArticulation(Element*, Articulation* atr);

      bool playlistDirty();
      void setPlaylistDirty(bool val) { _playlistDirty = val; }
      void changeTimeSig(int tick, int st);

      void cmd(const QAction*);
      int fileDivision(int t) const { return (t * AL::division + _fileDivision/2) / _fileDivision; }
      bool saveFile(bool autosave);

      QString filePath() const       { return info.filePath(); }
      QFileInfo* fileInfo()          { return &info; }

      QString name() const           { return info.completeBaseName(); }
      void setName(const QString& s) { info.setFile(s); }

      bool isSavable() const;
      bool dirty() const             { return _dirty;         }
      void setCreated(bool val)      { _created = val;        }
      bool created() const           { return _created;       }
      bool saved() const             { return _saved;         }
      void setSaved(bool v)          { _saved = v;            }
      bool printing() const          { return _printing;      }
      void setAutosaveDirty(bool v)  { _autosaveDirty = v;    }
      bool autosaveDirty() const     { return _autosaveDirty; }

      bool pos2TickAnchor(const QPointF&, int staffIdx, int* tick, QPointF* anchor) const;
      void spell();
      void spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment);
      void spell(Note*);
      int nextSeg(int tick, int track);
      int nextSeg1(int tick, int& track);
      int prevSeg1(int tick, int& track);

      Style& style()                           { return _style;                   }
      const Style& style() const               { return _style;                   }
      void setStyle(const Style& s)            { _style = s;                      }
      StyleVal style(StyleIdx idx) const;
      Spatium styleS(StyleIdx idx) const;
      bool styleB(StyleIdx idx) const;
      double styleD(StyleIdx idx) const;
      int styleI(StyleIdx idx) const;
      void setStyle(StyleIdx idx, const StyleVal& v);

      void insertTime(int tick, int len);
      void cmdRemoveTime(int tick, int len);
      int playPos() const                      { return _playPos;    }
      void setPlayPos(int val)                 { _playPos = val;     }

      bool noteEntryMode() const               { return _is.noteEntryMode; }
      int inputPos() const;
      int inputTrack() const                   { return _is.track;   }
      InputState& inputState()                 { return _is;         }
      void setInputState(const InputState& st) { _is = st;           }
      void setInputTrack(int);

      TextStyle* textStyle(int idx) { return idx < 0 ? 0 : _textStyles[idx]; }
      const QVector<TextStyle*>& textStyles() const { return _textStyles; }
      void setTextStyles(const QVector<TextStyle*>&s);
      QVector<TextStyle*> swapTextStyles(QVector<TextStyle*> s);
      bool loadStyle(QFile* qf);
      void loadStyle();
      void saveStyle();
      void spatiumChanged(double oldValue, double newValue);

      void pasteStaff(QDomElement, ChordRest* dst);
      Volta* searchVolta(int tick) const;
      void toEList(EventMap* events);
      void toEList(EventMap* events, int staffIdx);
      int mscVersion() const    { return _mscVersion; }
      void setMscVersion(int v) { _mscVersion = v; }

      AL::TimeSigMap* sigmap()  { return _sigmap; }
      MeasureBase* appendMeasure(int type);
      void addLyrics(int tick, int staffIdx, const QString&);

      QList<Excerpt*>* excerpts()  { return &_excerpts; }
      Score* createExcerpt(Excerpt*);
      MeasureBaseList* measures()  { return &_measures; }

      bool checkHasMeasures() const;

      QList<Element*>* gel()                  { return &_gel; }
      const QList<Element*>* gel() const      { return &_gel; }
      void setLayout(Measure* m);
      int midiPort(int idx) const;
      int midiChannel(int idx) const;
      QList<MidiMapping>* midiMapping()       { return &_midiMapping; }
      void rebuildMidiMapping();
      void updateChannel();
      void cmdTransposeStaff(int staffIdx, Interval, bool useDoubleSharpsFlats);
      void cmdConcertPitchChanged(bool, bool useSharpsFlats);
      AL::TempoMap* tempomap() const { return _tempomap; }

      double swingRatio()                            { return _swingRatio;}
      void setSwingRatio(double d)                   { _swingRatio = d;}

      QString movementNumber() const                 { return _movementNumber;  }
      QString movementTitle() const                  { return _movementTitle;   }
      QString workNumber() const                     { return _workNumber;      }
      QString workTitle() const                      { return _workTitle;       }
      QString source() const                         { return _source;          }
      QString mxmlRights() const                     { return _rights;          }
      bool creditsRead() const                       { return _creditsRead;     }
      bool defaultsRead() const                      { return _defaultsRead;    }
      void setMovementNumber(const QString& s)       { _movementNumber = s;     }
      void setMovementTitle(const QString& s)        { _movementTitle = s;      }
      void setWorkNumber(const QString& s)           { _workNumber = s;         }
      void setWorkTitle(const QString& s)            { _workTitle = s;          }
      void setSource(const QString& s)               { _source = s;             }
      void setmxmlRights(const QString& s)           { _rights = s;             }
      void setCreditsRead(bool b)                    { _creditsRead = b;        }
      void setDefaultsRead(bool b)                   { _defaultsRead = b;       }
      void addCreator(MusicXmlCreator* c)            { _creators.append(c);     }
      const MusicXmlCreator* getCreator(int i) const { return _creators.at(i);  }
      int numberOfCreators() const                   { return _creators.size(); }
      Text* getText(int subtype);

      void lassoSelect(const QRectF&);
      void lassoSelectEnd();

      Page* searchPage(const QPointF&) const;
      bool getPosition(Position* pos, const QPointF&, int voice) const;

      void cmdDeleteTuplet(Tuplet*, bool replaceWithRest);

      ImagePath* addImage(const QString&);      // add image to imagePathList
      void moveBracket(int staffIdx, int srcCol, int dstCol);
      Measure* getCreateMeasure(int tick);

      void adjustBracketsDel(int sidx, int eidx);
      void adjustBracketsIns(int sidx, int eidx);
      void renumberMeasures();
      UndoStack* undo() const { return _undo; }
      TextC* copyright() { return rights; }
      void setCopyright(const QString& s);
      void setCopyrightHtml(const QString& s);
      void endUndoRedo();
      Measure* searchLabel(const QString& s, Measure* start = 0);
      RepeatList* repeatList() { return _repeatList; }
      double utick2utime(int tick) const;
      int utime2utick(double utime);
      void updateRepeatList(bool expandRepeats);
      void fixPpitch();

      void nextInputPos(ChordRest* cr, bool);
      void cmdMirrorNoteHead();

      void layout()                           { _needLayout = true; }
      double spatium() const                  { return _spatium; }
      PageFormat* pageFormat() const          { return _pageFormat; }
      void setPageFormat(const PageFormat& pf);
      const QList<Page*>& pages() const       { return _pages; }
      QList<System*>* systems()               { return &_systems; }
      bool needLayout() const                 { return _needLayout; }

      MeasureBase* first() const;
      MeasureBase* last()  const;
      Measure* firstMeasure() const;
      Measure* lastMeasure() const;
      Segment* firstSegment() const;
      Segment* lastSegment() const;

      void setPaintDevice(QPaintDevice* d)          { _paintDevice = d; }
      QPaintDevice* paintDevice() const             { return _paintDevice; }
      QList<const Element*> items(const QRectF& r)  { return bspTree.items(r); }
      QList<const Element*> items(const QPointF& p) { return bspTree.items(p); }

      void insertBsp(Element* e)                    { bspTree.insert(e); }
      void removeBsp(Element* e)                    { bspTree.remove(e); }

      void setInstrumentNames();
      void connectTies();

      virtual void add(Element*);
      virtual void remove(Element*);
      double point(const Spatium sp) const { return sp.val() * _spatium; }

      void scanElements(void* data, void (*func)(void*, Element*));
      void selectSimilar(Element*, bool);
      void selectElementDialog(Element* e);
      QByteArray buildCanonical(int track);
      int fileDivision() const { return _fileDivision; } ///< division of current loading *.msc file
      void splitStaff(int staffIdx, int splitPoint);
      QString tmpName() const           { return _tmpName;      }
      void setTmpName(const QString& s) { _tmpName = s;         }
      QList<Beam*> beams() const        { return _beams; }
      QList<Beam*>& beams()             { return _beams; }
      Beam* beam(int id) const;
      void processMidiInput();
      Lyrics* addLyrics();
      void expandVoice();
      Note* addPitch(int pitch, bool addFlag);
      void insertMeasures(int, int);
      int customKeySigIdx(KeySig*) const;
      int addCustomKeySig(KeySig*);
      KeySig* customKeySig(int) const;
      KeySig* keySigFactory(KeySigEvent);
      void cmdPaste(ScoreView*);
      Element* selectMove(const QString& cmd);
      Element* move(const QString& cmd);
      void cmdEnterRest(const Duration& d);
      void cmdAddInterval(int, const QList<Note*>&);
      void cmdCreateTuplet(ChordRest*, Tuplet*);
      void enqueueMidiEvent(MidiInputEvent ev) { midiInputQueue.enqueue(ev); }
      void doLayout();
      void layoutChords1(Segment* segment, int staffIdx);
      };

extern Score* gscore;
extern void fixTicks();

#endif

