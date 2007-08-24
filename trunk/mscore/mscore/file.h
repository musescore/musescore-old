//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: file.h,v 1.5 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __FILE_H__
#define __FILE_H__

//---------------------------------------------------------
//   LoadFile
//---------------------------------------------------------

class LoadFile {
      bool popenFlag;
      QString _name;

   protected:
      QString error;

   public:
      LoadFile() {}
      virtual ~LoadFile() {}
      virtual bool loader(QFile* f) = 0;
      bool load(QWidget* parent, const QString& base, const QString& ext,
         const QString& caption);
      bool load(const QString& name);
      QString name() const { return _name; }
      };

#endif

