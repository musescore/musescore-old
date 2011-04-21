//=============================================================================
//  MusE Score
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

#ifndef __STYLE_H__
#define __STYLE_H__

#include "globals.h"
#include "spatium.h"

class Xml;
struct ChordDescription;
class ChordList;
class Element;

class TextStyleData;

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

class TextStyle {
      QSharedDataPointer<TextStyleData> d;

   public:
      TextStyle();
      TextStyle(QString _name, QString _family,
         double _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         double _xoff = 0, double _yoff = 0, OffsetType _ot = OFFSET_SPATIUM,
         double _rxoff = 0, double _ryoff = 0,
         bool sd = false,
         double fw = 0.0, double pw = 0.0, int fr = 25,
         QColor co = QColor(Qt::black), bool circle = false, bool systemFlag = false,
         QColor fg = QColor(Qt::black));

      TextStyle(const TextStyle&);
      ~TextStyle();
      TextStyle& operator=(const TextStyle&);

      QString name() const;
      QString family() const;
      double size() const;
      bool bold() const;
      bool italic() const;
      bool underline() const;
      bool hasFrame() const;
      Align align() const;
      double xoff() const;
      double yoff() const;
      OffsetType offsetType() const;
      double rxoff() const;
      double ryoff() const;
      QPointF reloff() const;
      void setReloff(const QPointF& p);
      bool sizeIsSpatiumDependent() const;
      double frameWidth()  const;
      double paddingWidth() const;
      int frameRound() const;
      QColor frameColor() const;
      bool circle() const;
      bool systemFlag() const;
      QColor foregroundColor() const;
      void setName(const QString& s);
      void setFamily(const QString& s);
      void setSize(double v);
      void setBold(bool v);
      void setItalic(bool v);
      void setUnderline(bool v);
      void setHasFrame(bool v);
      void setAlign(Align v);
      void setXoff(double v);
      void setYoff(double v);
      void setOffsetType(OffsetType v);
      void setRxoff(double v);
      void setRyoff(double v);
      void setSizeIsSpatiumDependent(bool v);
      void setFrameWidth(double v);
      void setPaddingWidth(double v);
      void setFrameRound(int v);
      void setFrameColor(const QColor& v);
      void setCircle(bool v);
      void setSystemFlag(bool v);
      void setForegroundColor(const QColor& v);
      void write(Xml& xml) const;
      void writeProperties(Xml& xml) const;
      void read(QDomElement v);
      bool readProperties(QDomElement v);
      QFont font(double space) const;
      QFont fontPx(double spatium) const;
      QRectF bbox(double space, const QString& s) const;
      QFontMetricsF fontMetrics(double space) const;
      bool operator!=(const TextStyle& s) const;
      void layout(Element*) const;
      void setFont(const QFont& f);
      };

//---------------------------------------------------------
//   StyleValueType
//---------------------------------------------------------

enum StyleValueType {
      ST_SPATIUM, ST_DOUBLE, ST_BOOL, ST_INT, ST_DIRECTION, ST_STRING
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
//   StyleIdx
//---------------------------------------------------------

enum StyleIdx {
      ST_staffUpperBorder,
      ST_staffLowerBorder,
      ST_staffDistance,
      ST_akkoladeDistance,
      ST_systemDistance,
      ST_lyricsDistance,
      ST_lyricsMinBottomDistance,
      ST_lyricsLineHeight,
      ST_systemFrameDistance,
      ST_frameSystemDistance,
      ST_minMeasureWidth,

      ST_barWidth,
      ST_doubleBarWidth,
      ST_endBarWidth,
      ST_doubleBarDistance,
      ST_endBarDistance,
      ST_repeatBarTips,
      ST_startBarlineSingle,
      ST_startBarlineMultiple,

      ST_bracketWidth,
      ST_bracketDistance,
      ST_clefLeftMargin,
      ST_keysigLeftMargin,
      ST_timesigLeftMargin,

      ST_clefKeyRightMargin,
      ST_clefBarlineDistance,
      ST_stemWidth,
      ST_shortenStem,
      ST_shortStemProgression,
      ST_shortestStem,
      ST_beginRepeatLeftMargin,
      ST_minNoteDistance,
      ST_barNoteDistance,
      ST_noteBarDistance,           // used??

      ST_measureSpacing,
      ST_staffLineWidth,
      ST_ledgerLineWidth,
      ST_ledgerLineLength,
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
      ST_genCourtesyClef,

      // 75
      ST_useGermanNoteNames,
      //
      ST_chordDescriptionFile,
      ST_concertPitch,
      ST_createMultiMeasureRests,
      ST_minEmptyMeasures,
      ST_minMMRestWidth,
      ST_hideEmptyStaves,
      ST_dontHideStavesInFirstSystem,
      ST_stemDir1,
      ST_stemDir2,

      // 84
      ST_stemDir3,
      ST_stemDir4,
      ST_gateTime,
      ST_tenutoGateTime,
      ST_staccatoGateTime,
      ST_slurGateTime,

      ST_UfermataAnchor,
      ST_DfermataAnchor,
      ST_UshortfermataAnchor,
      ST_DshortfermataAnchor,
      ST_UlongfermataAnchor,
      ST_DlongfermataAnchor,
      ST_UverylongfermataAnchor,
      ST_DverylongfermataAnchor,
      ST_ThumbAnchor,
      ST_SforzatoaccentAnchor,
      ST_EspressivoAnchor,
      ST_StaccatoAnchor,

      // 102
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

      // 112
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

      // 121
      ST_UpMordentAnchor,
      ST_DownMordentAnchor,
      ST_SnappizzicatoAnchor,
      ST_TappingAnchor,
      ST_SlappingAnchor,
      ST_PoppingAnchor,

      ST_ArpeggioNoteDistance,
      ST_ArpeggioLineWidth,
      ST_ArpeggioHookLen,
      ST_FixMeasureNumbers,
      ST_FixMeasureWidth,

      ST_SlurEndWidth,
      ST_SlurMidWidth,
      ST_SlurDottedWidth,
      ST_SlurBow,

      ST_SectionPause,
      ST_MusicalSymbolFont,

      ST_showHeader,
      ST_headerFirstPage,
      ST_headerOddEven,
      ST_evenHeaderL,
      ST_evenHeaderC,
      ST_evenHeaderR,
      ST_oddHeaderL,
      ST_oddHeaderC,
      ST_oddHeaderR,

      ST_showFooter,
      ST_footerFirstPage,
      ST_footerOddEven,
      ST_evenFooterL,
      ST_evenFooterC,
      ST_evenFooterR,
      ST_oddFooterL,
      ST_oddFooterC,
      ST_oddFooterR,

      ST_STYLES
      };

//---------------------------------------------------------
//   StyleVal
//---------------------------------------------------------

class StyleVal {
      StyleIdx idx;
      QString   s;
      union {
            double  dbl;
            bool      b;
            int       i;
            Direction d;
            } v;

   public:
      StyleVal()                  { idx = StyleIdx(-1); }
      StyleVal(const StyleVal& val);
      StyleVal& operator=(const StyleVal& val);

      StyleVal(StyleIdx t, Spatium val);
      StyleVal(StyleIdx t, double val);
      StyleVal(StyleIdx t, bool val);
      StyleVal(StyleIdx t, int val);
      StyleVal(StyleIdx t, Direction val);
      StyleVal(StyleIdx t, const QString& val);

      Spatium toSpatium() const       { return Spatium(v.dbl); }
      double toDouble() const         { return v.dbl;  }
      bool toBool() const             { return v.b;  }
      int toInt() const               { return v.i;  }
      QString toString() const        { return s;    }
      Direction toDirection() const   { return v.d;  }
      StyleIdx getIdx() const         { return idx;  }
      StyleVal(const QString& name, const QString& val);
      };

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

class StyleData;

class Style {
      QSharedDataPointer<StyleData> d;

   public:
      Style();
      Style(const Style&);
      Style& operator=(const Style&);
      ~Style();

      bool isDefault(StyleIdx idx) const;
      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList() const;
      void setChordList(ChordList*);      // Style gets ownership of ChordList

      const TextStyle& textStyle(TextStyleType idx) const;
      const TextStyle& textStyle(const QString& name) const;
      TextStyleType textStyleType(const QString& name) const;
      void setTextStyle(const TextStyle& ts);
      void addTextStyle(const TextStyle& ts);
      void removeTextStyle(const TextStyle& ts);
      const QList<TextStyle>& textStyles() const;
      void set(StyleIdx t, Spatium val);
      void set(StyleIdx t, const QString& val);
      void set(StyleIdx t, bool val);
      void set(StyleIdx t, double val);
      void set(StyleIdx t, int val);
      void set(StyleIdx t, Direction val);
      void set(const StyleVal& v);

      StyleVal value(StyleIdx idx) const;
      Spatium  valueS(StyleIdx idx) const;
      QString  valueSt(StyleIdx idx) const;
      bool     valueB(StyleIdx idx) const;
      double   valueD(StyleIdx idx) const;
      int      valueI(StyleIdx idx) const;

      bool load(QFile* qf);
      void load(QDomElement e);
      void save(Xml& xml, bool optimize);
      };

extern QVector<TextStyle> defaultTextStyles;
extern const TextStyle defaultTextStyleArray[];

extern void setDefaultStyle(Style*);

#endif
