//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: globals.h,v 1.17 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define MSC_VERSION     "1.9"
static const int MSCVERSION = 109;

// History:
//    1.3   added staff->_barLineSpan
//    1.5   save xoff/yoff in mm instead of pixel
//    1.6   save harmony base/root as tpc value
//    1.7   invert semantic of page fill limit
//    1.8   slur id, slur anchor in in Note
//    1.8   image size stored in mm instead of pixel

extern bool debugMode;
extern bool layoutDebug;
extern bool noSeq;            ///< Dont use sequencer; cmd line option.
extern bool noMidi;           ///< Dont use midi; cmd line option.
extern bool midiInputTrace;   ///< debug option: dump midi input
extern bool midiOutputTrace;  ///< debug option: dump midi output
extern bool converterMode;

static const QString mimeSymbolFormat("application/mscore/symbol");
static const QString mimeSymbolListFormat("application/mscore/symbollist");
static const QString mimeStaffListFormat("application/mscore/stafflist");
static const QString mimeMeasureListFormat("application/mscore/measurelist");

static const qreal INCH = 25.4;
static const qreal PPI  = 72.0;           // printer points per inch
static const qreal SPATIUM20 = 5.0 / PPI; // size of Spatium for 20pt font in inch

extern qreal PDPI;      // physical drawing resolution
extern qreal DPI;       // logical drawing resolution
extern qreal DPMM;      // logical dots/mm

// used for stem and slur:
enum Direction { AUTO, UP, DOWN };

enum Placement {
      PLACE_AUTO, PLACE_ABOVE, PLACE_BELOW
      };

enum DurationType {
      D_QUARTER, D_EIGHT, D_256TH, D_128TH, D_64TH, D_32ND,
      D_16TH, D_HALF, D_WHOLE, D_BREVE, D_LONG, D_MEASURE
      };

enum LineSegmentType {
      SEGMENT_SINGLE, SEGMENT_BEGIN, SEGMENT_MIDDLE, SEGMENT_END
      };

enum AlignmentFlags {
             ALIGN_LEFT     = 0,
             ALIGN_RIGHT    = 1,
             ALIGN_HCENTER  = 2,
             ALIGN_TOP      = 0,
             ALIGN_BOTTOM   = 4,
             ALIGN_VCENTER  = 8,
             ALIGN_BASELINE = 16
      };

Q_DECLARE_FLAGS(Align, AlignmentFlags);
Q_DECLARE_OPERATORS_FOR_FLAGS(Align);

static const Align ALIGN_CENTER = ALIGN_HCENTER | ALIGN_VCENTER;


enum OffsetType {
      OFFSET_ABS,       ///< offset in point units
      OFFSET_SPATIUM    ///< offset in space units
      };

enum BeamMode { BEAM_AUTO, BEAM_BEGIN, BEAM_MID, BEAM_END,
      BEAM_NO, BEAM_BEGIN32, BEAM_INVALID
      };

//---------------------------------------------------------
//   NoteType
//---------------------------------------------------------

enum NoteType {
      NOTE_NORMAL,
      NOTE_ACCIACCATURA,
      NOTE_APPOGGIATURA,       // grace notes
      NOTE_INVALID
      };

static const int VOICES = 4;
static const int MAX_STAVES = 4;

static const qreal DPMM_DISPLAY = 4;   // 100 DPI
static const qreal PALETTE_SPATIUM = 1.9 * DPMM_DISPLAY;

extern QString language;

extern QTextStream cout, eout;

extern QPaintDevice* pdev;
#endif
