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
#include "undo.h"
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

void Image::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
      p.drawPixmap(0, 0, buffer);
      if (selected()) {
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

void RasterImage::draw(Painter* painter) const
      {
      QPainter& p = *painter->painter();
      QTransform t = p.worldTransform();
      QSize s = QSizeF(sz.width() * t.m11(), sz.height() * t.m22()).toSize();
      t.setMatrix(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33());
      p.setWorldTransform(t);
      if (buffer.size() != s || _dirty) {
            buffer = QPixmap::fromImage(doc.scaled(s, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            _dirty = false;
            }
      Image::draw(painter);
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
                  if (vp.getLockAspectRatio() != lockAspectRatio()
                     || vp.getAutoScale() != autoScale()) {
                        score()->undo()->push(new ChangeImage(this,
                           vp.getLockAspectRatio(), vp.getAutoScale()));
                        }
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
      autoScale->setChecked(img->autoScale());
      }

//---------------------------------------------------------
//   addImage
//---------------------------------------------------------

void Score::addImage(Element* e)
      {
      QString fn = QFileDialog::getOpenFileName(
         mscore,
         tr("MuseScore: InsertImage"),
         "",            // lastOpenPath,
         tr("All Supported Files (*.svg *.jpg *.png *.xpm);;"
            "Scalable vector graphics (*.svg);;"
            "JPEG (*.jpg);;"
            "PNG (*.png);;"
            "XPM (*.xpm);;"
            "All Files (*)"
            )
         );
      if (fn.isEmpty())
            return;

      QFileInfo fi(fn);
      Image* s = 0;
      QString suffix(fi.suffix().toLower());

      if (suffix == "svg")
            s = new SvgImage(this);
      else if (suffix == "jpg" || suffix == "png" || suffix == "xpm")
            s = new RasterImage(this);
      else
            return;
      s->setPath(fn);
      s->setSize(QSizeF(200, 200));
      s->setParent(e);
      undoAddElement(s);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Image::layout()
      {
      if (!autoScale() || !parent() || (parent()->type() != HBOX && parent()->type() != VBOX))
            return;

      if (_lockAspectRatio) {
            QSizeF size(imageSize());
            double ratio = size.width() / size.height();
            double w = parent()->width();
            double h = parent()->height();
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

