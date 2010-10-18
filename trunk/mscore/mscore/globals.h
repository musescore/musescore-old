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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define MSC_VERSION     "1.18"

static const int MSCVERSION = 118;

// History:
//    1.3   added staff->_barLineSpan
//    1.5   save xoff/yoff in mm instead of pixel
//    1.6   save harmony base/root as tpc value
//    1.7   invert semantic of page fill limit
//    1.8   slur id, slur anchor in in Note
//    1.9   image size stored in mm instead of pixel
//    1.10  TextLine properties changed
//    1.11  Instrument name in part saved as TextC
//    1.12  use durationType, remove tickLen
//    1.13  Clefs: userOffset is not (mis)used for vertical layout position
//    1.14  save user modified beam position as spatium value
//    1.15  save timesig inline; Lyrics "endTick" replaced by "ticks"
//    1.16  spanners (hairpin, trill etc.) are now inline and have no ticks anymore
//    1.17  new <Score> toplevel structure to support linked parts (excerpts)
//    1.18  save lyrics as subtype to chord/rest to allow them associated with
//          grace notes
//    1.19  replace text style numbers by text style names; box margins are now
//          used

extern bool debugMode;
extern bool enableExperimental;
extern bool scriptDebug;
extern bool layoutDebug;
extern bool noSeq;            ///< Dont use sequencer; cmd line option.
extern bool noMidi;           ///< Dont use midi; cmd line option.
extern bool midiInputTrace;   ///< debug option: dump midi input
extern bool midiOutputTrace;  ///< debug option: dump midi output
extern bool converterMode;
extern double converterDpi;

static const char mimeSymbolFormat[]      = "application/mscore/symbol";
static const char mimeSymbolListFormat[]  = "application/mscore/symbollist";
static const char mimeStaffListFormat[]   = "application/mscore/stafflist";

static const qreal INCH = 25.4;
static const qreal PPI  = 72.0;           // printer points per inch
static const qreal SPATIUM20 = 5.0 / PPI; // size of Spatium for 20pt font in inch

extern qreal PDPI;      // physical drawing resolution
extern qreal DPI;       // logical drawing resolution
extern qreal DPMM;      // logical dots/mm

//---------------------------------------------------------
//   NoteHeadGroup
//---------------------------------------------------------

enum NoteHeadGroup {
      HEAD_NORMAL, HEAD_CROSS, HEAD_DIAMOND, HEAD_TRIANGLE, HEAD_MI,
      HEAD_SLASH, HEAD_XCIRCLE, HEAD_DO, HEAD_RE, HEAD_FA, HEAD_LA, HEAD_TI,
      HEAD_GROUPS
      };

//---------------------------------------------------------
//   NoteHeadType
//---------------------------------------------------------

enum NoteHeadType {
      HEAD_AUTO, HEAD_WHOLE, HEAD_HALF, HEAD_QUARTER, HEAD_BREVIS
      };

//---------------------------------------------------------
//   Anchor
//---------------------------------------------------------

enum Anchor {
      ANCHOR_SEGMENT, ANCHOR_MEASURE
      };

//---------------------------------------------------------
//   Direction
//    used for stem and slur
//---------------------------------------------------------

enum Direction  {
      AUTO, UP, DOWN
      };

//---------------------------------------------------------
//   DirectionH
//    used for note head mirror
//---------------------------------------------------------

enum DirectionH {
      DH_AUTO, DH_LEFT, DH_RIGHT
      };

//---------------------------------------------------------
//   ValueType
//    used for Note->velocity
//---------------------------------------------------------

enum ValueType {
      AUTO_VAL, USER_VAL, OFFSET_VAL
      };

//---------------------------------------------------------
//   Placement
//---------------------------------------------------------

enum Placement {
      PLACE_AUTO, PLACE_ABOVE, PLACE_BELOW, PLACE_LEFT
      };

//---------------------------------------------------------
//   AlignmentFlags
//---------------------------------------------------------

enum AlignmentFlags {
      ALIGN_LEFT     = 0,
      ALIGN_RIGHT    = 1,
      ALIGN_HCENTER  = 2,
      ALIGN_TOP      = 0,
      ALIGN_BOTTOM   = 4,
      ALIGN_VCENTER  = 8,
      ALIGN_BASELINE = 16,
      ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER,
      ALIGN_HMASK = ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER,
      ALIGN_VMASK = ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER | ALIGN_BASELINE
      };

Q_DECLARE_FLAGS(Align, AlignmentFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Align);

//---------------------------------------------------------
//   OffsetType
//---------------------------------------------------------

enum OffsetType {
      OFFSET_ABS,       ///< offset in point units
      OFFSET_SPATIUM    ///< offset in space units
      };

//---------------------------------------------------------
//   BeamMode
//---------------------------------------------------------

enum BeamMode {
      BEAM_AUTO, BEAM_BEGIN, BEAM_MID, BEAM_END,
      BEAM_NO, BEAM_BEGIN32, BEAM_BEGIN64, BEAM_INVALID
      };

//---------------------------------------------------------
//   TransposeDirection
//---------------------------------------------------------

enum TransposeDirection {
      TRANSPOSE_UP, TRANSPOSE_DOWN, TRANSPOSE_CLOSEST
      };

//---------------------------------------------------------
//   TransposeMode
//---------------------------------------------------------

enum TransposeMode {
      TRANSPOSE_BY_KEY, TRANSPOSE_BY_INTERVAL
      };

//---------------------------------------------------------
//   DynamicType
//---------------------------------------------------------

enum DynamicType {
      DYNAMIC_STAFF, DYNAMIC_PART, DYNAMIC_SYSTEM
      };

//---------------------------------------------------------
//   NoteType
//---------------------------------------------------------

enum NoteType {
      NOTE_NORMAL,
      NOTE_ACCIACCATURA,
      NOTE_APPOGGIATURA,       // grace notes
      NOTE_GRACE4,
      NOTE_GRACE16,
      NOTE_GRACE32,
      NOTE_INVALID
      };

//---------------------------------------------------------
//   SelectType
//---------------------------------------------------------

enum SelectType {
      SELECT_SINGLE, SELECT_RANGE, SELECT_ADD
      };

//---------------------------------------------------------
//    ScoreState
//    used also to mask out shortcuts (actions.cpp)
//---------------------------------------------------------

enum ScoreState {
      STATE_INIT = 0, STATE_DISABLED = 1, STATE_NORMAL = 2, STATE_NOTE_ENTRY = 4, STATE_EDIT = 8,
      STATE_PLAY = 16, STATE_SEARCH = 32, STATE_FOTO = 64
      };

//---------------------------------------------------------
//   MidiRemoteType
//---------------------------------------------------------

enum MidiRemoteType {
      MIDI_REMOTE_TYPE_INACTIVE = -1,
      MIDI_REMOTE_TYPE_NOTEON = 0, MIDI_REMOTE_TYPE_CTRL
      };

//---------------------------------------------------------
//   MidiRemote
//---------------------------------------------------------

struct MidiRemote {
      int channel;
      MidiRemoteType type;
      int data;         // pitch or controller number
      };

//---------------------------------------------------------
//    Accidental Values
//---------------------------------------------------------

enum AccidentalType {
      ACC_NONE,
      ACC_SHARP,
      ACC_FLAT,
      ACC_SHARP2,
      ACC_FLAT2,
      ACC_NATURAL,

      ACC_FLAT_SLASH,
      ACC_FLAT_SLASH2,
      ACC_MIRRORED_FLAT2,
      ACC_MIRRORED_FLAT,
      ACC_MIRRIRED_FLAT_SLASH,
      ACC_FLAT_FLAT_SLASH,

      ACC_SHARP_SLASH,
      ACC_SHARP_SLASH2,
      ACC_SHARP_SLASH3,
      ACC_SHARP_SLASH4,

      ACC_SHARP_ARROW_UP,
      ACC_SHARP_ARROW_DOWN,
      ACC_SHARP_ARROW_BOTH,
      ACC_FLAT_ARROW_UP,
      ACC_FLAT_ARROW_DOWN,
      ACC_FLAT_ARROW_BOTH,
      ACC_NATURAL_ARROW_UP,
      ACC_NATURAL_ARROW_DOWN,
      ACC_NATURAL_ARROW_BOTH,
      ACC_END
      };

//---------------------------------------------------------
//   UpDownMode
//---------------------------------------------------------

enum UpDownMode {
      UP_DOWN_CHROMATIC, UP_DOWN_OCTAVE, UP_DOWN_DIATONIC
      };

//---------------------------------------------------------
//   StaffGroup
//---------------------------------------------------------

enum StaffGroup {
      PITCHED_STAFF, PERCUSSION_STAFF, TAB_STAFF
      };

//---------------------------------------------------------
//   ClefType
//---------------------------------------------------------

enum ClefType {
      CLEF_INVALID = -1,
      CLEF_G = 0,
      CLEF_G1,
      CLEF_G2,
      CLEF_G3,
      CLEF_F,
      CLEF_F8,
      CLEF_F15,
      CLEF_F_B,
      CLEF_F_C,
      CLEF_C1,
      CLEF_C2,
      CLEF_C3,
      CLEF_C4,
      CLEF_TAB,
      CLEF_PERC,
      CLEF_C5,
      CLEF_G4,
      CLEF_F_8VA,
      CLEF_F_15MA,
      CLEF_PERC2,
      CLEF_MAX
      };

//---------------------------------------------------------
//   TextStyleType
//    must be in sync with list in setDefaultStyle()
//---------------------------------------------------------

enum TextStyleType {
      TEXT_STYLE_INVALID = -1,      // unstyled

      TEXT_STYLE_TITLE = 0,
      TEXT_STYLE_SUBTITLE,
      TEXT_STYLE_COMPOSER,
      TEXT_STYLE_POET,
      TEXT_STYLE_LYRIC1,
      TEXT_STYLE_LYRIC2,
      TEXT_STYLE_FINGERING,
      TEXT_STYLE_INSTRUMENT_LONG,
      TEXT_STYLE_INSTRUMENT_SHORT,
      TEXT_STYLE_INSTRUMENT_EXCERPT,

      TEXT_STYLE_DYNAMICS,
      TEXT_STYLE_TECHNIK,
      TEXT_STYLE_TEMPO,
      TEXT_STYLE_METRONOME,
      TEXT_STYLE_MEASURE_NUMBER,
      TEXT_STYLE_TRANSLATOR,
      TEXT_STYLE_TUPLET,

      TEXT_STYLE_SYSTEM,
      TEXT_STYLE_STAFF,
      TEXT_STYLE_HARMONY,
      TEXT_STYLE_REHEARSAL_MARK,
      TEXT_STYLE_REPEAT,
      TEXT_STYLE_VOLTA,
      TEXT_STYLE_FRAME,
      TEXT_STYLE_TEXTLINE,
      TEXT_STYLE_GLISSANDO,
      TEXT_STYLE_STRING_NUMBER,

      TEXT_STYLE_OTTAVA,
      TEXT_STYLE_BENCH,
      TEXT_STYLE_HEADER,
      TEXT_STYLE_FOOTER,
      TEXT_STYLES
      };

//---------------------------------------------------------
//   SegmentType
//---------------------------------------------------------

enum SegmentType {
      SegClef                 = 0x1,
      SegKeySig               = 0x2,
      SegTimeSig              = 0x4,
      SegStartRepeatBarLine   = 0x8,
      SegBarLine              = 0x10,
      SegGrace                = 0x20,
      SegChordRest            = 0x40,
      SegBreath               = 0x80,
      SegEndBarLine           = 0x100,
      SegTimeSigAnnounce      = 0x200,
      SegKeySigAnnounce       = 0x400,
      SegAll                  = 0xfff
      };
typedef QFlags<SegmentType> SegmentTypes;
Q_DECLARE_OPERATORS_FOR_FLAGS(SegmentTypes)

//---------------------------------------------------------
//   BarLineType
//---------------------------------------------------------

enum BarLineType {
      NORMAL_BAR, DOUBLE_BAR, START_REPEAT, END_REPEAT,
      BROKEN_BAR, END_BAR, END_START_REPEAT
      };

extern const char* stateName(ScoreState);

static const int VOICES = 4;
static const int MAX_STAVES = 4;

static const qreal DPMM_DISPLAY = 4;   // 100 DPI
static const qreal PALETTE_SPATIUM = 1.9 * DPMM_DISPLAY;

extern QPaintDevice* pdev;
#endif
