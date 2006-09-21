//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: textelement.cpp,v 1.6 2006/04/05 08:15:12 wschweer Exp $
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
#include "textelement.h"
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

TextPalette* palette;

//---------------------------------------------------------
//   TextElement
//---------------------------------------------------------

TextElement::TextElement(Score* s)
   : Element(s)
      {
      textStyle = TEXT_STYLE_LYRIC;
      text.setStyle(textStyle);
      editMode = false;
      }

TextElement::TextElement(Score* s, int style)
   : Element(s)
      {
      textStyle = style;
      text.setStyle(textStyle);
      editMode = false;
      }

TextElement::~TextElement()
      {
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextElement::setSelected(bool val)
      {
      Element::setSelected(val);
      foreach (const VBox& vbox, text)
            foreach (HBox* box, vbox.hlist)
                  box->setSelected(val);
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool TextElement::isEmpty() const
      {
      return text.isEmpty();
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString TextElement::getText() const
      {
      return text.text();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextElement::layout()
      {
      if (parent() == 0) {
            // printf("TextElement::layout: %p no parent\n", this);
            return;
            }
      if (text.isEmpty())
            return;

      //
      // search for widest line
      //
      double tw = 0.0;        // width of text
      int nn = text.size();
      for (int k = 0; k < nn; ++k) {
            VBox& vbox = text[k];
            vbox.w = 0.0;
            int boxes = vbox.hlist.size();
            for (int i = 0; i < boxes; ++i)
                  vbox.w += vbox.hlist[i]->width();
            if (vbox.w > tw)
                  tw = vbox.w;
            }

      TextStyle* s = &textStyles[textStyle];

      //
      // align individual lines horizontally
      //
      double py = 0.0;
      for (int k = 0; k < nn; ++k) {
            double x = 0.0;
            if (s->align & ALIGN_LEFT)
                  x = 0.0;
            else if (s->align & ALIGN_RIGHT)
                  x  = tw - text[k].w;
            else if (s->align & ALIGN_HCENTER)
                  x  = (tw - text[k].w) * .5;
            QList<HBox*> hlist = text[k].hlist;
            int n = hlist.size();
            // double lineSpacing = 0.0;
            text[k].bbox = QRectF();
            for (int i = 0; i < n; ++i) {
                  QPointF p(x, py);
                  hlist[i]->setPos(p);
                  text[k].bbox |= hlist[i]->bbox().translated(p);
                  x += hlist[i]->width();
                  }
            py += text[k].lineSpacing;
            }
      double height = py;

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
                  y = page->tm() + h * .5 - bbox().height() * .5;
            x += _off.x();
            y += _off.y();
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
                  y = -height;
            else if (s->align & ALIGN_VCENTER) {
                  bboxUpdate();
                  y = -(bbox().height() * .5) - bbox().y();
                  }
            x += _off.x();
            y += _off.y();
            int nn = text.size();
            for (int k = 0; k < nn; ++k) {
                  QList<HBox*> hlist = text[k].hlist;
                  QPointF p(x, y);
                  text[k].bbox.translate(p);
                  int n = hlist.size();
                  for (int i = 0; i < n; ++i) {
                        QPointF p(x, y);
                        hlist[i]->setPos(hlist[i]->pos() + p);
                        }
                  }
            }
      setPos(x, y);
      bboxUpdate();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextElement::setText(const QString& s)
      {
      text.clear();
      if (!s.isEmpty()) {
            VBox vb;
            HBoxText* hb = new HBoxText(s, font());
            vb.w    = hb->width();
            vb.append(hb);
            text.append(vb);
            }
      layout();
      }

void TextElement::setText(const Text& v) 
      { 
      text = v; 
      layout();
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void TextElement::setStyle(int n)
      {
      if (textStyle != n) {
            textStyle = n;
            text.setStyle(textStyle);
            layout();
            }
      }

//---------------------------------------------------------
//   TextElement::write
//---------------------------------------------------------

void TextElement::write(Xml& xml) const
      {
      write(xml, "Text");
      }

//---------------------------------------------------------
//   TextElement::write
//---------------------------------------------------------

void TextElement::write(Xml& xml, const char* name) const
      {
      if (text.isEmpty())
            return;
      xml.stag(name);
      xml.tag("style", textStyle);

      foreach (const VBox& vbox, text) {
            xml.ntag("data");
            foreach (HBox* box, vbox.hlist)
                  box->write(xml, textStyle);
            xml.netag("data");
            }
      Element::writeProperties(xml);
      xml.etag(name);
      }

//---------------------------------------------------------
//   TextElement::read
//---------------------------------------------------------

void TextElement::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!node.isElement())
                  continue;
            QDomElement e = node.toElement();
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "data")
                  text.read(node);
            else if (tag == "style") {
                  text.setStyle(i);
                  textStyle = i;
                  }
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      layout();
      }

//---------------------------------------------------------
//   setCursor
//---------------------------------------------------------

void TextElement::setCursor()
      {
      VBox& vbox = text[cursorLine];
      QPointF cp;
      int col  = 0;
      int boxes = vbox.hlist.size();
      HBox* box = 0;
      for (int i = 0; i < boxes; ++i) {
            box = vbox.hlist[i];
            int n = box->size();
            if (i == (boxes-1) || (cursorColumn >= col && cursorColumn < col+n)) {
                  cursor = box->cursor(cursorColumn - col);
//                  if (i == (boxes-1) && cursorColumn >= (n+col))
//                        printf("at end\n");
                  break;
                  }
            col += n;
            }
      QFont* f = box->font();
      if (box && palette && f) {
            palette->setFontFamily(f->family());
            palette->setBold(f->bold());
            palette->setItalic(f->italic());
            palette->setFontSize(f->pixelSize());
            }
      cursor.setY(vbox.bbox.y());
      cursor.setHeight(vbox.bbox.height());
      bboxUpdate();
      }

//---------------------------------------------------------
//   columns
//    return number of columns in current line
//---------------------------------------------------------

int TextElement::columns() const
      {
      const VBox& vbox = text[cursorLine];
      int col = 0;
      foreach(HBox* hb, vbox.hlist)
            col += hb->size();
      return col;
      }

//---------------------------------------------------------
//   insertChar
//    insert character at current cursor position
//---------------------------------------------------------

void TextElement::insertChar(const QString& s)
      {
      VBox& vbox = text[cursorLine];
      int boxes = vbox.hlist.size();
      int col = 0;
      for (int i = 0; i < boxes; ++i) {
            HBox* box = vbox.hlist[i];
            int n = box->size();
            if (i == (boxes-1) || (cursorColumn >= col && cursorColumn < col+n)) {
                  int rx = cursorColumn - col;
                  if (!box->insertChar(rx, s)) {
                        //
                        // we cannot insert into HBoxElement
                        //
                        if (i) {
                              // append char to previous box
                              box = vbox.hlist[i];
                              if (!box->insertChar(rx+1, s)) {
                                    // create new box
                                    HBoxText* nb = new HBoxText(s, font());
                                    vbox.insert(i, nb);
                                    }
                              }
                        else {
                              printf("TODO: create newbox\n");
                              // create new box
                              }
                        }
                  break;
                  }
            col += n;
            }
      if (boxes == 0) {
            HBoxText* nb = new HBoxText(s, font());
            vbox.insert(0, nb);
            }
      layout();
      }

//---------------------------------------------------------
//   removeChar
//---------------------------------------------------------

void TextElement::removeChar()
      {
      VBox& vbox = text[cursorLine];
      int boxes = vbox.hlist.size();
      int col = 0;
      for (int i = 0; i < boxes; ++i) {
            HBox* box = vbox.hlist[i];
            int n = box->size();
            if (i == (boxes-1) || (cursorColumn >= col && cursorColumn <= col+n)) {
                  int rx = cursorColumn - col;
                  if (rx == 0)
                        break;
                  if (box->size() == 1) {
                        vbox.hlist.removeAt(i);
                        delete box;
                        }
                  else
                        box->removeChar(rx);
                  break;
                  }
            col += n;
            }
      layout();
      }

//---------------------------------------------------------
//   splitLine
//---------------------------------------------------------

void TextElement::splitLine()
      {
      VBox& vbox = text[cursorLine];
      int boxes  = vbox.hlist.size();
      int col = 0;
      for (int i = 0; i < boxes; ++i) {
            HBox* box = vbox.hlist[i];
            int n = box->size();
            if (i == (boxes-1) || (cursorColumn >= col && cursorColumn < col+n)) {
                  VBox nVbox;
                  // HBox nHbox;
                  // nHbox.font = box.font;

                  int rx = cursorColumn - col;
                  if (rx == n) {
                        HBoxText* nHbox = new HBoxText(QString(""), *box->font());
                        nVbox.append(nHbox);
                        }
                  else if (rx == 0) {
                        HBoxText* nHbox = new HBoxText(QString(""), *box->font());
                        nVbox.append(nHbox);
                        int k = i;
                        for (; i < boxes; ++i) {
                              nVbox.append(vbox.hlist[k]);
                              vbox.hlist.removeAt(k);
                              }
                        }
                  else {
                        HBox* nHbox = box->split(rx);
                        nVbox.append(nHbox);
                        int k = ++i;
                        for (; i < boxes; ++i) {
                              nVbox.append(vbox.hlist[k]);
                              vbox.hlist.removeAt(k);
                              }
                        }
                  text.insert(cursorLine+1, nVbox);
                  break;
                  }
            col += n;
            }
      layout();
      }

//---------------------------------------------------------
//   concatLine
//---------------------------------------------------------

void TextElement::concatLine()
      {
      VBox& dbox = text[cursorLine];
      VBox& sbox = text[cursorLine+1];
      foreach(HBox* hb, sbox.hlist)
            dbox.hlist.append(hb->clone());
      text.removeAt(cursorLine+1);
      layout();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool TextElement::startEdit(QMatrix&)
      {
      editMode   = true;
      cursorLine = text.size() - 1;

      //
      // set cursor to last column in last line
      //      
      int columns = 0;
      VBox& vbox = text[cursorLine];
      foreach(HBox* box, vbox.hlist)
            columns += box->size();
      cursorColumn  = columns;
      setCursor();

      bboxUpdate();
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool TextElement::edit(QKeyEvent* ev)
      {
      int lines  = text.size();

      if (ev->key() == Qt::Key_F2) {
            if (palette == 0)
                  palette = new TextPalette(0);
            if (palette->isVisible())
                  palette->hide();
            else {
                  palette->setTextElement(this);
                  palette->show();
                  setCursor();
                  mscore->activateWindow();
                  }
            return false;
            }
      switch (ev->key()) {
            case Qt::Key_Return:
                  splitLine();
                  ++cursorLine;
                  cursorColumn = 0;
                  break;

            case Qt::Key_Backspace:
            case Qt::Key_Delete:
                  if (cursorColumn == 0) {
                        if (cursorLine == 0)
                              break;
                        --cursorLine;
                        cursorColumn = columns();
                        concatLine();
                        break;
                        }
                  removeChar();
                  --cursorColumn;
                  break;

            case Qt::Key_Left:
                  if (cursorColumn == 0) {
                        if (cursorLine == 0)
                              break;
                        --cursorLine;
                        cursorColumn = columns();
                        }
                  else
                        --cursorColumn;
                  break;

            case Qt::Key_Right:
                  if (cursorColumn == columns()) {
                        if (cursorLine == (lines-1))
                              break;
                        ++cursorLine;
                        cursorColumn = 0;
                        }
                  else
                        ++cursorColumn;
                  break;

            case Qt::Key_Up:
                  if (--cursorLine < 0)
                        cursorLine = 0;
                  if (cursorColumn > columns())
                        cursorColumn = columns();
                  break;

            case Qt::Key_Down:
                  if (++cursorLine >= lines)
                        cursorLine = lines-1;
                  if (cursorColumn > columns())
                        cursorColumn = columns();
                  break;

            case Qt::Key_Home:
                  cursorColumn = 0;
                  break;

            case Qt::Key_End:
                  cursorColumn = columns();
                  break;

            default:
                  if (ev->text()[0].isPrint()) {
                        insertChar(ev->text());
                        ++cursorColumn;
                        }
                  break;
            }
      setCursor();
      return false;
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void TextElement::addSymbol(int code)
      {
      if (!editMode)
            return;
      Symbol* s = new Symbol(0);
//TODO      s->setSym(Sym(QString("mops"), 0, code, QPointF()));
//      s->sym()->setSize(text.fontSize());

      VBox& vbox = text[cursorLine];
      int boxes = vbox.hlist.size();
      int col = 0;

      HBoxElement* nb = new HBoxElement(s);

      nb->setSelected(true);
      for (int i = 0; i < boxes; ++i) {
            HBox* box = vbox.hlist[i];
            int n = box->size();
            if (i == (boxes-1) || (cursorColumn >= col && cursorColumn < col+n)) {
                  int rx = cursorColumn - col;
                  if (rx == 0) {
                        vbox.insert(i, nb);
                        }
                  else if (rx == n) {
                        vbox.insert(i+1, nb);
                        }
                  else {
                        HBox* hb = box->split(rx);
                        vbox.insert(i+1, nb);
                        vbox.insert(i+2, hb);
                        }
                  break;
                  }
            col += n;
            }
      if (boxes == 0) {
            vbox.insert(0, nb);
            }
      ++cursorColumn;
      layout();
      setCursor();
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void TextElement::endEdit()
      {
      if (palette)
            palette->hide();
      editMode = false;
      bboxUpdate();
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextElement::font() const
      {
      TextStyle* s = &textStyles[textStyle];
      QFont f;
      f.setFamily(s->family);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      f.setPixelSize(lrint(s->size * _spatium * .2));
      return f;
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double TextElement::lineSpacing() const
      {
      QFontMetricsF fm(font());
      return fm.lineSpacing();
      }

//---------------------------------------------------------
//   bboxUpdate
//---------------------------------------------------------

void TextElement::bboxUpdate()
      {
      QRectF b;
      foreach (const VBox& vbox, text)
            b |= vbox.bbox;
      if (editMode)
            b |= cursor;
      setbbox(b);
      }

//---------------------------------------------------------
//   TextElement::draw
//---------------------------------------------------------

void TextElement::draw1(Painter& p) const
      {
      foreach (const VBox& vbox, text) {
            foreach (HBox* box, vbox.hlist) {
                  p.save();
                  box->draw(p);
                  p.restore();
                  }
            }
      if (editMode) {
            p.setBrush(Qt::blue);
            p.drawRect(cursor);
            }
      }

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : TextElement(s, TEXT_STYLE_LYRIC)
      {
      _no = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Lyrics::write(Xml& xml) const
      {
      xml.stag("Lyrics");
      xml.tag("data", getText());
      if (_no)
            xml.tag("no", _no);
      Element::writeProperties(xml);
      xml.etag("Lyrics");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "data")
                  setText(val);
            else if (tag == "no")
                  _no = i;
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

Fingering::Fingering(Score* s)
   : TextElement(s, TEXT_STYLE_FINGERING)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(Xml& xml) const
      {
      TextElement::write(xml, "Fingering");
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Fingering::setSubtype(int n)
      {
      Element::setSubtype(n);
      switch(n) {
            case 1: setText("1"); break;
            case 2: setText("2"); break;
            case 3: setText("3"); break;
            case 4: setText("4"); break;
            case 5: setText("5"); break;
            }
      }

//---------------------------------------------------------
//   InstrumentName1
//---------------------------------------------------------

InstrumentName1::InstrumentName1(Score* s)
   : TextElement(s, TEXT_STYLE_INSTRUMENT_LONG)
      {
      }

//---------------------------------------------------------
//   InstrumentName2
//---------------------------------------------------------

InstrumentName2::InstrumentName2(Score* s)
   : TextElement(s, TEXT_STYLE_INSTRUMENT_SHORT)
      {
      }

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : TextElement(s, TEXT_STYLE_TEMPO)
      {
      }

