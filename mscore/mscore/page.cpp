//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#include "mscore.h"
#include "preferences.h"
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

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(Score* s)
   : Element(s),
   _no(0), _footer(0), _header(0)
      {
      }

Page::~Page()
      {
      delete _footer;
      delete _header;
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

      // add page number
      int n = no() + 1 + _score->pageFormat()->_pageOffset;

      if (_score->styleB(ST_showHeader) && (n != 0 || _score->styleB(ST_headerFirstPage))) {
            if (_header == 0) {
                  _header = new Text(score());
                  _header->setFlag(ELEMENT_MOVABLE, false);
                  _header->setGenerated(true);
                  _header->setParent(this);
                  _header->setTextStyle(TEXT_STYLE_HEADER);
                  _header->setSubtype(TEXT_FOOTER);
                  _header->setLayoutToParentWidth(true);
                  }
            QString s;
            if (_score->styleB(ST_headerOddEven))
                  s = _score->styleSt(n & 0x1 ? ST_oddHeader : ST_evenHeader);
            else
                  s = _score->styleSt(ST_evenHeader);
            _header->setHtml(replaceTextMacros(s));
            _header->layout();
            }
      else {
            delete _header;
            _header = 0;
            }

      if (_score->styleB(ST_showFooter) && (n != 0 || _score->styleB(ST_footerFirstPage))) {
            if (_footer == 0) {
                  _footer = new Text(score());
                  _footer->setFlag(ELEMENT_MOVABLE, false);
                  _footer->setGenerated(true);
                  _footer->setParent(this);
                  _footer->setTextStyle(TEXT_STYLE_FOOTER);
                  _footer->setSubtype(TEXT_FOOTER);
                  _footer->setLayoutToParentWidth(true);
                  }
            QString s;
            if (_score->styleB(ST_footerOddEven))
                  s = _score->styleSt(n & 0x1 ? ST_oddFooter : ST_evenFooter);
            else
                  s = _score->styleSt(ST_evenFooter);
            _footer->setHtml(replaceTextMacros(s));
            _footer->layout();
            }
      else {
            delete _footer;
            _footer = 0;
            }
      }

//---------------------------------------------------------
//   draw
//    bounding rectange fr is relative to page QPointF
//---------------------------------------------------------

void Page::draw(QPainter& p, ScoreView*) const
      {
      QRectF r = bbox();
      qreal x1 = r.x();
      qreal y1 = r.y();
      qreal x2 = x1 + r.width();
      qreal y2 = y1 + r.height();

      //-----------------------------------------
      // draw page border gradients
      //-----------------------------------------

      const QColor c1("#befbbefbbefb");
      const QColor c2("#79e779e779e7");
      int h1, h2, s1, s2, v1, v2;
      int bw = 6;
      c2.getHsv(&h1, &s1, &v1);
      c1.getHsv(&h2, &s2, &v2);

      if ((no() & 1) == 0) {
            int bbw = bw/2-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw/2; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(c);
                  p.drawLine(QLineF(x1+i, y1, x1+i, y2));
                  }
            c1.getHsv(&h1, &s1, &v1);
            c2.getHsv(&h2, &s2, &v2);

            p.fillRect(QRectF(x2-bw, y1, bw, bw), preferences.bgColor);
            p.fillRect(QRectF(x2-bw, y2-bw, bw, bw), preferences.bgColor);

            bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(c);
                  p.drawLine(QLineF(x2-bw+i, y1+i+1, x2-bw+i, y2-i-1));
                  }
            }
      else {
            c2.getHsv(&h1, &s1, &v1);
            c1.getHsv(&h2, &s2, &v2);

            p.fillRect(QRectF(x1, y1, bw, bw), preferences.bgColor);
            p.fillRect(QRectF(x1, y2-bw, bw, bw), preferences.bgColor);
            int bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(c);
                  p.drawLine(QLineF(x1+i, y1+(bw-i), x1+i, y2-(bw-i)-1));
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
                  p.setPen(c);
                  p.drawLine(QLineF(x2-bw+i, y1, x2-bw+i, y2));
                  }
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(void* data, void (*func)(void*, Element*))
      {
      if (_header)
            func(data, _header);
      if (_footer)
            func(data, _footer);
      foreach(System* s, _systems)
            s->scanElements(data, func);
      func(data, this);
      }

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

PageFormat::PageFormat()
   : size(preferences.paperSize),
   _width(preferences.paperWidth),
   _height(preferences.paperHeight),
   evenLeftMargin(10.0 / INCH),
   evenRightMargin(10.0 / INCH),
   evenTopMargin(10.0 / INCH),
   evenBottomMargin(20.0 / INCH),
   oddLeftMargin(10.0 / INCH),
   oddRightMargin(10.0 / INCH),
   oddTopMargin(10.0 / INCH),
   oddBottomMargin(20.0 / INCH),
   landscape(preferences.landscape),
   twosided(preferences.twosided),
   _pageOffset(0)
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

void PageFormat::read(QDomElement e)
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
            else if (tag == "page-offset")
                  _pageOffset = val.toInt();
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

      if (_pageOffset)
            xml.tag("page-offset", _pageOffset);

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
//   rebuildBspTree
//---------------------------------------------------------

void Page::rebuildBspTree()
      {
      QList<Element*> el;
      foreach(System* s, _systems)
            foreach(MeasureBase* m, s->measures()) {
                  m->scanElements(&el, collectElements);
            }
      scanElements(&el, collectElements);

      // TODO: optimize away:
      QRectF bb(abbox());
      foreach(Beam* b, score()->beams()) {
            if (b->abbox().intersects(bb))
                  el.append(b);
            }

      int n = el.size();
      bspTree.initialize(abbox(), n);
      for (int i = 0; i < n; ++i)
            bspTree.insert(el.at(i));
      }

//---------------------------------------------------------
//   replaceTextMacros
//---------------------------------------------------------

QString Page::replaceTextMacros(const QString& s)
      {
      int pageno = no() + 1 + _score->pageFormat()->_pageOffset;
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
                        case '$':
                              d += '$';
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


