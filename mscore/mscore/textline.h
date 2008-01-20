//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "line.h"
#include "text.h"

class TextLine;

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

class TextLineSegment : public LineSegment {
   protected:

   public:
      TextLineSegment(Score* s) : LineSegment(s) {}
      virtual ElementType type() const       { return TEXTLINE_SEGMENT; }
      virtual TextLineSegment* clone() const { return new TextLineSegment(*this); }
      TextLine* textLine() const             { return (TextLine*)parent(); }
      virtual void draw(QPainter&) const;
      virtual QRectF bbox() const;
      virtual bool startEdit(const QPointF&);
      virtual bool edit(int, QKeyEvent*);
      virtual void endEdit();
      virtual bool mousePress(const QPointF&, QMouseEvent* ev);
      };

//---------------------------------------------------------
//   TextLine
//    brackets
//---------------------------------------------------------

class TextLine : public SLine {

   protected:
      Text* _text;
      mutable qreal textHeight;     ///< cached value

      friend class TextLineSegment;

   public:
      TextLine(Score* s);
      TextLine(const TextLine&);
      virtual ~TextLine();
      virtual TextLine* clone() const     { return new TextLine(*this); }
      virtual ElementType type() const { return TEXTLINE; }
      virtual void layout(ScoreLayout*);
      virtual void setSubtype(int val);
      virtual LineSegment* createLineSegment();
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      void setText(const QString& s) { _text->setText(s);    }
      QString text() const           { return _text->getText(); }
      };

#endif


