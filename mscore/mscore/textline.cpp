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
//   TextLineSegment
//---------------------------------------------------------

TextLineSegment::TextLineSegment(Score* s)
   : LineSegment(s)
      {
      _text = 0;
      }

TextLineSegment::TextLineSegment(const TextLineSegment& seg)
   : LineSegment(seg)
      {
      _text = 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TextLineSegment::add(Element* e)
      {
      if (e->type() != TEXT) {
            printf("TextLineSegment: add illegal element\n");
            return;
            }
      _text = (TextC*)e;
      _text->setParent(this);
      TextLine* tl = (TextLine*)line();

      TextBase* tb = 0;
      if (_text->otb()) {
            tb = _text->otb();
            _text->setOtb(0);
            }
      else {
            tb = new TextBase(*tl->textBase());
            }
      tl->setTextBase(tb);
      _text->baseChanged();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TextLineSegment::remove(Element* e)
      {
      if (e != _text) {
            printf("TextLineSegment: cannot remove %s %p %p\n", e->name(), e, _text);
            return;
            }
      _text->setOtb(_text->textBase());
      _text = 0;
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void TextLineSegment::collectElements(QList<const Element*>& el) const
      {
      if (_text)
            el.append(_text);
      el.append(this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextLineSegment::draw(QPainter& p) const
      {
      qreal textlineLineWidth    = _spatium * .15;
      qreal textlineTextDistance = _spatium * .25;

      QPointF pp2(pos2());

      qreal h = 0.0;
      qreal w = 0.0;
      if (_text) {
            QRectF bb(_text->bbox());
            h = bb.height() * .5 - textlineLineWidth * .5;
            w = bb.width();
            }

      QPointF pp1(w + textlineTextDistance, 0.0);

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

      qreal h1 = _text ? _text->height() : 20.0;
      QRectF r(.0, -h1, pp2.x(), 2 * h1);
      return r;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout(ScoreLayout* l)
      {
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN) {
            if (_text == 0) {
                  TextLine* tl = (TextLine*)line();
                  _text = new TextC(tl->textBasePtr(), score());
                  _text->setSubtype(TEXT_TEXTLINE);
                  _text->setMovable(false);
                  _text->setParent(this);
                  }
            _text->layout(l);
            QRectF bb(_text->bbox());
            qreal textlineLineWidth = _spatium * .15;
            qreal h = bb.height() * .5 - textlineLineWidth * .5;
            _text->setPos(0.0, -h);
            }
      else if (_text) {
            delete _text;
            _text = 0;
            }
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : SLine(s)
      {
      _text = new TextBase;
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _text = e._text;
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::~TextLine()
      {
      // TextLine has no ownership of _text
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLine::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      xml.stag("TextLine");
      SLine::writeProperties(xml);
      _text->writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (_text->readProperties(e))
                  ;
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

