//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2007 Werner Schweer and others
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
#include "mscore.h"
#include "preferences.h"

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(Score* s)
   : BSymbol(s)
      {
      _dirty = false;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Image::draw(QPainter& p) const
      {
      p.drawImage(0, 0, buffer);
      if (selected()) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
            QRectF r(0.0, 0.0, sz.width(), sz.height());
            p.drawRect(r);
            }
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

void Image::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "path")
                  setPath(e.text());
            else if (tag == "size")
                  sz = readSize(e);
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void Image::setPath(const QString& ss)
      {
      QString s(ss);
      if (s.startsWith(preferences.imagePath)) {
            s = s.mid(preferences.imagePath.size());
            if (s[0] == '/')
                  s = s.mid(1);
            }
      _path = s;
      }

//---------------------------------------------------------
//   path
//---------------------------------------------------------

QString Image::path() const
      {
      if (_path[0] == '/')
            return _path;
      return preferences.imagePath + '/' + _path;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Image::bbox() const
      {
      return QRectF(0.0, 0.0, sz.width(), sz.height());
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Image::startEdit(const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Image::editDrag(int curGrip, const QPointF& d)
      {
      if (curGrip == 0)
            sz.setWidth(sz.width() + d.x());
      else
            sz.setHeight(sz.height() + d.y());
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Image::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      QRectF r(abbox());
      grip[0].translate(QPointF(r.x() + r.width(), r.y() + r.height() * .5));
      grip[1].translate(QPointF(r.x() + r.width() * .5, r.y() + r.height()));
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF Image::gripAnchor(int) const
      {
      return QPointF();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Image::endEdit()
      {
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

void SvgImage::draw(QPainter& p) const
      {
      if (!doc)
            return;
      QSizeF sf(sz.width() * p.device()->logicalDpiX() / mscore->logicalDpiX(),
              sz.height() * p.device()->logicalDpiY() / mscore->logicalDpiY());
      QSize s = sf.toSize();

      if (buffer.size() != s || _dirty) {
            buffer = QImage(s, QImage::Format_ARGB32_Premultiplied);
            buffer.fill(0x0);
            QPainter pp(&buffer);
            pp.setViewport(0, 0, s.width(), s.height());
            doc->render(&pp);
            _dirty = false;
            }
      Image::draw(p);
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void SvgImage::setPath(const QString& s)
      {
      Image::setPath(s);
      if (doc == 0)
            doc = new QSvgRenderer;
      doc->load(path());
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

void RasterImage::draw(QPainter& p) const
      {
      QSizeF sf(sz.width() * p.device()->logicalDpiX() / mscore->logicalDpiX(),
              sz.height() * p.device()->logicalDpiY() / mscore->logicalDpiY());
      QSize s = sf.toSize();

      if (buffer.size() != s || _dirty) {
            buffer = doc.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            _dirty = false;
            }
      Image::draw(p);
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void RasterImage::setPath(const QString& s)
      {
      Image::setPath(s);
      doc.load(path());
      if (!doc.isNull()) {
            sz = doc.size();
            _dirty = true;
            }
      }

