//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

#include "box.h"

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

class TBox : public VBox {
      Q_DECLARE_TR_FUNCTIONS(TBox)

   public:
      TBox(Score* score);
      ~TBox() {}
      virtual TBox* clone() const      { return new TBox(*this); }
      virtual ElementType type() const { return TBOX;       }

      virtual void layout();
      virtual void add(Element*);
      Text* getText();
      };

#endif

