//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#ifndef __FONT_H__
#define __FONT_H__

#include <QtCore/QRectF>
#include <QtCore/QChar>
#include <QtCore/QString>

//---------------------------------------------------------
//   Font
//---------------------------------------------------------

class Font {
      QString _family;
      qreal _size;

   public:
      Font()                            {}
      Font(const QString& s)           { _family = s; }
      void setFamily(const QString& s) { _family = s; }
      void setBold(bool);
      void setItalic(bool);
      void setUnderline(bool);
      void setSize(qreal val)       { _size = val; }
      qreal size() const            { return _size; }
      QString family() const        { return _family; }
      };

//---------------------------------------------------------
//   FontMetrics
//---------------------------------------------------------

class FontMetricsF {
      Font _font;

   public:
//      FontMetricsF();
      FontMetricsF(const Font&);
      QRectF boundingRect(const QString&) const;
      QRectF boundingRect(const QChar&) const;
      QRectF tightBoundingRect(const QString& s);
      QRectF boundingRect(const QRectF&, int, const QString&);
      qreal width(const QChar&) const;
      qreal width(const QString&) const;
      qreal height() const;
      qreal lineSpacing() const;
      };

extern qreal textMetrics(const QString&, qreal fontsize, qreal*, qreal*, qreal*);
extern qreal textMetrics(const QString&, const QString&, qreal fontsize, qreal*, qreal*, qreal*);
#endif

