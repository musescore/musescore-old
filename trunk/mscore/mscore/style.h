//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: style.h,v 1.11 2006/03/22 12:04:14 wschweer Exp $
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

#ifndef __STYLE_H__
#define __STYLE_H__

#include "spatium.h"

enum Align  { ALIGN_LEFT=1, ALIGN_RIGHT=2, ALIGN_HCENTER=4, ALIGN_TOP=8,
      ALIGN_BOTTOM=16, ALIGN_VCENTER=32};

enum Anchor { ANCHOR_PAGE, ANCHOR_STAFF, ANCHOR_NOTE, ANCHOR_SYSTEM };

enum OffsetType { OFFSET_ABS, OFFSET_REL, OFFSET_SPATIUM };

class Xml;

enum TEXT_STYLE {
      TEXT_STYLE_SYMBOL1 = 0,
      TEXT_STYLE_SYMBOL3,
      TEXT_STYLE_TITLE,
      TEXT_STYLE_SUBTITLE,
      TEXT_STYLE_COMPOSER,
      TEXT_STYLE_POET,
      TEXT_STYLE_LYRIC,
      TEXT_STYLE_FINGERING,
      TEXT_STYLE_INSTRUMENT_LONG,
      TEXT_STYLE_INSTRUMENT_SHORT,
      TEXT_STYLE_DYNAMICS,
      TEXT_STYLE_TECHNIK,
      TEXT_STYLE_TEMPO,
      TEXT_STYLE_METRONOME,
      TEXT_STYLE_COPYRIGHT,
      TEXT_STYLE_MEASURE_NUMBER,
      TEXT_STYLE_PAGE_NUMBER_ODD,
      TEXT_STYLE_PAGE_NUMBER_EVEN,
      TEXT_STYLE_TRANSLATOR,
      TEXT_STYLE_DYNAMICS1,
      TEXT_STYLE_TUPLET,
      TEXT_STYLE_SYSTEM,
      TEXT_STYLE_STAFF
      };

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

struct TextStyle {
      QString name;
      QString family;
      int size;
      bool bold;
      bool italic;
      bool underline;
      int align;
      Anchor anchor;
      double xoff, yoff;                  // inch or spatium
      OffsetType offsetType;
      bool sizeIsSpatiumDependent;        // size depends on _spatium unit
      double frameWidth;
      double marginWidth;
      double paddingWidth;
      int frameRound;
      QColor frameColor;

      TextStyle(QString _name, QString _family, int _size,
         bool _bold, bool _italic, bool _underline,
         int _align, Anchor _anchor,
         double _xoff, double _yoff, OffsetType _ot, bool sd = false,
         double fw = 0.0, double mw = 0.0, double pw = 0.0, int fr = 25,
         QColor co = QColor(Qt::black));
      TextStyle() {}
      void write(Xml&) const;
      void read(QDomElement);
      QFont font() const;
      QRectF bbox(const QString& s) const { return fontMetrics().boundingRect(s); }
      QFontMetricsF fontMetrics() const   { return QFontMetricsF(font()); }
      };

typedef std::vector<TextStyle> TextStyleList;
typedef TextStyleList::iterator iTextStyle;
typedef TextStyleList::const_iterator ciTextStyle;

//---------------------------------------------------------
//   Style
//    this structure contains all style elements
//---------------------------------------------------------

struct Style {
      Spatium staffUpperBorder;
      Spatium staffLowerBorder;
      Spatium staffDistance;
      Spatium accoladeDistance;
      Spatium systemDistance;

      Spatium minMeasureWidth;
      Spatium barWidth;
      Spatium doubleBarWidth;
      Spatium endBarWidth;
      Spatium doubleBarDistance;
      Spatium endBarDistance;
      Spatium bracketWidth;         // system bracket width
      Spatium bracketDistance;      // system bracket distance

      Spatium clefLeftMargin;
      Spatium keysigLeftMargin;
      Spatium timesigLeftMargin;
      Spatium clefKeyRightMargin;
      Spatium stemWidth;

      Spatium minNoteDistance;
      Spatium barNoteDistance;
      Spatium noteBarDistance;

      double spacing16;
      double spacing8;
      double spacing4;
      double spacing2;
      double measureSpacing;

      Spatium staffLineWidth;
      Spatium ledgerLineWidth;
      Spatium akkoladeWidth;
      Spatium akkoladeDistance;
      Spatium prefixDistance;
      Spatium prefixNoteDistance;
      Spatium beamWidth;
      double beamDistance;          // in beamWidth units
      Spatium beamMinLen;           // len for broken beams
      double beamMinSlope;
      double beamMaxSlope;
      int maxBeamTicks;
      Spatium dotNoteDistance;
      Spatium dotRestDistance;
      Spatium dotDotDistance;
      Spatium propertyDistanceHead;  // note property to note head
      Spatium propertyDistanceStem;  // note property to note stem
      Spatium propertyDistance;      // note property to note property
      double ticklen2Width;         // 1.0 - with of elements is proportional to
                                    // ticklen
      double pageFillLimit;         // 0-1.0
      Spatium hairpinHeight;
      Spatium hairpinContHeight;
      Spatium hairpinWidth;

      bool showPageNumber;
      bool showPageNumberOne;
      bool pageNumberOddEven;
      bool showMeasureNumber;
      bool showMeasureNumberOne;
      int measureNumberInterval;
      bool measureNumberSystem;
      bool measureNumberAllStaffs;
      };

extern void setDefaultStyle();
extern void setTextStyle(const TextStyle& ts);
extern void loadStyle(QDomElement);
extern void saveStyle(Xml& xml);

extern Style* style;
extern TextStyleList textStyles;
#endif
