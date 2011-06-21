//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

class ImagePath;

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

class Image : public BSymbol {
      Q_DECLARE_TR_FUNCTIONS(Image)

   protected:
      ImagePath* _ip;
      mutable QPixmap buffer;        ///< cached rendering
      QSizeF sz;
      bool _lockAspectRatio;
      bool _autoScale;              ///< fill parent frame
      mutable bool _dirty;

      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual void endEdit();
      virtual void draw(Painter*) const;
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip) const;
      virtual QSizeF imageSize() const = 0;

   public:
      Image(Score*);
      virtual ElementType type() const { return IMAGE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void setPath(const QString& s);
      virtual void layout();

      QString path() const;
      virtual QRectF bbox() const;
      void setSize(QSizeF s)          { sz = s; }
      bool lockAspectRatio() const    { return _lockAspectRatio; }
      void setLockAspectRatio(bool v) { _lockAspectRatio = v; }
      bool autoScale() const          { return _autoScale; }
      void setAutoScale(bool v)       { _autoScale = v; }

      void reference();
      void dereference();
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
      virtual void draw(Painter*) const;
      virtual void setPath(const QString& s);
      virtual QSizeF imageSize() const { return doc.size(); }
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
      virtual void draw(Painter*) const;
      virtual void setPath(const QString& s);
      virtual QSizeF imageSize() const { return doc->defaultSize(); }
      };

#endif

