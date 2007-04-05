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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "element.h"
#include "style.h"

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

class Image : public Element {
      QString _path;
      QSvgRenderer* doc;
      QImage buffer;
      QSize sz;
      bool _dirty;

   public:
      Image(Score*);
      ~Image();
      virtual Image* clone() const;
      virtual ElementType type() const { return IMAGE; }
      virtual void draw(QPainter&);
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      void setPath(const QString& s);
      virtual QRectF bbox() const;
      virtual bool isMovable() const { return true; }
      void setAnchor(Anchor a) { setSubtype(int(a)); }
      Anchor anchor() const    { return (Anchor)subtype(); }
      };

#endif

