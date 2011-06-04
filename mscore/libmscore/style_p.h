//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#ifndef __STYLE_P_H__
#define __STYLE_P_H__

//
// private header for Style
//

#include <QtCore/QRectF>
#include "elementlayout.h"
#include "font.h"

class Xml;
class ChordDescription;
class ChordList;

//---------------------------------------------------------
//   TextStyleData
//---------------------------------------------------------

class TextStyleData : public QSharedData, public ElementLayout {
   protected:
      QString name;
      QString family;
      qreal size;
      bool bold;
      bool italic;
      bool underline;
      bool hasFrame;

      bool sizeIsSpatiumDependent;        // text point size depends on _spatium unit

      qreal frameWidth;
      qreal paddingWidth;
      int frameRound;
      Color frameColor;
      bool circle;
      bool systemFlag;
      Color foregroundColor;

   public:
      TextStyleData(QString _name, QString _family, qreal _size,
         bool _bold, bool _italic, bool _underline,
         Align _align,
         qreal _xoff, qreal _yoff, OffsetType _ot,
         qreal _rxoff, qreal _ryoff,
         bool sd,
         qreal fw, qreal pw, int fr,
         Color co, bool circle, bool systemFlag,
         Color fg);
      TextStyleData();

      void read(XmlReader*);
      bool readProperties(XmlReader* v);

      Font font(qreal space) const;
      Font fontPx(qreal spatium) const;
      QRectF bbox(qreal space, const QString& s) const { return fontMetrics(space).boundingRect(s); }
      FontMetricsF fontMetrics(qreal space) const     { return FontMetricsF(font(space)); }
      bool operator!=(const TextStyleData& s) const;
      friend class TextStyle;
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

      void load(XmlReader* e);
      bool isDefault(StyleIdx) const;

      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList() const;
      friend class MStyle;
      };

#endif
