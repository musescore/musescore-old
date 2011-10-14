//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "config.h"
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

#ifdef SVG_IMAGES
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

#endif

