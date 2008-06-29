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

Style* style;
//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

double _spatium;
double _spatiumMag;
QVector<TextStyle> defaultTextStyles;

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

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Lyrics"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, 0, 7),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Fingering"), ff,  8, false, false, false,
         ALIGN_CENTER, 0.0, 0.0, OA, 0.0, 0.0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "InstrumentsLong"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "InstrumentsShort"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "InstrumentsExcerpt"), ff, 18, false, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM, 0.0, 0.0, OA, 0, 100),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Dynamics"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0.0, 8.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Technik"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0.0, -2.0, OS),

/*12*/
      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Tempo"), ff, 12, true, false, false,
         ALIGN_LEFT, 0, -5.0, OS, 0, 0, true),

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

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Dynamics1"), QString("MScore1"), 20, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, 0.0, 8.0, OS, 0.0, 0.0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Tuplets"), ff,  8, false, false, false,
         ALIGN_CENTER),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "System"), ff,  10, false, false, false,
         ALIGN_LEFT, 0, -4.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Staff"), ff,  10, false, false, false,
         ALIGN_LEFT, 0, -4.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Chordname"), ff,  12, false, false, false,
         ALIGN_LEFT, 0, -5.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Rehearsal Mark"), ff,  14, true, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0, -3.0, OS, 0, 0, true,
         0.3, 1.0, 20, Qt::black, false, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Repeat Text"), ff,  12, false, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0, -2.0, OS, 100, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Volta"), ff, 11, true, false, false,
         ALIGN_LEFT, 0, -5.0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Frame"), ff, 11, true, false, false,
         ALIGN_LEFT, 0, 0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "TextLine"), ff,  12, false, false, false,
         ALIGN_HCENTER, 0, 0, OS, 0, 0, true),

      TextStyle(QT_TRANSLATE_NOOP("MuseScore", "Glissando"), ff, 8, false, true, false,
         ALIGN_HCENTER | ALIGN_BASELINE, 0.0, 0.0, OS, 0, 0, true),
      };

#undef MM
#undef OA
#undef OS

//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

Style defaultStyle = {
      Spatium(7.0),   // staffUpperBorder
      Spatium(7.0),   // staffLowerBorder
      Spatium(6.5),   // staffDistance
      Spatium(6.5),   // accoladeDistance
      Spatium(9.25),  // systemDistance
      Spatium(2),     // lyricsDistance
      Spatium(2),     // lyricsMinBottomDistance
      Spatium(7.0),   // dist. between system and vertical box
      Spatium(7.0),   // dist. between vertical box and next system

      Spatium(4.0),   // minMeasureWidth  12.0
      Spatium(0.16),  // barWidth;
      Spatium(0.16),  // doubleBarWidth;
      Spatium(0.3),   // endBarWidth
      Spatium(0.30),  // doubleBarDistance;
      Spatium(0.30),  // endBarDistance
      Spatium(0.35),  // system bracket width
      Spatium(0.3),   // system bracket distance

      Spatium(0.5),   // clefLeftMargin;
      Spatium(0.5),   // keysig left margin
      Spatium(0.5),   // timesigLeftMargin
      Spatium(1.75),  // clef/key/sig right margin

      Spatium(0.13),  // stemWidth = 1.3 * staffLineWidth

      Spatium(0.4),   // minNoteDistance
      Spatium(1.5),   // barNoteDistance
      Spatium(1.0),   // noteBarDistance

      1.3,              // measureSpacing

      Spatium(0.08),    // staff line width
      Spatium(0.08),    // ledgerLineWidth;
      Spatium(1.6),     // akkoladeWidth;
      Spatium(5.5),     // akkoladeDistance;
      Spatium(0.13),    // prefixDistance

      Spatium(0.22),    // prefixNoteDistance

      Spatium(0.48),    // beamWidth
      0.5,              // beamDistance, units of beamWidth
      Spatium(1.25),    // beamMinLen, len for broken beams
      0.05,             // beamMinSlope
      0.2,              // beamMaxSlope
      division,         //  maxBeamTicks : 1/4 or 1/2 groups
      Spatium(0.5),     // dotNoteDistance
      Spatium(0.25),    // dotRestDistance
      Spatium(0.5),     // dotDotDistance

      Spatium(0.5),     // propertyDistanceHead
      Spatium(0.5),     // propertyDistanceStem; note articulation to note stem
      Spatium(0.25),    // propertyDistance; note articulation to note articulation

      0.7,              // pageFillLimit
      0.3,              // lastSystemFillLimit
      Spatium(1.2),     // hairpinHeight
      Spatium(0.5),     // hairpinContHeight
      Spatium(0.13),    // hairpinWidth

      true,             // showPageNumber
      false,            // showPageNumberOne
      true,             // pageNumberOddEven
      true,             // showMeasureNumber
      false,            // showMeasureNumberOne
      5,                // measureNumberInterval;
      true,             // measureNumberSystem
      false,            // showMeasureNumberAllStaffs
      0.7,              // smallNoteMag
      0.7,              // graceNoteMag
      0.7,              // smallStaffMag
      0.8,              // smallClefMag
      true,             // genClef
      true,             // genKeySig
      true,             // genTimesig
      true,             // genCourtesyTimesig

      false,            // use german note names
      false,            // use symbols in chord names
      false,            // display in concert pitch

      // play style
      70,               // gateTime
      100,              // tenutoGateTime
      40,               // staccatoGateTime
      90                // slurGateTime
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

   : name(_name), family(_family), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   align(_align),
   xoff(_xoff), yoff(_yoff), offsetType(_ot), rxoff(_rxoff), ryoff(_ryoff),
   sizeIsSpatiumDependent(sd), frameWidth(fw), paddingWidth(pw),
   frameRound(fr), frameColor(co), circle(_circle), systemFlag(_systemFlag)
      {
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

QFont TextStyle::font() const
      {
      double mag = ::_spatium / (SPATIUM20 * DPI);

      double m = size * DPI / PPI;
      if (sizeIsSpatiumDependent)
            m *= mag;
      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
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

void Style::load(QDomElement e, int version)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i    = val.toInt();
            double d = val.toDouble();

            if (tag == "staffUpperBorder")
                  staffUpperBorder = Spatium(d);
            else if (tag == "staffLowerBorder")
                  staffLowerBorder = Spatium(d);
            else if (tag == "staffDistance")
                  staffDistance = Spatium(d);
            else if (tag == "systemDistance")
                  systemDistance = Spatium(d);
            else if (tag == "lyricsDistance")
                  lyricsDistance = Spatium(d);
            else if (tag == "lyricsMinBottomDistance")
                  lyricsMinBottomDistance = Spatium(d);
            else if (tag == "systemFrameDistance")
                  systemBoxDistance = Spatium(d);
            else if (tag == "frameSystemDistance")
                  boxSystemDistance = Spatium(d);
            else if (tag == "minMeasureWidth")
                  minMeasureWidth = Spatium(d);
            else if (tag == "barWidth")
                  barWidth = Spatium(d);
            else if (tag == "doubleBarWidth")
                  doubleBarWidth = Spatium(d);
            else if (tag == "endBarWidth")
                  endBarWidth = Spatium(d);
            else if (tag == "doubleBarDistance")
                  doubleBarDistance = Spatium(d);
            else if (tag == "endBarDistance")
                  endBarDistance = Spatium(d);
            else if (tag == "bracketWidth")
                  bracketWidth = Spatium(d);
            else if (tag == "bracketDistance")
                  bracketDistance = Spatium(d);
            else if (tag == "clefLeftMargin")
                  clefLeftMargin = Spatium(d);
            else if (tag == "keysigLeftMargin")
                  keysigLeftMargin = Spatium(d);
            else if (tag == "timesigLeftMargin")
                  timesigLeftMargin = Spatium(d);
            else if (tag == "clefKeyRightMargin")
                  clefKeyRightMargin = Spatium(d);
            else if (tag == "stemWidth")
                  stemWidth = Spatium(d);
            else if (tag == "minNoteDistance")
                  minNoteDistance = Spatium(d);
            else if (tag == "spacing16")        // obsolete
                  ;
            else if (tag == "spacing8")         // obsolete
                  ;
            else if (tag == "spacing4")         // obsolete
                  ;
            else if (tag == "spacing2")         // obsolete
                  ;
            else if (tag == "measureSpacing")
                  measureSpacing = d;
            else if (tag == "barNoteDistance")
                  barNoteDistance = Spatium(d);
            else if (tag == "noteBarDistance")
                  noteBarDistance = Spatium(d);
            else if (tag == "staffLineWidth")
                  staffLineWidth = Spatium(d);
            else if (tag == "ledgerLineWidth")
                  ledgerLineWidth = Spatium(d);
            else if (tag == "akkoladeWidth")
                  akkoladeWidth = Spatium(d);
            else if (tag == "akkoladeDistance")
                  akkoladeDistance = Spatium(d);
            else if (tag == "prefixDistance")
                  prefixDistance = Spatium(d);
            else if (tag == "prefixNoteDistance")
                  prefixNoteDistance = Spatium(d);
            else if (tag == "beamWidth")
                  beamWidth = Spatium(d);
            else if (tag == "beamDistance")
                  beamDistance = d;
            else if (tag == "beamMinLen")
                  beamMinLen = Spatium(d);
            else if (tag == "beamMinSlope")
                  beamMinSlope = d;
            else if (tag == "beamMaxSlope")
                  beamMaxSlope = d;
            else if (tag == "maxBeamTicks")
                  maxBeamTicks = i;
            else if (tag == "dotNoteDistance")
                  dotNoteDistance = Spatium(d);
            else if (tag == "dotRestDistance")
                  dotRestDistance = Spatium(d);
            else if (tag == "dotDotDistance")
                  dotDotDistance = Spatium(d);
            else if (tag == "propertyDistanceHead")
                  propertyDistanceHead = Spatium(d);
            else if (tag == "propertyDistanceStem")
                  propertyDistanceStem = Spatium(d);
            else if (tag == "propertyDistance")
                  propertyDistance = Spatium(d);
            else if (tag == "ticklen2Width")    // obsolete
                  ;
            else if (tag == "pageFillLimit") {
                  pageFillLimit = d;
                  if (version < 107)
                        pageFillLimit = 1.0 - pageFillLimit;
                  }
            else if (tag == "lastSystemFillLimit")
                  lastSystemFillLimit = d;
            else if (tag == "hairpinHeight")
                  hairpinHeight = Spatium(d);
            else if (tag == "hairpinContHeight")
                  hairpinContHeight = Spatium(d);
            else if (tag == "hairpinWidth")
                  hairpinWidth = Spatium(d);
            else if (tag == "showPageNumber")
                  showPageNumber = i;
            else if (tag == "showPageNumberOne")
                  showPageNumberOne = i;
            else if (tag == "pageNumberOddEven")
                  pageNumberOddEven = i;
            else if (tag == "showMeasureNumber")
                  showMeasureNumber = i;
            else if (tag == "showMeasureNumberOne")
                  showMeasureNumberOne = i;
            else if (tag == "measureNumberInterval")
                  measureNumberInterval = i;
            else if (tag == "measureNumberSystem")
                  measureNumberSystem = i;
            else if (tag == "measureNumberAllStaffs")
                  measureNumberAllStaffs = i;
            else if (tag == "graceNoteMag")
                  graceNoteMag = d;
            else if (tag == "smallStaffMag")
                  smallStaffMag = d;
            else if (tag == "smallNoteMag")
                  smallNoteMag = d;
            else if (tag == "smallClefMag")
                  smallClefMag = d;
            else if (tag == "genClef")
                  genClef = i;
            else if (tag == "genKeysig")
                  genKeysig = i;
            else if (tag == "genTimesig")
                  genTimesig = i;
            else if (tag == "genCourtesyTimesig")
                  genCourtesyTimesig = i;
            else if (tag == "useGermanNoteNames")
                  useGermanNoteNames = i;
            else if (tag == "chordNamesUseSymbols")
                  chordNamesUseSymbols = i;
            else if (tag == "displayInConcertPitch")
                  concertPitch = i;
            else if (tag == "gateTime")
                  gateTime  = i;
            else if (tag == "tenutoGateTime")
                  tenutoGateTime = i;
            else if (tag == "staccatoGateTime")
                  staccatoGateTime = i;
            else if (tag == "slurGateTime")
                  slurGateTime = i;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Style::save(Xml& xml)
      {
      xml.stag("Style");

      xml.tag("staffUpperBorder",       staffUpperBorder.val());
      xml.tag("staffLowerBorder",       staffLowerBorder.val());
      xml.tag("staffDistance",          staffDistance.val());
      xml.tag("akkoladeDistance",       akkoladeDistance.val());
      xml.tag("systemDistance",         systemDistance.val());
      xml.tag("lyricsDistance",         lyricsDistance.val());
      xml.tag("lyricsMinBottomDistance", lyricsMinBottomDistance.val());
      xml.tag("systemFrameDistance",    systemBoxDistance.val());
      xml.tag("frameSystemDistance",    boxSystemDistance.val());

      xml.tag("minMeasureWidth",        minMeasureWidth.val());
      xml.tag("barWidth",               barWidth.val());
      xml.tag("doubleBarWidth",         doubleBarWidth.val());
      xml.tag("endBarWidth",            endBarWidth.val());
      xml.tag("doubleBarDistance",      doubleBarDistance.val());
      xml.tag("endBarDistance",         endBarDistance.val());
      xml.tag("bracketWidth",           bracketWidth.val());
      xml.tag("bracketDistance",        bracketDistance.val());

      xml.tag("clefLeftMargin",         clefLeftMargin.val());
      xml.tag("keysigLeftMargin",       keysigLeftMargin.val());
      xml.tag("timesigLeftMargin",      timesigLeftMargin.val());
      xml.tag("clefKeyRightMargin",     clefKeyRightMargin.val());
      xml.tag("stemWidth",              stemWidth.val());
      xml.tag("minNoteDistance",        minNoteDistance.val());

      xml.tag("measureSpacing",         measureSpacing);

      xml.tag("barNoteDistance",        barNoteDistance.val());
      xml.tag("noteBarDistance",        noteBarDistance.val());
      xml.tag("staffLineWidth",         staffLineWidth.val());
      xml.tag("ledgerLineWidth",        ledgerLineWidth.val());
      xml.tag("akkoladeWidth",          akkoladeWidth.val());
      xml.tag("prefixDistance",         prefixDistance.val());
      xml.tag("prefixNoteDistance",     prefixNoteDistance.val());
      xml.tag("beamWidth",              beamWidth.val());
      xml.tag("beamDistance",           beamDistance);
      xml.tag("beamMinLen",             beamMinLen.val());
      xml.tag("beamMinSlope",           beamMinSlope);
      xml.tag("beamMaxSlope",           beamMaxSlope);
      xml.tag("maxBeamTicks",           maxBeamTicks);
      xml.tag("dotNoteDistance",        dotNoteDistance.val());
      xml.tag("dotRestDistance",        dotRestDistance.val());
      xml.tag("dotDotDistance",         dotDotDistance.val());
      xml.tag("propertyDistanceHead",   propertyDistanceHead.val());
      xml.tag("propertyDistanceStem",   propertyDistanceStem.val());
      xml.tag("propertyDistance",       propertyDistance.val());
      xml.tag("pageFillLimit",          pageFillLimit);
      xml.tag("lastSystemFillLimit",    lastSystemFillLimit);
      xml.tag("hairpinHeight",          hairpinHeight.val());
      xml.tag("hairpinContHeight",      hairpinContHeight.val());
      xml.tag("hairpinWidth",           hairpinWidth.val());

      xml.tag("showPageNumber",         showPageNumber);
      xml.tag("showPageNumberOne",      showPageNumberOne);
      xml.tag("pageNumberOddEven",      pageNumberOddEven);
      xml.tag("showMeasureNumber",      showMeasureNumber);
      xml.tag("showMeasureNumberOne",   showMeasureNumberOne);
      xml.tag("measureNumberInterval",  measureNumberInterval);
      xml.tag("measureNumberAllStaffs", measureNumberAllStaffs);
      xml.tag("measureNumberSystem",    measureNumberSystem);

      xml.tag("graceNoteMag",           graceNoteMag);
      xml.tag("smallStaffMag",          smallStaffMag);
      xml.tag("smallNoteMag",           smallNoteMag);
      xml.tag("smallClefMag",           smallClefMag);
      xml.tag("genClef",                genClef);
      xml.tag("genKeysig",              genKeysig);
      if (!genTimesig)
            xml.tag("genTimesig", genTimesig);
      if (!genCourtesyTimesig)
            xml.tag("genCourtesyTimesig", genCourtesyTimesig);
      if (useGermanNoteNames)
            xml.tag("useGermanNoteNames", useGermanNoteNames);
      if (chordNamesUseSymbols)
            xml.tag("chordNamesUseSymbols", chordNamesUseSymbols);
      if (concertPitch)
            xml.tag("displayInConcertPitch", concertPitch);

      xml.tag("gateTime",               gateTime);
      xml.tag("tenutoGateTime",         tenutoGateTime);
      xml.tag("staccatoGateTime",       staccatoGateTime);
      xml.tag("slurGateTime",           slurGateTime);

      xml.etag();
      }
