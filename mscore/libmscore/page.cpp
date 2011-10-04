//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "page.h"
#include "score.h"
#include "text.h"
#include "xml.h"
#include "measure.h"
#include "style.h"
#include "chord.h"
#include "beam.h"
#include "tuplet.h"
#include "note.h"
#include "barline.h"
#include "slur.h"
#include "hook.h"
#include "lyrics.h"
#include "bracket.h"
#include "line.h"
#include "staff.h"
#include "system.h"
#include "painter.h"
#include "mscore.h"

#define MM(x) ((x)/INCH)

const PaperSize paperSizes[] = {
      PaperSize(QPrinter::A4,      "A4",        MM(210),  MM(297)),
      PaperSize(QPrinter::B5,      "B5",        MM(176),  MM(250)),
      PaperSize(QPrinter::Letter,  "Letter",    8.5,      11),
      PaperSize(QPrinter::Legal,   "Legal",     8.5,      14),
      PaperSize(QPrinter::Executive,"Executive",7.5,      10),
      PaperSize(QPrinter::A0,      "A0",        MM(841),  MM(1189)),
      PaperSize(QPrinter::A1,      "A1",        MM(594),  MM(841)),
      PaperSize(QPrinter::A2,      "A2",        MM(420),  MM(594)),
      PaperSize(QPrinter::A3,      "A3",        MM(297),  MM(420)),
      PaperSize(QPrinter::A5,      "A5",        MM(148),  MM(210)),
      PaperSize(QPrinter::A6,      "A6",        MM(105),  MM(148)),
      PaperSize(QPrinter::A7,      "A7",        MM(74),   MM(105)),
      PaperSize(QPrinter::A8,      "A8",        MM(52),   MM(74)),
      PaperSize(QPrinter::A9,      "A9",        MM(37),   MM(52)),
      PaperSize(QPrinter::B0,      "B0",        MM(1000), MM(1414)),
      PaperSize(QPrinter::B1,      "B1",        MM(707),  MM(1000)),
      PaperSize(QPrinter::B10,     "B10",       MM(31),   MM(44)),
      PaperSize(QPrinter::B2,      "B2",        MM(500),  MM(707)),
      PaperSize(QPrinter::B3,      "B3",        MM(353),  MM(500)),
      PaperSize(QPrinter::B4,      "B4",        MM(250),  MM(353)),
      PaperSize(QPrinter::B5,      "B5",        MM(125),  MM(176)),
      PaperSize(QPrinter::B6,      "B6",        MM(88),   MM(125)),
      PaperSize(QPrinter::B7,      "B7",        MM(62),   MM(88)),
      PaperSize(QPrinter::B8,      "B8",        MM(44),   MM(62)),
      PaperSize(QPrinter::B9,      "B9",        MM(163),  MM(229)),
      PaperSize(QPrinter::Comm10E, "Comm10E",   MM(105),  MM(241)),
      PaperSize(QPrinter::DLE,     "DLE",       MM(110),  MM(220)),
      PaperSize(QPrinter::Folio,   "Folio",     MM(210),  MM(330)),
      PaperSize(QPrinter::Ledger,  "Ledger",    MM(432),  MM(279)),
      PaperSize(QPrinter::Tabloid, "Tabloid",   MM(279),  MM(432)),
      PaperSize(int(QPrinter::Custom)+1, "iPad", MM(148),  MM(197)),
      PaperSize(QPrinter::Custom,  "Custom",    MM(210),  MM(297)),
      PaperSize(QPrinter::A4,      0, 0, 0  )
      };

//---------------------------------------------------------
//   paperSizeNameToIndex
//---------------------------------------------------------

int paperSizeNameToIndex(const QString& name)
      {
      int i;
      for (i = 0;;++i) {
            if (paperSizes[i].name == 0)
                  break;
            if (name == paperSizes[i].name)
                  return i;
            }
      printf("unknown paper size\n");
      return 0;
      }

//---------------------------------------------------------
//   paperSizeSizeToIndex
//---------------------------------------------------------

static const qreal minSize = 0.1;      // minimum paper size for sanity check
static const qreal maxError = 0.01;    // max allowed error when matching sizes

static qreal sizeError(const qreal si, const qreal sref)
      {
      qreal relErr = (si - sref) / sref;
      return relErr > 0 ? relErr : -relErr;
      }

//---------------------------------------------------------
//   paperSizeSizeToIndex
//---------------------------------------------------------

int paperSizeSizeToIndex(const qreal wi, const qreal hi)
      {
      if (wi < minSize || hi < minSize)
            return -1;
      int i;
      for (i = 0;;++i) {
            if (paperSizes[i].name == 0)
                  break;
            if (sizeError(wi, paperSizes[i].w) < maxError && sizeError(hi, paperSizes[i].h) < maxError)
                  return i;
            }
      printf("unknown paper size\n");
      return -1;
      }

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(Score* s)
   : Element(s),
   _no(0)
      {
      bspTreeValid = false;
      }

Page::~Page()
      {
      }

//---------------------------------------------------------
//   items
//---------------------------------------------------------

QList<const Element*> Page::items(const QRectF& r)
      {
      if (!bspTreeValid)
            doRebuildBspTree();
      return bspTree.items(r);
      }

QList<const Element*> Page::items(const QPointF& p)
      {
      if (!bspTreeValid)
            doRebuildBspTree();
      return bspTree.items(p);
      }

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

qreal Page::tm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddTopMargin() : pf->evenTopMargin()) * DPI;
      }

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

qreal Page::bm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddBottomMargin() : pf->evenBottomMargin()) * DPI;
      }

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

qreal Page::lm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddLeftMargin() : pf->evenLeftMargin()) * DPI;
      }

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

qreal Page::rm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided() || isOdd()) ? pf->oddRightMargin() : pf->evenRightMargin()) * DPI;
      }

//---------------------------------------------------------
//   loWidth
//---------------------------------------------------------

qreal Page::loWidth() const
      {
      return score()->pageFormat()->width() * DPI;
      }

//---------------------------------------------------------
//   loHeight
//---------------------------------------------------------

qreal Page::loHeight() const
      {
      return score()->pageFormat()->height() * DPI;
      }

//---------------------------------------------------------
//   appendSystem
//---------------------------------------------------------

void Page::appendSystem(System* s)
      {
      s->setParent(this);
      _systems.append(s);
      }

//---------------------------------------------------------
//   setNo
//---------------------------------------------------------

void Page::setNo(int n)
      {
      _no = n;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Page::layout()
      {
      setbbox(QRectF(0.0, 0.0, loWidth(), loHeight()));
      }

//---------------------------------------------------------
//   draw
//    bounding rectange fr is relative to page QPointF
//---------------------------------------------------------

void Page::draw(Painter* painter) const
      {
//      QRectF r = bbox();
//      qreal x1 = r.x();
//      qreal y1 = r.y();
//      qreal x2 = x1 + r.width();
//      qreal y2 = y1 + r.height();

      //
      // draw header/footer
      //

      QTextDocument d;
      int n = no() + 1 + _score->pageNumberOffset();
      d.setTextWidth(loWidth() - lm() - rm());

      painter->translate(QPointF(lm(), 0.0));

      if (_score->styleB(ST_showHeader) && (n || _score->styleB(ST_headerFirstPage))) {
            TextStyle ts = score()->textStyle(TEXT_STYLE_HEADER);

            QPointF o(ts.xoff(), ts.yoff());
            if (ts.offsetType() == OFFSET_SPATIUM)
                  o *= spatium();
            else
                  o *= DPI;
            painter->translate(o);
            d.setTextWidth(loWidth() - lm() - rm() - (2.0 * o.x()));

            bool odd = (n & 1) && _score->styleB(ST_headerOddEven);
            QString s = _score->styleSt(odd ? ST_oddHeaderL : ST_evenHeaderL);

            QAbstractTextDocumentLayout::PaintContext c;
            c.cursorPosition = -1;
            c.palette.setColor(QPalette::Text, ts.foregroundColor());

            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, c);
                  }
            s = _score->styleSt(odd ? ST_oddHeaderC : ST_evenHeaderC);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, c);
                  }
            s = _score->styleSt(odd ? ST_oddHeaderR : ST_evenHeaderR);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, c);
                  }
            painter->translate(-o);
            }
      if (_score->styleB(ST_showFooter) && (n || _score->styleB(ST_footerFirstPage))) {
            TextStyle ts = score()->textStyle(TEXT_STYLE_FOOTER);

            QPointF o(ts.xoff(), ts.yoff());
            if (ts.offsetType() == OFFSET_SPATIUM)
                  o *= spatium();
            else
                  o *= DPI;
            painter->translate(o);
            d.setTextWidth(loWidth() - lm() - rm() - (2.0 * o.x()));

            bool odd = (n & 1) && _score->styleB(ST_footerOddEven);
            QString s = _score->styleSt(odd ? ST_oddFooterL : ST_evenFooterL);
            painter->translate(QPointF(0.0, loHeight() - (tm()+bm())));

            QAbstractTextDocumentLayout::PaintContext c;
            c.cursorPosition = -1;
            c.palette.setColor(QPalette::Text, ts.foregroundColor());

            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, c);
                  }
            s = _score->styleSt(odd ? ST_oddFooterC : ST_evenFooterC);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, c);
                  }
            s = _score->styleSt(odd ? ST_oddFooterR : ST_evenFooterR);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, c);
                  }
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      foreach(System* s, _systems)
            s->scanElements(data, func, all);
      func(data, this);
      }

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

PageFormat::PageFormat()
      {
      _size             = MScore::paperSize;
      _width            = MScore::paperWidth;
      _height           = MScore::paperHeight;
      _evenLeftMargin   = 10.0 / INCH;
      _oddLeftMargin    = 10.0 / INCH;
      _printableWidth   = _width - 20.0 / INCH;
      _evenTopMargin    = 10.0 / INCH;
      _evenBottomMargin = 20.0 / INCH;
      _oddTopMargin     = 10.0 / INCH;
      _oddBottomMargin  = 20.0 / INCH;
      _landscape        = MScore::landscape;
      _twosided         = MScore::twosided;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PageFormat::name() const
      {
      return QString(paperSizes[_size].name);
      }

//---------------------------------------------------------
//   width
//    return in inch
//---------------------------------------------------------

qreal PageFormat::width() const
      {
      if (paperSizes[_size].qtsize == QPrinter::Custom)
            return _landscape ? _height : _width;
      return _landscape ? paperSizes[_size].h : paperSizes[_size].w;
      }

//---------------------------------------------------------
//   height
//    return in inch
//---------------------------------------------------------

qreal PageFormat::height() const
      {
      if (paperSizes[_size].qtsize == QPrinter::Custom)
            return _landscape ? _width : _height;
      return _landscape ? paperSizes[_size].w : paperSizes[_size].h;
      }

//---------------------------------------------------------
//   read
//  <page-layout>
//	  <page-height>
//	  <page-width>
//      <pageFormat>A6</pageFormat>
//      <landscape>1</landscape>
//      <page-margins>
//         <left-margin>28.3465</left-margin>
//         <right-margin>28.3465</right-margin>
//         <top-margin>28.3465</top-margin>
//         <bottom-margin>56.6929</bottom-margin>
//         </page-margins>
//      </page-layout>
//
//    sizes are given in units of 1/10 spatium;
//---------------------------------------------------------

void PageFormat::read(QDomElement e, Score* score)
      {
      _landscape = false;      // for compatibility with old versions
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      QString type;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "pageFormat")
                  _size = paperSizeNameToIndex(val);
            else if (tag == "landscape")
                  _landscape = i;
            else if (tag == "page-margins") {
                  type = e.attribute("type","both");
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        qreal val = ee.text().toDouble() * 0.5 / PPI;
                        if (tag == "left-margin")
                              lm = val;
                        else if (tag == "right-margin")
                              rm = val;
                        else if (tag == "top-margin")
                              tm = val;
                        else if (tag == "bottom-margin")
                              bm = val;
                        else
                              domError(ee);
                        }
                  _twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        _oddLeftMargin   = lm;
                        _oddRightMargin  = rm;
                        _oddTopMargin    = tm;
                        _oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        _evenLeftMargin   = lm;
                        _evenRightMargin  = rm;
                        _evenTopMargin    = tm;
                        _evenBottomMargin = bm;
                        }
                  }
            else if (tag == "page-height") {
                  _size = paperSizeNameToIndex("Custom");
                  _height = val.toDouble() * 0.5 / PPI;
                  }
            else if (tag == "page-width") {
                  _size = paperSizeNameToIndex("Custom");
                  _width = val.toDouble() * .5 / PPI;
                  }
            else if (tag == "page-offset")            // obsolete, moved to Score
                  score->setPageNumberOffset(val.toInt());
            else
                  domError(e);
            }
      qreal w1 = width() - _oddLeftMargin - _oddRightMargin;
      qreal w2 = width() - _evenLeftMargin - _evenRightMargin;
      _printableWidth = qMax(w1, w2);     // silently adjust right margins
      }

//---------------------------------------------------------
//   read
//  <page-layout>
//      <page-height>
//      <page-width>
//      <page-margins>
//         <left-margin>28.3465</left-margin>
//         <right-margin>28.3465</right-margin>
//         <top-margin>28.3465</top-margin>
//         <bottom-margin>56.6929</bottom-margin>
//         </page-margins>
//      </page-layout>
//
//    sizes are given in units of 1/10 spatium;
//---------------------------------------------------------

void PageFormat::readMusicXML(QDomElement e, qreal conversion)
      {
      _landscape = false;
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "page-margins") {
                  QString type = e.attribute("type","both");
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        //qreal val = ee.text().toDouble() * (18/4)/ PPI  * .1;
                        //qreal val = ee.text().toDouble() * 0.45 / PPI; OLD!!!
                        qreal val = ee.text().toDouble() * conversion;
                        if (tag == "left-margin")
                              lm = val;
                        else if (tag == "right-margin")
                              rm = val;
                        else if (tag == "top-margin")
                              tm = val;
                        else if (tag == "bottom-margin")
                              bm = val;
                        else
                              domError(ee);
                        }
                  _twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        _oddLeftMargin   = lm;
                        _oddRightMargin  = rm;
                        _oddTopMargin    = tm;
                        _oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        _evenLeftMargin   = lm;
                        _evenRightMargin  = rm;
                        _evenTopMargin    = tm;
                        _evenBottomMargin = bm;
                        }
                  }
            else if (tag == "page-height") {
                  _size = paperSizeNameToIndex("Custom");
                  _height = val.toDouble() * conversion;
                  }
            else if (tag == "page-width") {
                  _size = paperSizeNameToIndex("Custom");
                  _width = val.toDouble() * conversion;
                  }
            else
                  domError(e);
            }
      int match = paperSizeSizeToIndex(_width, _height);
      if (match >= 0)
            _size = match;
      qreal w1 = width() - _oddLeftMargin - _oddRightMargin;
      qreal w2 = width() - _evenLeftMargin - _evenRightMargin;
      _printableWidth = qMax(w1, w2);     // silently adjust right margins
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void PageFormat::write(Xml& xml)
      {
      xml.stag("page-layout");

      // convert inch to 1/10 spatium units
      // 20 - font design size in point
      // SPATIUM = 20/4
      // qreal t = 10 * PPI / (20 / 4);
      qreal t = 2 * PPI;

      if (name() != "Custom") {
            xml.tag("pageFormat", QString(name()));
            if (_landscape)
                  xml.tag("landscape", _landscape);
            }
      else {
            xml.tag("page-height", height() * t);
            xml.tag("page-width", width() * t);
            }
      QString type("both");
      if (_twosided) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   evenLeftMargin() * t);
            xml.tag("right-margin",  evenRightMargin() * t);
            xml.tag("top-margin",    evenTopMargin() * t);
            xml.tag("bottom-margin", evenBottomMargin() * t);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   oddLeftMargin() * t);
      xml.tag("right-margin",  oddRightMargin() * t);
      xml.tag("top-margin",    oddTopMargin() * t);
      xml.tag("bottom-margin", oddBottomMargin() * t);
      xml.etag();

      xml.etag();
      }

//---------------------------------------------------------
//   writeMusicXML
//---------------------------------------------------------

void PageFormat::writeMusicXML(Xml& xml, qreal conversion )
      {
      xml.stag("page-layout");

      //qreal t = 2 * PPI * 10 / 9;

      xml.tag("page-height", height() * conversion);
      xml.tag("page-width", width() * conversion);
      QString type("both");
      if (_twosided) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   evenLeftMargin() * conversion);
            xml.tag("right-margin",  evenRightMargin() * conversion);
            xml.tag("top-margin",    evenTopMargin() * conversion);
            xml.tag("bottom-margin", evenBottomMargin() * conversion);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   oddLeftMargin() * conversion);
      xml.tag("right-margin",  oddRightMargin() * conversion);
      xml.tag("top-margin",    oddTopMargin() * conversion);
      xml.tag("bottom-margin", oddBottomMargin() * conversion);
      xml.etag();

      xml.etag();
      }

//---------------------------------------------------------
//   doRebuildBspTree
//---------------------------------------------------------

void Page::doRebuildBspTree()
      {
      QList<Element*> el;
      foreach(System* s, _systems) {
            foreach(MeasureBase* m, s->measures()) {
                  m->scanElements(&el, collectElements, false);
                  }
            }
      scanElements(&el, collectElements, false);

      int n = el.size();
      bspTree.initialize(abbox(), n);
      for (int i = 0; i < n; ++i)
            bspTree.insert(el.at(i));
      }

//---------------------------------------------------------
//   replaceTextMacros
//    $p          - page number
//    $$          - $
//    $n          - number of pages
//    $f          - file name
//    $F          - file path+name
//    $d          - current date
//    $D          - creation date
//    $:tag:      - meta data tag
//       already defined tags:
//       movementNumber
//       movementTitle
//       workNumber
//       workTitle
//       source
//       copyright
//---------------------------------------------------------

QString Page::replaceTextMacros(const QString& s) const
      {
      int pageno = no() + 1 + _score->pageNumberOffset();
      QString d;
      int n = s.size();
      for (int i = 0; i < n; ++i) {
            QChar c = s[i];
            if (c == '$' && (i < (n-1))) {
                  QChar c = s[i+1];
                  switch(c.toAscii()) {
                        case 'p':
                              d += QString("%1").arg(pageno);
                              break;
                        case 'n':
                              d += QString("%1").arg(_score->pages().size());
                              break;
                        case 'f':
                              d += _score->name();
                              break;
                        case 'F':
                              d += _score->absoluteFilePath();
                              break;
                        case 'd':
                              d += QDate::currentDate().toString(Qt::DefaultLocaleShortDate);
                              break;
                        case 'D':
                              d += _score->creationDate().toString(Qt::DefaultLocaleShortDate);
                              break;
                        case '$':
                              d += '$';
                              break;
                        case ':':
                              {
                              QString tag;
                              int k = i+2;
                              for (; k < n; ++k) {
                                    if (s[k].toAscii() == ':')
                                          break;
                                    tag += s[k];
                                    }
                              if (k != n) {       // found ':' ?
                                    d += score()->metaTag(tag);
                                    i = k-1;
                                    }
                              }
                              break;
                        default:
                              d += '$';
                              d += c;
                              break;
                        }
                  ++i;
                  }
            else
                  d += c;
            }
      return d;
      }

//---------------------------------------------------------
//   isOdd
//---------------------------------------------------------

bool Page::isOdd() const
      {
      return (_no + 1 + score()->pageNumberOffset()) & 1;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Page::write(Xml& xml) const
      {
      xml.stag("Page");
      QList<System*> _systems;
      foreach(System* system, _systems) {
            system->write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Page::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());

            if (tag == "System") {
                  System* system = new System(score());
                  score()->systems()->append(system);
                  system->read(e);
                  }
            else
                  domError(e);
            }
      }
