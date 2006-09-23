//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: dynamics.h,v 1.11 2006/03/13 21:35:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __DYNAMICS_H__
#define __DYNAMICS_H__

#include "textelement.h"

enum DynVal {
      DynPPPPPP, DynPPPPP, DynPPPP, DynPPP, DynPP, DynP,
      DynMP, DynMF,
      DynF, DynFF, DynFFF, DynFFFF, DynFFFFF, DynFFFFFF,
      DynFP, DynSF, DynSFZ, DynSFFZ, DynSFP, DynSFPP,
      DynRFZ, DynRF, DynFZ,
      DynM, DynR, DynS, DynZ,
      DynOTHER,
      DynINVALID
      };

struct Dyn {
      int val;
      int textStyle;
      const QString str;
      const char* tag;

      Dyn(int v, int style, const char* t, const QString& s)
         : val(v), textStyle(style), str(s), tag(t) {}
      };

//---------------------------------------------------------
//   Dynamic
//---------------------------------------------------------

class Dynamic : public TextElement {
      DynVal val;

   public:
      Dynamic(Score*);
      Dynamic(Score*, DynVal val);
      Dynamic(Score*, const QString&);
      virtual ElementType type() const { return DYNAMIC; }

      void setVal(DynVal val);

      virtual bool isMovable() const { return true; }
      virtual void endDrag();

      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);

      static DynVal tag2val(const QString& tag);
      };

#endif
