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

#include "textline.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "preferences.h"
#include "sym.h"
#include "text.h"

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
            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN) {
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

void TextLineSegment::draw(QPainter& p) const
      {
      TextLine* tl    = textLine();
      double _spatium = spatium();

      qreal textlineLineWidth    = tl->lineWidth().val() * _spatium;
      qreal textlineTextDistance = _spatium * .5;

      QPointF pp2(pos2());

      qreal l = 0.0;
      int sym = _segmentType == SEGMENT_MIDDLE ? tl->continueSymbol() : tl->beginSymbol();
      if (_text) {
            QFont f = _text->defaultFont();
            if (
               ((_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN) && (tl->beginTextPlace() == PLACE_LEFT))
               || ((_segmentType == SEGMENT_MIDDLE || _segmentType == SEGMENT_END) && (tl->continueTextPlace() == PLACE_LEFT))
               ) {
                  QRectF bb(_text->bbox());
                  l = _text->pos().x() + bb.width() + textlineTextDistance;
                  }
            p.save();
            p.translate(_text->pos());
            p.setPen(QPen(_text->curColor()));
            _text->draw(p);
            p.restore();
            }
      else if (sym != -1) {
            const QRectF& bb = symbols[sym].bbox(magS());
            qreal h = bb.height() * .5;
            QPointF o = tl->beginSymbolOffset() * _spatium;
            symbols[sym].draw(p, 1.0, o.x(), h + o.y());
            l = bb.width() + textlineTextDistance;
            }
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END) {
            if (tl->endSymbol() != -1) {
                  int sym = tl->endSymbol();
                  const QRectF& bb = symbols[sym].bbox(magS());
                  qreal h = bb.height() * .5;
                  QPointF o = tl->endSymbolOffset() * _spatium;
                  pp2.setX(pp2.x() - bb.width() + textlineTextDistance);
                  symbols[sym].draw(p, 1.0, pp2.x() + textlineTextDistance + o.x(), h + o.y());
                  }
            }

      QPointF pp1(l, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(textlineLineWidth);
      pen.setStyle(tl->lineStyle());

      if (selected() && !(score() && score()->printing()))
            pen.setColor(preferences.selectColor[0]);
      else if (!visible())
            pen.setColor(Qt::gray);
      else
            pen.setColor(tl->lineColor());

      p.setPen(pen);
      p.drawLine(QLineF(pp1, pp2));

      if (tl->endHook()) {
            double hh = point(textLine()->endHookHeight());
            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END) {
                  p.drawLine(QLineF(pp2, QPointF(pp2.x(), pp2.y() + hh)));
                  }
            }
      if (tl->beginHook()) {
            double hh = point(textLine()->beginHookHeight());
            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN) {
                  p.drawLine(QLineF(pp1, QPointF(pp1.x(), pp1.y() + hh)));
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
      double y1 = point(-textLine()->lineWidth());
      double y2 = -y1;

      int sym = textLine()->beginSymbol();
      if (_text) {
            double h = _text->height();
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
            double hh = symbols[sym].height(magS()) * .5;
            y1 = -hh;
            y2 = hh;
            }
      if (textLine()->endHook()) {
            double h = point(textLine()->endHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      if (textLine()->beginHook()) {
            double h = point(textLine()->beginHookHeight());
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
      switch (_segmentType) {
            case SEGMENT_SINGLE:
            case SEGMENT_BEGIN:
                  if (tl->beginText()) {
                        if (_text == 0) {
                              _text = new TextC(*tl->beginText());
                              _text->setMovable(false);
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
                              _text = new TextC(*tl->continueText());
                              _text->setMovable(false);
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

      _lineWidth         = Spatium(0.15);
      _lineStyle         = Qt::SolidLine;
      _beginTextPlace    = PLACE_LEFT;
      _continueTextPlace = PLACE_LEFT;
      _lineColor         = Qt::black;
      _mxmlOff2          = 0;

      _beginSymbol       = -1;
      _continueSymbol    = -1;
      _endSymbol         = -1;

      setLen(spatium() * 7);   // for use in palettes
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
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(const QString& s, int textStyle)
      {
      if (!_beginText) {
            _beginText = new TextC(score());
            _beginText->setSubtype(TEXT_TEXTLINE);
            _beginText->setTextStyle(textStyle);
            _beginText->setParent(this);
            }
      _beginText->setText(s);
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(const QString& s, int textStyle)
      {
      if (!_continueText) {
            _continueText = new TextC(score());
            _continueText->setSubtype(TEXT_TEXTLINE);
            _continueText->setTextStyle(textStyle);
            _continueText->setParent(this);
            }
      _continueText->setText(s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(QDomElement e)
      {
      foreach(LineSegment* seg, segments)
            delete seg;
      segments.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void TextLine::writeProperties(Xml& xml, const TextLine* proto) const
      {
      if (_beginHook && (proto == 0 || proto->beginHookHeight() != _beginHookHeight))
            xml.tag("beginHookHeight", _beginHookHeight.val());
      if (_endHook && (proto == 0 || proto->endHookHeight() != _endHookHeight))
            xml.tag("endHookHeight", _endHookHeight.val());

      if (proto == 0 || proto->lineWidth() != _lineWidth)
            xml.tag("lineWidth", _lineWidth.val());
      if (proto == 0 || proto->lineStyle() != _lineStyle)
            xml.tag("lineStyle", _lineStyle);
      if (proto == 0 || proto->lineColor() != _lineColor)
            xml.tag("lineColor", _lineColor);
      if (proto == 0 || proto->beginTextPlace() != _beginTextPlace)
            xml.pTag("beginTextPlace", _beginTextPlace);
      if (proto == 0 || proto->continueTextPlace() != _continueTextPlace)
            xml.pTag("continueTextPlace", _continueTextPlace);

      SLine::writeProperties(xml);
      if (_beginText) {
            xml.stag("beginText");
            _beginText->writeProperties(xml);
            xml.etag();
            }
      if (_continueText) {
            xml.stag("continueText");
            _continueText->writeProperties(xml);
            xml.etag();
            }
      if (_beginSymbol != -1) {
            xml.tag("beginSymbol", _beginSymbol);   // symbols[_symbol].name();
            xml.tag("beginSymbolOffset", _beginSymbolOffset);
            }
      if (_continueSymbol != -1) {
            xml.tag("continueSymbol", _continueSymbol);
            xml.tag("continueSymbolOffset", _continueSymbolOffset);
            }
      if (_endSymbol != -1) {
            xml.tag("endSymbol", _endSymbol);
            xml.tag("endSymbolOffset", _endSymbolOffset);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextLine::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      const QString& text = e.text();
      if (tag == "beginHookHeight") {
            _beginHookHeight = Spatium(text.toDouble());
            _beginHook = true;
            }
      else if (tag == "endHookHeight" || tag == "hookHeight") { // hookHeight is obsolete
            _endHookHeight = Spatium(text.toDouble());
            _endHook = true;
            }
      else if (tag == "hookUp")           // obsolete
            _endHookHeight *= -1.0;
      else if (tag == "beginSymbol" || tag == "symbol")     // "symbol" is obsolete
            _beginSymbol = resolveSymCompatibility(text.toInt());
      else if (tag == "continueSymbol")
            _continueSymbol = resolveSymCompatibility(text.toInt());
      else if (tag == "endSymbol")
            _endSymbol = resolveSymCompatibility(text.toInt());
      else if (tag == "beginSymbolOffset")
            _beginSymbolOffset = readPoint(e);
      else if (tag == "continueSymbolOffset")
            _continueSymbolOffset = readPoint(e);
      else if (tag == "endSymbolOffset")
            _endSymbolOffset = readPoint(e);
      else if (tag == "lineWidth")
            _lineWidth = Spatium(text.toDouble());
      else if (tag == "lineStyle")
            _lineStyle = Qt::PenStyle(text.toInt());
      else if (tag == "beginTextPlace")
            _beginTextPlace = readPlacement(e);
      else if (tag == "continueTextPlace")
            _continueTextPlace = readPlacement(e);
      else if (tag == "lineColor")
            _lineColor = readColor(e);
      else if (tag == "beginText") {
            _beginText = new TextC(score());
            _beginText->setSubtype(TEXT_TEXTLINE);
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (!_beginText->readProperties(ee))
                        domError(e);
                  }
            }
      else if (tag == "continueText") {
            _continueText = new TextC(score());
            _continueText->setSubtype(TEXT_TEXTLINE);
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (!_continueText->readProperties(ee))
                        domError(e);
                  }
            }
      else if (!SLine::readProperties(e))
            return false;
      return true;
      }

int TextLine::resolveSymCompatibility(int i)
      {
      if(score()->programVersion() < 10100)
          return i + 5;
      return i;  
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      return new TextLineSegment(score());
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool TextLineSegment::genPropertyMenu(QMenu* popup) const
      {
      QAction* a;
      if (visible())
            a = popup->addAction(tr("Set Invisible"));
      else
            a = popup->addAction(tr("Set Visible"));
      a->setData("invisible");
      a = popup->addAction(tr("Line Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TextLineSegment::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            TextLine* nTl  = textLine()->clone();
            LineProperties lp(nTl);
            if (lp.exec()) {
                  score()->undoChangeElement(textLine(), nTl);
                  // force new text
                  foreach(LineSegment* l, nTl->lineSegments()) {
                        static_cast<TextLineSegment*>(l)->clearText();
                        }
                  }
            else
                  delete nTl;
            }
      else if (s == "invisible")
            score()->toggleInvisible(this);
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   populateLineSymbolComboBox
//---------------------------------------------------------

static void populateLineSymbolComboBox(QComboBox* cb)
      {
      cb->clear();
      cb->addItem(QComboBox::tr("Ped (Pedal)"), pedalPedSym);
      cb->addItem(QComboBox::tr("* (Pedal)"), pedalasteriskSym);
      cb->addItem(QComboBox::tr(". (Pedal)"), pedaldotSym);
      cb->addItem(QComboBox::tr("dash (Pedal)"), pedaldashSym);
      cb->addItem(QComboBox::tr("tr (Trill)"), trillSym);
      }

//---------------------------------------------------------
//   setLineSymbolComboBox
//---------------------------------------------------------

static void setLineSymbolComboBox(QComboBox* cb, int sym)
      {
      if (sym == -1)
            return;
      for (int i = 0; i < cb->count(); ++i) {
            if (cb->itemData(i).toInt() == sym) {
                  cb->setCurrentIndex(i);
                  return;
                  }
            }
      printf("setLineSymbol: not found %d\n", sym);
      }

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::LineProperties(TextLine* l, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      tl = l;

      populateLineSymbolComboBox(beginSymbol);
      populateLineSymbolComboBox(continueSymbol);
      populateLineSymbolComboBox(endSymbol);

      lineWidth->setValue(tl->lineWidth().val());
      lineStyle->setCurrentIndex(int(tl->lineStyle() - 1));
      linecolor->setColor(tl->lineColor());

      beginTextRb->setChecked(tl->beginText());
      continueTextRb->setChecked(tl->continueText());
      beginSymbolRb->setChecked(tl->beginSymbol() != -1);
      continueSymbolRb->setChecked(tl->continueSymbol() != -1);
      endSymbolRb->setChecked(tl->endSymbol() != -1);

      bool bt = beginTextRb->isChecked();
      beginText->setEnabled(bt);
      beginTextTb->setEnabled(bt);
      beginTextPlace->setEnabled(bt);
      bt = beginSymbolRb->isChecked();
      beginSymbol->setEnabled(bt);
      beginSymbolX->setEnabled(bt);
      beginSymbolY->setEnabled(bt);

      bt = continueTextRb->isChecked();
      continueText->setEnabled(bt);
      continueTextTb->setEnabled(bt);
      continueTextPlace->setEnabled(bt);
      bt = continueSymbolRb->isChecked();
      continueSymbol->setEnabled(bt);
      continueSymbolX->setEnabled(bt);
      continueSymbolY->setEnabled(bt);

      beginText->setText(tl->beginText() ? tl->beginText()->getText() : "");
      continueText->setText(tl->continueText() ? tl->continueText()->getText() : "");

      setLineSymbolComboBox(beginSymbol, tl->beginSymbol());
      setLineSymbolComboBox(continueSymbol, tl->continueSymbol());
      setLineSymbolComboBox(endSymbol, tl->endSymbol());

      beginSymbolX->setValue(tl->beginSymbolOffset().x());
      beginSymbolY->setValue(tl->beginSymbolOffset().y());
      continueSymbolX->setValue(tl->continueSymbolOffset().x());
      continueSymbolY->setValue(tl->continueSymbolOffset().y());
      endSymbolX->setValue(tl->endSymbolOffset().x());
      endSymbolY->setValue(tl->endSymbolOffset().y());

      beginTextPlace->setCurrentIndex(tl->beginTextPlace() == PLACE_LEFT ? 0 : 1);
      continueTextPlace->setCurrentIndex(tl->continueTextPlace() == PLACE_LEFT ? 0 : 1);

      beginHook->setChecked(tl->beginHook());
      endHook->setChecked(tl->endHook());
      beginHookHeight->setValue(tl->beginHookHeight().val());
      endHookHeight->setValue(tl->endHookHeight().val());

      diagonal->setChecked(tl->diagonal());

      connect(beginTextRb, SIGNAL(toggled(bool)), SLOT(beginTextToggled(bool)));
      connect(beginSymbolRb, SIGNAL(toggled(bool)), SLOT(beginSymbolToggled(bool)));
      connect(continueTextRb, SIGNAL(toggled(bool)), SLOT(continueTextToggled(bool)));
      connect(continueSymbolRb, SIGNAL(toggled(bool)), SLOT(continueSymbolToggled(bool)));
      connect(beginTextTb, SIGNAL(clicked()), SLOT(beginTextProperties()));
      connect(continueTextTb, SIGNAL(clicked()), SLOT(continueTextProperties()));
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void LineProperties::accept()
      {
      tl->setLineWidth(Spatium(lineWidth->value()));
      tl->setLineStyle(Qt::PenStyle(lineStyle->currentIndex() + 1));
      tl->setLineColor(linecolor->color());

      tl->setBeginHookHeight(Spatium(beginHookHeight->value()));
      tl->setBeginHook(beginHook->isChecked());
      tl->setEndHookHeight(Spatium(endHookHeight->value()));
      tl->setEndHook(endHook->isChecked());

      if (beginTextRb->isChecked())
            tl->setBeginText(beginText->text());
      else
            tl->setBeginText(0);

      if (continueTextRb->isChecked())
            tl->setContinueText(continueText->text());
      else
            tl->setContinueText(0);

      int sym = beginSymbol->itemData(beginSymbol->currentIndex()).toInt();
      tl->setBeginSymbol(beginSymbolRb->isChecked() ? sym : -1);

      sym = continueSymbol->itemData(continueSymbol->currentIndex()).toInt();
      tl->setContinueSymbol(continueSymbolRb->isChecked() ? sym : -1);

      sym = endSymbol->itemData(endSymbol->currentIndex()).toInt();
      tl->setEndSymbol(endSymbolRb->isChecked() ? sym : -1);

      tl->setBeginTextPlace(beginTextPlace->currentIndex() == 0 ? PLACE_LEFT : PLACE_ABOVE);
      tl->setContinueTextPlace(continueTextPlace->currentIndex() == 0 ? PLACE_LEFT : PLACE_ABOVE);

      tl->setBeginSymbolOffset(QPointF(beginSymbolX->value(), beginSymbolY->value()));
      tl->setContinueSymbolOffset(QPointF(continueSymbolX->value(), continueSymbolY->value()));
      tl->setEndSymbolOffset(QPointF(endSymbolX->value(), endSymbolY->value()));

      tl->setDiagonal(diagonal->isChecked());
      QDialog::accept();
      }

//---------------------------------------------------------
//   beginTextToggled
//---------------------------------------------------------

void LineProperties::beginTextToggled(bool val)
      {
      if (val)
            beginSymbolRb->setChecked(false);
      beginText->setEnabled(val);
      beginTextPlace->setEnabled(val);
      beginTextTb->setEnabled(val);
      }

//---------------------------------------------------------
//   beginSymbolToggled
//---------------------------------------------------------

void LineProperties::beginSymbolToggled(bool val)
      {
      if (val)
            beginTextRb->setChecked(false);
      beginSymbol->setEnabled(val);
      beginSymbolX->setEnabled(val);
      beginSymbolY->setEnabled(val);
      }

//---------------------------------------------------------
//   continueTextToggled
//---------------------------------------------------------

void LineProperties::continueTextToggled(bool val)
      {
      if (val)
            continueSymbolRb->setChecked(false);
      continueText->setEnabled(val);
      continueTextPlace->setEnabled(val);
      continueTextTb->setEnabled(val);
      }

//---------------------------------------------------------
//   continueSymbolToggled
//---------------------------------------------------------

void LineProperties::continueSymbolToggled(bool val)
      {
      if (val)
            continueTextRb->setChecked(false);
      continueSymbol->setEnabled(val);
      continueSymbolX->setEnabled(val);
      continueSymbolY->setEnabled(val);
      }

//---------------------------------------------------------
//   beginTextProperties
//---------------------------------------------------------

void LineProperties::beginTextProperties()
      {
      if (!tl->beginText())
            return;
      TextProperties t(tl->beginText(), this);
      if (t.exec()) {
            foreach(LineSegment* ls, tl->lineSegments()) {
                  if (ls->segmentType() != SEGMENT_SINGLE && ls->segmentType() != SEGMENT_BEGIN)
                        continue;
                  TextLineSegment* tls = static_cast<TextLineSegment*>(ls);
                  if (!tls->text())
                        continue;
                  TextC* t = tls->text();
                  t->setColor(tl->beginText()->color());
                  }
            }
      }

//---------------------------------------------------------
//   continueTextProperties
//---------------------------------------------------------

void LineProperties::continueTextProperties()
      {
      if (!tl->continueText())
            return;
      TextProperties t(tl->continueText(), this);
      if (t.exec()) {
            foreach(LineSegment* ls, tl->lineSegments()) {
                  if (ls->segmentType() != SEGMENT_MIDDLE)
                        continue;
                  TextLineSegment* tls = static_cast<TextLineSegment*>(ls);
                  if (!tls->text())
                        continue;
                  TextC* t = tls->text();
                  t->setColor(tl->continueText()->color());
                  }
            }
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(TextC* v)
      {
      delete _beginText;
      _beginText = v;
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(TextC* v)
      {
      delete _continueText;
      _continueText = v;
      }

