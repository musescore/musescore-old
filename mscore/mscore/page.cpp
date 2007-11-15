//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: page.cpp,v 1.30 2006/03/28 14:58:58 wschweer Exp $
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

#include "page.h"
#include "score.h"
#include "text.h"
#include "xml.h"
#include "mscore.h"
#include "preferences.h"
#include "measure.h"
#include "style.h"
#include "layout.h"
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

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(ScoreLayout* s)
   : Element(s->score())
      {
      _layout    = s;
      _no        = 0;
      _pageNo    = 0;
      _copyright = 0;
      }

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

double Page::tm() const
      {
      PageFormat* pf = _layout->pageFormat();
      return (isOdd() ? pf->oddTopMargin : pf->evenTopMargin) * DPI;
      }

double Page::bm() const
      {
      PageFormat* pf = _layout->pageFormat();
      return (isOdd() ? pf->oddBottomMargin : pf->evenBottomMargin) * DPI;
      }

double Page::lm() const
      {
      PageFormat* pf = _layout->pageFormat();
      return (isOdd() ? pf->oddLeftMargin : pf->evenLeftMargin) * DPI;
      }

double Page::rm() const
      {
      PageFormat* pf = _layout->pageFormat();
      return (isOdd() ? pf->oddRightMargin : pf->evenRightMargin) * DPI;
      }

double Page::loWidth() const
      {
      return _layout->pageFormat()->width() * DPI;
      }
double Page::loHeight() const
      {
      return _layout->pageFormat()->height() * DPI;
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
//   bbox
//---------------------------------------------------------

QRectF Page::bbox() const
      {
      return QRectF(0, 0, loWidth(), loHeight());
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Page::layout(ScoreLayout* layout)
      {
      // add page number
      if (score()->style()->showPageNumber) {
            int n = no() + 1 + _score->_pageOffset;
            int subtype = (n & 1) ? TEXT_PAGE_NUMBER_ODD : TEXT_PAGE_NUMBER_EVEN;
            if ((n > 1) || score()->style()->showPageNumberOne) {
                  if (_pageNo == 0) {
                        _pageNo = new Text(score());
                        _pageNo->setSubtype(subtype);
                        _pageNo->setParent(this);
                        }
                  QString pnt;
                  pnt.setNum(n);
                  if (pnt != _pageNo->getText()) {
                        _pageNo->setText(QString("%1").arg(n));
                        _pageNo->layout(layout);
                        }
                  }
            else if (_pageNo) {
                  delete _pageNo;
                  _pageNo = 0;
                  }
            }
      else if (_pageNo) {
            delete _pageNo;
            _pageNo = 0;
            }

      // add copyright to page
      if (mscore->state() == STATE_EDIT) {      // for special case: edit copyright
            if (_copyright)
                  _copyright->layout(layout);
            }
      else {
            if (_score->rights) {
                  if (_copyright == 0) {
                        _copyright = new Text(score());
                        _copyright->setSubtype(TEXT_COPYRIGHT);
                        _copyright->setParent(this);
                        }
                  if (_copyright->getText() != _score->rights->toHtml()) {
                        _copyright->setHtml(_score->rights->toHtml());
                        _copyright->layout(layout);
                        }
                  }
            else if (_copyright) {
                  delete _copyright;
                  _copyright = 0;
                  }
            }
      }

//---------------------------------------------------------
//   draw
//    bounding rectange fr is relative to page QPointF
//---------------------------------------------------------

void Page::draw(QPainter& p) const
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
//   collectElements
//    collect all visible elements
//---------------------------------------------------------

void Page::collectElements(QList<const Element*>& el) const
      {
      if (_copyright)
            el.append(_copyright);
      if (_pageNo)
            el.append(_pageNo);
      int staves = score()->nstaves();
      foreach(const System* s, _systems) {
            if (s->isVbox())
                  continue;
            if (s->getBarLine())
                  el.append(s->getBarLine());
            for (int i = 0; i < staves; ++i) {
                  if (!score()->staff(i)->show())
                        continue;
                  SysStaff* st = s->staff(i);
                  if (st == 0)
                        continue;
                  foreach(Bracket* b, st->brackets) {
                        if (b)
                              el.append(b);
                        }
                  if (st->instrumentName)
                        el.append(st->instrumentName);
                  }
            }
      }

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

PageFormat::PageFormat()
      {
      size             = 0;   // A4
      evenLeftMargin   = 10.0 / INCH;
      evenRightMargin  = 10.0 / INCH;
      evenTopMargin    = 10.0 / INCH;
      evenBottomMargin = 20.0 / INCH;
      oddLeftMargin    = 10.0 / INCH;
      oddRightMargin   = 10.0 / INCH;
      oddTopMargin     = 10.0 / INCH;
      oddBottomMargin  = 20.0 / INCH;
      landscape        = false;
      twosided         = false;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* PageFormat::name() const
      {
      return paperSizes[size].name;
      }

//---------------------------------------------------------
//   width
//    return in inch
//---------------------------------------------------------

double PageFormat::width() const
      {
      return landscape ? paperSizes[size].h : paperSizes[size].w;
      }

//---------------------------------------------------------
//   height
//    return in inch
//---------------------------------------------------------

double PageFormat::height() const
      {
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
//    sizes are given in units of 1/10 spatium; this allows
//    to reuse this code with MusicXml routines
//---------------------------------------------------------

void PageFormat::read(QDomElement e)
      {
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
            else if (tag == "page-height")	// TODO
                  ;
            else if (tag == "page-width")	 	// TODO
                  ;
            else
                  domError(e);
            }
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

      if (name() != "Other") {
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
//   clear
//---------------------------------------------------------

void Page::clear()
      {
      _systems.clear();
      }

