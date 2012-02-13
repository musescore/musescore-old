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
#include "mscore.h"
#include "imageStore.h"

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(Score* s)
   : BSymbol(s)
      {
      _storeItem       = 0;
      _dirty           = false;
      _lockAspectRatio = true;
      _autoScale       = false;
      setZ(IMAGE * 100);
      }

Image::Image(const Image& img)
   : BSymbol(img)
      {
      buffer           = img.buffer;
      sz               = img.sz;
      _lockAspectRatio = img._lockAspectRatio;
      _autoScale       = img._autoScale;
      _dirty           = img._dirty;
      _storeItem       = img._storeItem;
      _storeItem->reference(this);
      }

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::~Image()
      {
      if (_storeItem)
            _storeItem->dereference(this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Image::draw(QPainter* painter, QSize size) const
      {
      if (buffer.isNull()) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::black);
            QPointF p[6];
            qreal w = size.width();
            qreal h = size.height();
            p[0] = QPointF(0.0, 0.0);
            p[1] = QPointF(w,   0.0);
            p[2] = QPointF(w,   h);
            p[3] = QPointF(0.0, 0.0);
            p[4] = QPointF(0.0, h);
            p[5] = QPointF(w,  0.0);
            painter->drawPolyline(p, 6);
            p[0] = QPointF(0.0, h);
            p[1] = QPointF(w, h);
            painter->drawPolyline(p, 2);
            }
      else
            painter->drawPixmap(QPointF(0.0, 0.0), buffer);
      if (selected() && !(score() && score()->printing())) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(Qt::blue);

            QPointF p[5];
            qreal w = size.width();
            qreal h = size.height();

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
      xml.tag("path", _storeItem ? _storeItem->hashName() : _path);
      if (!_lockAspectRatio)
            xml.tag("lockAspectRatio", _lockAspectRatio);
      if (_autoScale)
            xml.tag("autoScale", _autoScale);
      else
            xml.tag("size", sz / DPMM);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Image::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (tag == "path") {
                  _path = e.text();
                  _storeItem = imageStore.getImage(_path);
                  if (_storeItem)
                        _storeItem->reference(this);
                  }
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
//   load
//    load image from file and put into ImageStore
//    return true on success
//---------------------------------------------------------

bool Image::load(const QString& ss)
      {
      QFile f(ss);
      if (!f.open(QIODevice::ReadOnly))
            return false;
      QByteArray ba = f.readAll();
      f.close();
      _storeItem = imageStore.add(ss, ba);
      _storeItem->reference(this);
      return true;
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
      layout();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Image::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      QRectF r(pageBoundingRect());
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
      delete doc;
      }

SvgImage* SvgImage::clone() const
      {
      return new SvgImage(*this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SvgImage::draw(QPainter* painter) const
      {
      if (!doc)
            return;
      painter->save();
      QTransform t = painter->transform();
      qreal xscale = t.m11();
      qreal yscale = t.m22();
      QSize s      = QSizeF(sz.width() * xscale, sz.height() * yscale).toSize();
      t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
      painter->setWorldTransform(t);
      if (buffer.size() != s || _dirty || score()->printing()) {
            if (score()->printing()) {
                  QPointF pt(canvasPos());
                  painter->setViewport(pt.x() * xscale, pt.y() * yscale, s.width(), s.height());
                  doc->render(painter);
                  }
            else {
                  buffer = QPixmap(s);
                  buffer.fill(Qt::transparent);
                  QPainter pp(&buffer);
                  pp.setViewport(0, 0, s.width(), s.height());
                  doc->render(&pp);
                  _dirty = false;
                  }
            }
      Image::draw(painter, s);
      painter->restore();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SvgImage::layout()
      {
      if (!doc->isValid()) {
            if (_storeItem) {
                  doc->load(_storeItem->buffer());
                  if (doc->isValid()) {
                        if (sz.isNull())
                              sz = doc->defaultSize();
                        _dirty = true;
                        }
                  }
            }
      Image::layout();
      }

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

void RasterImage::draw(QPainter* painter) const
      {
      painter->save();
      if (score()->printing()) {
            // use original image size for printing
            painter->scale(sz.width() / doc.width(), sz.height() / doc.height());
            painter->drawPixmap(QPointF(0, 0), QPixmap::fromImage(doc));
            }
      else {
            QTransform t = painter->transform();
            QSize s = QSizeF(sz.width() * t.m11(), sz.height() * t.m22()).toSize();
            t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
            painter->setWorldTransform(t);
            if ((buffer.size() != s || _dirty) && !doc.isNull()) {
                  buffer = QPixmap::fromImage(doc.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                  _dirty = false;
                  }
            Image::draw(painter, s);
            }
      painter->restore();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RasterImage::layout()
      {
      if (doc.isNull()) {
            if (_storeItem) {
                  doc.loadFromData(_storeItem->buffer());
                  if (!doc.isNull()) {
                        if (sz.isNull())
                              sz = doc.size() * 0.4;
                        _dirty = true;
                        }
                  }
            }
      Image::layout();
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
      setbbox(QRectF(0.0, 0.0, sz.width(), sz.height()));
      }

