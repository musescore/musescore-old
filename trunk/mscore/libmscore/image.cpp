//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "image.h"
#include "xml.h"
#include "score.h"
#include "undo.h"
#include "painter.h"
#include "mscore.h"

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
      setZ(IMAGE * 100);
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

void Image::draw(Painter* painter) const
      {
      painter->drawPixmap(0, 0, buffer);
      if (selected() && !(score() && score()->printing())) {
            painter->setNoBrush(true);
            painter->setPenColor(Qt::blue);   // , 0, Qt::SolidLine));

            QPointF p[5];
            qreal w = buffer.size().width();
            qreal h = buffer.size().height();
            p[0] = QPointF(0.0, 0.0);
            p[1] = QPointF(w,   0.0);
            p[2] = QPointF(w,   h);
            p[3] = QPointF(0.0, h);
            p[4] = QPointF(0.0, 0.0);
            painter->drawPolyline(p, 5);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Image::write(Xml& xml) const
      {
      xml.stag("Image");
      Element::writeProperties(xml);
      xml.tag("path", path());
      if (!_autoScale)
            xml.tag("size", sz / DPMM);
      if (!_lockAspectRatio)
            xml.tag("lockAspectRatio", _lockAspectRatio);
      if (_autoScale)
            xml.tag("autoScale", _autoScale);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Image::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "path")
                  setPath(e.text());
            else if (tag == "size") {
                  sz = readSize(e);
                  if (score()->mscVersion() >= 109)
                        sz *= DPMM;
                  }
            else if (tag == "lockAspectRatio")
                  _lockAspectRatio = e.text().toInt();
            else if (tag == "autoScale")
                  _autoScale = e.text().toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void Image::setPath(const QString& ss)
      {
      _ip = score()->addImage(ss);
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
//   editDrag
//---------------------------------------------------------

void Image::editDrag(const EditData& ed)
      {
      qreal ratio = sz.width() / sz.height();
      if (ed.curGrip == 0) {
            sz.setWidth(sz.width() + ed.delta.x());
            if (_lockAspectRatio)
                  sz.setHeight(sz.width() / ratio);
            }
      else {
            sz.setHeight(sz.height() + ed.delta.y());
            if (_lockAspectRatio)
                  sz.setWidth(sz.height() * ratio);
            }
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

#ifdef SVG_IMAGES
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
      delete doc;
      }

SvgImage* SvgImage::clone() const
      {
      return new SvgImage(*this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SvgImage::draw(Painter* painter) const
      {
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
      Image::draw(painter);
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void SvgImage::setPath(const QString& s)
      {
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
      }
#endif

//---------------------------------------------------------
//   RasterImage
//---------------------------------------------------------

RasterImage::RasterImage(Score* s)
   : Image(s)
      {
      }

//---------------------------------------------------------
//   RasterImage
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

void RasterImage::draw(Painter* painter) const
      {
      if (score()->printing()) {
            // use original image size for printing
            painter->scale(sz.width() / doc.width(), sz.height() / doc.height());
            painter->drawPixmap(0, 0, QPixmap::fromImage(doc));
            }
      else {
            QTransform t = painter->transform();
            QSize s = QSizeF(sz.width() * t.m11(), sz.height() * t.m22()).toSize();
            t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
            painter->setTransform(t);
            if (buffer.size() != s || _dirty) {
                  buffer = QPixmap::fromImage(doc.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                  _dirty = false;
                  }
            Image::draw(painter);
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void RasterImage::setPath(const QString& s)
      {
      Image::setPath(s);
      if (_ip->loaded())
            doc.loadFromData(_ip->buffer().buffer());
      else
            doc.load(path());
      if (!doc.isNull()) {
            sz = doc.size() * 0.4;
            _dirty = true;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Image::layout()
      {
      // if autoscale && inside a box, scale to box relevant size
      if (autoScale() && parent() && ((parent()->type() == HBOX || parent()->type() == VBOX))) {
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
            }

      // in any case, adjust position relative to parent
      adjustReadPos();
      }

