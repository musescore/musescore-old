//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: style.h,v 1.11 2006/03/22 12:04:14 wschweer Exp $
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

#ifndef __STYLE_H__
#define __STYLE_H__

#include "globals.h"
#include "spatium.h"

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
      TEXT_STYLE_INSTRUMENT_EXCERPT,
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
      TEXT_STYLE_STAFF,
      TEXT_STYLE_CHORD,
      TEXT_STYLE_REHEARSAL_MARK,
      TEXT_STYLE_REPEAT,
      TEXT_STYLE_VOLTA,
      TEXT_STYLE_FRAME,
      TEXT_STYLE_TEXTLINE,
      TEXT_STYLE_GLISSANDO,
      TEXT_STYLES
      };

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

class TextStyle {
   public:
      QString name;
      QString family;
      int size;
      bool bold;
      bool italic;
      bool underline;
      Align align;
      double xoff, yoff;                  // absolute offset: inch or spatium
      OffsetType offsetType;
      double rxoff, ryoff;                // relative offset: % of parent width/height
      bool sizeIsSpatiumDependent;        // text point size depends on _spatium unit

      double frameWidth;
      double paddingWidth;
      int frameRound;
      QColor frameColor;
      bool circle;
      bool systemFlag;

      TextStyle(QString _name, QString _family, int _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         double _xoff = 0, double _yoff = 0, OffsetType _ot = OFFSET_SPATIUM,
         double _rxoff = 0, double _ryoff = 0,
         bool sd = false,
         double fw = 0.0, double pw = 0.0, int fr = 25,
         QColor co = QColor(Qt::black), bool circle = false, bool systemFlag = false);

      TextStyle() {}
      void write(Xml&) const;
      void read(QDomElement);
      QFont font() const;
      QRectF bbox(const QString& s) const { return fontMetrics().boundingRect(s); }
      QFontMetricsF fontMetrics() const   { return QFontMetricsF(font()); }
      bool operator!=(const TextStyle& s) const {
            return s.name != name
                || s.family != family
                || s.size != size
                || s.bold != bold
                || s.italic != italic
                || s.underline != underline
                || s.align != align
                || s.xoff   != xoff
                || s.yoff   != yoff
                || s.rxoff != rxoff
                || s.ryoff != ryoff
                || s.offsetType != offsetType
                || s.sizeIsSpatiumDependent != sizeIsSpatiumDependent
                || s.frameWidth != frameWidth
                || s.paddingWidth != paddingWidth
                || s.frameRound != frameRound
                || s.frameColor != frameColor
                || s.circle != circle
                || s.systemFlag != systemFlag;
            }
      };

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
      Spatium lyricsDistance;             // bottom staff to top of lyrics line
      Spatium lyricsMinBottomDistance;    // min. dist. between bottom of lyrics and next staff
      Spatium systemBoxDistance;          // dist. between staff and vertical box
      Spatium boxSystemDistance;          // dist. between vertical box and next system

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

      double measureSpacing;

      Spatium staffLineWidth;
      Spatium ledgerLineWidth;
      Spatium akkoladeWidth;
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

      double pageFillLimit;         // 0-1.0
      double lastSystemFillLimit;

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

      double smallNoteMag;
      double graceNoteMag;
      double smallStaffMag;
      double smallClefMag;

      bool genClef;           // create clef for all systems, not only for first
      bool genKeysig;         // create key signature for all systems
      bool genTimesig;
      bool genCourtesyTimesig;

      bool useGermanNoteNames;
      bool chordNamesUseSymbols;

      bool concertPitch;      // display transposing instruments in concert pitch
      bool createMultiMeasureRests;
      bool hideEmptyStaves;

      //---------------------------------------------------------
      //   PlayStyle
      //---------------------------------------------------------

      int gateTime;           // 0-100%
      int tenutoGateTime;
      int staccatoGateTime;
      int slurGateTime;


      void load(QDomElement e, int version);
      void save(Xml& xml);
      };


extern QVector<TextStyle> defaultTextStyles;
extern const TextStyle defaultTextStyleArray[];

extern Style defaultStyle;
extern void setDefaultStyle();

#endif
