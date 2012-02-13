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

class ImageStoreItem;

//---------------------------------------------------------
//   Image
//---------------------------------------------------------

class Image : public BSymbol {
      Q_DECLARE_TR_FUNCTIONS(Image)

   protected:
      ImageStoreItem* _storeItem;
      QString _path;
      mutable QPixmap buffer;        ///< cached rendering
      QSizeF sz;
      bool _lockAspectRatio;
      bool _autoScale;              ///< fill parent frame
      mutable bool _dirty;

      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual void endEdit();
      void draw(QPainter*, QSize size) const;
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int grip) const;
      virtual QSizeF imageSize() const = 0;

   public:
      Image(Score*);
      Image(const Image&);
      ~Image();
      virtual ElementType type() const { return IMAGE; }
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      bool load(const QString& s);
      virtual void layout();

      void setSize(QSizeF s)            { sz = s; }
      bool lockAspectRatio() const      { return _lockAspectRatio; }
      void setLockAspectRatio(bool v)   { _lockAspectRatio = v; }
      bool autoScale() const            { return _autoScale; }
      void setAutoScale(bool v)         { _autoScale = v; }
      ImageStoreItem* storeItem() const { return _storeItem; }
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
      virtual void draw(QPainter*) const;
      virtual QSizeF imageSize() const { return doc.size(); }
      virtual void layout();
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
      virtual void draw(QPainter*) const;
      virtual QSizeF imageSize() const { return doc->defaultSize(); }
      virtual void layout();
      };

#endif

