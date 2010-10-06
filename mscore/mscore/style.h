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
class ChordDescription;
class ChordList;

//---------------------------------------------------------
//   TextStyleData
//---------------------------------------------------------

class TextStyleData : public QSharedData {
   protected:
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
      QColor foregroundColor;

   public:
      TextStyleData(QString _name, QString _family, int _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         double _xoff, double _yoff, OffsetType _ot,
         double _rxoff, double _ryoff,
         bool sd,
         double fw, double pw, int fr,
         QColor co, bool circle, bool systemFlag,
         QColor fg);
      TextStyleData() {}

      void write(Xml&) const;
      void read(QDomElement);

      QFont font(double space) const;
      QFont fontPx(double spatium) const;
      QRectF bbox(double space, const QString& s) const { return fontMetrics(space).boundingRect(s); }
      QFontMetricsF fontMetrics(double space) const     { return QFontMetricsF(font(space)); }
      bool operator!=(const TextStyleData& s) const;
      friend class TextStyle;
      };

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

class TextStyle {
      QSharedDataPointer<TextStyleData> d;

   public:
      TextStyle();
      TextStyle(QString _name, QString _family, int _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         double _xoff = 0, double _yoff = 0, OffsetType _ot = OFFSET_SPATIUM,
         double _rxoff = 0, double _ryoff = 0,
         bool sd = false,
         double fw = 0.0, double pw = 0.0, int fr = 25,
         QColor co = QColor(Qt::black), bool circle = false, bool systemFlag = false,
         QColor fg = QColor(Qt::black));

      TextStyle(const TextStyle&);

      QString name() const                { return d->name; }
      QString family() const              { return d->family; }
      int size() const                    { return d->size; }
      bool bold() const                   { return d->bold; }
      bool italic() const                 { return d->italic; }
      bool underline() const              { return d->underline; }
      bool hasFrame() const               { return d->hasFrame; }
      Align align() const                 { return d->align; }
      double xoff() const                 { return d->xoff; }
      double yoff() const                 { return d->yoff; }
      OffsetType offsetType() const       { return d->offsetType; }
      double rxoff() const                { return d->rxoff; }
      double ryoff() const                { return d->ryoff; }
      bool sizeIsSpatiumDependent() const { return d->sizeIsSpatiumDependent; }

      double frameWidth()  const          { return d->frameWidth; }
      double paddingWidth() const         { return d->paddingWidth; }
      int frameRound() const              { return d->frameRound; }
      QColor frameColor() const           { return d->frameColor; }
      bool circle() const                 { return d->circle;     }
      bool systemFlag() const             { return d->systemFlag; }
      QColor foregroundColor() const      { return d->foregroundColor; }

      void setName(const QString& s)      { d->name = s; }
      void setFamily(const QString& s)    { d->family = s; }
      void setSize(int v)                 { d->size = v; }
      void setBold(bool v)                { d->bold = v; }
      void setItalic(bool v)              { d->italic = v; }
      void setUnderline(bool v)           { d->underline = v; }
      void setHasFrame(bool v)            { d->hasFrame = v; }
      void setAlign(Align v)              { d->align = v; }
      void setXoff(double v)              { d->xoff = v; }
      void setYoff(double v)              { d->yoff = v; }
      void setOffsetType(OffsetType v)    { d->offsetType = v; }
      void setRxoff(double v)             { d->rxoff = v; }
      void setRyoff(double v)             { d->ryoff = v; }
      void setSizeIsSpatiumDependent(bool v) { d->sizeIsSpatiumDependent = v; }

      void setFrameWidth(double v)        { d->frameWidth = v; }
      void setPaddingWidth(double v)      { d->paddingWidth = v; }
      void setFrameRound(int v)           { d->frameRound = v; }
      void setFrameColor(const QColor& v) { d->frameColor = v; }
      void setCircle(bool v)              { d->circle = v;     }
      void setSystemFlag(bool v)          { d->systemFlag = v; }
      void setForegroundColor(const QColor& v) { d->foregroundColor = v; }

      void write(Xml& xml) const                        { d->write(xml); }
      void read(QDomElement v)                          { d->read(v); }
      QFont font(double space) const                    { return d->font(space); }
      QFont fontPx(double spatium) const                { return d->fontPx(spatium); }
      QRectF bbox(double space, const QString& s) const { return d->bbox(space, s); }
      QFontMetricsF fontMetrics(double space) const     { return fontMetrics(space); }
      bool operator!=(const TextStyle& s) const         { return d != s.d; }
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

      // 74
      ST_useGermanNoteNames,
      //
      ST_chordDescriptionFile,
      ST_concertPitch,
      ST_createMultiMeasureRests,
      ST_minEmptyMeasures,
      ST_minMMRestWidth,
      ST_hideEmptyStaves,
      ST_stemDir1,
      ST_stemDir2,

      // 83
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

      // 101
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

      // 111
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
      ST_evenHeader,
      ST_oddHeader,

      ST_showFooter,
      ST_footerFirstPage,
      ST_footerOddEven,
      ST_evenFooter,
      ST_oddFooter,

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
//   StyleData
//    this structure contains all style elements
//---------------------------------------------------------

class StyleData : public QSharedData {
   protected:
      QVector<StyleVal> _values;
      mutable ChordList* _chordList;
      QList<TextStyle> _textStyles;

      void set(const StyleVal& v)                         { _values[v.getIdx()] = v; }
      StyleVal value(StyleIdx idx) const                  { return _values[idx];     }
      const TextStyle& textStyle(TextStyleType idx) const { return _textStyles[idx]; }
      const TextStyle& textStyle(const QString&) const;
      TextStyleType textStyleType(const QString&) const;
      void setTextStyle(const TextStyle& ts);

   public:
      StyleData();
      StyleData(const StyleData&);
      ~StyleData();

      bool load(QFile* qf);
      void load(QDomElement e);
      void save(Xml& xml, bool optimize) const;
      bool isDefault(StyleIdx) const;

      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList() const;
      friend class Style;
      };

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

class Style {
      QSharedDataPointer<StyleData> d;

   public:
      Style();
      Style(const Style&);

      void set(const StyleVal& v);
      void set(StyleIdx t, Spatium val)        { set(StyleVal(t, val)); }
      void set(StyleIdx t, const QString& val) { set(StyleVal(t, val)); }
      void set(StyleIdx t, bool val)           { set(StyleVal(t, val)); }
      void set(StyleIdx t, double val)         { set(StyleVal(t, val)); }
      void set(StyleIdx t, int val)            { set(StyleVal(t, val)); }
      void set(StyleIdx t, Direction val)      { set(StyleVal(t, val)); }

      StyleVal value(StyleIdx idx) const;
      Spatium  valueS(StyleIdx idx) const      { return value(idx).toSpatium(); }
      QString  valueSt(StyleIdx idx) const     { return value(idx).toString();  }
      bool     valueB(StyleIdx idx) const      { return value(idx).toBool();    }
      double   valueD(StyleIdx idx) const      { return value(idx).toDouble();  }
      int      valueI(StyleIdx idx) const      { return value(idx).toInt();     }

      bool load(QFile* qf)                     { return d->load(qf);            }
      void load(QDomElement e)                 { d->load(e);                    }
      void save(Xml& xml, bool optimize)       { d->save(xml, optimize);        }

      bool isDefault(StyleIdx idx) const;
      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList() const;
      const TextStyle& textStyle(TextStyleType idx) const   { return d->textStyle(idx); }
      const TextStyle& textStyle(const QString& name) const { return d->textStyle(name); }
      TextStyleType textStyleType(const QString& name) const { return d->textStyleType(name); }
      void setTextStyle(const TextStyle& ts)                { d->setTextStyle(ts); }
      void appendTextStyle(const TextStyle& ts)             { d->_textStyles.append(ts);  }
      const QList<TextStyle>& textStyles() const            { return d->_textStyles; }
      };

extern QVector<TextStyle> defaultTextStyles;
extern const TextStyle defaultTextStyleArray[];

extern Style* defaultStyle;
extern void setDefaultStyle();

#endif
