//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: text.cpp,v 1.41 2006/09/08 19:37:08 lvinken Exp $
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

#include "text.h"
#include "xml.h"
#include "painter.h"
#include "style.h"
#include "sym.h"
#include "symbol.h"

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBoxElement::HBoxElement()
      {
      element = 0;
      }

HBoxElement::~HBoxElement()
      {
//TODO      if (element)
//            delete element;
      }

HBoxElement::HBoxElement(const HBoxElement& e)
   : HBox(e)
      {
      if (e.element)
            element = e.element;
      else
            element = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void HBoxElement::write(Xml& xml, int) const
      {
      if (element->type() != SYMBOL) {
            printf("HBoxElement::write(): element type not implemented\n");
            return;
            }
      Symbol* s = (Symbol*)element;
      xml << "<symbol name=\"" << symbols[s->sym()].name() << "\"";
      QPointF offset = symbols[s->sym()].offset();
      if (offset.x() != 0.0)
            xml << " xoffset=\"" << offset.x() << "\"";
      if (offset.y() != 0.0)
            xml << " yoffset=\"" << offset.y() << "\"";
      xml << "/>";
      }

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

QRectF HBoxElement::cursor(int n) const
      {
      QPointF p = n ? bbox().topRight() : bbox().topLeft();
      p += pos();
      qreal w = bbox().width() * .1;
      return QRectF(p.x() - w * .5, p.y(), w, bbox().height());
      }

//---------------------------------------------------------
//   HBoxText
//---------------------------------------------------------

HBoxText::HBoxText(const QString& s, const QFont& f)
      {
      _txt  = s;
      _font = f;
      QFontMetricsF fm(_font);
      _bbox = fm.boundingRect(_txt);
      _w    = fm.width(_txt);
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

qreal HBoxText::lineSpacing() const
      {
      QFontMetricsF fm(_font);
      return fm.lineSpacing();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HBoxText::draw(Painter& p) const
      {
      p.setFont(_font);
      p.drawText(_pos, _txt);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void HBoxText::write(Xml& xml, int textStyle) const
      {
      QString family(textStyles[textStyle].family);
      int points  = lrint(textStyles[textStyle].size * _spatium * .2);

      if (_font.family() != family)
            xml << "<font family=\"" << _font.family() << "/>";
      if (_font.pixelSize() != points)
            xml << "<size points=\"" << _font.pointSize() << "\"/>";
      if (_font.bold())
            xml << "<b>";
      if (_font.italic())
            xml << "<i>";
      xml << Xml::xmlString(_txt);
      if (_font.italic())
            xml << "</i>";
      if (_font.bold())
            xml << "</b>";
      if (_font.pixelSize() != points)
            xml << "<size/>";
      if (_font.family() != family)
            xml << "<font/>";
      }

//---------------------------------------------------------
//   insertChar
//---------------------------------------------------------

bool HBoxText::insertChar(int col, const QString& s)
      {
      QFontMetricsF fm(_font);
      _txt  = _txt.left(col) + s + _txt.mid(col);
      _bbox = fm.boundingRect(_txt);
      _w    = fm.width(_txt);
      return true;
      }

//---------------------------------------------------------
//   removeChar
//---------------------------------------------------------

void HBoxText::removeChar(int col)
      {
      _txt = _txt.left(col - 1) + _txt.mid(col);
      QFontMetricsF fm(_font);
      _bbox = fm.boundingRect(_txt);
      _w    = fm.width(_txt);
      }

//---------------------------------------------------------
//   split
//---------------------------------------------------------

HBox* HBoxText::split(int col)
      {
      HBox* nHbox = new HBoxText(_txt.mid(col), _font);
      _txt = _txt.left(col);
      QFontMetricsF fm(_font);
      _bbox      = fm.boundingRect(_txt);
      _w         = fm.width(_txt);
      return nHbox;
      }

//---------------------------------------------------------
//   cursor
//---------------------------------------------------------

QRectF HBoxText::cursor(int col) const
      {
      QFontMetricsF fm(_font);
      qreal x = _pos.x() + fm.width(_txt.left(col));
      qreal y = _pos.y() - fm.ascent();
      double w = fm.width('x') * .1;
      return QRectF(x - w * .5, y, w, fm.height());
      }

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox() 
      {
      w = 0.0;
      lineSpacing = 0.0;
      }

//---------------------------------------------------------
//   ~VBox
//---------------------------------------------------------

VBox::~VBox()
      {
      foreach (HBox* b, hlist)
            delete b;
      }

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox(const VBox& b)
      {
      bbox = b.bbox;
      w    = b.w;
      lineSpacing = b.lineSpacing;
      foreach (HBox* hb, b.hlist)
            hlist.append(hb->clone());
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void VBox::clear()
      {
      bbox = QRectF();
      w    = 0.0;
      lineSpacing = 0.0;
      hlist.clear();
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void VBox::append(HBox* hb)
      {
      hlist.append(hb);
      qreal lsp = hb->lineSpacing();
      if (lsp > lineSpacing)
            lineSpacing = lsp;
      w += hb->width();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void VBox::insert(int idx, HBox* hb)
      {
      hlist.insert(idx, hb);
      qreal lsp = hb->lineSpacing();
      if (lsp > lineSpacing)
            lineSpacing = lsp;
      w += hb->width();
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(const QString& s, int style)
      {
      textStyle = style;
      if (s.isEmpty())
            return;

      TextStyle* ts = &textStyles[textStyle];
      QFont f;
      f.setFamily(ts->family);
      f.setItalic(ts->italic);
      f.setUnderline(ts->underline);
      f.setBold(ts->bold);
      f.setPixelSize(lrint(ts->size * _spatium * .2));
      HBoxText* hb = new HBoxText(s, f);

      VBox vb;
      vb.w = hb->width();
      vb.hlist.append(hb);
      append(vb);
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(const QString& s, int style, bool bold, double size)
      {
      textStyle = style;
      if (s.isEmpty())
            return;

      TextStyle* ts = &textStyles[textStyle];
      QFont f;
      f.setFamily(ts->family);
      f.setItalic(false);
      f.setUnderline(false);
      f.setBold(bold);
      f.setPixelSize(lrint(size * _spatium * .2));
      HBoxText* hb = new HBoxText(s, f);

      VBox vb;
      vb.w = hb->width();
      vb.hlist.append(hb);
      append(vb);
      }

//---------------------------------------------------------
//   text
//   Note: due to usage in MusicXML exporter, no trailing
//   newline should be added. Newlines are added at the
//   start of the second and following lines.
//---------------------------------------------------------

QString Text::text() const
      {
      QString s;
      bool needNewline = false;
      foreach (const VBox& vbox, *this) {
            if (needNewline)
                  s += "\n";
            needNewline = true;
            foreach (const HBox* box, vbox.hlist) {
                  s += box->txt();
                  }
            }
      return s;
      }

//---------------------------------------------------------
//   parseData
//---------------------------------------------------------

void Text::parseData(VBox& vb, QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (node.isText()) {
                  QString s(node.toText().data());
                  QStringList sl = s.split(QChar('\n'));
                  int n = sl.size();
                  QFont f(family);
                  f.setPixelSize(lrint(_fontSize * _spatium * .2));
                  f.setItalic(italic);
                  f.setBold(bold);
                  HBoxText* box = new HBoxText(sl[0], f);
                  vb.append(box);
                  for (int i = 1; i < n; ++i) {
                        if (!vb.hlist.isEmpty()) {
                              append(vb);
                              vb.clear();
                              }
                        box = new HBoxText(sl[i], f);
                        vb.append(box);
                        }
                  }
            else if (node.isElement()) {
                  QDomElement e = node.toElement();
                  QString tag(e.tagName());
                  if (tag == "b") {
                        bold = true;
                        parseData(vb, node);
                        bold = false;
                        }
                  else if (tag == "i") {
                        italic = true;
                        parseData(vb, node);
                        italic = false;
                        }
                  else if (tag == "font") {
                        QString oldFamily = family;
                        family = e.attribute("family");
                        parseData(vb, node);
                        family = oldFamily;
                        }
                  else if (tag == "size") {
                        int oldSize = _fontSize;
                        _fontSize = e.attribute("points").toInt();
                        parseData(vb, node);
                        _fontSize = oldSize;
                        }
                  else if (tag == "symbol") {
                        Symbol* s = new Symbol(0);
                        QString name = e.attribute("name");
                        s->setSym(Sym::buildin(name));
                        qreal xoffset = e.attribute("xoffset", "0.0").toDouble();
                        qreal yoffset = e.attribute("yoffset", "0.0").toDouble();
                        QPointF off(xoffset, yoffset);
                        symbols[s->sym()].setOffset(off);
//TODO                        s->sym()->setSize(_fontSize);
                        HBoxElement* box = new HBoxElement(s);
                        vb.hlist.append(box);
                        vb.w += box->width();
                        }
                  else {
                        domError(node);
                        }
                  }
            else  {
                  printf("unknown type %d\n", node.nodeType());
                  continue;
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Text::read(QDomNode node)
      {
      family    = textStyles[textStyle].family;
      _fontSize = textStyles[textStyle].size;
      bold      = false;
      italic    = false;
      VBox vb;
      parseData(vb, node);
      if (!vb.hlist.isEmpty())
            append(vb);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Text::write(Xml& xml, const char* s) const
      {
      xml.ntag(s);
      foreach (const VBox& vbox, *this) {
            foreach (const HBox* box, vbox.hlist)
                  box->write(xml, textStyle);
            }
      xml.netag(s);
      }

