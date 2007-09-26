//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "bsymbol.h"

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

class Image : public BSymbol {
   protected:
      QString _path;
      QImage buffer;                ///< cached rendering
      QSizeF sz;
      bool _dirty;

      virtual bool startEdit(const QPointF&);
      virtual void editDrag(int, const QPointF&, const QPointF&);
      virtual void endEdit();
      virtual void draw(QPainter&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip) const;

   public:
      Image(Score*);
      virtual ElementType type() const { return IMAGE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void setPath(const QString& s);
      QString path() const;
      virtual QRectF bbox() const;
      void setSize(QSizeF s) { sz = s; }
      };

//---------------------------------------------------------
//   RasterImage
//---------------------------------------------------------

class RasterImage : public Image {
      QImage doc;

   public:
      RasterImage(Score*);
      ~RasterImage();
      virtual RasterImage* clone() const;
      virtual void draw(QPainter&);
      virtual void setPath(const QString& s);
      };

//---------------------------------------------------------
//   SvgImage
//---------------------------------------------------------

class SvgImage : public Image {
      QSvgRenderer* doc;

   public:
      SvgImage(Score*);
      ~SvgImage();
      virtual SvgImage* clone() const;
      virtual void draw(QPainter&);
      virtual void setPath(const QString& s);
      };

#endif

