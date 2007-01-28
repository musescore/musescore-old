//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: style.cpp,v 1.35 2006/09/15 09:34:57 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

//---------------------------------------------------------
//   textStyles
//---------------------------------------------------------

TextStyleList textStyles;

static const QString ff("Times New Roman");

#define MM(x) ((x)/INCH)

static const TextStyle defaultTextStyles[] = {
      TextStyle(QString("Symbols1"), QString("Emmentaler"), 20, false, false, false,
         ALIGN_LEFT,    ANCHOR_TICK, 0, 0, OFFSET_ABS),
      TextStyle(QString("Symbols3"), QString("Emmentaler"), 14, false, false, false,
         ALIGN_LEFT,    ANCHOR_TICK, 0, 0, OFFSET_SPATIUM),
      TextStyle(QString("Title"), ff, 24, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, ANCHOR_PAGE, 0, MM(5), OFFSET_ABS),
      TextStyle(QString("Subtitle"), ff, 14, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, ANCHOR_PAGE, 0, MM(15), OFFSET_ABS),
      TextStyle(QString("Composer"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_TOP, ANCHOR_PAGE, 0, MM(15), OFFSET_ABS),
      TextStyle(QString("Poet"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, ANCHOR_PAGE, 0, MM(10), OFFSET_ABS),
      TextStyle(QString("Lyrics"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, ANCHOR_TICK, 0, 7, OFFSET_SPATIUM),
      TextStyle(QString("Fingering"), ff,  6, false, false, false,
         ALIGN_HCENTER | ALIGN_VCENTER, ANCHOR_NOTE, 0, 0, OFFSET_SPATIUM),
      TextStyle(QString("InstrumentsLong"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, ANCHOR_SYSTEM, 0, 0, OFFSET_SPATIUM),
      TextStyle(QString("InstrumentsShort"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, ANCHOR_SYSTEM, 0, 0, OFFSET_SPATIUM),
      TextStyle(QString("Dynamics"), ff, 12, false, true, false,
         ALIGN_LEFT,    ANCHOR_TICK, 0, 6, OFFSET_SPATIUM),
      TextStyle(QString("Technik"), ff, 12, false, true, false,
         ALIGN_LEFT,    ANCHOR_TICK, 0, 0, OFFSET_SPATIUM),
/*12*/
      TextStyle(QString("Tempo"), ff, 10, true, false, false,
         ALIGN_LEFT,    ANCHOR_TICK, 0, -2.0, OFFSET_SPATIUM, true),
      TextStyle(QString("Metronome"), ff, 12, true, false, false,
         ALIGN_LEFT,    ANCHOR_TICK, 0, 0, OFFSET_SPATIUM),
      TextStyle(QString("Copyright"), ff, 8, true, false, false,
         ALIGN_HCENTER | ALIGN_BOTTOM,    ANCHOR_PAGE, 0, MM(1), OFFSET_ABS),
      TextStyle(QString("Measure Number"), ff, 8, false, false, false,
         ALIGN_LEFT,    ANCHOR_TICK, 0, -2.2, OFFSET_SPATIUM),
      TextStyle(QString("Page Number Odd"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_TOP, ANCHOR_PAGE, MM(-10), MM(10), OFFSET_ABS),
      TextStyle(QString("Page Number Even"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, ANCHOR_PAGE, MM(10), MM(10), OFFSET_ABS),
      TextStyle(QString("Translator"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP,    ANCHOR_TICK, 0, 6, OFFSET_SPATIUM),
      TextStyle(QString("Dynamics1"), QString("MScore1"), 20, false, false, false,
         ALIGN_LEFT, ANCHOR_TICK, 0, 0, OFFSET_SPATIUM),
      TextStyle(QString("Tuplets"), ff,  8, false, false, false,
         ALIGN_HCENTER | ALIGN_VCENTER, ANCHOR_NOTE, 0, 0, OFFSET_SPATIUM),
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

      1.5,              // measureSpacing

      Spatium(0.08),    // staff line width
      Spatium(0.16),    // helpLineWidth;
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
      };

//---------------------------------------------------------
//   setDefaultStyle
//---------------------------------------------------------

void setDefaultStyle()
      {
      style = &defaultStyle;
      int n = sizeof(defaultTextStyles)/sizeof(*defaultTextStyles);
      textStyles.clear();
      for (int i = 0; i < n; ++i)
            textStyles.push_back(defaultTextStyles[i]);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyle::font() const
      {
      double mag = ::_spatium / (spatiumBase20 * DPI);

      double m = size * SRM;
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
      xml.stag("TextStyle");
      xml.tag("name", name);
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
      xml.etag("TextStyle");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextStyle::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "name")
                  name = val;
            else if (tag == "family")
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
                  domError(node);
            }
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void setTextStyle(const TextStyle& ts)
      {
      for (iTextStyle i = textStyles.begin(); i != textStyles.end(); ++i) {
            if (i->name == ts.name) {
                  *i = ts;
                  return;
                  }
            }
      textStyles.push_back(ts);
      }

//---------------------------------------------------------
//   loadStyle
//---------------------------------------------------------

void loadStyle(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            double d = val.toDouble();

            if (tag == "TextStyle") {
                  TextStyle ts;
                  ts.read(node);
                  setTextStyle(ts);
                  }
            else if (tag == "staffUpperBorder")
                  style->staffUpperBorder = Spatium(d);
            else if (tag == "staffLowerBorder")
                  ::style->staffLowerBorder = Spatium(d);
            else if (tag == "staffDistance")
                  ::style->staffDistance = Spatium(d);
            else if (tag == "systemDistance")
                  ::style->systemDistance = Spatium(d);
            else if (tag == "minMeasureWidth")
                  ::style->minMeasureWidth = Spatium(d);
            else if (tag == "barWidth")
                  ::style->barWidth = Spatium(d);
            else if (tag == "doubleBarWidth")
                  ::style->doubleBarWidth = Spatium(d);
            else if (tag == "endBarWidth")
                  ::style->endBarWidth = Spatium(d);
            else if (tag == "doubleBarDistance")
                  ::style->doubleBarDistance = Spatium(d);
            else if (tag == "endBarDistance")
                  ::style->endBarDistance = Spatium(d);
            else if (tag == "bracketWidth")
                  ::style->bracketWidth = Spatium(d);
            else if (tag == "bracketDistance")
                  ::style->bracketDistance = Spatium(d);
            else if (tag == "clefLeftMargin")
                  ::style->clefLeftMargin = Spatium(d);
            else if (tag == "keysigLeftMargin")
                  ::style->keysigLeftMargin = Spatium(d);
            else if (tag == "timesigLeftMargin")
                  ::style->timesigLeftMargin = Spatium(d);
            else if (tag == "clefKeyRightMargin")
                  ::style->clefKeyRightMargin = Spatium(d);
            else if (tag == "stemWidth")
                  ::style->stemWidth = Spatium(d);
            else if (tag == "minNoteDistance")
                  ::style->minNoteDistance = Spatium(d);
            else if (tag == "spacing16")
                  ::style->spacing16 = d;
            else if (tag == "spacing8")
                  ::style->spacing8 = d;
            else if (tag == "spacing4")
                  ::style->spacing4 = d;
            else if (tag == "spacing2")
                  ::style->spacing2 = d;
            else if (tag == "measureSpacing")
                  ::style->measureSpacing = d;
            else if (tag == "barNoteDistance")
                  ::style->barNoteDistance = Spatium(d);
            else if (tag == "noteBarDistance")
                  ::style->noteBarDistance = Spatium(d);
            else if (tag == "staffLineWidth")
                  ::style->staffLineWidth = Spatium(d);
            else if (tag == "helpLineWidth")
                  ::style->helpLineWidth = Spatium(d);
            else if (tag == "akkoladeWidth")
                  ::style->akkoladeWidth = Spatium(d);
            else if (tag == "akkoladeDistance")
                  ::style->akkoladeDistance = Spatium(d);
            else if (tag == "prefixDistance")
                  ::style->prefixDistance = Spatium(d);
            else if (tag == "prefixNoteDistance")
                  ::style->prefixNoteDistance = Spatium(d);
            else if (tag == "beamWidth")
                  ::style->beamWidth = Spatium(d);
            else if (tag == "beamDistance")
                  ::style->beamDistance = d;
            else if (tag == "beamMinLen")
                  ::style->beamMinLen = Spatium(d);
            else if (tag == "beamMinSlope")
                  ::style->beamMinSlope = d;
            else if (tag == "beamMaxSlope")
                  ::style->beamMaxSlope = d;
            else if (tag == "maxBeamTicks")
                  ::style->maxBeamTicks = i;
            else if (tag == "dotNoteDistance")
                  ::style->dotNoteDistance = Spatium(d);
            else if (tag == "dotRestDistance")
                  ::style->dotRestDistance = Spatium(d);
            else if (tag == "dotDotDistance")
                  ::style->dotDotDistance = Spatium(d);
            else if (tag == "propertyDistanceHead")
                  ::style->propertyDistanceHead = Spatium(d);
            else if (tag == "propertyDistanceStem")
                  ::style->propertyDistanceStem = Spatium(d);
            else if (tag == "propertyDistance")
                  ::style->propertyDistance = Spatium(d);
            else if (tag == "ticklen2Width")
                  ::style->ticklen2Width = d;
            else if (tag == "pageFillLimit")
                  ::style->pageFillLimit = d;
            else if (tag == "hairpinHeight")
                  ::style->hairpinHeight = Spatium(d);
            else if (tag == "hairpinContHeight")
                  ::style->hairpinContHeight = Spatium(d);
            else if (tag == "hairpinWidth")
                  ::style->hairpinWidth = Spatium(d);
            else if (tag == "showPageNumber")
                  ::style->showPageNumber = i;
            else if (tag == "showPageNumberOne")
                  ::style->showPageNumberOne = i;
            else if (tag == "pageNumberOddEven")
                  ::style->pageNumberOddEven = i;
            else if (tag == "showMeasureNumber")
                  ::style->showMeasureNumber = i;
            else if (tag == "showMeasureNumberOne")
                  ::style->showMeasureNumberOne = i;
            else if (tag == "measureNumberInterval")
                  ::style->measureNumberInterval = i;
            else if (tag == "measureNumberSystem")
                  ::style->measureNumberSystem = i;
            else if (tag == "measureNumberAllStaffs")
                  ::style->measureNumberAllStaffs = i;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void saveStyle(Xml& xml)
      {
      xml.stag("Style");
//DEBUG      for (iTextStyle i = textStyles.begin(); i != textStyles.end(); ++i)
//           i->write(xml);

      xml.tag("staffUpperBorder",       style->staffUpperBorder.val());
      xml.tag("staffLowerBorder",       style->staffLowerBorder.val());
      xml.tag("staffDistance",          style->staffDistance.val());
      xml.tag("akkoladeDistance",       style->akkoladeDistance.val());
      xml.tag("systemDistance",         style->systemDistance.val());

      xml.tag("minMeasureWidth",        style->minMeasureWidth.val());
      xml.tag("barWidth",               style->barWidth.val());
      xml.tag("doubleBarWidth",         style->doubleBarWidth.val());
      xml.tag("endBarWidth",            style->endBarWidth.val());
      xml.tag("doubleBarDistance",      style->doubleBarDistance.val());
      xml.tag("endBarDistance",         style->endBarDistance.val());
      xml.tag("bracketWidth",           style->bracketWidth.val());
      xml.tag("bracketDistance",        style->bracketDistance.val());

      xml.tag("clefLeftMargin",         style->clefLeftMargin.val());
      xml.tag("keysigLeftMargin",       style->keysigLeftMargin.val());
      xml.tag("timesigLeftMargin",      style->timesigLeftMargin.val());
      xml.tag("clefKeyRightMargin",     style->clefKeyRightMargin.val());
      xml.tag("stemWidth",              style->stemWidth.val());
      xml.tag("minNoteDistance",        style->minNoteDistance.val());

      xml.tag("spacing16",              style->spacing16);
      xml.tag("spacing8",               style->spacing8);
      xml.tag("spacing4",               style->spacing4);
      xml.tag("spacing2",               style->spacing2);
      xml.tag("measureSpacing",         style->measureSpacing);

      xml.tag("barNoteDistance",        style->barNoteDistance.val());
      xml.tag("noteBarDistance",        style->noteBarDistance.val());
      xml.tag("staffLineWidth",         style->staffLineWidth.val());
      xml.tag("helpLineWidth",          style->helpLineWidth.val());
      xml.tag("akkoladeWidth",          style->akkoladeWidth.val());
      xml.tag("prefixDistance",         style->prefixDistance.val());
      xml.tag("prefixNoteDistance",     style->prefixNoteDistance.val());
      xml.tag("beamWidth",              style->beamWidth.val());
      xml.tag("beamDistance",           style->beamDistance);
      xml.tag("beamMinLen",             style->beamMinLen.val());
      xml.tag("beamMinSlope",        style->beamMinSlope);
      xml.tag("beamMaxSlope",        style->beamMaxSlope);
      xml.tag("maxBeamTicks",           style->maxBeamTicks);
      xml.tag("dotNoteDistance",        style->dotNoteDistance.val());
      xml.tag("dotRestDistance",        style->dotRestDistance.val());
      xml.tag("dotDotDistance",         style->dotDotDistance.val());
      xml.tag("propertyDistanceHead",   style->propertyDistanceHead.val());
      xml.tag("propertyDistanceStem",   style->propertyDistanceStem.val());
      xml.tag("propertyDistance",       style->propertyDistance.val());
      xml.tag("ticklen2Width",          style->ticklen2Width);
      xml.tag("pageFillLimit",          style->pageFillLimit);
      xml.tag("hairpinHeight",          style->hairpinHeight.val());
      xml.tag("hairpinContHeight",      style->hairpinContHeight.val());
      xml.tag("hairpinWidth",           style->hairpinWidth.val());

      xml.tag("showPageNumber",         style->showPageNumber);
      xml.tag("showPageNumberOne",      style->showPageNumberOne);
      xml.tag("pageNumberOddEven",      style->pageNumberOddEven);
      xml.tag("showMeasureNumber",      style->showMeasureNumber);
      xml.tag("showMeasureNumberOne",   style->showMeasureNumberOne);
      xml.tag("measureNumberInterval",  style->measureNumberInterval);
      xml.tag("measureNumberAllStaffs", style->measureNumberAllStaffs);
      xml.tag("measureNumberSystem",    style->measureNumberSystem);

      xml.etag("Style");
      }


