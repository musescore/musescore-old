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
   : Element(s)
      {
      _dirty = false;
      doc    = 0;
      setAnchor(ANCHOR_PAGE);
      }

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

Image::~Image()
      {
      if (doc)
            delete doc;
      }

Image* Image::clone() const
      {
      return new Image(*this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Image::draw(QPainter& p)
      {
      if (!doc)
            return;
      QSize s(sz.width() * p.device()->logicalDpiX() / 120,
              sz.height() * p.device()->logicalDpiY() / 120);
      if (buffer.size() != s || _dirty) {
            buffer = QImage(s, QImage::Format_ARGB32_Premultiplied);
            buffer.fill(0x0);
            QPainter pp(&buffer);
            pp.setViewport(0, 0, s.width(), s.height());
            doc->render(&pp);
            _dirty = false;
            }
      p.drawImage(0, 0, buffer);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Image::write(Xml& xml) const
      {
      xml.stag("Image");
      Element::writeProperties(xml);
      xml.tag("path", _path);
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
                  _path = e.text();
            else if (!Element::readProperties(node))
                  domError(node);
            }
      setPath(_path);
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void Image::setPath(const QString& s)
      {
      _path = s;
      if (doc == 0)
            doc = new QSvgRenderer;
      doc->load(s);
      if (doc->isValid()) {
            sz = doc->defaultSize();
            _dirty = true;
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Image::bbox() const
      {
      return QRectF(0.0, 0.0, sz.width(), sz.height());
      }

