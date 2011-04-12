//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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
#include "score.h"

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::Image(Score* s)
   : BSymbol(s)
      {
      _ip              = 0;
      _dirty           = false;
      _lockAspectRatio = true;
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

void Image::draw(QPainter& p) const
      {
      p.drawPixmap(0, 0, buffer);
      if (selected() && !(score() && score()->printing())) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
            p.drawRect(QRect(QPoint(), buffer.size()));
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
      xml.tag("size", sz / DPMM);
      if (!_lockAspectRatio)
            xml.tag("lockAspectRatio", _lockAspectRatio);
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
            else if (tag == "size") {
                  sz = readSize(e);
                  if (score()->mscVersion() >= 109)
                        sz *= DPMM;
                  }
            else if (tag == "lockAspectRatio")
                  _lockAspectRatio = e.text().toInt();
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

void Image::editDrag(int curGrip, const QPointF& d)
      {
      double ratio = sz.width() / sz.height();
      if (curGrip == 0) {
            sz.setWidth(sz.width() + d.x());
            if (_lockAspectRatio)
                  sz.setHeight(sz.width() / ratio);
            }
      else {
            sz.setHeight(sz.height() + d.y());
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

void SvgImage::draw(QPainter& p) const
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
      if (_ip->loaded())
            doc->load(_ip->buffer().buffer());
      else
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
      p.save();
      QTransform t = p.worldTransform();
      QSize s = QSizeF(sz.width() * t.m11(), sz.height() * t.m22()).toSize();
      t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
      p.setWorldTransform(t);
      if (buffer.size() != s || _dirty) {
            buffer = QPixmap::fromImage(doc.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            _dirty = false;
            }
      Image::draw(p);
      p.restore();
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
            sz = doc.size() * 0.4 * DPI / PDPI;
            _dirty = true;
            }
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Image::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addSeparator();
      a->setText(tr("Image"));
      a = popup->addAction(tr("Image Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Image::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            ImageProperties vp(this);
            int rv = vp.exec();
            if (rv) {
                  }
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   ImageProperties
//---------------------------------------------------------

ImageProperties::ImageProperties(Image* i, QWidget* parent)
   : QDialog(parent)
      {
      img = i;
      setupUi(this);
      lockAspectRatio->setChecked(img->lockAspectRatio());
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(clicked(QAbstractButton*)));
      }

//---------------------------------------------------------
//   clicked
//---------------------------------------------------------

void ImageProperties::clicked(QAbstractButton* b)
      {
      switch (buttonBox->buttonRole(b)) {
            case QDialogButtonBox::AcceptRole:
            case QDialogButtonBox::ApplyRole:
                  if (img->lockAspectRatio() != lockAspectRatio->isChecked())
                        img->setLockAspectRatio(lockAspectRatio->isChecked());
                  break;
            default:
                  break;
            }
      }

