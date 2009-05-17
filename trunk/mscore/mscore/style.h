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
      TEXT_STYLE_COPYRIGHT,
      TEXT_STYLE_MEASURE_NUMBER,
      TEXT_STYLE_PAGE_NUMBER_ODD,
      TEXT_STYLE_PAGE_NUMBER_EVEN,
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
      bool hasFrame;
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
      QFont font(double space) const;
      QFont fontPx() const;
      QRectF bbox(double space, const QString& s) const { return fontMetrics(space).boundingRect(s); }
      QFontMetricsF fontMetrics(double space) const { return QFontMetricsF(font(space)); }
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
//   StyleValueType
//---------------------------------------------------------

enum StyleValueType {
      ST_SPATIUM, ST_DOUBLE, ST_BOOL, ST_INT, ST_DIRECTION
      };

//---------------------------------------------------------
//   StyleType
//---------------------------------------------------------

class StyleType {
      const char* _name;       // xml name for read()/write()
      StyleValueType _valueType;

   public:
      StyleType(const char* n, StyleValueType v) : _name(n), _valueType(v) {}
      StyleValueType valueType() const { return _valueType; }
      const char* name() const         { return _name; }
      };

//---------------------------------------------------------
//   StyleVal
//---------------------------------------------------------

class StyleVal {
      union {
            double  dbl;
            bool      b;
            int       i;
            Direction d;
            } v;
   public:
      StyleVal()                    {}
      StyleVal(Spatium val)         { v.dbl = val.val(); }
      StyleVal(double val)          { v.dbl = val;     }
      StyleVal(bool val)            { v.b   = val;     }
      StyleVal(int val)             { v.i   = val;     }
      StyleVal(Direction val)       { v.d   = val;     }

      Spatium toSpatium() const     { return Spatium(v.dbl); }
      double toDouble() const       { return v.dbl;  }
      bool toBool() const           { return v.b;  }
      int toInt() const             { return v.i;  }
      Direction toDirection() const { return v.d;  }
      };

enum STYLE_TYPE {
      ST_staffUpperBorder,
      ST_staffLowerBorder,
      ST_staffDistance,
      ST_akkoladeDistance,
      ST_systemDistance,
      ST_lyricsDistance,
      ST_lyricsMinBottomDistance,
      ST_systemFrameDistance,
      ST_frameSystemDistance,
      ST_minMeasureWidth,

      ST_barWidth,
      ST_doubleBarWidth,
      ST_endBarWidth,
      ST_doubleBarDistance,
      ST_endBarDistance,
      ST_bracketWidth,
      ST_bracketDistance,
      ST_clefLeftMargin,
      ST_keysigLeftMargin,
      ST_timesigLeftMargin,

      ST_clefKeyRightMargin,
      ST_stemWidth,
      ST_shortenStem,
      ST_shortStemProgression,
      ST_shortestStem,
      ST_beginRepeatLeftMargin,
      ST_minNoteDistance,
      ST_barNoteDistance,
      ST_noteBarDistance,
      ST_measureSpacing,
      ST_staffLineWidth,
      ST_ledgerLineWidth,
      ST_akkoladeWidth,

      ST_accidentalDistance,
      ST_accidentalNoteDistance,
      ST_beamWidth,
      ST_beamDistance,
      ST_beamMinLen,
      ST_beamMinSlope,
      ST_beamMaxSlope,
      ST_maxBeamTicks,
      ST_dotNoteDistance,
      ST_dotRestDistance,

      ST_dotDotDistance,
      ST_propertyDistanceHead,
      ST_propertyDistanceStem,
      ST_propertyDistance,
      ST_pageFillLimit,
      ST_lastSystemFillLimit,
      ST_hairpinHeight,
      ST_hairpinContHeight,
      ST_hairpinWidth,
      ST_showPageNumber,

      ST_showPageNumberOne,
      ST_pageNumberOddEven,
      ST_showMeasureNumber,
      ST_showMeasureNumberOne,
      ST_measureNumberInterval,
      ST_measureNumberSystem,
      ST_measureNumberAllStaffs,
      ST_smallNoteMag,
      ST_graceNoteMag,
      ST_smallStaffMag,

      ST_smallClefMag,
      ST_genClef,
      ST_genKeysig,
      ST_genTimesig,
      ST_genCourtesyTimesig,
      ST_genCourtesyKeysig,
      ST_useGermanNoteNames,
      ST_warnPitchRange,
      ST_chordNamesUseSymbols,
      ST_chordNamesUseJazzFont,
      ST_concertPitch,

      ST_createMultiMeasureRests,
      ST_minEmptyMeasures,
      ST_minMMRestWidth,
      ST_hideEmptyStaves,
      ST_stemDir1,
      ST_stemDir2,
      ST_stemDir3,
      ST_stemDir4,
      ST_gateTime,
      ST_tenutoGateTime,

      ST_staccatoGateTime,
      ST_slurGateTime,        // 81

      ST_UfermataAnchor,
      ST_DfermataAnchor,
      ST_ThumbAnchor,
      ST_SforzatoaccentAnchor,
      ST_EspressivoAnchor,
      ST_StaccatoAnchor,
      ST_UstaccatissimoAnchor,
      ST_DstaccatissimoAnchor,
      ST_TenutoAnchor,
      ST_UportatoAnchor,
      ST_DportatoAnchor,
      ST_UmarcatoAnchor,
      ST_DmarcatoAnchor,
      ST_OuvertAnchor,
      ST_PlusstopAnchor,
      ST_UpbowAnchor,
      ST_DownbowAnchor,
      ST_ReverseturnAnchor,
      ST_TurnAnchor,
      ST_TrillAnchor,
      ST_PrallAnchor,
      ST_MordentAnchor,
      ST_PrallPrallAnchor,
      ST_PrallMordentAnchor,
      ST_UpPrallAnchor,
      ST_DownPrallAnchor,
      ST_UpMordentAnchor,
      ST_DownMordentAnchor,

      ST_STYLES
      };

//---------------------------------------------------------
//   Style
//    this structure contains all style elements
//---------------------------------------------------------

class Style : public QVector<StyleVal> {

   public:
      Style();
      void load(QDomElement e, int version);
      void save(Xml& xml, bool optimize);
      bool isDefault(int);
      };

extern QVector<TextStyle> defaultTextStyles;
extern const TextStyle defaultTextStyleArray[];

extern Style defaultStyle;
extern void setDefaultStyle();

#endif
