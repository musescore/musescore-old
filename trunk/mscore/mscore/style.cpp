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
#include "mtime.h"

Style* style;
//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

double _spatium;
QVector<TextStyle> defaultTextStyles;

//---------------------------------------------------------
//   textStyles
//---------------------------------------------------------

static const QString ff("Times New Roman");

#define MM(x) ((x)/INCH)

const TextStyle defaultTextStyleArray[] = {
      TextStyle(QString("Symbols1"), QString("MScore"), 20, false, false, false,
         ALIGN_LEFT,    ANCHOR_STAFF, 0, 0, OFFSET_ABS),

      TextStyle(QString("Symbols3"), QString("MScore"), 14, false, false, false,
         ALIGN_LEFT,    ANCHOR_STAFF, 0, 0, OFFSET_SPATIUM),

      TextStyle(QString("Title"), ff, 24, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, ANCHOR_PAGE, 0, MM(0), OFFSET_ABS),

      TextStyle(QString("Subtitle"), ff, 14, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, ANCHOR_PAGE, 0, MM(15), OFFSET_ABS),

      TextStyle(QString("Composer"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_BOTTOM, ANCHOR_PAGE, 0, MM(0), OFFSET_ABS),

      TextStyle(QString("Poet"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM, ANCHOR_PAGE, 0, MM(0), OFFSET_ABS),

      TextStyle(QString("Lyrics"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, ANCHOR_STAFF, 0, 7, OFFSET_SPATIUM),

      TextStyle(QString("Fingering"), ff,  8, false, false, false,
         ALIGN_HCENTER | ALIGN_VCENTER, ANCHOR_NOTE, 0, 0, OFFSET_SPATIUM),

      TextStyle(QString("InstrumentsLong"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, ANCHOR_SYSTEM, 0, 0, OFFSET_SPATIUM),
      TextStyle(QString("InstrumentsShort"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, ANCHOR_SYSTEM, 0, 0, OFFSET_SPATIUM),

      TextStyle(QString("Dynamics"), ff, 12, false, true, false,
         ALIGN_LEFT,    ANCHOR_STAFF, 0, 0, OFFSET_SPATIUM),

      TextStyle(QString("Technik"), ff, 12, false, true, false,
         ALIGN_LEFT,    ANCHOR_STAFF, 0, 0, OFFSET_SPATIUM),
/*12*/

      TextStyle(QString("Tempo"), ff, 12, true, false, false,
         ALIGN_LEFT,    ANCHOR_STAFF, 0, -5.0, OFFSET_SPATIUM, true),

      TextStyle(QString("Metronome"), ff, 12, true, false, false,
         ALIGN_LEFT,    ANCHOR_STAFF, 0, 0, OFFSET_SPATIUM),

      TextStyle(QString("Copyright"), ff, 8, true, false, false,
         ALIGN_HCENTER | ALIGN_BOTTOM,    ANCHOR_PAGE, 0, MM(1), OFFSET_ABS),

      TextStyle(QString("Measure Number"), ff, 8, false, false, false,
         ALIGN_LEFT,    ANCHOR_STAFF, -1.0, -2.2, OFFSET_SPATIUM),

      TextStyle(QString("Page Number Odd"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_TOP, ANCHOR_PAGE, MM(0), MM(0), OFFSET_ABS),

      TextStyle(QString("Page Number Even"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, ANCHOR_PAGE, MM(0), MM(0), OFFSET_ABS),

      TextStyle(QString("Translator"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP,    ANCHOR_STAFF, 0, 6, OFFSET_SPATIUM),

      TextStyle(QString("Dynamics1"), QString("MScore1"), 20, false, false, false,
         ALIGN_LEFT, ANCHOR_STAFF, 0, 0, OFFSET_SPATIUM),

      TextStyle(QString("Tuplets"), ff,  8, false, false, false,
         ALIGN_HCENTER | ALIGN_VCENTER, ANCHOR_NOTE, 0, 0, OFFSET_SPATIUM),

      TextStyle(QString("System"), ff,  10, false, false, false,
         ALIGN_LEFT, ANCHOR_SYSTEM, 0, -4.0, OFFSET_SPATIUM, true),

      TextStyle(QString("Staff"), ff,  10, false, false, false,
         ALIGN_LEFT, ANCHOR_STAFF, 0, -4.0, OFFSET_SPATIUM, true),

      TextStyle(QString("Chordname"), ff,  12, false, false, false,
         ALIGN_HCENTER, ANCHOR_STAFF, 0, -5.0, OFFSET_SPATIUM, true),

      TextStyle(QString("Rehearsal Mark"), ff,  14, true, false, false,
         ALIGN_HCENTER, ANCHOR_SYSTEM, 0, -7.0, OFFSET_SPATIUM, true,
         0.3, 1.0, 1.0, 20, Qt::black),

      TextStyle(QString("Repeat Text"), ff,  12, false, false, false,
         ALIGN_HCENTER, ANCHOR_SYSTEM, 0, -4.5, OFFSET_SPATIUM, true),

      TextStyle(QString("Volta"), ff, 11, true, false, false,
         ALIGN_LEFT,    ANCHOR_STAFF, 0, -5.0, OFFSET_SPATIUM, true),
      };

//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

Style defaultStyle = {
      Spatium(8.0),   // staffUpperBorder
      Spatium(4.0),   // staffLowerBorder
      Spatium(6.5),   // staffDistance
      Spatium(6.5),   // accoladeDistance
      Spatium(9.25),  // systemDistance

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

      Spatium(.5),    // minNoteDistance
      Spatium(1.5),   // barNoteDistance
      Spatium(1.0),   // noteBarDistance

      1.0,              // spacing16;
      1.4,              // spacing8;
      1.8,              // spacing4;
      2.2,              // spacing2;

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
      0.1,              // beamMinSlope
      0.5,              // beamMaxSlope
      division,         //  maxBeamTicks : 1/4 or 1/2 groups
      Spatium(0.5),     // dotNoteDistance
      Spatium(0.25),    // dotRestDistance
      Spatium(0.5),     // dotDotDistance
      Spatium(0.5),     // propertyDistanceHead
      Spatium(0.5),     // propertyDistanceStem; note property to note stem
      Spatium(0.25),    // propertyDistance; note property to note property
      .5,               // ticklen2Width
      0.3,              // pageFillLimit
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

      Spatium(6.5),     // lyricsDistance
      };

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle(
   QString _name, QString _family, int _size,
   bool _bold, bool _italic, bool _underline,
   int _align, Anchor _anchor,
   double _xoff, double _yoff, OffsetType _ot, bool sd,
   double fw, double mw, double pw, int fr, QColor co)
   : name(_name), family(_family), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   align(_align), anchor(_anchor),
   xoff(_xoff), yoff(_yoff), offsetType(_ot),
   sizeIsSpatiumDependent(sd), frameWidth(fw), marginWidth(mw), paddingWidth(pw),
   frameRound(fr), frameColor(co)
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
      double mag = ::_spatium / (spatiumBase20 * DPI);

      double m = size;
      if (sizeIsSpatiumDependent)
            m *= mag;
      QFont f(family);
      f.setWeight(bold ? QFont::Bold : QFont::Normal);
      f.setItalic(italic);
      f.setPointSizeF(m);
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
      xml.tag("align", align);
      xml.tag("anchor", anchor);
      xml.tag("offsetType", offsetType);
      if (offsetType == OFFSET_ABS || offsetType == OFFSET_REL) {
            xml.tag("xoffset", xoff * INCH);
            xml.tag("yoffset", yoff * INCH);
            }
      else {
            xml.tag("xoffset", xoff);
            xml.tag("yoffset", yoff);
            }
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
                  align = i;
            else if (tag == "anchor")
                  anchor = (Anchor)i;
            else if (tag == "xoffset")
                  xoff = val.toDouble();
            else if (tag == "yoffset")
                  yoff = val.toDouble();
            else if (tag == "offsetType")
                  offsetType = (OffsetType)i;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

void Style::loadStyle(QDomElement e)
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
            else if (tag == "spacing16")
                  spacing16 = d;
            else if (tag == "spacing8")
                  spacing8 = d;
            else if (tag == "spacing4")
                  spacing4 = d;
            else if (tag == "spacing2")
                  spacing2 = d;
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
            else if (tag == "ticklen2Width")
                  ticklen2Width = d;
            else if (tag == "pageFillLimit")
                  pageFillLimit = d;
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
            else if (tag == "lyricsDistance")
                  lyricsDistance = Spatium(d);
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void Style::saveStyle(Xml& xml)
      {
      xml.stag("Style");

      xml.tag("staffUpperBorder",       staffUpperBorder.val());
      xml.tag("staffLowerBorder",       staffLowerBorder.val());
      xml.tag("staffDistance",          staffDistance.val());
      xml.tag("akkoladeDistance",       akkoladeDistance.val());
      xml.tag("systemDistance",         systemDistance.val());

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

      xml.tag("spacing16",              spacing16);
      xml.tag("spacing8",               spacing8);
      xml.tag("spacing4",               spacing4);
      xml.tag("spacing2",               spacing2);
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
      xml.tag("ticklen2Width",          ticklen2Width);
      xml.tag("pageFillLimit",          pageFillLimit);
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

      xml.tag("lyricsDistance",         lyricsDistance.val());

      xml.etag();
      }
