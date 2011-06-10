//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include <math.h>

#include "textline.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "m-al/xml.h"
#include "utils.h"
#include "score.h"
#include "preferences.h"
#include "sym.h"
#include "text.h"
#include "painter.h"

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
      layout();    // set right _text
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineSegment::setSelected(bool f)
      {
      Element::setSelected(f);
      if (_text) {
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
                  if (textLine()->beginText())
                        _text->setSelected(f);
                  }
            else if (textLine()->continueText())
                  _text->setSelected(f);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextLineSegment::draw(Painter* p) const
      {
      TextLine* tl    = textLine();
      qreal _spatium = spatium();

      qreal textlineLineWidth    = tl->lineWidth().val() * _spatium;
      qreal textlineTextDistance = _spatium * .5;

      QPointF pp2(pos2());

      qreal l = 0.0;
      int sym = spannerSegmentType() == SEGMENT_MIDDLE ? tl->continueSymbol() : tl->beginSymbol();
      if (_text) {
            if (
               ((spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) && (tl->beginTextPlace() == PLACE_LEFT))
               || ((spannerSegmentType() == SEGMENT_MIDDLE || spannerSegmentType() == SEGMENT_END) && (tl->continueTextPlace() == PLACE_LEFT))
               ) {
                  QRectF bb(_text->bbox());
                  l = _text->pos().x() + bb.width() + textlineTextDistance;
                  }
            p->save();
            p->translate(_text->pos());
            p->setPenColor(_text->curColor());
            _text->draw(p);
            p->restore();
            }
      else if (sym != -1) {
            const QRectF& bb = symbols[score()->symIdx()][sym].bbox(magS());
            qreal h = bb.height() * .5;
            QPointF o = tl->beginSymbolOffset() * _spatium;
            symbols[score()->symIdx()][sym].draw(p, 1.0, o.x(), h + o.y());
            l = bb.width() + textlineTextDistance;
            }
      if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END) {
            if (tl->endSymbol() != -1) {
                  int sym = tl->endSymbol();
                  const QRectF& bb = symbols[score()->symIdx()][sym].bbox(magS());
                  qreal h = bb.height() * .5;
                  QPointF o = tl->endSymbolOffset() * _spatium;
                  pp2.setX(pp2.x() - bb.width() + textlineTextDistance);
                  symbols[score()->symIdx()][sym].draw(p, 1.0, pp2.x() + textlineTextDistance + o.x(), h + o.y());
                  }
            }

      QPointF pp1(l, 0.0);

      p->setLineStyle(tl->lineStyle());

      if (selected() && !(score() && score()->printing()))
            p->setPenColor(preferences.selectColor[0]);
      else if (!visible())
            p->setPenColor(Color(40, 40, 40));    // Qt::gray);
      else
            p->setPenColor(tl->lineColor());

      p->setPenWidth(textlineLineWidth);

      if (tl->beginHook() && tl->beginHookType() == HOOK_45)
            pp1.rx() += fabs(tl->beginHookHeight().val() * _spatium * .4);
      if (tl->endHook() && tl->endHookType() == HOOK_45)
            pp2.rx() -= fabs(tl->endHookHeight().val() * _spatium * .4);
      p->drawLine(pp1.x(), pp1.y(), pp2.x(), pp2.y());

      if (tl->beginHook()) {
            qreal hh = tl->beginHookHeight().val() * _spatium;
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
                  if (tl->beginHookType() == HOOK_45)
                        p->drawLine(pp1.x(), pp1.y(), pp1.x() - fabs(hh * .4), pp1.y() + hh);
                  else
                        p->drawLine(pp1.x(), pp1.y(), pp1.x(), pp1.y() + hh);
                  }
            }
      if (tl->endHook()) {
            qreal hh = tl->endHookHeight().val() * _spatium;
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END) {
                  if (tl->endHookType() == HOOK_45)
                        p->drawLine(pp2.x(), pp2.y(), pp2.x() + fabs(hh * .4), pp2.y() + hh);
                  else
                        p->drawLine(pp2.x(), pp2.y(), pp2.x(), pp2.y() + hh);
                  }
            }
      }

//---------------------------------------------------------
//   bbox
//    FIXME
//---------------------------------------------------------

QRectF TextLineSegment::bbox() const
      {
      QPointF pp1;
      QPointF pp2(pos2());

      if (!_text && pp2.y() != 0)
            return QRectF(pp1, pp2).normalized();
      qreal y1 = point(-textLine()->lineWidth());
      qreal y2 = -y1;

      int sym = textLine()->beginSymbol();
      if (_text) {
            qreal h = _text->height();
            if (textLine()->beginTextPlace() == PLACE_ABOVE)
                  y1 = -h;
            else if (textLine()->beginTextPlace() == PLACE_BELOW)
                  y2 = h;
            else {
                  y1 = -h * .5;
                  y2 = h * .5;
                  }
            }
      else if (sym != -1) {
            qreal hh = symbols[score()->symIdx()][sym].height(magS()) * .5;
            y1 = -hh;
            y2 = hh;
            }
      if (textLine()->endHook()) {
            qreal h = point(textLine()->endHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      if (textLine()->beginHook()) {
            qreal h = point(textLine()->beginHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      return QRectF(.0, y1, pp2.x(), y2 - y1);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout()
      {
      TextLine* tl = (TextLine*)line();
      if (!tl->diagonal())
            _userOff2.setY(0);
      switch (spannerSegmentType()) {
            case SEGMENT_SINGLE:
            case SEGMENT_BEGIN:
                  if (tl->beginText()) {
                        if (_text == 0) {
                              _text = new Text(*tl->beginText());
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
                              }
                        }
                  else {
                        delete _text;
                        _text = 0;
                        }
                  break;
            case SEGMENT_MIDDLE:
            case SEGMENT_END:
                  if (tl->continueText()) {
                        if (_text == 0) {
                              _text = new Text(*tl->continueText());
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
                              }
                        }
                  else {
                        delete _text;
                        _text = 0;
                        }
                  break;
            }
      if (_text)
            _text->layout();
      }

//---------------------------------------------------------
//   clearText
//---------------------------------------------------------

void TextLineSegment::clearText()
      {
      delete _text;
      _text = 0;
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : SLine(s)
      {
      _beginText         = 0;
      _continueText      = 0;

      _beginHookHeight   = Spatium(1.5);
      _endHookHeight     = Spatium(1.5);
      _beginHook         = false;
      _endHook           = false;
      _beginHookType     = HOOK_90;
      _endHookType       = HOOK_90;

      _lineWidth         = Spatium(0.15);
      _lineStyle         = Qt::SolidLine;
      _beginTextPlace    = PLACE_LEFT;
      _continueTextPlace = PLACE_LEFT;
      _lineColor         = Color(0, 0, 0);
      _mxmlOff2          = 0;

      _beginSymbol       = -1;
      _continueSymbol    = -1;
      _endSymbol         = -1;

      setLen(spatium() * 7);   // for use in palettes
      _sp  = 0;
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _lineWidth            = e._lineWidth;
      _lineColor            = e._lineColor;
      _lineStyle            = e._lineStyle;
      _beginTextPlace       = e._beginTextPlace;
      _continueTextPlace    = e._continueTextPlace;

      _beginHook            = e._beginHook;
      _endHook              = e._endHook;
      _beginHookHeight      = e._beginHookHeight;
      _endHookHeight        = e._endHookHeight;
      _beginHookType        = e._beginHookType;
      _endHookType          = e._endHookType;

      _beginSymbol          = e._beginSymbol;
      _continueSymbol       = e._continueSymbol;
      _endSymbol            = e._endSymbol;
      _beginSymbolOffset    = e._beginSymbolOffset;
      _continueSymbolOffset = e._continueSymbolOffset;
      _endSymbolOffset      = e._endSymbolOffset;
      _mxmlOff2             = e._mxmlOff2;
      _beginText            = 0;
      _continueText         = 0;
      if (e._beginText)
            _beginText = e._beginText->clone(); // deep copy
      if (e._continueText)
            _continueText = e._continueText->clone();
      _sp = 0;
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(const QString& s, TextStyleType textStyle)
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setSubtype(TEXT_TEXTLINE);
            _beginText->setTextStyle(textStyle);
            _beginText->setParent(this);
            }
      _beginText->setText(s);
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(const QString& s, TextStyleType textStyle)
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setSubtype(TEXT_TEXTLINE);
            _continueText->setTextStyle(textStyle);
            _continueText->setParent(this);
            }
      _continueText->setText(s);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(XmlReader* r)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();

      int id = -1;
      while (r->readAttribute()) {
            if (r->tag() == "id")
                  id = r->intValue();
            }
      setId(id);

      while (r->readElement()) {
            if (!readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextLine::readProperties(XmlReader* r)
      {
      MString8 tag = r->tag();
      QString text;
      qreal d;
      int i;

      if (r->readReal("beginHookHeight", &d)) {
            _beginHookHeight = Spatium(d);
            _beginHook = true;
            }
      else if (r->readInt("beginHookType", &i))
            _beginHookType = HookType(i);
      else if (r->readReal("endHookHeight", &d)) {
            _endHookHeight = Spatium(d);
            _endHook = true;
            }
      else if (r->readInt("endHookType", &i))
            _endHookType = HookType(i);
      else if (r->readInt("beginSymbol", &_beginSymbol))
            ;
      else if (r->readInt("continueSymbol", &_continueSymbol))
            ;
      else if (r->readInt("endSymbol", &_endSymbol))
            ;
      else if (r->readPoint("beginSymbolOffset", &_beginSymbolOffset))
            ;
      else if (r->readPoint("continueSymbolOffset", &_continueSymbolOffset))
            ;
      else if (r->readPoint("endSymbolOffset", &_endSymbolOffset))
            ;
      else if (r->readReal("lineWidth", &d))
            _lineWidth = Spatium(d);
      else if (r->readInt("lineStyle", &i))
            _lineStyle = Qt::PenStyle(i);
      else if (r->readPlacement("beginTextPlace", &_beginTextPlace))
            ;
      else if (r->readPlacement("continueTextPlace", &_continueTextPlace))
            ;
      else if (r->readColor("lineColor", &_lineColor))
            ;
      else if (tag == "beginText") {
            _beginText = new Text(score());
            _beginText->setSubtype(TEXT_TEXTLINE);
            while (r->readElement()) {
                  if (!_beginText->readProperties(r))
                        r->unknown();
                  }
            }
      else if (tag == "continueText") {
            _continueText = new Text(score());
            _continueText->setSubtype(TEXT_TEXTLINE);
            while (r->readElement()) {
                  if (!_continueText->readProperties(r))
                        r->unknown();
                  }
            }
      else if (!SLine::readProperties(r))
            r->unknown();
      return true;
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      return new TextLineSegment(score());
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(Text* v)
      {
      delete _beginText;
      _beginText = v;
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(Text* v)
      {
      delete _continueText;
      _continueText = v;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void TextLine::layout()
      {
      setPos(0.0, yoff() * spatium());
      SLine::layout();
      }

