//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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

#include "image.h"
#include "xml.h"

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(Score* s)
   : BSymbol(s)
      {
      _dirty = false;
      mode   = NORMAL;
      setAnchor(ANCHOR_PAGE);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Image::write(Xml& xml) const
      {
      xml.stag("Image");
      Element::writeProperties(xml);
      xml.tag("path", _path);
      xml.tag("size", sz);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Image::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "path")
                  setPath(e.text());
            else if (tag == "size")
                  sz = readSize(node);
            else if (!Element::readProperties(node))
                  domError(node);
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void Image::setPath(const QString& s)
      {
      _path = s;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Image::bbox() const
      {
      QRectF r(0.0, 0.0, sz.width(), sz.height());
      if (mode) {
            r |= bbr1;
            r |= bbr2;
            }
      return r;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Image::startEdit(QMatrix& matrix, const QPointF&)
      {
      mode      = DRAG2;
      qreal w   = 8.0 / matrix.m11();
      qreal h   = 8.0 / matrix.m22();
      QRectF r(-w/2, -h/2, w, h);
      qreal lw  = 1.0 / matrix.m11();
      QRectF br = r.adjusted(-lw, -lw, lw, lw);
      QPointF pp1(sz.width(), sz.height() * .5);
      QPointF pp2(sz.width() * .5, sz.height());
      r1        = r.translated(pp1);
      bbr1      = br.translated(pp1);
      r2        = r.translated(pp2);
      bbr2      = br.translated(pp2);
      return true;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

bool Image::startEditDrag(Viewer*, const QPointF& p)
      {
      if (bbr1.contains(p))
            mode = DRAG1;
      else if (bbr2.contains(p))
            mode = DRAG2;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

bool Image::editDrag(Viewer*, QPointF*, const QPointF& d)
      {
      if (mode == DRAG1) {
            QPointF delta(d.x(), 0);    // only x-axis move
            r1.translate(delta);
            bbr1.translate(delta);
            sz.setWidth(sz.width() + d.x());
            }
      else {
            QPointF delta(0, d.y());    // only y-axis move
            r2.translate(delta);
            bbr2.translate(delta);
            sz.setHeight(sz.height() + d.y());
            }
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Image::edit(QKeyEvent* ev)
      {
      QPointF delta;
      switch (ev->key()) {
            case Qt::Key_Up:
                  _userOff.ry() += -.3;
                  break;
            case Qt::Key_Down:
                  _userOff.ry() += .3;
                  break;
            case Qt::Key_Left:
                  delta = QPointF(-1, 0);
                  break;
            case Qt::Key_Right:
                  delta = QPointF(1, 0);
                  break;
            case Qt::Key_Tab:
                  if (mode == DRAG1)
                        mode = DRAG2;
                  else if (mode == DRAG2)
                        mode = DRAG1;
                  break;
            }
      if (mode == DRAG1) {
            r1.moveTopLeft(r1.topLeft() + delta * _spatium);
            bbr1.moveTopLeft(bbr1.topLeft() + delta * _spatium);
            sz.setWidth(sz.width() + delta.x() * _spatium);
            }
      else if (mode == DRAG2) {
            r2.moveTopLeft(r2.topLeft() + delta * _spatium);
            bbr2.moveTopLeft(bbr2.topLeft() + delta * _spatium);
            sz.setHeight(sz.height() + delta.y() * _spatium);
            }
      return false;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

bool Image::endEditDrag()
      {
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Image::endEdit()
      {
      mode = NORMAL;
      }

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

SvgImage::SvgImage(Score* s)
   : Image(s)
      {
      doc = 0;
      }

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

SvgImage::~SvgImage()
      {
      if (doc)
            delete doc;
      }

SvgImage* SvgImage::clone() const
      {
      return new SvgImage(*this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SvgImage::draw(QPainter& p)
      {
      if (!doc)
            return;
/*      QSizeF s(sz.width() * p.device()->logicalDpiX() / 120.0,
              sz.height() * p.device()->logicalDpiY() / 120.0);
      */
      QSize s = sz.toSize();

      if (buffer.size() != s || _dirty) {
            buffer = QImage(s, QImage::Format_ARGB32_Premultiplied);
            buffer.fill(0x0);
            QPainter pp(&buffer);
            pp.setViewport(0, 0, s.width(), s.height());
            doc->render(&pp);
            _dirty = false;
            }
      p.drawImage(0, 0, buffer);
      if (mode != NORMAL) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            if (mode == DRAG1) {
                  p.setBrush(Qt::blue);
                  p.drawRect(r1);
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r2);
                  }
            else {
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r1);
                  p.setBrush(Qt::blue);
                  p.drawRect(r2);
                  }
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void SvgImage::setPath(const QString& s)
      {
      Image::setPath(s);
      if (doc == 0)
            doc = new QSvgRenderer;
      doc->load(s);
      if (doc->isValid()) {
            sz = doc->defaultSize();
            _dirty = true;
            }
      }

//---------------------------------------------------------
//   RasterImage
//---------------------------------------------------------

RasterImage::RasterImage(Score* s)
   : Image(s)
      {
      }

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

RasterImage::~RasterImage()
      {
      }

RasterImage* RasterImage::clone() const
      {
      return new RasterImage(*this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RasterImage::draw(QPainter& p)
      {
/*      QSizeF s(sz.width() * p.device()->logicalDpiX() / 120.0,
              sz.height() * p.device()->logicalDpiY() / 120.0);
      */
      QSize s = sz.toSize();

      if (buffer.size() != s || _dirty) {
            buffer = doc.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            _dirty = false;
            }
      p.drawImage(0, 0, buffer);
      if (mode != NORMAL) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            if (mode == DRAG1) {
                  p.setBrush(Qt::blue);
                  p.drawRect(r1);
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r2);
                  }
            else {
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r1);
                  p.setBrush(Qt::blue);
                  p.drawRect(r2);
                  }
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void RasterImage::setPath(const QString& s)
      {
      Image::setPath(s);
      doc.load(s);
      if (!doc.isNull()) {
            sz = doc.size();
            _dirty = true;
            }
      }

