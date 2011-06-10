//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: image.cpp 3592 2010-10-18 17:24:18Z wschweer $
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
#include "m-al/xml.h"
#include "preferences.h"
#include "score.h"
#include "painter.h"

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(Score* s)
   : BSymbol(s)
      {
      _ip              = 0;
      _dirty           = false;
      _lockAspectRatio = true;
      _autoScale       = false;
      }

//---------------------------------------------------------
//   reference
//---------------------------------------------------------

void Image::reference()
      {
      _ip->reference();
      }

//---------------------------------------------------------
//   dereference
//---------------------------------------------------------

void Image::dereference()
      {
      _ip->dereference();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Image::draw(Painter*) const
      {
//      p->drawPixmap(buffer);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Image::read(XmlReader* r)
      {
      while (r->readElement()) {
            QString s;
            if (r->readString("path", &s))
                  setPath(s);
            else if (r->readSize("size", &sz))
                  sz *= DPMM;
            else if (r->readBool("lockAspectRatio", &_lockAspectRatio))
                  ;
            else if (r->readBool("autoScale", &_autoScale))
                  ;
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void Image::setPath(const QString& s)
      {
//      _ip = score()->addImage(s);
      }

//---------------------------------------------------------
//   path
//---------------------------------------------------------

QString Image::path() const
      {
      return _ip->path();
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Image::bbox() const
      {
      return QRectF(0.0, 0.0, sz.width(), sz.height());
      }

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

SvgImage::SvgImage(Score* s)
   : Image(s)
      {
//      doc = 0;
      }

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

SvgImage::~SvgImage()
      {
//      delete doc;
      }

SvgImage* SvgImage::clone() const
      {
      return new SvgImage(*this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SvgImage::draw(Painter* /*p*/) const
      {
#if 0
      if (!doc)
            return;
      QSize s = sz.toSize();

      if (buffer.size() != s || _dirty) {
            buffer = QPixmap(s);
            buffer.fill();
            QPainter pp(&buffer);
            pp.setViewport(0, 0, s.width(), s.height());
            doc->render(&pp);
            _dirty = false;
            }
      Image::draw(p, v);
#endif
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void SvgImage::setPath(const QString& /*s*/)
      {
#if 0
      Image::setPath(s);
      if (doc == 0)
            doc = new QSvgRenderer;
      if (_ip->loaded())
            doc->load(_ip->buffer().buffer());
      else
            doc->load(path());
      if (doc->isValid()) {
            sz = doc->defaultSize();
            _dirty = true;
            }
#endif
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

void RasterImage::draw(Painter*) const
      {
#if 0
      QTransform t = p.worldTransform();
      QSize s      = QSizeF(sz.width() * t.m11(), sz.height() * t.m22()).toSize();
      t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
      p.setWorldTransform(t);
      if (buffer.size() != s || _dirty) {
            buffer = QPixmap::fromImage(doc.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            _dirty = false;
            }
      Image::draw(p, v);
#endif
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void RasterImage::setPath(const QString&)
      {
#if 0
      Image::setPath(s);
      if (_ip->loaded())
            doc.loadFromData(_ip->buffer().buffer());
      else
            doc.load(path());
      if (!doc.isNull()) {
            sz = doc.size() * 0.4;
            _dirty = true;
            }
#endif
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Image::layout()
      {
#if 0
      if (!autoScale() || !parent() || (parent()->type() != HBOX && parent()->type() != VBOX))
            return;

      if (_lockAspectRatio) {
            QSizeF size(imageSize());
            qreal ratio = size.width() / size.height();
            qreal w = parent()->width();
            qreal h = parent()->height();
            if ((w / h) < ratio) {
                  sz.setWidth(w);
                  sz.setHeight(w / ratio);
                  }
            else {
                  sz.setHeight(h);
                  sz.setWidth(h * ratio);
                  }
            }
      else
            sz = parent()->bbox().size();
#endif
      }

