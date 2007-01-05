//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "globals.h"
#include "textline.h"
#include "xml.h"
#include "style.h"
#include "mscore.h"
#include "canvas.h"
#include "score.h"
#include "painter.h"
#include "utils.h"
#include "page.h"
#include "textpalette.h"
#include "sym.h"
#include "symbol.h"

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : Element(s)
      {
      textStyle = -1;
      setStyle(TEXT_STYLE_LYRIC);
      editMode = false;
      }

TextLine::TextLine(Score* s, int style)
   : Element(s)
      {
      textStyle = -1;
      setStyle(style);
      editMode = false;
      }

TextLine::TextLine(const TextLine& e)
   : Element(e)
      {
      _text     = e._text;
      textStyle = e.textStyle;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF& TextLine::bbox() const
      {
//TODO      _bbox = QRectF(0.0, 0.0, doc->size().width(), doc->size().height());
//      QFontMetrics fm(font());
      return _bbox;
      }

//---------------------------------------------------------
//   resetMode
//---------------------------------------------------------

void TextLine::resetMode()
      {
      editMode = 0;
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool TextLine::isEmpty() const
      {
      return _text.isEmpty();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLine::layout()
      {
#if 0
      if (parent() == 0)
            return;
      TextLineStyle* s = &textStyles[textStyle];

      double tw = bbox().width();
      double th = bbox().height();

      QPointF _off(QPointF(s->xoff, s->yoff));
      if (s->offsetType == OFFSET_SPATIUM)
            _off *= _score->spatium();

      double x = 0.0, y = 0.0;
      if (s->anchor == ANCHOR_PAGE) {
            Page* page = (Page*)parent();
            if (parent()->type() != PAGE) {
                  printf("fatal: text parent is not PAGE\n");
                  return;
                  }
            double w = page->loWidth() - page->lm() - page->rm();
            double h = page->loHeight() - page->tm() - page->bm();

            if (s->offsetType == OFFSET_REL)
                  _off = QPointF(s->xoff * w * 0.01, s->yoff * h * 0.01);

            if (s->align & ALIGN_LEFT)
                  x = page->lm();
            else if (s->align & ALIGN_RIGHT)
                  x  = page->lm() + w - tw;
            else if (s->align & ALIGN_HCENTER)
                  x  = page->lm() + w * .5 - tw * .5;
            if (s->align & ALIGN_TOP)
                  y = page->tm();
            else if (s->align & ALIGN_BOTTOM)
                  y = page->tm() + h;
            else if (s->align & ALIGN_VCENTER)
                  y = page->tm() + h * .5 - th * .5;
            }
      else {
            if (s->align & ALIGN_LEFT)
                  ;
            else if (s->align & ALIGN_RIGHT)
                  x  = -tw;
            else if (s->align & ALIGN_HCENTER)
                  x  = -(tw *.5);
            if (s->align & ALIGN_TOP)
                  ;
            else if (s->align & ALIGN_BOTTOM)
                  y = -th;
            else if (s->align & ALIGN_VCENTER) {
                  y = -(th * .5) - bbox().y();
                  }
            }
      setPos(x + _off.x(), y + _off.y());
#endif
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextLine::setText(const QString& txt)
      {
      _text = txt;
      layout();
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void TextLine::setStyle(int n)
      {
      if (textStyle != n) {
            textStyle = n;
            layout();
            }
      }

//---------------------------------------------------------
//   TextLine::write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      write(xml, "TextLine");
      }

//---------------------------------------------------------
//   TextLine::write
//---------------------------------------------------------

void TextLine::write(Xml& xml, const char* name) const
      {
      xml.stag(name);
      xml.tag("style", textStyle);
      xml.tag("text", _text);
      Element::writeProperties(xml);
      xml.etag(name);
      }

//---------------------------------------------------------
//   TextLine::read
//---------------------------------------------------------

void TextLine::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!node.isElement())
                  continue;
            QDomElement e = node.toElement();
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "text")
                  _text = val;
            else if (tag == "style")
                  textStyle = val.toInt();
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      layout();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool TextLine::startEdit(QMatrix&)
      {
      editMode = true;
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool TextLine::edit(QKeyEvent* ev)
      {
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void TextLine::endEdit()
      {
      editMode = false;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextLine::font() const
      {
      return textStyles[textStyle].font();
      }

//---------------------------------------------------------
//   TextLine::draw
//---------------------------------------------------------

void TextLine::draw1(Painter& p)
      {
      p.save();
      p.setRenderHint(QPainter::Antialiasing, false);
      p.setFont(font());
      //
      p.restore();
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double TextLine::lineSpacing() const
      {
      QFontMetrics fm(font());
      return fm.lineSpacing();
      }
