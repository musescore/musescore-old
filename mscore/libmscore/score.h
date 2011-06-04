//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#ifndef __SCORE_P_H__
#define __SCORE_P_H__

/**
 \file
 Definition of Score class.
*/

#include <QtCore/QBuffer>
#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QFileInfo>

#include "globals.h"
#include "style.h"
#include "durationtype.h"
#include "select.h"
#include "al/fraction.h"
#include "painter.h"
#include "msynth/sparm.h"

namespace AL {
      class TempoMap;
      struct TEvent;
      };

class TimeSigMap;
struct SigEvent;
class Element;
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
class KeySigEvent;
class Volta;
class BBTrack;
class MidiEvent;
struct MNote;
class EventMap;
class Harmony;
struct Channel;
class Tuplet;
class Capella;
class CapVoice;
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
class Spanner;
class LinkedElements;
class Cursor;
class ScoreProxy;
class XmlReader;

extern int division;

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
//   Score
//---------------------------------------------------------

class Score {
      QFileInfo info;
      Score* _parentScore;

   protected:
      qreal _spatium;
      PageFormat* _pageFormat;

      // values used during doLayout:
      int curPage;
      int curSystem;
      bool firstSystem;
      int _symIdx;
      MeasureBase* curMeasure;

      QList<ImagePath*> imagePathList;

      QList<MidiMapping> _midiMapping;

      MeasureBaseList _measures;          // here are the notes
      QList<Beam*>    _beams;
      QList<Page*>    _pages;
      QList<System*>  _systems;

      RepeatList* _repeatList;
      TimeSigMap* _sigmap;
      AL::TempoMap* _tempomap;

      MStyle _style;

      QList<StaffType*> _staffTypes;

      // the following variables are reset on startCmd()
      //   modified during cmd processing and used in endCmd() to
      //   determine what to layout and what to repaint:

      QRectF refresh;
      bool _updateAll;

      bool _showInvisible;
      bool _showFrames;

      QList<Part*> _parts;
      QList<Staff*> _staves;

      bool _printing;         ///< True if we are drawing to a printer

      int _fileDivision;      ///< division of current loading *.msc file
      int _mscVersion;        ///< version of current loading *.msc file
      QHash<int, LinkedElements*> _elinks;

      QMap<QString, QString> _metaTags;

      bool _creditsRead;             ///< credits were read at MusicXML import
      bool _defaultsRead;            ///< defaults were read at MusicXML import, allow export of defaults in convertermode

      Selection _selection;
      QList<KeySig*> customKeysigs;

      int _playPos;
      Cursor* _cursor;
      Segment* cursorSegment;
      SyntiState _syntiState;

      //------------------

      ChordRest* nextMeasure(ChordRest* element, bool selectBehavior = false);
      ChordRest* prevMeasure(ChordRest* element);
      void cmdSetBeamMode(int);
      Note* getSelectedNote();

      void cmdAddPitch(int note, bool addFlag);

      void printFile();
      void addTempo();
      void addMetronome();

      void cmdAddChordName2();
      int processPendingNotes(QList<MNote*>* notes, int, int);
      void cmdResetBeamMode();
      void tupletDialog();

      void cmdInsertClef(ClefType);
      void cmdMoveRest(Rest*, Direction);
      void cmdMoveLyrics(Lyrics*, Direction);

      void resetUserStretch();

      Page* addPage();
      bool layoutPage();
      bool layoutSystem1(qreal& minWidth, qreal w, bool);
      QList<System*> layoutSystemRow(qreal x, qreal y, qreal w, bool, qreal*);
      void processSystemHeader(Measure* m, bool);
      System* getNextSystem(bool, bool);
      void getCurPage();
      Measure* skipEmptyMeasures(Measure*, System*);

      void layoutStage1();
      void layoutStage2();
      void layoutStage3();
      void transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, int semitones);

      void checkSlurs();
      void checkTuplets();
      void checkScore();
      void updateVelo();
      QString createDefaultFileName();

      void rebuildBspTree();
      bool noStaves() const         { return _staves.empty(); }
      void insertPart(Part*, int);
      void removePart(Part*);
      void insertStaff(Staff*, int);
      void cmdRemoveStaff(int staffIdx);
      void removeStaff(Staff*);
      void readStaff(XmlReader*);

      void cmdInsertPart(Part*, int);
      void cmdRemovePart(Part*);
      void cmdAddStretch(qreal);
      bool read(XmlReader*);
      bool read1(XmlReader*);

      MeasureBase* pos2measure(const QPointF&, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;

      void setGraceNote(Chord*,  int pitch, NoteType type, int len);
      int clefOffset(int tick, Staff*) const;

      Segment* setNoteRest(ChordRest*, int track, NoteVal nval, Fraction, Direction stemDirection = AUTO);
      void changeCRlen(ChordRest* cr, const TimeDuration&);

      Fraction makeGap(ChordRest*, const Fraction&, Tuplet*);
      bool makeGap1(int tick, int staffIdx, Fraction len);

      void select(Element* obj, SelectType = SELECT_SINGLE, int staff = 0);
      void deselectAll()     { _selection.deselectAll(); }
      void updateSelection() { _selection.update(); }

      void upDown(bool up, UpDownMode);

      void addArticulation(int);
      void changeAccidental(AccidentalType);
      void changeAccidental(Note* oNote, AccidentalType);

      void addElement(Element*);
      void toggleInvisible(Element* obj);

      void end();             // layout & update canvas
      void end1();

      void setUpdateAll(bool v = true) { _updateAll = v;   }
      void setLayoutAll(bool val);
      void addRefresh(const QRectF& r) { refresh |= r;     }

      void changeVoice(int);

      Part* part(int n) const            { return _parts[n]; }
      void appendPart(Part* p);
      void updateStaffIndex();

      void setSpatium(qreal v);

      bool showInvisible() const { return _showInvisible; }
      void setShowInvisible(bool v);
      void setShowFrames(bool v);

      bool loadMsc(QString name);
      bool loadCompressedMsc(QString name);

      ChordRest* getSelectedChordRest() const;
      void getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const;

      Element* getSelectedElement() const   { return _selection.element(); }
      void setSelection(const Selection& s);

      int pos();
      Measure* tick2measure(int tick) const;
      MeasureBase* tick2measureBase(int tick) const;
      void fixTicks();
      void addArticulation(Element*, Articulation* atr);

      void setPrinting(bool val)     { _printing = val;      }

      void spell();
      void spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment);
      void spell(Note*);
      int nextSeg(int tick, int track);

      void setStyle(const MStyle& s)            { _style = s;                      }

      int inputPos() const;
      void setInputTrack(int);

//      void pasteStaff(XmlReader*, ChordRest* dst);
      void renderPart(EventMap* events, Part*);
      void setMscVersion(int v) { _mscVersion = v; }

      void addLyrics(int tick, int staffIdx, const QString&);

      MeasureBaseList* measures()  { return &_measures; }

      bool checkHasMeasures() const;

      void setLayout(Measure* m);

      int midiPort(int idx) const;
      QList<MidiMapping>* midiMapping()       { return &_midiMapping;          }
      MidiMapping* midiMapping(int channel)   { return &_midiMapping[channel]; }
      void rebuildMidiMapping();
      void updateChannel();


      bool creditsRead() const                       { return _creditsRead;     }
      void setCreditsRead(bool val)                  { _creditsRead = val;      }
      bool defaultsRead() const                      { return _defaultsRead;    }
      void setDefaultsRead(bool b)                   { _defaultsRead = b;       }
      Text* getText(int subtype);

      void lassoSelect(const QRectF&);
      void lassoSelectEnd();

      Page* searchPage(const QPointF&) const;
      QList<System*> searchSystem(const QPointF& p) const;
      Measure* searchMeasure(const QPointF& p) const;

      bool getPosition(Position* pos, const QPointF&, int voice) const;


      void moveBracket(int staffIdx, int srcCol, int dstCol);
      Measure* getCreateMeasure(int tick);

      void adjustBracketsDel(int sidx, int eidx);
      void adjustBracketsIns(int sidx, int eidx);
      void renumberMeasures();

      void endUndoRedo();
      void updateRepeatList(bool expandRepeats);

      void setPageFormat(const PageFormat& pf);

      void setInstrumentNames();
      void connectTies();

      void scanElements(void* data, void (*func)(void*, Element*));
      void selectSimilar(Element*, bool);
      QByteArray buildCanonical(int track);
      int fileDivision() const { return _fileDivision; } ///< division of current loading *.msc file
      void splitStaff(int staffIdx, int splitPoint);
      Lyrics* addLyrics();
      Note* addPitch(int pitch, bool addFlag);

      int addCustomKeySig(KeySig*);
      KeySig* keySigFactory(const KeySigEvent&);
      Element* selectMove(const QString& cmd);
      Element* move(const QString& cmd);
      void cmdAddInterval(int, const QList<Note*>&);
      void cmdCreateTuplet(ChordRest*, Tuplet*);
      void doLayout();
      void setStaffTypeList(const QList<StaffType*>& tl) { _staffTypes = tl;   }
      void addImage(Element*);
      QByteArray readCompressedToBuffer();
      QByteArray readToBuffer();
      void writeSegments(Xml& xml, const Measure*, int strack, int etrack, Segment* first, Segment* last, bool);

      const QMap<QString, QString> metaTags() const           { return _metaTags; }
      QMap<QString, QString> metaTags()                       { return _metaTags; }
      void setMetaTag(const QString& tag, const QString& val) { _metaTags.insert(tag, val); }
      void updateNotes();
      QHash<int, LinkedElements*>& links();
      void appendMeasures(int, ElementType);
      MeasureBase* appendMeasure(ElementType type);
      qreal point(const Spatium sp) const { return sp.val() * _spatium; }
      bool checkProgramVersion(const QString&);
      bool checkScoreVersion(const QString& version);

   public:
      Score();

      ~Score();
      bool read(const QString&);
      Page* page(int page)    { return _pages[page]; }
      int pageIdx(Page* page) const;
      int numPages() const    { return _pages.size(); }

      // read context
      int curTick;                  // for read optimizations
      int curTrack;
      QList<Slur*> slurs;

      StyleVal style(StyleIdx idx) const  { return _style.value(idx);   }
      Spatium styleS(StyleIdx idx) const  { return _style.valueS(idx);  }
      QString styleSt(StyleIdx idx) const { return _style.valueSt(idx); }
      bool    styleB(StyleIdx idx) const  { return _style.valueB(idx);  }
      qreal  styleD(StyleIdx idx) const  { return _style.valueD(idx);  }
      int     styleI(StyleIdx idx) const  { return _style.valueI(idx);  }

      QList<Staff*>& staves()             { return _staves; }
      const QList<Staff*>& staves() const { return _staves; }
      int nstaves() const                 { return _staves.size(); }
      Staff* staff(int n) const           { return _staves.value(n); }
      int fileDivision(int t) const       { return (t * division + _fileDivision/2) / _fileDivision; }
      const QList<Part*>* parts() const   { return &_parts; }
      Spanner* findSpanner(int id) const;
      void updateHairpin(Hairpin*);       // add/modify hairpin to pitchOffset list
      QList<Beam*> beams() const          { return _beams; }
      QList<Beam*>& beams()               { return _beams; }
      MStyle& style()                     { return _style;                   }
      const MStyle& style() const         { return _style;                   }
      int mscVersion() const              { return _mscVersion; }
      const TextStyle& textStyle(TextStyleType idx) const { return _style.textStyle(idx); }
      int staffIdx(const Part*) const;
      int staffIdx(const Staff* staff) const { return _staves.indexOf((Staff*)staff, 0); }
      const QList<StaffType*>& staffTypes() const        { return _staffTypes; }
      QList<StaffType*>& staffTypes()                    { return _staffTypes; }
      Score* parentScore() const          { return _parentScore; }
      void setParentScore(Score* s)       { _parentScore = s; }
      qreal spatium() const               { return _spatium; }
      void deselect(Element* obj);
      const Selection& selection() const  { return _selection; }
      bool printing() const               { return _printing;      }

      QString filePath() const            { return info.filePath(); }
      QFileInfo* fileInfo()               { return &info; }

      QString name() const           { return info.completeBaseName(); }
      void setName(const QString& s) { info.setFile(s); }
      void setStyle(MStyle& s)       { _style = s; }
      int symIdx() const             { return 0;   }
      void removeHairpin(Hairpin*);       // remove hairpin from pitchOffset list
      void layoutChords1(Segment* segment, int staffIdx);
      TimeSigMap* sigmap() const;
      AL::TempoMap* tempomap() const;
      Beam* beam(int id) const;
      Part* part(int staffIdx);

      MeasureBase* first() const;
      MeasureBase* last()  const;
      Measure* firstMeasure() const;
      Measure* lastMeasure() const;
      Segment* firstSegment(SegmentTypes s = SegAll) const;
      Segment* lastSegment() const;
      int midiChannel(int idx) const;
      QList<System*>* systems()               { return &_systems; }
      int customKeySigIdx(KeySig*) const;
      KeySig* customKeySig(int) const;
      void addMeasure(MeasureBase*);
      void add(Element*);
      void remove(Element*);
      Segment* tick2segment(int tick, bool first = false, SegmentTypes st = SegAll) const;
      bool showFrames() const { return _showFrames; }
      PageFormat* pageFormat() const          { return _pageFormat; }
      const QList<Page*>& pages() const       { return _pages; }
      QString metaTag(const QString& s) const                 { return _metaTags.value(s);}
      Volta* searchVolta(int tick) const;
      Measure* searchLabel(const QString& s, Measure* start = 0);
      qreal utick2utime(int tick) const;
      RepeatList* repeatList() const;

      int playPos() const      { return _playPos; }
      void setPlayPos(int val) { _playPos = val; }
      void toEList(EventMap* events);
      MRect moveCursor(Segment*);
      void showCursor(bool val);
      Cursor* cursor() const { return _cursor; }

      qreal pageWidth() const;
      qreal pageHeight() const;
      ScoreProxy* scoreProxy;
      int utime2utick(qreal utime) const;
      };

#endif

