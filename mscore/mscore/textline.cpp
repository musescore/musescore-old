//=============================================================================
//  MuseScore
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

#include "textline.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "layout.h"
#include "score.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextLineSegment::draw(QPainter& p) const
      {
      qreal textlineLineWidth    = _spatium * .15;
      qreal textlineTextDistance = _spatium * .25;

      QPointF pp2(pos2());

      QRectF bb(textLine()->_text->bbox());
      qreal h = bb.height() * .5 - textlineLineWidth * .5;

      p.save();
      p.translate(0.0, -h);
      textLine()->_text->draw(p);
      p.restore();

      QPointF pp1(bb.width() + textlineTextDistance, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(textlineLineWidth);

      p.setPen(pen);
      p.drawLine(QLineF(pp1, pp2));
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END)
            p.drawLine(QLineF(pp2, QPointF(pp2.x(), h * .55)));
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF TextLineSegment::bbox() const
      {
      QPointF pp1;
      QPointF pp2(pos2());

      qreal h1 = textLine()->_text->height();
      QRectF r(.0, -h1, pp2.x(), 2 * h1);
      return r;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool TextLineSegment::startEdit(const QPointF& p)
      {
      bool r = textLine()->_text->startEdit(p);
      return r;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool TextLineSegment::edit(int curGrip, QKeyEvent* ev)
      {
      bool r1 = LineSegment::edit(curGrip, ev);
      bool r2 = textLine()->_text->edit(curGrip, ev);
      return r1 || r2;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void TextLineSegment::endEdit()
      {
      LineSegment::endEdit();
      textLine()->_text->endEdit();
      }

//---------------------------------------------------------
//   mousePress
//---------------------------------------------------------

bool TextLineSegment::mousePress(const QPointF& p, QMouseEvent* ev)
      {
      return textLine()->_text->mousePress(p + canvasPos(), ev);
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : SLine(s)
      {
      setSubtype(0);
      _text = new Text(s);
      _text->setParent(this);
      _text->setStyle(s->textStyle(TEXT_STYLE_TEXTLINE));
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _text = e._text->clone();
      }

TextLine::~TextLine()
      {
      delete _text;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void TextLine::setSubtype(int val)
      {
      Element::setSubtype(val);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLine::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);
      _text->layout(layout);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      xml.stag("TextLine");
      SLine::writeProperties(xml);
      _text->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
          QString tag(e.tagName());
          if (tag == "Text") {
              _text = new Text(score());
              _text->read(e);
          }
          else if (!SLine::readProperties(e))
              domError(e);
      }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      LineSegment* seg = new TextLineSegment(score());
      return seg;
      }

