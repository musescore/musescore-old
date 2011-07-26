//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: image.h -1   $
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

#ifndef __IMAGEPROPERTIES_H__
#define __IMAGEPROPERTIES_H__

#include "ui_imageproperties.h"

class Image;

//---------------------------------------------------------
//   ImageProperties
//---------------------------------------------------------

class ImageProperties : public QDialog, public Ui::ImageProperties {
      Q_OBJECT

      Image* img;

   public:
      ImageProperties(Image*, QWidget* parent = 0);
      bool getLockAspectRatio() const { return lockAspectRatio->isChecked();}
      bool getAutoScale() const       { return autoScale->isChecked(); }
      int getZ() const                { return z->value(); }
      };

#endif

