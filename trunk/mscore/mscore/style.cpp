//=============================================================================
//  MuseScore
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

#include "globals.h"
#include "style.h"
#include "style_p.h"
#include "xml.h"
#include "mscore.h"
#include "score.h"
#include "articulation.h"
#include "harmony.h"
#include "preferences.h"
#include "chordlist.h"

Style* style;
//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

//---------------------------------------------------------
//   styleTypes
//---------------------------------------------------------

StyleType styleTypes[] = {
      StyleType("staffUpperBorder",        ST_SPATIUM),
      StyleType("staffLowerBorder",        ST_SPATIUM),
      StyleType("staffDistance",           ST_SPATIUM),
      StyleType("akkoladeDistance",        ST_SPATIUM),
      StyleType("systemDistance",          ST_SPATIUM),
      StyleType("lyricsDistance",          ST_SPATIUM),
      StyleType("lyricsMinBottomDistance", ST_SPATIUM),
      StyleType("lyricsLineHeight",        ST_DOUBLE),      // in % of normal height (default: 1.0)
      StyleType("systemFrameDistance",     ST_SPATIUM),     // dist. between staff and vertical box
      StyleType("frameSystemDistance",     ST_SPATIUM),     // dist. between vertical box and next system
      StyleType("minMeasureWidth",         ST_SPATIUM),

      StyleType("barWidth",                ST_SPATIUM),
      StyleType("doubleBarWidth",          ST_SPATIUM),
      StyleType("endBarWidth",             ST_SPATIUM),
      StyleType("doubleBarDistance",       ST_SPATIUM),
      StyleType("endBarDistance",          ST_SPATIUM),
      StyleType("repeatBarTips",           ST_BOOL),
      StyleType("startBarlineSingle",      ST_BOOL),
      StyleType("startBarlineMultiple",    ST_BOOL),
      StyleType("bracketWidth",            ST_SPATIUM),     // system bracket width
      StyleType("bracketDistance",         ST_SPATIUM),     // system bracket distance

      StyleType("clefLeftMargin",          ST_SPATIUM),
      StyleType("keysigLeftMargin",        ST_SPATIUM),
      StyleType("timesigLeftMargin",       ST_SPATIUM),
      StyleType("clefKeyRightMargin",      ST_SPATIUM),
      StyleType("clefBarlineDistance",     ST_SPATIUM),
      StyleType("stemWidth",               ST_SPATIUM),
      StyleType("shortenStem",             ST_BOOL),        // ST_shortenStem,
      StyleType("shortStemProgression",    ST_SPATIUM),     // ST_shortStemProgression,
      StyleType("shortestStem",            ST_SPATIUM),
      StyleType("beginRepeatLeftMargin",   ST_SPATIUM),

      StyleType("minNoteDistance",         ST_SPATIUM),
      StyleType("barNoteDistance",         ST_SPATIUM),
      StyleType("noteBarDistance",         ST_SPATIUM),
      StyleType("measureSpacing",          ST_DOUBLE),
      StyleType("staffLineWidth",          ST_SPATIUM),
      StyleType("ledgerLineWidth",         ST_SPATIUM),
      StyleType("ledgerLineLength",        ST_SPATIUM),
      StyleType("akkoladeWidth",           ST_SPATIUM),
      StyleType("accidentalDistance",      ST_SPATIUM),
      StyleType("accidentalNoteDistance",  ST_SPATIUM),
      StyleType("beamWidth",               ST_SPATIUM),

      StyleType("beamDistance",            ST_DOUBLE),        // in beamWidth units
      StyleType("beamMinLen",              ST_SPATIUM),           // len for broken beams
      StyleType("beamMinSlope",            ST_DOUBLE),
      StyleType("beamMaxSlope",            ST_DOUBLE),
      StyleType("maxBeamTicks",            ST_INT),
      StyleType("dotNoteDistance",         ST_SPATIUM),
      StyleType("dotRestDistance",         ST_SPATIUM),
      StyleType("dotDotDistance",          ST_SPATIUM),
      StyleType("propertyDistanceHead",    ST_SPATIUM),     // note property to note head
      StyleType("propertyDistanceStem",    ST_SPATIUM),     // note property to note stem

      StyleType("propertyDistance",        ST_SPATIUM),     // note property to note property
      StyleType("pageFillLimit",           ST_DOUBLE),      // 0-1.0
      StyleType("lastSystemFillLimit",     ST_DOUBLE),
      StyleType("hairpinHeight",           ST_SPATIUM),
      StyleType("hairpinContHeight",       ST_SPATIUM),
      StyleType("hairpinWidth",            ST_SPATIUM),
      StyleType("showPageNumber",          ST_BOOL),
      StyleType("showPageNumberOne",       ST_BOOL),
      StyleType("pageNumberOddEven",       ST_BOOL),
      StyleType("showMeasureNumber",       ST_BOOL),

      StyleType("showMeasureNumberOne",    ST_BOOL),
      StyleType("measureNumberInterval",   ST_INT),
      StyleType("measureNumberSystem",     ST_BOOL),
      StyleType("measureNumberAllStaffs",  ST_BOOL),
      StyleType("smallNoteMag",            ST_DOUBLE),
      StyleType("graceNoteMag",            ST_DOUBLE),
      StyleType("smallStaffMag",           ST_DOUBLE),
      StyleType("smallClefMag",            ST_DOUBLE),
      StyleType("genClef",                 ST_BOOL),           // create clef for all systems, not only for first
      StyleType("genKeysig",               ST_BOOL),         // create key signature for all systems

      StyleType("genTimesig",              ST_BOOL),
      StyleType("genCourtesyTimesig",      ST_BOOL),
      StyleType("genCourtesyKeysig",       ST_BOOL),
      StyleType("genCourtesyClef",         ST_BOOL),
      StyleType("useGermanNoteNames",      ST_BOOL),
      StyleType("chordDescriptionFile",    ST_STRING),
      StyleType("concertPitch",            ST_BOOL),            // display transposing instruments in concert pitch
      StyleType("createMultiMeasureRests", ST_BOOL),
      StyleType("minEmptyMeasures",        ST_INT),         // minimum number of empty measures for multi measure rest
      StyleType("minMMRestWidth",          ST_SPATIUM),       // minimum width of multi measure rest
      StyleType("hideEmptyStaves",         ST_BOOL),
      StyleType("dontHidStavesInFirstSystm", ST_BOOL),

      StyleType("stemDir1",                ST_DIRECTION),
      StyleType("stemDir2",                ST_DIRECTION),
      StyleType("stemDir3",                ST_DIRECTION),
      StyleType("stemDir4",                ST_DIRECTION),

      //---------------------------------------------------------
      //   PlayStyle
      //---------------------------------------------------------

      StyleType("gateTime",                ST_INT),           // 0-100%
      StyleType("tenutoGateTime",          ST_INT),
      StyleType("staccatoGateTime",        ST_INT),
      StyleType("slurGateTime",            ST_INT),

      //---------------------------------------------------------
      //    Articulations
      //---------------------------------------------------------

      StyleType("UfermataAnchor",          ST_INT),
      StyleType("DfermataAnchor",          ST_INT),

      StyleType("UshortfermataAnchor",     ST_INT),
      StyleType("DshortfermataAnchor",     ST_INT),
      StyleType("UlongfermataAnchor",      ST_INT),
      StyleType("DlongfermataAnchor",      ST_INT),
      StyleType("UverylongfermataAnchor",  ST_INT),
      StyleType("DverylongfermataAnchor",  ST_INT),
      StyleType("ThumbAnchor",             ST_INT),
      StyleType("SforzatoaccentAnchor",    ST_INT),
      StyleType("EspressivoAnchor",        ST_INT),
      StyleType("StaccatoAnchor",          ST_INT),

      StyleType("UstaccatissimoAnchor",    ST_INT),
      StyleType("DstaccatissimoAnchor",    ST_INT),
      StyleType("TenutoAnchor",            ST_INT),
      StyleType("UportatoAnchor",          ST_INT),
      StyleType("DportatoAnchor",          ST_INT),
      StyleType("UmarcatoAnchor",          ST_INT),
      StyleType("DmarcatoAnchor",          ST_INT),
      StyleType("OuvertAnchor",            ST_INT),
      StyleType("PlusstopAnchor",          ST_INT),
      StyleType("UpbowAnchor",             ST_INT),

      StyleType("DownbowAnchor",           ST_INT),
      StyleType("ReverseturnAnchor",       ST_INT),
      StyleType("TurnAnchor",              ST_INT),
      StyleType("TrillAnchor",             ST_INT),
      StyleType("PrallAnchor",             ST_INT),
      StyleType("MordentAnchor",           ST_INT),
      StyleType("PrallPrallAnchor",        ST_INT),
      StyleType("PrallMordentAnchor",      ST_INT),
      StyleType("UpPrallAnchor",           ST_INT),
      StyleType("DownPrallAnchor",         ST_INT),

      StyleType("UpMordentAnchor",         ST_INT),
      StyleType("DownMordentAnchor",       ST_INT),
      StyleType("SnappizzicatorAnchor",    ST_INT),

      StyleType("TappingAnchor",           ST_INT),
      StyleType("SlappingAnchor",          ST_INT),
      StyleType("PoppingAnchor",           ST_INT),

      StyleType("ArpeggioNoteDistance",    ST_SPATIUM),
      StyleType("ArpeggioLineWidth",       ST_SPATIUM),
      StyleType("ArpeggioHookLen",         ST_SPATIUM),
      StyleType("FixMeasureNumbers",       ST_INT),
      StyleType("FixMeasureWidth",         ST_BOOL),

      StyleType("slurEndWidth",            ST_SPATIUM),
      StyleType("slurMidWidth",            ST_SPATIUM),
      StyleType("slurDottedWidth",         ST_SPATIUM),
      StyleType("slurBow",                 ST_SPATIUM),
      StyleType("sectionPause",            ST_DOUBLE),

      StyleType("musicalSymbolFont",       ST_STRING),

      StyleType("showHeader",              ST_BOOL),
      StyleType("headerFirstPage",         ST_BOOL),
      StyleType("headerOddEven",           ST_BOOL),
      StyleType("evenHeaderL",             ST_STRING),
      StyleType("evenHeaderC",             ST_STRING),
      StyleType("evenHeaderR",             ST_STRING),
      StyleType("oddHeaderL",              ST_STRING),
      StyleType("oddHeaderC",              ST_STRING),
      StyleType("oddHeaderR",              ST_STRING),

      StyleType("showFooter",              ST_BOOL),
      StyleType("footerFirstPage",         ST_BOOL),
      StyleType("footerOddEven",           ST_BOOL),
      StyleType("evenFooterL",             ST_STRING),
      StyleType("evenFooterC",             ST_STRING),
      StyleType("evenFooterR",             ST_STRING),
      StyleType("oddFooterL",              ST_STRING),
      StyleType("oddFooterC",              ST_STRING),
      StyleType("oddFooterR",              ST_STRING)
      };

static const QString ff("FreeSerif");

#define MM(x) ((x)/INCH)
#define OA     OFFSET_ABS
#define OS     OFFSET_SPATIUM
#define TR(x)  QT_TRANSLATE_NOOP("MuseScore", x)
#define AS(x)  s->addTextStyle(x)

//---------------------------------------------------------
//   setDefaultStyle
//    synchronize with TextStyleType
//---------------------------------------------------------

void setDefaultStyle(Style* s)
      {
      AS(TextStyle(TR("Title"), ff, 24, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP));

      AS(TextStyle(TR("Subtitle"), ff, 14, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, MM(10), OA));

      AS(TextStyle(TR("Composer"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_BOTTOM));

      AS(TextStyle(TR("Lyricist"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM));

      AS(TextStyle(TR("Lyrics odd lines"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 7, OS, 0.0, 0.0, true));

      AS(TextStyle(TR("Lyrics even lines"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 7, OS, 0.0, 0.0, true));

      AS(TextStyle(TR( "Fingering"), ff,  8, false, false, false,
         ALIGN_CENTER, 0.0, 0.0, OA, 0.0, 0.0, true));

      AS(TextStyle(TR( "InstrumentsLong"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, 0.0, 0.0, OA, 0.0, 0.0, true));

      AS(TextStyle(TR( "InstrumentsShort"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, 0.0, 0.0, OA, 0.0, 0.0, true));

      AS(TextStyle(TR( "InstrumentsExcerpt"), ff, 18, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, 0.0, 0.0, OA, 0, 0));

      AS(TextStyle(TR( "Dynamics"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0.0, 8.0, OS, 0, 0, true));

      AS(TextStyle(TR( "Technik"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0.0, -2.0, OS));

//      AS(TextStyle(TR( "Tempo"), ff, 12, true, false, false,
// (temporarly) switch bold off as Qt cannot show bold glyphs with codepoint > 16bit
// (used for musical symbols)
      AS(TextStyle(TR( "Tempo"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0, -4.0, OS, 0, 0, true, .0, .0, 0, Qt::black, false, true));

      AS(TextStyle(TR( "Metronome"), ff, 12, true, false, false, ALIGN_LEFT));

      AS(TextStyle(TR( "Measure Number"), ff, 8, false, false, false,
         ALIGN_CENTER | ALIGN_BOTTOM, -1, 0.0, OS, 0.0, 0.0, true));

      AS(TextStyle(TR( "Translator"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 6));

      AS(TextStyle(TR( "Tuplets"), ff,  8, false, false, false,
         ALIGN_CENTER, 0.0, 0.0, OA, 0.0, 0.0, true));

      AS(TextStyle(TR( "System"), ff,  10, false, false, false,
         ALIGN_LEFT, 0, -4.0, OS, 0, 0, true,
         0.0, 0.0, 25, Qt::black, false, true));

      AS(TextStyle(TR( "Staff"), ff,  10, false, false, false,
         ALIGN_LEFT, 0, -4.0, OS, 0, 0, true));

      AS(TextStyle(TR( "Chordname"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0, -4.0, OS, 0, 0, true));

      AS(TextStyle(TR( "Rehearsal Mark"), ff,  14, true, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0, -3.0, OS, 0, 0, true,
         0.3, 1.0, 20, Qt::black, false, true));

      AS(TextStyle(TR( "Repeat Text Left"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0, -2.0, OS, 0, 0, true,
         0.0, 0.0, 25, Qt::black, false, true));

      AS(TextStyle(TR( "Repeat Text Right"), ff,  12, false, false, false,
         ALIGN_RIGHT | ALIGN_BASELINE, 0, -2.0, OS, 100, 0, true,
         0.0, 0.0, 25, Qt::black, false, true));

      AS(TextStyle(TR( "Repeat Text"), ff,  12, false, false, false,          // for backward compatibility
         ALIGN_HCENTER | ALIGN_BASELINE, 0, -2.0, OS, 100, 0, true,
         0.0, 0.0, 25, Qt::black, false, true));

      AS(TextStyle(TR( "Volta"), ff, 11, true, false, false,
         ALIGN_LEFT, 0.5, .0, OS, 0, 0, true));

      AS(TextStyle(TR( "Frame"), ff, 12, false, false, false, ALIGN_LEFT | ALIGN_TOP));

      AS(TextStyle(TR( "TextLine"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_VCENTER, 0, 0, OS, 0, 0, true));

      AS(TextStyle(TR( "Glissando"), ff, 8, false, true, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0.0, 0.0, OS, 0, 0, true));

      AS(TextStyle(TR( "String Number"), ff,  8, false, false, false,
         ALIGN_CENTER, 0, -5.0, OS, 100, 0, true, 0.2, -0.2, 0, Qt::black, true, false));

      AS(TextStyle(TR( "Ottava"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_VCENTER, 0.0, 0.0, OS, 0, 0, true));

      AS(TextStyle(TR( "Bend"), ff, 8, false, false, false,
         ALIGN_CENTER | ALIGN_BOTTOM, 0.0, 0.0, OS, 0.0, 0.0, true));

      AS(TextStyle(TR( "Header"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP));

      AS(TextStyle(TR( "Footer"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_BOTTOM, 0.0, MM(5), OA));

      AS(TextStyle(TR( "Instrument Change"), ff,  12, true, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM, 0, -3.0, OS, 0, 0, true));

      AS(TextStyle(TR("Lyrics Verse"), ff, 11, false, false, false,
         ALIGN_RIGHT | ALIGN_TOP, 0, 0, OS, 0.0, 0.0, true));

#undef MM
#undef OA
#undef OS
#undef TR
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::StyleData()
   : _values(ST_STYLES)
      {
      _customChordList = false;
      static const StyleVal values[ST_STYLES] = {
            StyleVal(ST_staffUpperBorder, Spatium(7.0)),
            StyleVal(ST_staffLowerBorder, Spatium(7.0)),
            StyleVal(ST_staffDistance, Spatium(6.5)),
            StyleVal(ST_akkoladeDistance, Spatium(6.5)),
            StyleVal(ST_systemDistance, Spatium(9.25)),
            StyleVal(ST_lyricsDistance, Spatium(2)),
            StyleVal(ST_lyricsMinBottomDistance, Spatium(2)),
            StyleVal(ST_lyricsLineHeight, 1.0),
            StyleVal(ST_systemFrameDistance, Spatium(7.0)),
            StyleVal(ST_frameSystemDistance, Spatium(1.0)),
            StyleVal(ST_minMeasureWidth, Spatium(4.0)),
                                                            // finale european style
            StyleVal(ST_barWidth, Spatium(0.16)),           // 0.1875
            StyleVal(ST_doubleBarWidth, Spatium(0.16)),
            StyleVal(ST_endBarWidth, Spatium(0.3)),         // 0.5
            StyleVal(ST_doubleBarDistance, Spatium(0.30)),
            StyleVal(ST_endBarDistance, Spatium(0.40)),     // 0.3
            StyleVal(ST_repeatBarTips, false),
            StyleVal(ST_startBarlineSingle, false),
            StyleVal(ST_startBarlineMultiple, true),

            StyleVal(ST_bracketWidth, Spatium(0.35)),
            StyleVal(ST_bracketDistance, Spatium(0.25)),

            StyleVal(ST_clefLeftMargin, Spatium(0.8)),
            StyleVal(ST_keysigLeftMargin, Spatium(0.5)),
            StyleVal(ST_timesigLeftMargin, Spatium(0.5)),

            StyleVal(ST_clefKeyRightMargin, Spatium(1.75)),
            StyleVal(ST_clefBarlineDistance, Spatium(0.18)),      // was 0.5
            StyleVal(ST_stemWidth, Spatium(0.13)),          // 0.09375
            StyleVal(ST_shortenStem, true),
            StyleVal(ST_shortStemProgression, Spatium(0.25)),
            StyleVal(ST_shortestStem, Spatium(2.25)),
            StyleVal(ST_beginRepeatLeftMargin,Spatium(1.0)),
            StyleVal(ST_minNoteDistance,Spatium(0.25)),           // 0.4
            StyleVal(ST_barNoteDistance,Spatium(1.2)),
            StyleVal(ST_noteBarDistance,Spatium(1.0)),

            StyleVal(ST_measureSpacing,1.2),
            StyleVal(ST_staffLineWidth,Spatium(0.08)),      // 0.09375
            StyleVal(ST_ledgerLineWidth,Spatium(0.12)),     // 0.1875
            StyleVal(ST_ledgerLineLength, Spatium(.6)),
            StyleVal(ST_akkoladeWidth,Spatium(1.6)),
            StyleVal(ST_accidentalDistance,Spatium(0.22)),
            StyleVal(ST_accidentalNoteDistance,Spatium(0.22)),
            StyleVal(ST_beamWidth,Spatium(0.48)),
            StyleVal(ST_beamDistance,0.5),
            StyleVal(ST_beamMinLen,Spatium(1.25)),
            StyleVal(ST_beamMinSlope,0.05),

            StyleVal(ST_beamMaxSlope,         0.2),
            StyleVal(ST_maxBeamTicks,         AL::division),
            StyleVal(ST_dotNoteDistance,      Spatium(0.35)),
            StyleVal(ST_dotRestDistance,      Spatium(0.25)),
            StyleVal(ST_dotDotDistance,       Spatium(0.5)),
            StyleVal(ST_propertyDistanceHead, Spatium(1.0)),
            StyleVal(ST_propertyDistanceStem, Spatium(1.8)),
            StyleVal(ST_propertyDistance,     Spatium(1.0)),
            StyleVal(ST_pageFillLimit,        0.7),
            StyleVal(ST_lastSystemFillLimit,  0.3),

            StyleVal(ST_hairpinHeight,Spatium(1.2)),
            StyleVal(ST_hairpinContHeight,Spatium(0.5)),
            StyleVal(ST_hairpinWidth,Spatium(0.13)),
            StyleVal(ST_showPageNumber,true),
            StyleVal(ST_showPageNumberOne,false),
            StyleVal(ST_pageNumberOddEven,true),
            StyleVal(ST_showMeasureNumber,true),
            StyleVal(ST_showMeasureNumberOne,false),
            StyleVal(ST_measureNumberInterval,5),
            StyleVal(ST_measureNumberSystem,true),

            StyleVal(ST_measureNumberAllStaffs,false),
            StyleVal(ST_smallNoteMag,0.7),
            StyleVal(ST_graceNoteMag,0.7),
            StyleVal(ST_smallStaffMag,0.7),
            StyleVal(ST_smallClefMag,0.8),
            StyleVal(ST_genClef,true),
            StyleVal(ST_genKeysig,true),
            StyleVal(ST_genTimesig,true),
            StyleVal(ST_genCourtesyTimesig, true),
            StyleVal(ST_genCourtesyKeysig, true),
            StyleVal(ST_genCourtesyClef, true),

            StyleVal(ST_useGermanNoteNames, false),
            StyleVal(ST_chordDescriptionFile, QString("stdchords.xml")),
            StyleVal(ST_concertPitch, false),
            StyleVal(ST_createMultiMeasureRests, false),
            StyleVal(ST_minEmptyMeasures, 2),
            StyleVal(ST_minMMRestWidth, Spatium(4)),
            StyleVal(ST_hideEmptyStaves, false),
            StyleVal(ST_dontHideStavesInFirstSystem, true),
            StyleVal(ST_stemDir1, UP),
            StyleVal(ST_stemDir2, DOWN),
            StyleVal(ST_stemDir3, UP),
            StyleVal(ST_stemDir4, DOWN),

            StyleVal(ST_gateTime, 100),
            StyleVal(ST_tenutoGateTime, 100),
            StyleVal(ST_staccatoGateTime, 50),
            StyleVal(ST_slurGateTime, 100),

            StyleVal(ST_UfermataAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_DfermataAnchor, int(A_BOTTOM_STAFF)),
            StyleVal(ST_UshortfermataAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_DshortfermataAnchor, int(A_BOTTOM_STAFF)),
            StyleVal(ST_UlongfermataAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_DlongfermataAnchor, int(A_BOTTOM_STAFF)),
            StyleVal(ST_UverylongfermataAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_DverylongfermataAnchor, int(A_BOTTOM_STAFF)),

            StyleVal(ST_ThumbAnchor, int(A_CHORD)),
            StyleVal(ST_SforzatoaccentAnchor, int(A_CHORD)),
            StyleVal(ST_EspressivoAnchor, int(A_CHORD)),
            StyleVal(ST_StaccatoAnchor, int(A_CHORD)),
            StyleVal(ST_UstaccatissimoAnchor, int(A_CHORD)),
            StyleVal(ST_DstaccatissimoAnchor, int(A_CHORD)),
            StyleVal(ST_TenutoAnchor, int(A_CHORD)),
            StyleVal(ST_UportatoAnchor, int(A_CHORD)),
            StyleVal(ST_DportatoAnchor, int(A_CHORD)),
            StyleVal(ST_UmarcatoAnchor, int(A_CHORD)),
            StyleVal(ST_DmarcatoAnchor, int(A_CHORD)),
            StyleVal(ST_OuvertAnchor, int(A_CHORD)),
            StyleVal(ST_PlusstopAnchor, int(A_CHORD)),
            StyleVal(ST_UpbowAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_DownbowAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_ReverseturnAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_TurnAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_TrillAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_PrallAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_MordentAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_PrallPrallAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_PrallMordentAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_UpPrallAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_DownPrallAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_UpMordentAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_DownMordentAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_SnappizzicatoAnchor, int(A_CHORD)),
            StyleVal(ST_TappingAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_SlappingAnchor, int(A_TOP_STAFF)),
            StyleVal(ST_PoppingAnchor, int(A_TOP_STAFF)),

            StyleVal(ST_ArpeggioNoteDistance, Spatium(.5)),
            StyleVal(ST_ArpeggioLineWidth, Spatium(.18)),
            StyleVal(ST_ArpeggioHookLen, Spatium(.8)),
            StyleVal(ST_FixMeasureNumbers, 0),
            StyleVal(ST_FixMeasureWidth, false),

            StyleVal(ST_SlurEndWidth, Spatium(.07)),
            StyleVal(ST_SlurMidWidth, Spatium(.15)),
            StyleVal(ST_SlurDottedWidth, Spatium(.1)),
            StyleVal(ST_SlurBow, Spatium(1.6)),

            StyleVal(ST_SectionPause, 3.0),

            StyleVal(ST_MusicalSymbolFont, QString("Emmentaler")),

            StyleVal(ST_showHeader, false),
            StyleVal(ST_headerFirstPage, false),
            StyleVal(ST_headerOddEven,  true),
            StyleVal(ST_evenHeaderL,  ""),
            StyleVal(ST_evenHeaderC,  ""),
            StyleVal(ST_evenHeaderR,  ""),
            StyleVal(ST_oddHeaderL,   ""),
            StyleVal(ST_oddHeaderC,   ""),
            StyleVal(ST_oddHeaderR,   ""),

            StyleVal(ST_showFooter,  true),
            StyleVal(ST_footerFirstPage, true),
            StyleVal(ST_footerOddEven, true),
            StyleVal(ST_evenFooterL,
               QString("<html>"
                 "<head>"
                   "<meta name=\"qrichtext\" content=\"1\" >"
                   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
                   "<style type=\"text/css\">"
                     "p, li { white-space: pre-wrap; }"
                     "</style>"
                   "</head>"
                 "<body style=\" font-family:'%1'; font-size:%2pt;\">"
                   "<p align=\"left\" style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                       "$p"
                     "</p>"
                   "</body>"
                 "</html>")),
            StyleVal(ST_evenFooterR, ""),
            StyleVal(ST_evenFooterC,
               QString("<html>"
                 "<head>"
                   "<meta name=\"qrichtext\" content=\"1\" >"
                   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
                   "<style type=\"text/css\">"
                     "p, li { white-space: pre-wrap; }"
                     "</style>"
                   "</head>"
                 "<body style=\" font-family:'%1'; font-size:%2pt;\">"
                   "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                       "$:copyright:"
                     "</p>"
                   "</body>"
                 "</html>")),

            StyleVal(ST_oddFooterL,
               QString("<html>"
                 "<head>"
                   "<meta name=\"qrichtext\" content=\"1\" >"
                   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
                   "<style type=\"text/css\">"
                     "p, li { white-space: pre-wrap; }"
                     "</style>"
                   "</head>"
                 "<body style=\" font-family:'%1'; font-size:%2pt;\">"
                   "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                       "$:copyright:"
                     "</p>"
                   "</body>"
                 "</html>")),

            StyleVal(ST_oddFooterR, ""),
            StyleVal(ST_oddFooterC,
               QString("<html>"
                 "<head>"
                   "<meta name=\"qrichtext\" content=\"1\" >"
                   "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
                   "<style type=\"text/css\">"
                     "p, li { white-space: pre-wrap; }"
                     "</style>"
                   "</head>"
                 "<body style=\" font-family:'%1'; font-size:%2pt;\">"
                   "<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                       "$p"
                     "</p>"
                   "</body>"
                 "</html>")),
            };

      for (int idx = 0; idx < ST_STYLES; ++idx)
            _values[idx] = values[idx];

// _textStyles.append(TextStyle(defaultTextStyles[i]));
      _chordList = 0;
      };

StyleData::StyleData(const StyleData& s)
   : QSharedData(s)
      {
      _values = s._values;
      if (s._chordList)
            _chordList = new ChordList(*(s._chordList));
      else
            _chordList = 0;
      _customChordList = s._customChordList;
      _textStyles      = s._textStyles;
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::~StyleData()
      {
      delete _chordList;
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle()
      {
      d = new TextStyleData;
      }

TextStyle::TextStyle(
   QString _name, QString _family, double _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   double _xoff, double _yoff, OffsetType _ot, double _rxoff, double _ryoff,
   bool sd,
   double fw, double pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg)
      {
      d = new TextStyleData(_name, _family, _size,
         _bold, _italic, _underline, _align, _xoff, _yoff, _ot, _rxoff, _ryoff,
         sd, fw, pw, fr, co, _circle, _systemFlag, fg);
      }

TextStyle::TextStyle(const TextStyle& s)
   : d(s.d)
      {
      }
TextStyle::~TextStyle()
      {
      }
TextStyle& TextStyle::operator=(const TextStyle& s)
      {
      d = s.d;
      return *this;
      }

//---------------------------------------------------------
//   TextStyleData
//---------------------------------------------------------

TextStyleData::TextStyleData()
      {
      name                   = "default";
      family                 = "FreeSerif";
      size                   = 10.0;
      bold                   = false;
      italic                 = false;
      underline              = false;
      hasFrame               = false;
      sizeIsSpatiumDependent = false;
      frameWidth             = 0.35;
      paddingWidth           = 0.0;
      frameRound             = 25;
      frameColor             = preferences.defaultColor;
      circle                 = false;
      systemFlag             = false;
      foregroundColor        = Qt::black;
      }

TextStyleData::TextStyleData(
   QString _name, QString _family, double _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   double _xoff, double _yoff, OffsetType _ot, double _rxoff, double _ryoff,
   bool sd,
   double fw, double pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg)
   :
   ElementLayout(_align, _xoff, _yoff, _ot, _rxoff, _ryoff),
   name(_name), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   sizeIsSpatiumDependent(sd), frameWidth(fw), paddingWidth(pw),
   frameRound(fr), frameColor(co), circle(_circle), systemFlag(_systemFlag),
   foregroundColor(fg)
      {
      hasFrame = fw != 0.0;
	family = _family;
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool TextStyleData::operator!=(const TextStyleData& s) const
      {
      return s.name                   != name
          || s.family                 != family
          || s.size                   != size
          || s.bold                   != bold
          || s.italic                 != italic
          || s.underline              != underline
          || s.align()                != align()
          || s.xoff()                 != xoff()
          || s.yoff()                 != yoff()
          || s.rxoff()                != rxoff()
          || s.ryoff()                != ryoff()
          || s.offsetType()           != offsetType()
          || s.sizeIsSpatiumDependent != sizeIsSpatiumDependent
          || s.frameWidth             != frameWidth
          || s.paddingWidth           != paddingWidth
          || s.frameRound             != frameRound
          || s.frameColor             != frameColor
          || s.foregroundColor        != foregroundColor
          || s.circle                 != circle
          || s.systemFlag             != systemFlag;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyleData::font(double _spatium) const
      {
      double m = size;

      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      f.setUnderline(underline);

      if (sizeIsSpatiumDependent)
            m *= _spatium / ( SPATIUM20 * DPI);

      f.setPointSizeF(m);
      return f;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyleData::fontPx(double _spatium) const
      {
      double m = size * DPI / PPI;

      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      f.setUnderline(underline);

      if (sizeIsSpatiumDependent)
            m *= _spatium / (SPATIUM20 * DPI);

      f.setPixelSize(lrint(m));
      return f;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextStyleData::write(Xml& xml) const
      {
      xml.stag(QString("TextStyle name=\"%1\"").arg(name));
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextStyleData::writeProperties(Xml& xml) const
      {
      ElementLayout::writeProperties(xml);
      xml.tag("family", family);
      xml.tag("size", size);
      if (bold)
            xml.tag("bold", bold);
      if (italic)
            xml.tag("italic", italic);
      if (underline)
            xml.tag("underline", underline);
      if (sizeIsSpatiumDependent)
            xml.tag("sizeIsSpatiumDependent", sizeIsSpatiumDependent);
      if (foregroundColor != Qt::black)
            xml.tag("foregroundColor", foregroundColor);

      if (hasFrame) {
            xml.tag("frameWidth", frameWidth);
            xml.tag("paddingWidth", paddingWidth);
            xml.tag("frameRound", frameRound);
            xml.tag("frameColor", frameColor);
            if (circle)
                  xml.tag("circle", circle);
            }
      if (systemFlag)
            xml.tag("systemFlag", systemFlag);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextStyleData::read(QDomElement e)
      {
      frameWidth = 0.0;
      name = e.attribute("name");

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextStyleData::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "family")
            family = val;
      else if (tag == "size")
            size = val.toDouble();
      else if (tag == "bold")
            bold = i;
      else if (tag == "italic")
            italic = i;
      else if (tag == "underline")
            underline = i;
      else if (tag == "align")
            setAlign(Align(i));
      else if (tag == "anchor")     // obsolete
            ;
      else if (ElementLayout::readProperties(e))
            ;
      else if (tag == "sizeIsSpatiumDependent")
            sizeIsSpatiumDependent = val.toDouble();
      else if (tag == "frameWidth") {
            frameWidth = val.toDouble();
            hasFrame = frameWidth != 0.0;
            }
      else if (tag == "paddingWidth")
            paddingWidth = val.toDouble();
      else if (tag == "frameRound")
            frameRound = i;
      else if (tag == "frameColor")
            frameColor = readColor(e);
      else if (tag == "foregroundColor")
            foregroundColor = readColor(e);
      else if (tag == "circle")
            circle = val.toInt();
      else if (tag == "systemFlag")
            systemFlag = val.toInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void StyleData::load(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "TextStyle") {
//                  QString name = e.attribute("name");
                  TextStyle s;
                  s.read(e);
                  setTextStyle(s);
                  }
            else if (tag == "displayInConcertPitch") {
                  set(StyleVal(ST_concertPitch, bool(val.toInt())));
                  }
            else if (tag == "ChordList") {
                  delete _chordList;
                  _chordList = new ChordList;
                  _chordList->read(e);
                  _customChordList = true;
                  }
            else {
                  if (tag == "stemDir") {
                        int voice = e.attribute("voice", "1").toInt() - 1;
                        switch(voice) {
                              case 0: tag = "StemDir1"; break;
                              case 1: tag = "StemDir2"; break;
                              case 2: tag = "StemDir3"; break;
                              case 3: tag = "StemDir4"; break;
                              }
                        }
                  // for compatibility:
                  if (tag == "oddHeader" || tag == "evenHeader"
                     || tag == "oddFooter" || tag == "evenFooter")
                        tag += "C";

                  int idx;
                  for (idx = 0; idx < ST_STYLES; ++idx) {
                        if (styleTypes[idx].name() == tag) {
                              StyleIdx i = StyleIdx(idx);
                              switch(styleTypes[idx].valueType()) {
                                    case ST_SPATIUM:   set(StyleVal(i, Spatium(val.toDouble()))); break;
                                    case ST_DOUBLE:    set(StyleVal(i, val.toDouble()));          break;
                                    case ST_BOOL:      set(StyleVal(i, bool(val.toInt())));       break;
                                    case ST_INT:       set(StyleVal(i, val.toInt()));             break;
                                    case ST_DIRECTION: set(StyleVal(i, Direction(val.toInt())));  break;
                                    case ST_STRING:    set(StyleVal(i, val));                     break;
                                    }
                              break;
                              }
                        }
                  if (idx >= ST_STYLES) {
                        if (tag == "oddHeader")
                              ;
                        else if (tag == "evenHeader")
                              ;
                        else if (tag == "oddFooter")
                              ;
                        else if (tag == "evenHeader")
                              ;
                        else
                              domError(e);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool StyleData::isDefault(StyleIdx idx) const
      {
      switch(styleTypes[idx].valueType()) {
            case ST_DOUBLE:
            case ST_SPATIUM:
                  return _values[idx].toDouble() == mscore->baseStyle()->valueD(idx);
            case ST_BOOL:
                  return _values[idx].toBool() == mscore->baseStyle()->valueB(idx);
            case ST_INT:
            case ST_DIRECTION:
                  return _values[idx].toInt() == mscore->baseStyle()->valueI(idx);
            case ST_STRING:
                  return _values[idx].toString() == mscore->baseStyle()->valueSt(idx);
            }
      return false;
      }

//---------------------------------------------------------
//   save
//    if optimize is true, save only if different to default
//    style
//---------------------------------------------------------

void StyleData::save(Xml& xml, bool optimize) const
      {
      xml.stag("Style");

      for (int i = 0; i < ST_STYLES; ++i) {
            StyleIdx idx = StyleIdx(i);
            if (optimize && isDefault(idx))
                  continue;
            switch(styleTypes[idx].valueType()) {
                  case ST_SPATIUM:
                  case ST_DOUBLE:    xml.tag(styleTypes[idx].name(), value(idx).toDouble()); break;
                  case ST_BOOL:      xml.tag(styleTypes[idx].name(), value(idx).toBool()); break;
                  case ST_INT:       xml.tag(styleTypes[idx].name(), value(idx).toInt()); break;
                  case ST_DIRECTION: xml.tag(styleTypes[idx].name(), int(value(idx).toDirection())); break;
                  case ST_STRING:    xml.tag(styleTypes[idx].name(), value(idx).toString()); break;
                  }
            }
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (!optimize || _textStyles[i] != mscore->defaultStyle()->textStyle(TextStyleType(i)))
                  _textStyles[i].write(xml);
            }
      for (int i = TEXT_STYLES; i < _textStyles.size(); ++i)
            _textStyles[i].write(xml);
      if (_customChordList && _chordList) {
            xml.stag("ChordList");
            _chordList->write(xml);
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* StyleData::chordDescription(int id) const
      {
      return chordList()->value(id);
      }

//---------------------------------------------------------
//   chordList
//---------------------------------------------------------

ChordList* StyleData::chordList()  const
      {
      if (_chordList == 0) {
            _chordList = new ChordList();
            _chordList->read("chords.xml");
            _chordList->read(value(ST_chordDescriptionFile).toString());
            }
      return _chordList;
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void StyleData::setChordList(ChordList* cl)
      {
      delete _chordList;
      _chordList = cl;
      _customChordList = true;      // TODO: check
      }

StyleVal::StyleVal(StyleIdx t, Spatium val)
      {
      idx  = t;
      v.dbl = val.val();
      }

StyleVal::StyleVal(StyleIdx t, double val)
      {
      idx  = t;
      v.dbl = val;
      }

StyleVal::StyleVal(StyleIdx t, bool val)
      {
      idx  = t;
      v.b   = val;
      }

StyleVal::StyleVal(StyleIdx t, int val)
      {
      idx  = t;
      v.i   = val;
      }

StyleVal::StyleVal(StyleIdx t, Direction val)
      {
      idx  = t;
      v.d   = val;
      }

StyleVal::StyleVal(StyleIdx t, const QString& val)
      {
      idx  = t;
      s    = val;
      }

StyleVal::StyleVal(const StyleVal& val)
      {
      idx = val.idx;
      s   = val.s;
      v   = val.v;
      }

StyleVal& StyleVal::operator=(const StyleVal& val)
      {
      idx = val.idx;
      s   = val.s;
      v   = val.v;
      return *this;
      }

StyleVal::StyleVal(const QString& name, const QString& val)
      {
      for (int i = 0; i < ST_STYLES; ++i) {
            if (styleTypes[i].name() != name)
                  continue;
            idx = StyleIdx(i);
            switch(styleTypes[i].valueType()) {
                  case ST_DOUBLE:
                  case ST_SPATIUM:
                        v.dbl = val.toDouble();
                        break;
                  case ST_BOOL:
                        v.b  = val.toInt();
                        break;
                  case ST_INT:
                        v.i = val.toInt();
                        break;
                  case ST_DIRECTION:
                        v.d = Direction(val.toInt());
                        break;
                  case ST_STRING:
                        s = val;
                        break;
                  }
            break;
            }
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

StyleVal Style::value(StyleIdx idx) const
      {
      return d->_values[idx];
      }

bool Style::isDefault(StyleIdx idx) const
      {
      return d->isDefault(idx);
      }

const ChordDescription* Style::chordDescription(int id) const
      {
      return d->chordDescription(id);
      }

ChordList* Style::chordList() const
      {
      return d->chordList();
      }

void Style::setChordList(ChordList* cl)
      {
      d->setChordList(cl);
      }

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle& StyleData::textStyle(const QString& name) const
      {
      foreach(const TextStyle& s, _textStyles) {
            if (s.name() == name)
                  return s;
            }
printf("TextStyle <%s> not found\n", qPrintable(name));
      return _textStyles[0];
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

TextStyleType StyleData::textStyleType(const QString& name) const
      {
      int idx = 0;
      foreach(const TextStyle& s, _textStyles) {
            if (s.name() == name)
                  return TextStyleType(idx);
            ++idx;
            }
printf("TextStyle <%s> not found\n", qPrintable(name));
      return TEXT_STYLE_INVALID;
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void StyleData::setTextStyle(const TextStyle& ts)
      {
      int idx = 0;
      foreach(const TextStyle& s, _textStyles) {
            if (s.name() == ts.name())
                  break;
//            printf("<%s><%s>\n", qPrintable(s.name()), qPrintable(ts.name()));
            ++idx;
            }
      if (idx < _textStyles.size())
            _textStyles[idx] = ts;
      else {
            _textStyles.append(ts);
            if (debugMode)
                  printf("StyleData::setTextStyle(): TextStyle <%s> not found\n", qPrintable(ts.name()));
            }
      }

QString TextStyle::name() const                           { return d->name; }
QString TextStyle::family() const                         { return d->family; }
double TextStyle::size() const                            { return d->size; }
bool TextStyle::bold() const                              { return d->bold; }
bool TextStyle::italic() const                            { return d->italic; }
bool TextStyle::underline() const                         { return d->underline; }
bool TextStyle::hasFrame() const                          { return d->hasFrame; }
Align TextStyle::align() const                            { return d->align(); }
double TextStyle::xoff() const                            { return d->xoff(); }
double TextStyle::yoff() const                            { return d->yoff(); }
OffsetType TextStyle::offsetType() const                  { return d->offsetType(); }
double TextStyle::rxoff() const                           { return d->rxoff(); }
double TextStyle::ryoff() const                           { return d->ryoff(); }
bool TextStyle::sizeIsSpatiumDependent() const            { return d->sizeIsSpatiumDependent; }
double TextStyle::frameWidth()  const                     { return d->frameWidth; }
double TextStyle::paddingWidth() const                    { return d->paddingWidth; }
int TextStyle::frameRound() const                         { return d->frameRound; }
QColor TextStyle::frameColor() const                      { return d->frameColor; }
bool TextStyle::circle() const                            { return d->circle;     }
bool TextStyle::systemFlag() const                        { return d->systemFlag; }
QColor TextStyle::foregroundColor() const                 { return d->foregroundColor; }
void TextStyle::setName(const QString& s)                 { d->name = s; }
void TextStyle::setFamily(const QString& s)               { d->family = s; }
void TextStyle::setSize(double v)                         { d->size = v; }
void TextStyle::setBold(bool v)                           { d->bold = v; }
void TextStyle::setItalic(bool v)                         { d->italic = v; }
void TextStyle::setUnderline(bool v)                      { d->underline = v; }
void TextStyle::setHasFrame(bool v)                       { d->hasFrame = v; }
void TextStyle::setAlign(Align v)                         { d->setAlign(v); }
void TextStyle::setXoff(double v)                         { d->setXoff(v); }
void TextStyle::setYoff(double v)                         { d->setYoff(v); }
void TextStyle::setOffsetType(OffsetType v)               { d->setOffsetType(v); }
void TextStyle::setRxoff(double v)                        { d->setRxoff(v); }
void TextStyle::setRyoff(double v)                        { d->setRyoff(v); }
void TextStyle::setSizeIsSpatiumDependent(bool v)         { d->sizeIsSpatiumDependent = v; }
void TextStyle::setFrameWidth(double v)                   { d->frameWidth = v; }
void TextStyle::setPaddingWidth(double v)                 { d->paddingWidth = v; }
void TextStyle::setFrameRound(int v)                      { d->frameRound = v; }
void TextStyle::setFrameColor(const QColor& v)            { d->frameColor = v; }
void TextStyle::setCircle(bool v)                         { d->circle = v;     }
void TextStyle::setSystemFlag(bool v)                     { d->systemFlag = v; }
void TextStyle::setForegroundColor(const QColor& v)       { d->foregroundColor = v; }
void TextStyle::write(Xml& xml) const                     { d->write(xml); }
void TextStyle::read(QDomElement v)                       { d->read(v); }
QFont TextStyle::font(double space) const                 { return d->font(space); }
QFont TextStyle::fontPx(double spatium) const             { return d->fontPx(spatium); }
QRectF TextStyle::bbox(double sp, const QString& s) const { return d->bbox(sp, s); }
QFontMetricsF TextStyle::fontMetrics(double space) const  { return fontMetrics(space); }
bool TextStyle::operator!=(const TextStyle& s) const      { return d->operator!=(*s.d); }
void TextStyle::layout(Element* e) const                  { d->layout(e);    }
void TextStyle::writeProperties(Xml& xml) const           { d->writeProperties(xml); }
QPointF TextStyle::reloff() const                         { return QPointF(rxoff(), ryoff()); }
void TextStyle::setReloff(const QPointF& p)               { setRxoff(p.x()), setRyoff(p.y()); }
bool TextStyle::readProperties(QDomElement v)             { return d->readProperties(v); }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void TextStyle::setFont(const QFont&)
      {
      //TODOxx
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

Style::Style()
      {
      d = new StyleData;
      }

Style::Style(const Style& s)
   : d(s.d)
      {
      }

Style::~Style()
      {
      }

Style& Style::operator=(const Style& s)
      {
      d = s.d;
      return *this;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void Style::set(const StyleVal& v)
      {
      d->_values[v.getIdx()] = v;
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

const TextStyle& Style::textStyle(TextStyleType idx) const
      {
      return d->textStyle(idx);
      }

const TextStyle& Style::textStyle(const QString& name) const
      {
      return d->textStyle(name);
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

TextStyleType Style::textStyleType(const QString& name) const
      {
      return d->textStyleType(name);
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void Style::setTextStyle(const TextStyle& ts)
      {
      d->setTextStyle(ts);
      }

//---------------------------------------------------------
//   addTextStyle
//---------------------------------------------------------

void Style::addTextStyle(const TextStyle& ts)
      {
      d->_textStyles.append(ts);
      }

//---------------------------------------------------------
//   removeTextStyle
//---------------------------------------------------------

void Style::removeTextStyle(const TextStyle& ts)
      {
      // TODO: d->_textStyles.append(ts);
      }

//---------------------------------------------------------
//   textStyles
//---------------------------------------------------------

const QList<TextStyle>& Style::textStyles() const
      {
      return d->_textStyles;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void Style::set(StyleIdx t, Spatium val)
      {
      set(StyleVal(t, val));
      }

void Style::set(StyleIdx t, const QString& val)
      {
      set(StyleVal(t, val));
      }

void Style::set(StyleIdx t, bool val)
      {
      set(StyleVal(t, val));
      }

void Style::set(StyleIdx t, double val)
      {
      set(StyleVal(t, val));
      }

void Style::set(StyleIdx t, int val)
      {
      set(StyleVal(t, val));
      }

void Style::set(StyleIdx t, Direction val)
      {
      set(StyleVal(t, val));
      }

//---------------------------------------------------------
//   valueS
//---------------------------------------------------------

Spatium Style::valueS(StyleIdx idx) const
      {
      return value(idx).toSpatium();
      }

//---------------------------------------------------------
//   valueSt
//---------------------------------------------------------

QString Style::valueSt(StyleIdx idx) const
      {
      return value(idx).toString();
      }

//---------------------------------------------------------
//   valueB
//---------------------------------------------------------

bool Style::valueB(StyleIdx idx) const
      {
      return value(idx).toBool();
      }

//---------------------------------------------------------
//   valueD
//---------------------------------------------------------

double Style::valueD(StyleIdx idx) const
      {
      return value(idx).toDouble();
      }

//---------------------------------------------------------
//   valueI
//---------------------------------------------------------

int Style::valueI(StyleIdx idx) const
      {
      return value(idx).toInt();
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool Style::load(QFile* qf)
      {
      return d->load(qf);
      }

void Style::load(QDomElement e)
      {
      d->load(e);
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Style::save(Xml& xml, bool optimize)
      {
      d->save(xml, optimize);
      }

//---------------------------------------------------------
//   load
//    return true on success
//---------------------------------------------------------

bool StyleData::load(QFile* qf)
      {
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(qf, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading style file %s at line %d column %d: %s\n",
               qf->fileName().toLatin1().data(), line, column, err.toLatin1().data());
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load Style failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return false;
            }
      docName = qf->fileName();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  QString version = e.attribute(QString("version"));
                  QStringList sl = version.split('.');
                  // _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        QString val(ee.text());
                        if (tag == "Style")
                              load(ee);
                        else
                              domError(ee);
                        }
                  }
            }
      return true;
      }


