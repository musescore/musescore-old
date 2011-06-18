//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "mscore.h"
#include "style.h"
#include "durationtype.h"
#include "select.h"
#include "config.h"
#include "al/fraction.h"
#include "al/al.h"
#include "interval.h"
#include "msynth/sparm.h"
#include "mscoreview.h"

namespace AL {
      class TempoMap;
      struct TEvent;
      class SigEvent;
      class TimeSigMap;
      };

class System;
class TextStyle;
class Page;
struct PageFormat;
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
class KeySigEvent;
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
struct CapVoice;
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
class Beam;
class Lyrics;
class Text;
class Omr;
class Parameter;
class StaffType;
class Revisions;
class Spanner;
class ScoreView;
class LinkedElements;
class Fingering;

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
      void insert(MeasureBase*, MeasureBase*);
      void remove(MeasureBase*, MeasureBase*);
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
      Segment* segment;
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
//   LayoutFlag bits
//---------------------------------------------------------

enum LayoutFlag {
      LAYOUT_FIX_TICKS = 1,
      LAYOUT_FIX_PITCH_VELO = 2
      };

typedef QFlags<LayoutFlag> LayoutFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(LayoutFlags)


//---------------------------------------------------------
//   Layer
//---------------------------------------------------------

struct Layer {
      QString name;
      uint tags;
      };

//---------------------------------------------------------
//   Score
//---------------------------------------------------------

class Score {
      Score* _parentScore;          // set if score is an excerpt (part)
      QList<MuseScoreView*> viewer;

      QDate _creationDate;

      Revisions* _revisions;
      QList<Excerpt*> _excerpts;

      QString _layerTags[32];
      QString _layerTagComments[32];
      QList<Layer> _layer;
      int _currentLayer;

      int _pageNumberOffset;        ///< Offset for page numbers.

      //
      // generated objects during layout:
      //
      int _symIdx;                  // used symbol set, derived from style
      QList<Page*> _pages;          // pages are build from systems
      QList<System*> _systems;      // measures are akkumulated to systems

      // values used during doLayout:
      int curPage;
      int curSystem;
      bool firstSystem;
      bool startWithLongNames;            // long instrument names
      MeasureBase* curMeasure;

      UndoStack* _undo;
      QList<ImagePath*> imagePathList;

      QQueue<MidiInputEvent> midiInputQueue;
      QList<MidiMapping> _midiMapping;

      MeasureBaseList _measures;          // here are the notes

      RepeatList* _repeatList;
      AL::TimeSigMap* _sigmap;
      AL::TempoMap* _tempomap;

      InputState _is;

      Style _style;

      QList<StaffType*> _staffTypes;

      QFileInfo info;
      bool _created;          ///< file is never saved, has generated name
      QString _tmpName;       ///< auto saved with this name if not empty

      // the following variables are reset on startCmd()
      //   modified during cmd processing and used in endCmd() to
      //   determine what to layout and what to repaint:

      QRectF refresh;
      bool _updateAll;
      Measure* startLayout;   ///< start a relayout at this measure
      bool layoutAll;         ///< do a complete relayout
      LayoutFlags layoutFlags;

      Qt::KeyboardModifiers keyState;

      bool _showInvisible;
      bool _showUnprintable;
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
      QHash<int, LinkedElements*> _elinks;

      QMap<QString, QString> _metaTags;

      QList<MusicXmlCreator*> _creators;
      bool _creditsRead;             ///< credits were read at MusicXML import
      bool _defaultsRead;            ///< defaults were read at MusicXML import, allow export of defaults in convertermode

      Selection _selection;
      QList<KeySig*> customKeysigs;
      Omr* _omr;
      bool _showOmr;

      SyntiState _syntiState;

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

      void padToggle(int n);

      void cmdAddPitch(int note, bool addFlag);

      void printFile();
      void addTempo();
      void addMetronome();

      void cmdAddChordName2();
      int processPendingNotes(QList<MNote*>* notes, int, int);
      void cmdResetBeamMode();
      void connectSlurs();

      void cmdInsertClef(ClefType);
      void cmdExchangeVoice(int, int);

      void removeChordRest(ChordRest* cr, bool clearSegment);
      void cmdMoveRest(Rest*, Direction);
      void cmdMoveLyrics(Lyrics*, Direction);

      void cmdHalfDuration();
      void cmdDoubleDuration();

      void resetUserStretch();

      Page* addPage();
      bool layoutPage();
      bool layoutSystem1(double& minWidth, double w, bool, bool);
      QList<System*> layoutSystemRow(qreal x, qreal y, qreal w, bool, bool, double*);
      void processSystemHeader(Measure* m, bool);
      System* getNextSystem(bool, bool);
      void getCurPage();
      bool doReLayout();
      Measure* skipEmptyMeasures(Measure*, System*);

      void layoutStage1();
      void layoutStage2();
      void layoutStage3();
      void transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, int semitones);
      void reLayout(Measure*);

      void checkSlurs();
      void checkTuplets();
      void checkScore();
      bool rewriteMeasures(Measure* fm, Measure* lm, const Fraction&);
      void rewriteMeasures(Measure* fm, const Fraction& ns);
      void updateVelo();
      QString createDefaultFileName();
      void addAudioTrack();
      void parseVersion(const QString&);
      QList<Fraction> splitGapToMeasureBoundaries(ChordRest*, Fraction);
      void pasteChordRest(ChordRest* cr, int tick);
      void adjustReadPos();

//   signals:
//      void selectionChanged(int);
//      void dirtyChanged(Score*);
//      void posChanged(int);
//      void updateAll();
//      void dataChanged(const QRectF&);
//      void layoutChanged();
//      void inputCursorChanged();
//
   public:
      void setClean(bool val);
      void setDirty(bool val = true) { setClean(!val); }

   public:
      // read context
      int curTick;                  // for read optimizations
      int curTrack;
      QList<Slur*> slurs;
      QList<Beam*> beams;

      void rebuildBspTree();
      bool noStaves() const         { return _staves.empty(); }
      void insertPart(Part*, int);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void cmdRemoveStaff(int staffIdx);
      void removeStaff(Staff*);
      void addMeasure(MeasureBase*);
      void readStaff(QDomElement);

      void cmdInsertPart(Part*, int);
      void cmdRemovePart(Part*);
      void cmdAddTie();
      void cmdAddHairpin(bool);
      void cmdAddStretch(double);
      void transpose(Note* n, Interval, bool useSharpsFlats);

      Score(const Style*);
      Score(Score*);                // used for excerpts
      ~Score();

      Score* clone();
      bool appendScore(Score*);

      void write(Xml&, bool autosave);
      bool read(QDomElement);
      bool read1(QDomElement);

      QList<Staff*>& staves()                { return _staves; }
      const QList<Staff*>& staves() const    { return _staves; }
      Q_INVOKABLE int nstaves() const        { return _staves.size(); }

      int staffIdx(const Part*) const;
      int staffIdx(const Staff* staff) const { return _staves.indexOf((Staff*)staff, 0); }
      Staff* staff(int n) const              { return _staves.value(n); }

      Part* part(int staffIdx);

      MeasureBase* pos2measure(const QPointF&, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;

      void undoAddElement(Element* element);
      void undoAddCR(ChordRest* element, Measure*, int tick);
      void undoRemoveElement(Element* element);
      void undoChangeMeasureLen(Measure* m, int oldTicks, int newTicks);
      void undoChangeElement(Element* oldElement, Element* newElement);
      void undoInsertTime(int tick, int len);
      void undoChangeRepeatFlags(Measure*, int);
      void undoChangeVoltaEnding(Volta* volta, const QList<int>& l);
      void undoChangeVoltaText(Volta* volta, const QString& s);
      void undoChangeChordRestSize(ChordRest* cr, bool small);
      void undoChangeChordNoStem(Chord* cr, bool noStem);
      void undoChangeChordRestSpace(ChordRest* cr, Spatium l, Spatium t);
      void undoChangeSubtype(Element* element, int st);
      void undoChangePitch(Note* note, int pitch, int tpc, int line, int fret, int string);
      void spellNotelist(QList<Note*>& notes);
      void undoChangeTpc(Note* note, int tpc);
      void undoChangeBeamMode(ChordRest* cr, BeamMode mode);
      void undoChangeChordRestLen(ChordRest* cr, const Duration&);
      void undoChangeEndBarLineType(Measure*, BarLineType);
      void undoChangeBarLineSpan(Staff*, int);
      void undoChangeUserOffset(Element* e, const QPointF& offset);
      void undoChangeDynamic(Dynamic* e, int velocity, DynamicType type);
      void undoTransposeHarmony(Harmony*, int, int);
      void undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2);
      void undoRemovePart(Part* part, int idx);
      void undoInsertPart(Part* part, int idx);
      void undoRemoveStaff(Staff* staff, int idx);
      void undoInsertStaff(Staff* staff, int idx);
      void undoInsertMeasure(MeasureBase*);
      void undoChangeInvisible(Element*, bool);
      void undoMove(Element* e, const QPointF& pt);
      void undoChangeBracketSpan(Staff* staff, int column, int span);
      void undoChangeTuning(Note*, double);
      void undoChangePageFormat(PageFormat*, double spatium, int);
      void undoChangeUserMirror(Note*, DirectionH);
      void undoChangeKeySig(Staff* ostaff, int tick, KeySigEvent st);
      void undoChangeClef(Staff* ostaff, Segment*, ClefType st);
      void undoChangeBarLine(Measure* m, BarLineType);
      void undoSwapCR(ChordRest* cr1, ChordRest* cr2);

      void setGraceNote(Chord*,  int pitch, NoteType type, int len);
      int clefOffset(int tick, Staff*) const;

      Segment* setNoteRest(Segment*, int track, NoteVal nval, Fraction, Direction stemDirection = AUTO);
      void changeCRlen(ChordRest* cr, const Duration&);

      Fraction makeGap(Segment*, int track, const Fraction&, Tuplet*);
      bool makeGap1(int tick, int staffIdx, Fraction len);

      Rest* addRest(int tick, int track, Duration, Tuplet*);
      Rest* addRest(Segment* seg, int track, Duration d, Tuplet*);
      Chord* addChord(int tick, Duration d, Chord* oc, bool genTie, Tuplet* tuplet);

      ChordRest* addClone(ChordRest* cr, int tick, const Duration& d);
      Rest* setRest(int tick,  int track, Fraction, bool useDots, Tuplet* tuplet);

      void searchSelectedElements();

      void upDown(bool up, UpDownMode);
      ChordRest* searchNote(int tick, int track) const;

      // undo/redo ops
      void endUndoRedo(Undo*);
      void addArticulation(int);
      void changeAccidental(AccidentalType);
      void changeAccidental(Note* oNote, AccidentalType);

      void addElement(Element*);
      void removeElement(Element*);

      void cmdAddSpanner(Spanner* e, const QPointF& pos, const QPointF& dragOffset);
      void cmdAddBSymbol(BSymbol*, const QPointF&, const QPointF&);

      Note* addNote(Chord*, int pitch);

      void deleteItem(Element*);
      void cmdDeleteSelectedMeasures();
      void cmdDeleteSelection();

      void putNote(const QPointF& pos, bool replace);
      void setInputState(Element* obj);

      void startCmd();        // start undoable command
      void endCmd();          // end undoable command
      void end();             // layout & update canvas
      void end1();

      void cmdRemoveTimeSig(TimeSig*);
      void cmdAddTimeSig(Measure*, int staffIdx, TimeSig*);

      void setUpdateAll(bool v = true) { _updateAll = v;   }
      void setLayoutAll(bool val);
      void addRefresh(const QRectF& r) { refresh |= r;     }

      void changeVoice(int);

      void colorItem(Element*);
      const QList<Part*>* parts() const  { return &_parts; }
      Part* part(int n) const            { return _parts[n]; }
      void appendPart(Part* p);
      void updateStaffIndex();
      void sortStaves(QList<int>& dst);
      int readScore(QString name);

      bool showInvisible() const   { return _showInvisible; }
      bool showUnprintable() const { return _showUnprintable; }
      bool showFrames() const      { return _showFrames; }
      void setShowInvisible(bool v);
      void setShowUnprintable(bool v);
      void setShowFrames(bool v);

      bool loadMsc(QString name);
      bool loadCompressedMsc(QString name);
      bool importMusicXml(const QString&);
      bool importCompressedMusicXml(const QString&);
      bool importMidi(const QString& name);
      bool importMuseData(const QString& name);
      bool importLilypond(const QString& name);
      bool importBB(const QString& name);
      bool importCapella(const QString& name);
      bool importOve(const QString& name);
      bool importPdf(const QString& name);
      bool importBww(const QString& name);
      bool importGTP(const QString& name);

      void saveFile(QFileInfo& info, bool autosave);
      void saveFile(QIODevice* f, bool msczFormat, bool autosave);
      void saveCompressedFile(QFileInfo&, bool autosave);
      void saveCompressedFile(QIODevice*, QFileInfo&, bool autosave);
      bool saveAs(bool saveCopy = false);
      bool saveAs(bool saveCopy, const QString& name, const QString& ext);
      bool exportFile();

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
      bool saveMp3(const QString& name, QString soundFont = QString());
      ChordRest* getSelectedChordRest() const;
      void getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const;

      void select(Element* obj, SelectType = SELECT_SINGLE, int staff = 0);
      void deselect(Element* obj);
      void deselectAll()                    { _selection.deselectAll(); }
      void updateSelection()                { _selection.update(); }
      Element* getSelectedElement() const   { return _selection.element(); }
      const Selection& selection() const    { return _selection; }
      Selection& selection()                { return _selection; }
      void setSelection(const Selection& s);

      int pos();
      Measure* tick2measure(int tick) const;
      MeasureBase* tick2measureBase(int tick) const;
      Segment* tick2segment(int tick, bool first = false, SegmentTypes st = SegAll) const;
      Segment* tick2segmentEnd(int track, int tick) const;
      void fixTicks();
      void addArticulation(Element*, Articulation* atr);

      bool playlistDirty();
      void setPlaylistDirty(bool val) { _playlistDirty = val; }

      void cmd(const QAction*);
      int fileDivision(int t) const { return (t * AL::division + _fileDivision/2) / _fileDivision; }
      bool saveFile(bool autosave);

      QString filePath() const       { return info.filePath(); }
      QString absoluteFilePath() const { return info.absoluteFilePath(); }
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
      void setPrinting(bool val)     { _printing = val;      }
      void setAutosaveDirty(bool v)  { _autosaveDirty = v;    }
      bool autosaveDirty() const     { return _autosaveDirty; }

      void spell();
      void spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment);
      void spell(Note*);
      int nextSeg(int tick, int track);

      Style* style()                           { return &_style;                  }
      const Style* style() const               { return &_style;                  }
      void setStyle(const Style& s)            { _style = s;                      }
      void loadStyle();
      void saveStyle();

      StyleVal style(StyleIdx idx) const       { return _style.value(idx);   }
      Spatium styleS(StyleIdx idx) const       { return _style.valueS(idx);  }
      qreal   styleP(StyleIdx idx) const       { return _style.valueS(idx).val() * spatium();  }
      QString styleSt(StyleIdx idx) const      { return _style.valueSt(idx); }
      bool    styleB(StyleIdx idx) const       { return _style.valueB(idx);  }
      double  styleD(StyleIdx idx) const       { return _style.valueD(idx);  }
      int     styleI(StyleIdx idx) const       { return _style.valueI(idx);  }
      const TextStyle& textStyle(TextStyleType idx) const { return _style.textStyle(idx); }

      void insertTime(int tick, int len);
      void cmdRemoveTime(int tick, int len);
      int playPos() const                      { return _playPos;    }
      void setPlayPos(int val)                 { _playPos = val;     }

      bool noteEntryMode() const               { return _is.noteEntryMode; }
      int inputPos() const;
      int inputTrack() const                   { return _is.track(); }
      InputState& inputState()                 { return _is;         }
      void setInputState(const InputState& st) { _is = st;           }
      void setInputTrack(int);

      void spatiumChanged(double oldValue, double newValue);

      void pasteStaff(QDomElement, ChordRest* dst);
      void toEList(EventMap* events);
      void renderPart(EventMap* events, Part*);
      int mscVersion() const    { return _mscVersion; }
      void setMscVersion(int v) { _mscVersion = v; }

      void addLyrics(int tick, int staffIdx, const QString&);

      QList<Excerpt*>* excerpts()  { return &_excerpts; }
      MeasureBaseList* measures()  { return &_measures; }

      bool checkHasMeasures() const;

      void setLayout(Measure* m);

      int midiPort(int idx) const;
      int midiChannel(int idx) const;
      QList<MidiMapping>* midiMapping()       { return &_midiMapping;          }
      MidiMapping* midiMapping(int channel)   { return &_midiMapping[channel]; }
      void rebuildMidiMapping();
      void updateChannel();

      void cmdTransposeStaff(int staffIdx, Interval, bool useDoubleSharpsFlats);
      void cmdConcertPitchChanged(bool, bool useSharpsFlats);

      AL::TempoMap* tempomap() const;
      AL::TimeSigMap* sigmap() const;

      double swingRatio()                            { return _swingRatio;}
      void setSwingRatio(double d)                   { _swingRatio = d;}

      bool creditsRead() const                       { return _creditsRead;     }
      void setCreditsRead(bool val)                  { _creditsRead = val;      }
      bool defaultsRead() const                      { return _defaultsRead;    }
      void setDefaultsRead(bool b)                   { _defaultsRead = b;       }
      void addCreator(MusicXmlCreator* c)            { _creators.append(c);     }
      const MusicXmlCreator* getCreator(int i) const { return _creators.at(i);  }
      int numberOfCreators() const                   { return _creators.size(); }
      Text* getText(int subtype);

      void lassoSelect(const QRectF&);
      void lassoSelectEnd();

      Page* searchPage(const QPointF&) const;
      QList<System*> searchSystem(const QPointF& p) const;
      Measure* searchMeasure(const QPointF& p) const;

      bool getPosition(Position* pos, const QPointF&, int voice) const;

      void cmdDeleteTuplet(Tuplet*, bool replaceWithRest);

      ImagePath* addImage(const QString&);      // add image to imagePathList
      void moveBracket(int staffIdx, int srcCol, int dstCol);
      Measure* getCreateMeasure(int tick);

      void adjustBracketsDel(int sidx, int eidx);
      void adjustBracketsIns(int sidx, int eidx);
      void renumberMeasures();
      UndoStack* undo() const;

      void endUndoRedo();
      Measure* searchLabel(const QString& s);
      RepeatList* repeatList() const;
      double utick2utime(int tick) const;
      int utime2utick(double utime) const;
      void updateRepeatList(bool expandRepeats);

      void nextInputPos(ChordRest* cr, bool);
      void cmdMirrorNoteHead();

      double spatium() const                   { return style()->spatium();    }
      void setSpatium(double v)                { style()->setSpatium(v);       }
      PageFormat* pageFormat() const           { return style()->pageFormat(); }
      void setPageFormat(const PageFormat& pf) { style()->setPageFormat(pf);   }
      const QList<Page*>& pages() const        { return _pages;                }
      QList<System*>* systems()                { return &_systems;             }

      MeasureBase* first() const;
      MeasureBase* last()  const;
      Measure* firstMeasure() const;
      Measure* lastMeasure() const;
      Segment* firstSegment(SegmentTypes s = SegAll) const;
      Segment* lastSegment() const;

      void connectTies();

      virtual void add(Element*);
      virtual void remove(Element*);
      double point(const Spatium sp) const { return sp.val() * spatium(); }

      void scanElements(void* data, void (*func)(void*, Element*));
      QByteArray buildCanonical(int track);
      int fileDivision() const { return _fileDivision; } ///< division of current loading *.msc file
      void splitStaff(int staffIdx, int splitPoint);
      QString tmpName() const           { return _tmpName;      }
      void setTmpName(const QString& s) { _tmpName = s;         }
      void processMidiInput();
      Lyrics* addLyrics();
      void expandVoice(Segment* s, int track);
      void expandVoice();
      Note* addPitch(int pitch, bool addFlag);

      int customKeySigIdx(KeySig*) const;
      int addCustomKeySig(KeySig*);
      KeySig* customKeySig(int) const;
      KeySig* keySigFactory(const KeySigEvent&);
      void cmdPaste(ScoreView*);
      Element* selectMove(const QString& cmd);
      Element* move(const QString& cmd);
      void cmdEnterRest(const Duration& d);
      void cmdAddInterval(int, const QList<Note*>&);
      void cmdCreateTuplet(ChordRest*, Tuplet*);
      Omr* omr() const                         { return _omr;     }
      void setOmr(Omr* o)                      { _omr = o;        }
      void removeOmr();
      bool showOmr() const                     { return _showOmr; }
      void setShowOmr(bool v)                  { _showOmr = v;    }
      void enqueueMidiEvent(MidiInputEvent ev) { midiInputQueue.enqueue(ev); }
      void doLayout();
      void layoutChords1(Segment* segment, int staffIdx);
//      void emitSelectionChanged(int val)                 { emit selectionChanged(val); }
      SyntiState& syntiState()                           { return _syntiState;         }
      void setSyntiState();

      const QList<StaffType*>& staffTypes() const;
      QList<StaffType*>& staffTypes();
      void addStaffTypes(const QList<StaffType*>& tl);
      void replaceStaffTypes(const QList<StaffType*>&);

      void addLayoutFlags(LayoutFlags val)               { layoutFlags |= val; }
      int symIdx() const                                 { return _symIdx; }
      void addImage(Element*);
      void updateHairpin(Hairpin*);       // add/modify hairpin to pitchOffset list
      void removeHairpin(Hairpin*);       // remove hairpin from pitchOffset list
      Volta* searchVolta(int tick) const;
      Score* parentScore() const    { return _parentScore; }
      void setParentScore(Score* s) { _parentScore = s;    }
      const Score* rootScore() const;
      Score* rootScore();
      void addExcerpt(Score*);
      void removeExcerpt(Score*);
      void createRevision();
      QByteArray readCompressedToBuffer();
      QByteArray readToBuffer();
      void writeSegments(Xml& xml, const Measure*, int strack, int etrack, Segment* first, Segment* last, bool);
      Spanner* findSpanner(int id) const;

      const QMap<QString, QString> metaTags() const           { return _metaTags; }
      QMap<QString, QString> metaTags()                       { return _metaTags; }
      QString metaTag(const QString& s) const                 { return _metaTags.value(s);}
      void setMetaTag(const QString& tag, const QString& val) { _metaTags.insert(tag, val); }
      void updateNotes();
      void cmdUpdateNotes();
      void updateAccidentals(Measure* m, int staffIdx);
      QHash<int, LinkedElements*>& links();
      void appendMeasures(int, ElementType);
      MeasureBase* appendMeasure(ElementType type);
      bool concertPitch() const { return styleB(ST_concertPitch); }
      void layoutFingering(Fingering*);
      void cmdSplitMeasure();
      void cmdJoinMeasure();
      void timesigStretchChanged(TimeSig* ts, Measure* fm, int staffIdx);
      int pageNumberOffset() const          { return _pageNumberOffset; }
      void setPageNumberOffset(int v)       { _pageNumberOffset = v; }
      const QDate& creationDate() const     { return _creationDate;  }
      uint currentLayerMask() const         { return _layer[_currentLayer].tags; }
      void setCurrentLayer(int val)         { _currentLayer = val;  }
      int currentLayer() const              { return _currentLayer; }
      QString* layerTags()                  { return _layerTags;    }
      QString* layerTagComments()           { return _layerTagComments;    }
      QList<Layer>* layer()                 { return &_layer;       }
      bool tagIsValid(uint tag) const       { return tag & _layer[_currentLayer].tags; }
      void transpose(int mode, TransposeDirection, int transposeKey, int transposeInterval,
         bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats);
      void addViewer(MuseScoreView* v)      { viewer.append(v);   }
      void removeViewer(MuseScoreView* v)   { viewer.removeAll(v); }
      };

extern Score* gscore;
extern void fixTicks();

#endif

