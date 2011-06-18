//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

static const double minSize = 0.1;      // minimum paper size for sanity check
static const double maxError = 0.01;    // max allowed error when matching sizes

static double sizeError(const double si, const double sref)
      {
      double relErr = (si - sref) / sref;
      return relErr > 0 ? relErr : -relErr;
      }

//---------------------------------------------------------
//   paperSizeSizeToIndex
//---------------------------------------------------------

int paperSizeSizeToIndex(const double wi, const double hi)
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

double Page::tm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided || isOdd()) ? pf->oddTopMargin : pf->evenTopMargin) * DPI;
      }

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

double Page::bm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided || isOdd()) ? pf->oddBottomMargin : pf->evenBottomMargin) * DPI;
      }

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

double Page::lm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided || isOdd()) ? pf->oddLeftMargin : pf->evenLeftMargin) * DPI;
      }

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

double Page::rm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided || isOdd()) ? pf->oddRightMargin : pf->evenRightMargin) * DPI;
      }

//---------------------------------------------------------
//   loWidth
//---------------------------------------------------------

double Page::loWidth() const
      {
      return score()->pageFormat()->width() * DPI;
      }

//---------------------------------------------------------
//   loHeight
//---------------------------------------------------------

double Page::loHeight() const
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
      QRectF r = bbox();
      qreal x1 = r.x();
      qreal y1 = r.y();
      qreal x2 = x1 + r.width();
      qreal y2 = y1 + r.height();

      //-----------------------------------------
      // draw page border gradients
      //-----------------------------------------

      if (!score()->printing()) {
            const QColor c1("#befbbefbbefb");
            const QColor c2("#79e779e779e7");
            int h1, h2, s1, s2, v1, v2;
            int bw = 6;
            c2.getHsv(&h1, &s1, &v1);
            c1.getHsv(&h2, &s2, &v2);

            if (no() & 1) {
                  int bbw = bw/2-1;
                  bbw = bbw >= 1 ? bbw : 1;
                  for (int i = 0; i < bw/2; ++i) {
                        QColor c;
                        c.setHsv(h1+((h2-h1)*i)/bbw,
                           s1+((s2-s1)*i)/bbw,
                           v1+((v2-v1)*i)/bbw);
                        painter->setPenColor(c);
                        painter->drawLine(x1+i, y1, x1+i, y2);
                        }
                  c1.getHsv(&h1, &s1, &v1);
                  c2.getHsv(&h2, &s2, &v2);

                  painter->setBrushColor(MScore::bgColor);
                  painter->fillRect(x2-bw, y1, bw, bw);
                  painter->fillRect(x2-bw, y2-bw, bw, bw);

                  bbw = bw-1;
                  bbw = bbw >= 1 ? bbw : 1;
                  for (int i = 0; i < bw; ++i) {
                        QColor c;
                        c.setHsv(h1+((h2-h1)*i)/bbw,
                           s1+((s2-s1)*i)/bbw,
                           v1+((v2-v1)*i)/bbw);
                        painter->setPenColor(c);
                        painter->drawLine(x2-bw+i, y1+i+1, x2-bw+i, y2-i-1);
                        }
                  }
            else {
                  c2.getHsv(&h1, &s1, &v1);
                  c1.getHsv(&h2, &s2, &v2);

                  painter->setBrushColor(MScore::bgColor);
                  painter->fillRect(x1, y1, bw, bw);
                  painter->fillRect(x1, y2-bw, bw, bw);
                  int bbw = bw-1;
                  bbw = bbw >= 1 ? bbw : 1;
                  for (int i = 0; i < bw; ++i) {
                        QColor c;
                        c.setHsv(h1+((h2-h1)*i)/bbw,
                           s1+((s2-s1)*i)/bbw,
                           v1+((v2-v1)*i)/bbw);
                        painter->setPenColor(c);
                        painter->drawLine(x1+i, y1+(bw-i), x1+i, y2-(bw-i)-1);
                        }
                  c1.getHsv(&h1, &s1, &v1);
                  c2.getHsv(&h2, &s2, &v2);

                  bw/=2;
                  bbw = bw-1;
                  bbw = bbw >= 1 ? bbw : 1;
                  for (int i = 0; i < bw; ++i) {
                           QColor c;
                           c.setHsv(h1+((h2-h1)*i)/bbw,
                           s1+((s2-s1)*i)/bbw,
                           v1+((v2-v1)*i)/bbw);
                        painter->setPenColor(c);
                        painter->drawLine(x2-bw+i, y1, x2-bw+i, y2);
                        }
                  }
            }
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
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, ts.foregroundColor(), -1);
                  }
            s = _score->styleSt(odd ? ST_oddHeaderC : ST_evenHeaderC);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, ts.foregroundColor(), -1);
                  }
            s = _score->styleSt(odd ? ST_oddHeaderR : ST_evenHeaderR);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, ts.foregroundColor(), -1);
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
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, ts.foregroundColor(), -1);
                  }
            s = _score->styleSt(odd ? ST_oddFooterC : ST_evenFooterC);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, ts.foregroundColor(), -1);
                  }
            s = _score->styleSt(odd ? ST_oddFooterR : ST_evenFooterR);
            if (!s.isEmpty()) {
                  d.setHtml(replaceTextMacros(s));
                  painter->drawText(&d, ts.foregroundColor(), -1);
                  }
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(void* data, void (*func)(void*, Element*))
      {
      foreach(System* s, _systems)
            s->scanElements(data, func);
      func(data, this);
      }

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

PageFormat::PageFormat()
   : size(MScore::paperSize),
   _width(MScore::paperWidth),
   _height(MScore::paperHeight),
   evenLeftMargin(10.0 / INCH),
   evenRightMargin(10.0 / INCH),
   evenTopMargin(10.0 / INCH),
   evenBottomMargin(20.0 / INCH),
   oddLeftMargin(10.0 / INCH),
   oddRightMargin(10.0 / INCH),
   oddTopMargin(10.0 / INCH),
   oddBottomMargin(20.0 / INCH),
   landscape(MScore::landscape),
   twosided(MScore::twosided)
      {
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PageFormat::name() const
      {
      return QString(paperSizes[size].name);
      }

//---------------------------------------------------------
//   width
//    return in inch
//---------------------------------------------------------

double PageFormat::width() const
      {
      if (paperSizes[size].qtsize == QPrinter::Custom)
            return landscape ? _height : _width;
      return landscape ? paperSizes[size].h : paperSizes[size].w;
      }

//---------------------------------------------------------
//   height
//    return in inch
//---------------------------------------------------------

double PageFormat::height() const
      {
      if (paperSizes[size].qtsize == QPrinter::Custom)
            return landscape ? _width : _height;
      return landscape ? paperSizes[size].w : paperSizes[size].h;
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
      landscape = false;      // for compatibility with old versions
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "pageFormat") {
                  size = paperSizeNameToIndex(val);
                  }
            else if (tag == "landscape")
                  landscape = i;
            else if (tag == "page-margins") {
                  QString type = e.attribute("type","both");
                  double lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
//                        double val = ee.text().toDouble() * (20/4)/ PPI  * .1;
                        double val = ee.text().toDouble() * 0.5 / PPI;
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
                  twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        oddLeftMargin   = lm;
                        oddRightMargin  = rm;
                        oddTopMargin    = tm;
                        oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        evenLeftMargin   = lm;
                        evenRightMargin  = rm;
                        evenTopMargin    = tm;
                        evenBottomMargin = bm;
                        }
                  }
            else if (tag == "page-height") {
                  size = paperSizeNameToIndex("Custom");
                  _height = val.toDouble() * 0.5 / PPI;
                  }
            else if (tag == "page-width") {
                  size = paperSizeNameToIndex("Custom");
                  _width = val.toDouble() * .5 / PPI;
                  }
            else if (tag == "page-offset")            // obsolete, moved to Score
                  score->setPageNumberOffset(val.toInt());
            else
                  domError(e);
            }
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

void PageFormat::readMusicXML(QDomElement e, double conversion)
      {
      landscape = false;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "page-margins") {
                  QString type = e.attribute("type","both");
                  double lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        QString tag(ee.tagName());
                        //double val = ee.text().toDouble() * (18/4)/ PPI  * .1;
                        //double val = ee.text().toDouble() * 0.45 / PPI; OLD!!!
                        double val = ee.text().toDouble() * conversion;
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
                  twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        oddLeftMargin   = lm;
                        oddRightMargin  = rm;
                        oddTopMargin    = tm;
                        oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        evenLeftMargin   = lm;
                        evenRightMargin  = rm;
                        evenTopMargin    = tm;
                        evenBottomMargin = bm;
                        }
                  }
            else if (tag == "page-height") {
                  size = paperSizeNameToIndex("Custom");
                  _height = val.toDouble() * conversion;
                  }
            else if (tag == "page-width") {
                  size = paperSizeNameToIndex("Custom");
                  _width = val.toDouble() * conversion;
                  }
            else
                  domError(e);
            }
      printf("PageFormat::readMusicXML size=%d, height=%g, width=%g\n",
      size, _height, _width);
      int match = paperSizeSizeToIndex(_width, _height);
      printf("PageFormat::readMusicXML match=%d\n", match);
      if (match >= 0) size = match;
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
      // double t = 10 * PPI / (20 / 4);
      double t = 2 * PPI;

      if (name() != "Custom") {
            xml.tag("pageFormat", QString(name()));
            if (landscape)
                  xml.tag("landscape", landscape);
            }
      else {
            xml.tag("page-height", height() * t);
            xml.tag("page-width", width() * t);
            }
      QString type("both");
      if (twosided) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   evenLeftMargin * t);
            xml.tag("right-margin",  evenRightMargin * t);
            xml.tag("top-margin",    evenTopMargin * t);
            xml.tag("bottom-margin", evenBottomMargin * t);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   oddLeftMargin * t);
      xml.tag("right-margin",  oddRightMargin * t);
      xml.tag("top-margin",    oddTopMargin * t);
      xml.tag("bottom-margin", oddBottomMargin * t);
      xml.etag();

      xml.etag();
      }

//---------------------------------------------------------
//   writeMusicXML
//---------------------------------------------------------

void PageFormat::writeMusicXML(Xml& xml, double conversion )
      {
      xml.stag("page-layout");

      //double t = 2 * PPI * 10 / 9;

      xml.tag("page-height", height() * conversion);
      xml.tag("page-width", width() * conversion);
      QString type("both");
      if (twosided) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   evenLeftMargin * conversion);
            xml.tag("right-margin",  evenRightMargin * conversion);
            xml.tag("top-margin",    evenTopMargin * conversion);
            xml.tag("bottom-margin", evenBottomMargin * conversion);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   oddLeftMargin * conversion);
      xml.tag("right-margin",  oddRightMargin * conversion);
      xml.tag("top-margin",    oddTopMargin * conversion);
      xml.tag("bottom-margin", oddBottomMargin * conversion);
      xml.etag();

      xml.etag();
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Page::clear()
      {
      _systems.clear();
      }

//---------------------------------------------------------
//   doRebuildBspTree
//---------------------------------------------------------

void Page::doRebuildBspTree()
      {
      QList<Element*> el;
      foreach(System* s, _systems)
            foreach(MeasureBase* m, s->measures()) {
                  m->scanVisibleElements(&el, collectElements, true);
            }
      scanElements(&el, collectElements);

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

