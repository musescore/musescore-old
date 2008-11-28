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
#include "preferences.h"

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
      qreal textlineLineWidth    = textLine()->lineWidth().point();
      qreal textlineTextDistance = _spatium * .5;

      QPointF pp2(pos2());

      qreal l = 0.0;
      if (_text && textLine()->hasText()) {
            QRectF bb(_text->bbox());
            l = _text->pos().x() + bb.width() + textlineTextDistance;
            }

      QPointF pp1(l, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(textlineLineWidth);
      pen.setStyle(textLine()->lineStyle());

      if (selected() && !(score() && score()->printing()))
            pen.setColor(preferences.selectColor[0]);
      else
            pen.setColor(textLine()->lineColor());

      p.setPen(pen);
      p.drawLine(QLineF(pp1, pp2));

      if (textLine()->hook()) {
            double hh = textLine()->hookHeight().point();
            if (textLine()->hookUp())
                  hh *= -1;
            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END) {
                  p.drawLine(QLineF(pp2, QPointF(pp2.x(), pp2.y() + hh)));
                  }
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF TextLineSegment::bbox() const
      {
      QPointF pp1;
      QPointF pp2(pos2());

      qreal h1 = _text ? _text->height() * .5 : 10.0;
      if (!textLine()->hasText() && pp2.y() != 0)
            return QRectF(pp1, pp2).normalized();
      return QRectF(.0, -h1, pp2.x(), h1 * 2);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout(ScoreLayout* l)
      {
      TextLine* tl = (TextLine*)line();
      if (!tl->hasText())
            return;
      _userOff2.setY(0);
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN) {
            if (_text == 0) {
                  _text = new TextC(tl->textBasePtr(), score());
                  _text->setSubtype(TEXT_TEXTLINE);
                  _text->setMovable(false);
                  _text->setParent(this);
                  }
            _text->layout(l);
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
      _text       = new TextBase;
      _hookHeight = Spatium(1.5);
      _lineWidth  = Spatium(0.15);
      _lineStyle  = Qt::SolidLine;
      _hookUp     = false;
      _hook       = true;
      _lineColor  = Qt::black;
      _hasText    = true;
      _mxmlOff2   = 0;
      textBase()->setDefaultFont(score()->textStyle(TEXT_STYLE_TEXTLINE)->font());
      setLen(_spatium * 7);   // for use in palettes
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _text       = e._text;
      _hookHeight = e._hookHeight;
      _lineWidth  = e._lineWidth;
      _lineStyle  = e._lineStyle;
      _hookUp     = e._hookUp;
      _lineColor  = e._lineColor;
      _hook       = e._hook;
      _hasText    = e._hasText;
      _mxmlOff2   = e._mxmlOff2;
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
      xml.stag(name());
      if (_hook) {
            xml.tag("hookHeight", _hookHeight.val());
            xml.tag("hookUp", _hookUp);
            }
      xml.tag("lineWidth", _lineWidth.val());
      xml.tag("lineStyle", _lineStyle);
      xml.tag("lineColor", _lineColor);

      SLine::writeProperties(xml);
      if (_hasText)
            _text->writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(QDomElement e)
      {
      _hook    = false;
      _hasText = false;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            const QString& text = e.text();
            if (tag == "hookHeight") {
                  _hookHeight = Spatium(text.toDouble());
                  _hook = true;
                  }
            else if (tag == "lineWidth")
                  _lineWidth = Spatium(text.toDouble());
            else if (tag == "lineStyle")
                  _lineStyle = Qt::PenStyle(text.toInt());
            else if (tag == "hookUp") {
                  _hookUp = text.toInt();
                  _hook   = true;
                  }
            else if (tag == "lineColor")
                  _lineColor = readColor(e);
            else if (_text->readProperties(e))
                  _hasText = true;
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

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool TextLineSegment::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TextLineSegment::propertyAction(const QString& s)
      {
      if (s == "props") {
            LineProperties lp(textLine(), 0);
            lp.exec();
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::LineProperties(TextLine* l, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      tl = l;
      lineWidth->setValue(tl->lineWidth().val());
      hookHeight->setValue(tl->hookHeight().val());
      hook->setChecked(tl->hook());
      up->setChecked(tl->hookUp());

      lineStyle->setCurrentIndex(int(tl->lineStyle() - 1));
      text->setHtml(tl->getHtml());
      linecolor->setColor(tl->lineColor());
      TextBase* tb = tl->textBase();
      if (tb->frameWidth()) {
            frame->setChecked(true);
            frameWidth->setValue(tb->frameWidth());
            frameMargin->setValue(tb->paddingWidth());
            frameColor->setColor(tb->frameColor());
            frameCircled->setChecked(tb->circle());
            }
      else
            frame->setChecked(false);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void LineProperties::accept()
      {
      tl->setLineWidth(Spatium(lineWidth->value()));
      tl->setHookHeight(Spatium(hookHeight->value()));
      tl->setHookUp(up->isChecked());
      tl->setHook(hook->isChecked());

      tl->setLineStyle(Qt::PenStyle(lineStyle->currentIndex() + 1));
      tl->setLineColor(linecolor->color());
      TextBase* tb = tl->textBase();
      if (frame->isChecked()) {
            tb->setFrameWidth(frameWidth->value());
            tb->setPaddingWidth(frameMargin->value());
            tb->setFrameColor(frameColor->color());
            tb->setCircle(frameCircled->isChecked());
            }
      else
            tb->setFrameWidth(0.0);
      tl->setHtml(text->toHtml());
      tl->setHasText(!text->document()->isEmpty());

      QDialog::accept();
      }

