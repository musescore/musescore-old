//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h 4383 2011-06-14 21:28:16Z wschweer $
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

#ifndef __ICON_H__
#define __ICON_H__

#include "globals.h"
#include "libmscore/element.h"

//---------------------------------------------------------
//   Icon
//    dummy element, used for drag&drop
//---------------------------------------------------------

class Icon : public Element {
      QAction* _action;

   public:
      Icon(Score* s) : Element(s) {}
      virtual Icon* clone() const        { return new Icon(*this);   }
      virtual ElementType type() const   { return MAXTYPE;          }
      void setAction(QAction* a)         { _action = a;              }
      QIcon icon() const                 { return _action->icon();   }
      QAction* action() const            { return _action;           }
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      };

#endif

