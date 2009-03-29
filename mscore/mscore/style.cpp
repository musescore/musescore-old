//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: style.cpp,v 1.35 2006/09/15 09:34:57 wschweer Exp $
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

#include "globals.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "articulation.h"

Style* style;
//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

double _spatium;
double _spatiumMag;
QVector<TextStyle> defaultTextStyles;
Style defaultStyle;

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
      StyleType("systemFrameDistance",     ST_SPATIUM),     // dist. between staff and vertical box
      StyleType("frameSystemDistance",     ST_SPATIUM),     // dist. between vertical box and next system
      StyleType("minMeasureWidth",         ST_SPATIUM),
      StyleType("barWidth",                ST_SPATIUM),
      StyleType("doubleBarWidth",          ST_SPATIUM),
      StyleType("endBarWidth",             ST_SPATIUM),
      StyleType("doubleBarDistance",       ST_SPATIUM),
      StyleType("endBarDistance",          ST_SPATIUM),
      StyleType("bracketWidth",            ST_SPATIUM),     // system bracket width
      StyleType("bracketDistance",         ST_SPATIUM),     // system bracket distance
      StyleType("clefLeftMargin",          ST_SPATIUM),
      StyleType("keysigLeftMargin",        ST_SPATIUM),
      StyleType("timesigLeftMargin",       ST_SPATIUM),
      StyleType("clefKeyRightMargin",      ST_SPATIUM),
      StyleType("stemWidth",               ST_SPATIUM),
      StyleType("beginRepeatLeftMargin",   ST_SPATIUM),
      StyleType("minNoteDistance",         ST_SPATIUM),
      StyleType("barNoteDistance",         ST_SPATIUM),
      StyleType("noteBarDistance",         ST_SPATIUM),
      StyleType("measureSpacing",          ST_DOUBLE),
      StyleType("staffLineWidth",          ST_SPATIUM),
      StyleType("ledgerLineWidth",         ST_SPATIUM),
      StyleType("akkoladeWidth",           ST_SPATIUM),
      StyleType("prefixDistance",          ST_SPATIUM),
      StyleType("prefixNoteDistance",      ST_SPATIUM),
      StyleType("beamWidth",               ST_SPATIUM),
      StyleType("beamDistance",            ST_DOUBLE),        // in beamWidth units
      StyleType("beamMinLen",              ST_SPATIUM),           // len for broken beams
      StyleType("beamMinSlope",            ST_DOUBLE),
      StyleType("beamMaxSlope",            ST_DOUBLE),
      StyleType("maxBeamTicks",            ST_INT),
      StyleType("dotNoteDistance",         ST_SPATIUM),
      StyleType("dotRestDistance",         ST_SPATIUM),
      StyleType("dotDotDistance",          ST_SPATIUM),
      StyleType("propertyDistanceHead",    ST_SPATIUM),  // note property to note head
      StyleType("propertyDistanceStem",    ST_SPATIUM),  // note property to note stem
      StyleType("propertyDistance",        ST_SPATIUM),      // note property to note property
      StyleType("pageFillLimit",           ST_DOUBLE),         // 0-1.0
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
      StyleType("useGermanNoteNames",      ST_BOOL),
      StyleType("warnPitchRange",          ST_BOOL),
      StyleType("chordNamesUseSymbols",    ST_BOOL),
      StyleType("concertPitch",            ST_BOOL),            // display transposing instruments in concert pitch
      StyleType("createMultiMeasureRests", ST_BOOL),
      StyleType("minEmptyMeasures",        ST_INT),         // minimum number of empty measures for multi measure rest
      StyleType("minMMRestWidth",          ST_SPATIUM),       // minimum width of multi measure rest
      StyleType("hideEmptyStaves",         ST_BOOL),
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
      StyleType("DownMordentAnchor",       ST_INT)
      };

//---------------------------------------------------------
//   textStyles
//---------------------------------------------------------

static const QString ff("Times New Roman");

#define MM(x) ((x)/INCH)
#define OA     OFFSET_ABS
#define OS     OFFSET_SPATIUM

const TextStyle defaultTextStyleArray[] = {
      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Symbols1"), QString("MScore"), 20, false, false, false,
         ALIGN_LEFT),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Symbols3"), QString("MScore"), 14, false, false, false,
         ALIGN_LEFT),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Title"), ff, 24, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 0, OA, 50, 0),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Subtitle"), ff, 14, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, MM(10), OA, 50, 0),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Composer"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_BOTTOM, 0, 0, OA, 100, 100),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Poet"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM, 0, 0, OA, 0, 100),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Lyrics odd lines"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 7),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Lyrics even lines"), ff, 11, false, true, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 7),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Fingering"), ff,  8, false, false, false,
         ALIGN_CENTER, 0.0, 0.0, OA, 0.0, 0.0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "InstrumentsLong"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, 0.0, 0.0, OA, 0.0, 0.0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "InstrumentsShort"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, 0.0, 0.0, OA, 0.0, 0.0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "InstrumentsExcerpt"), ff, 18, false, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM, 0.0, 0.0, OA, 0, 100),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Dynamics"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0.0, 8.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Technik"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0.0, -2.0, OS),

/*13*/
      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Tempo"), ff, 12, true, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0, -4.0, OS, 0, 0, true, .0, .0, 0, Qt::black, false, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Metronome"), ff, 12, true, false, false,
         ALIGN_LEFT),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Copyright"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, MM(-15), OA, 50.0, 100.0),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Measure Number"), ff, 8, false, false, false,
         ALIGN_CENTER | ALIGN_BOTTOM, 0.0, 0.0, OS),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Page Number Odd"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_BASELINE, MM(-10), MM(-10), OA, 100.0, 100.0),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Page Number Even"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, MM(10), MM(-10), OA, 0.0, 100.0),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Translator"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 6),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Tuplets"), ff,  8, false, false, false,
         ALIGN_CENTER),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "System"), ff,  10, false, false, false,
         ALIGN_LEFT, 0, -4.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Staff"), ff,  10, false, false, false,
         ALIGN_LEFT, 0, -4.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Chordname"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0, -4.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Rehearsal Mark"), ff,  14, true, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0, -3.0, OS, 0, 0, true,
         0.3, 1.0, 20, Qt::black, false, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Repeat Text"), ff,  12, false, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0, -2.0, OS, 100, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Volta"), ff, 11, true, false, false,
         ALIGN_LEFT, 0.5, .0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Frame"), ff, 11, true, false, false,
         ALIGN_LEFT, 0, 0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "TextLine"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_VCENTER, 0, 0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Glissando"), ff, 8, false, true, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0.0, 0.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "String Number"), ff,  8, false, false, false,
         ALIGN_CENTER, 0, -5.0, OS, 100, 0, true, 0.2, -0.2, 0, Qt::black, true, false),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Ottava"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_VCENTER, 0.0, 0.0, OS, 0, 0, true),

      };

#undef MM
#undef OA
#undef OS

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

Style::Style()
   : QVector<StyleVal>(ST_STYLES)
      {
      StyleVal values[ST_STYLES] = {
            StyleVal(Spatium(7.0)),               // ST_staffUpperBorder,
            StyleVal(Spatium(7.0)),               // ST_staffLowerBorder,
            StyleVal(Spatium(6.5)),               // ST_staffDistance,
            StyleVal(Spatium(6.5)),               // ST_akkoladeDistance,
            StyleVal(Spatium(9.25)),              // ST_systemDistance,
            StyleVal(Spatium(2)),                 // ST_lyricsDistance,
            StyleVal(Spatium(2)),                 // ST_lyricsMinBottomDistance,
            StyleVal(Spatium(7.0)),               // ST_systemFrameDistance,
            StyleVal(Spatium(1.0)),               // ST_frameSystemDistance,
            StyleVal(Spatium(4.0)),               // ST_minMeasureWidth,
            StyleVal(Spatium(0.16)),              // ST_barWidth,
            StyleVal(Spatium(0.16)),              // ST_doubleBarWidth,
            StyleVal(Spatium(0.3)),               // ST_endBarWidth,
            StyleVal(Spatium(0.30)),              // ST_doubleBarDistance,
            StyleVal(Spatium(0.30)),              // ST_endBarDistance,
            StyleVal(Spatium(0.35)),              // ST_bracketWidth,
            StyleVal(Spatium(0.25)),              // ST_bracketDistance,
            StyleVal(Spatium(0.5)),               // ST_clefLeftMargin,
            StyleVal(Spatium(0.5)),               // ST_keysigLeftMargin,
            StyleVal(Spatium(0.5)),               // ST_timesigLeftMargin,
            StyleVal(Spatium(1.75)),              // ST_clefKeyRightMargin,
            StyleVal(Spatium(0.13)),              // ST_stemWidth,
            StyleVal(Spatium(1.0)),               // ST_beginRepeatLeftMargin,
            StyleVal(Spatium(0.4)),               // ST_minNoteDistance,
            StyleVal(Spatium(1.5)),               // ST_barNoteDistance,
            StyleVal(Spatium(1.0)),               // ST_noteBarDistance,
            StyleVal(1.2),                        // ST_measureSpacing,
            StyleVal(Spatium(0.08)),              // ST_staffLineWidth,
            StyleVal(Spatium(0.16)),              // ST_ledgerLineWidth,
            StyleVal(Spatium(1.6)),               // ST_akkoladeWidth,
            StyleVal(Spatium(0.13)),              // ST_prefixDistance,
            StyleVal(Spatium(0.22)),              // ST_prefixNoteDistance,
            StyleVal(Spatium(0.48)),              // ST_beamWidth,
            StyleVal(0.5),                        // ST_beamDistance,
            StyleVal(Spatium(1.25)),              // ST_beamMinLen,
            StyleVal(0.05),                       // ST_beamMinSlope,
            StyleVal(0.2),                        // ST_beamMaxSlope,
            StyleVal(division),                   // ST_maxBeamTicks,
            StyleVal(Spatium(0.35)),              // ST_dotNoteDistance,
            StyleVal(Spatium(0.25)),              // ST_dotRestDistance,
            StyleVal(Spatium(0.5)),               // ST_dotDotDistance,
            StyleVal(Spatium(0.5)),               // ST_propertyDistanceHead,
            StyleVal(Spatium(0.5)),               // ST_propertyDistanceStem,
            StyleVal(Spatium(0.25)),              // ST_propertyDistance,
            StyleVal(0.7),                        // ST_pageFillLimit,
            StyleVal(0.3),                        // ST_lastSystemFillLimit,
            StyleVal(Spatium(1.2)),               // ST_hairpinHeight,
            StyleVal(Spatium(0.5)),               // ST_hairpinContHeight,
            StyleVal(Spatium(0.13)),              // ST_hairpinWidth,
            StyleVal(true),                       // ST_showPageNumber,
            StyleVal(false),                      // ST_showPageNumberOne,
            StyleVal(true),                       // ST_pageNumberOddEven,
            StyleVal(true),                       // ST_showMeasureNumber,
            StyleVal(false),                      // ST_showMeasureNumberOne,
            StyleVal(5),                          // ST_measureNumberInterval,
            StyleVal(true),                       // ST_measureNumberSystem,
            StyleVal(false),                      // ST_measureNumberAllStaffs,
            StyleVal(0.7),                        // ST_smallNoteMag,
            StyleVal(0.7),                        // ST_graceNoteMag,
            StyleVal(0.7),                        // ST_smallStaffMag,
            StyleVal(0.8),                        // ST_smallClefMag,
            StyleVal(true),                       // ST_genClef,
            StyleVal(true),                       // ST_genKeysig,
            StyleVal(true),                       // ST_genTimesig,
            StyleVal(true),                       // ST_genCourtesyTimesig
            StyleVal(false),                      // ST_useGermanNoteNames
            StyleVal(true),                       // ST_warnPitchRange
            StyleVal(false),                      // ST_chordNamesUseSymbols
            StyleVal(false),                      // ST_chordNamesUseJazzFont
            StyleVal(false),                      // ST_concertPitch,
            StyleVal(false),                      // ST_createMultiMeasureRests,
            StyleVal(2),                          // ST_minEmptyMeasures,
            StyleVal(Spatium(4)),                 // ST_minMMRestWidth,
            StyleVal(false),                      // ST_hideEmptyStaves,
            StyleVal(UP),                         // ST_stemDir1,
            StyleVal(DOWN),                       // ST_stemDir2,
            StyleVal(UP),                         // ST_stemDir3,
            StyleVal(DOWN),                       // ST_stemDir4,
            StyleVal(85),                         // ST_gateTime,
            StyleVal(100),                        // ST_tenutoGateTime,
            StyleVal(50),                         // ST_staccatoGateTime,
            StyleVal(100),                        // ST_slurGateTime,

            StyleVal(int(A_TOP_STAFF)),           // ufermata
            StyleVal(int(A_BOTTOM_STAFF)),        // dfermata
            StyleVal(int(A_CHORD)),               // thumb
            StyleVal(int(A_CHORD)),               // sforzatoaccent
            StyleVal(int(A_CHORD)),               // espr
            StyleVal(int(A_CHORD)),               // staccato
            StyleVal(int(A_CHORD)),               // ustaccatissimo
            StyleVal(int(A_CHORD)),               // dstaccatissimo
            StyleVal(int(A_CHORD)),               // tenuto
            StyleVal(int(A_CHORD)),               // uportato
            StyleVal(int(A_CHORD)),               // dportato
            StyleVal(int(A_CHORD)),               // umarcato
            StyleVal(int(A_CHORD)),               // dmarcato
            StyleVal(int(A_CHORD)),               // ouvert
            StyleVal(int(A_CHORD)),               // plusstop
            StyleVal(int(A_TOP_STAFF)),           // upbow
            StyleVal(int(A_TOP_STAFF)),           // downbow
            StyleVal(int(A_TOP_STAFF)),           // reverseturn
            StyleVal(int(A_TOP_STAFF)),           // turn
            StyleVal(int(A_TOP_STAFF)),           // trill
            StyleVal(int(A_TOP_STAFF)),           // prall
            StyleVal(int(A_TOP_STAFF)),           // mordent
            StyleVal(int(A_TOP_STAFF)),           // prallprall
            StyleVal(int(A_TOP_STAFF)),           // prallmordent
            StyleVal(int(A_TOP_STAFF)),           // upprall
            StyleVal(int(A_TOP_STAFF)),    	  // downprall
            StyleVal(int(A_TOP_STAFF)),    	  // upmordent
            StyleVal(int(A_TOP_STAFF)),    	  // downmordent
            };

      StyleVal* d = data();
      for (int idx = 0; idx < ST_STYLES; ++idx)
            d[idx] = values[idx];
      };

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle(
   QString _name, QString _family, int _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   double _xoff, double _yoff, OffsetType _ot, double _rxoff, double _ryoff,
   bool sd,
   double fw, double pw, int fr, QColor co, bool _circle, bool _systemFlag)

   : name(_name), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   align(_align),
   xoff(_xoff), yoff(_yoff), offsetType(_ot), rxoff(_rxoff), ryoff(_ryoff),
   sizeIsSpatiumDependent(sd), frameWidth(fw), paddingWidth(pw),
   frameRound(fr), frameColor(co), circle(_circle), systemFlag(_systemFlag)
      {
		#ifdef Q_WS_MAC
		  family = _family+" 20";
		#else
		  family = _family;
		#endif
      }

//---------------------------------------------------------
//   setDefaultStyle
//---------------------------------------------------------

void setDefaultStyle()
      {
      defaultTextStyles.clear();
      for (int i = 0; i < TEXT_STYLES; ++i)
            defaultTextStyles.append(defaultTextStyleArray[i]);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyle::font(double _spatium) const
      {
      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      if (sizeIsSpatiumDependent)
            f.setPointSizeF(size * _spatium / (SPATIUM20 * DPI));
      else
            f.setPointSizeF(size);
      f.setUnderline(underline);
      return f;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyle::fontPx() const
      {
      double m = size * DPI / PPI;

      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      if (sizeIsSpatiumDependent)
            m = m * ::_spatium / (SPATIUM20 * DPI);
      f.setPixelSize(lrint(m));
      f.setUnderline(underline);
      return f;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextStyle::write(Xml& xml) const
      {
      xml.stag(QString("TextStyle name=\"%1\"").arg(name));
      xml.tag("family", family);
      xml.tag("size", size);
      xml.tag("bold", bold);
      xml.tag("italic", italic);
      xml.tag("underline", underline);
      xml.tag("align", int(align));
      xml.tag("offsetType", offsetType);
      xml.tag("sizeIsSpatiumDependent", sizeIsSpatiumDependent);
      if (offsetType == OFFSET_ABS) {
            xml.tag("xoffset", xoff * INCH);
            xml.tag("yoffset", yoff * INCH);
            }
      else {
            xml.tag("xoffset", xoff);
            xml.tag("yoffset", yoff);
            }
      xml.tag("rxoffset", rxoff);
      xml.tag("ryoffset", ryoff);
      xml.tag("frameWidth", frameWidth);
      xml.tag("paddingWidth", paddingWidth);
      xml.tag("frameRound", frameRound);
      xml.tag("frameColor", frameColor);
      if (circle)
            xml.tag("circle", circle);
      if (systemFlag)
            xml.tag("systemFlag", systemFlag);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextStyle::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "family")
                  family = val;
            else if (tag == "size")
                  size = i;
            else if (tag == "bold")
                  bold = i;
            else if (tag == "italic")
                  italic = i;
            else if (tag == "underline")
                  underline = i;
            else if (tag == "align")
                  align = Align(i);
            else if (tag == "anchor")     // obsolete
                  ;
            else if (tag == "xoffset")
                  xoff = val.toDouble();
            else if (tag == "yoffset")
                  yoff = val.toDouble();
            else if (tag == "rxoffset")
                  rxoff = val.toDouble();
            else if (tag == "ryoffset")
                  ryoff = val.toDouble();
            else if (tag == "offsetType")
                  offsetType = (OffsetType)i;
            else if (tag == "sizeIsSpatiumDependent")
                  sizeIsSpatiumDependent = val.toDouble();
            else if (tag == "frameWidth")
                  frameWidth = val.toDouble();
            else if (tag == "paddingWidth")
                  paddingWidth = val.toDouble();
            else if (tag == "frameRound")
                  frameRound = i;
            else if (tag == "frameColor")
                  frameColor = readColor(e);
            else if (tag == "circle")
                  circle = val.toInt();
            else if (tag == "systemFlag")
                  systemFlag = val.toInt();
            else
                  domError(e);
            }
      if (offsetType == OFFSET_ABS) {
            xoff /= INCH;
            yoff /= INCH;
            }
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Style::load(QDomElement e, int /*version*/)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "stemDir") {
                  int voice = e.attribute("voice", "1").toInt() - 1;
                  switch(voice) {
                        case 0: tag = "StemDir1"; break;
                        case 1: tag = "StemDir2"; break;
                        case 2: tag = "StemDir3"; break;
                        case 3: tag = "StemDir4"; break;
                        }
                  }
            int idx;
            for (idx = 0; idx < ST_STYLES; ++idx) {
                  if (styleTypes[idx].name() == tag) {
                        switch(styleTypes[idx].valueType()) {
                              case ST_SPATIUM:   data()[idx] = StyleVal(Spatium(val.toDouble()));   break;
                              case ST_DOUBLE:    data()[idx] = StyleVal(val.toDouble());            break;
                              case ST_BOOL:      data()[idx] = StyleVal(bool(val.toInt()));         break;
                              case ST_INT:       data()[idx] = StyleVal(val.toInt());               break;
                              case ST_DIRECTION: data()[idx] = StyleVal(Direction(val.toInt()));    break;
                              }
                        break;
                        }
                  }
            if (idx >= ST_STYLES)
                  domError(e);
            }
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool Style::isDefault(int idx)
      {
      switch(styleTypes[idx].valueType()) {
            case ST_DOUBLE:
            case ST_SPATIUM:
                  return at(idx).toDouble() == defaultStyle[idx].toDouble();
            case ST_BOOL:
                  return at(idx).toBool() == defaultStyle[idx].toBool();
            case ST_INT:
            case ST_DIRECTION:
                  return at(idx).toInt() == defaultStyle[idx].toInt();
            }
      return false;
      }

//---------------------------------------------------------
//   save
//    if optimize is true, save only if different to default
//    style
//---------------------------------------------------------

void Style::save(Xml& xml, bool optimize)
      {
      xml.stag("Style");

      for (int idx = 0; idx < ST_STYLES; ++idx) {
            if (optimize && isDefault(idx))
                  continue;
            switch(styleTypes[idx].valueType()) {
                  case ST_SPATIUM:   xml.tag(styleTypes[idx].name(), at(idx).toSpatium().val()); break;
                  case ST_DOUBLE:    xml.tag(styleTypes[idx].name(), at(idx).toDouble()); break;
                  case ST_BOOL:      xml.tag(styleTypes[idx].name(), at(idx).toBool()); break;
                  case ST_INT:       xml.tag(styleTypes[idx].name(), at(idx).toInt()); break;
                  case ST_DIRECTION: xml.tag(styleTypes[idx].name(), int(at(idx).toDirection())); break;
                  }
            }
      xml.etag();
      }

