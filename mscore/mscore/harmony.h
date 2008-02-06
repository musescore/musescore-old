//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#ifndef __HARMONY_H__
#define __HARMONY_H__

#include "text.h"

//---------------------------------------------------------
//   class Harmony
//---------------------------------------------------------

class Harmony : public Text {
      unsigned char _base;
      unsigned char _extension;
      unsigned char _root;

   public:
      Harmony(Score*);
      ~Harmony();
      virtual Harmony* clone() const   { return new Harmony(*this); }
      virtual ElementType type() const { return HARMONY; }

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);

      virtual void endEdit();

      unsigned char base() const           { return _base;      }
      void setBase(unsigned char val)      { _base = val;       }
      unsigned char extension() const      { return _extension; }
      void setExtension(unsigned char val) { _extension = val;  }
      unsigned char root() const           { return _root;      }
      void setRoot(unsigned char val)      { _root = val;       }

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      QString harmonyName() const   { return harmonyName(root(), extension(), base()); }
      QString extensionName() const { return getExtensionName(extension()); }
      void buildText();

      static const char* getExtensionName(int i);
      static QString harmonyName(int root, int extension, int base);
      static int parseHarmony(const QString& s, int* root, int* base);
      };

#endif

