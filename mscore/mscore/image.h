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

      QRectF r1, r2, bbr1, bbr2;     // "grips" for dragging

      enum { NORMAL, DRAG1, DRAG2 };
      int mode;

      virtual bool startEdit(QMatrix&, const QPointF&);
      virtual bool edit(QKeyEvent*);
      virtual bool startEditDrag(Viewer*, const QPointF&);
      virtual bool editDrag(Viewer*, QPointF*, const QPointF&);
      virtual bool endEditDrag();
      virtual void endEdit();
      virtual void draw(QPainter&);

   public:
      Image(Score*);
      virtual ElementType type() const { return IMAGE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      virtual void setPath(const QString& s);
      virtual QRectF bbox() const;
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

