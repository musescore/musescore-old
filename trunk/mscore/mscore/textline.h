//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "element.h"

class TextPalette;
extern TextPalette* palette;

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

class TextLine : public Element {
      QString _text;
      bool editMode;

   protected:
      int textStyle;
      QFont font() const;

   public:
      TextLine(Score*);
      TextLine(Score*, int style);
      TextLine(const TextLine&);

      TextLine &operator=(const TextLine&);

      virtual TextLine* clone() const { return new TextLine(*this); }
      virtual ElementType type() const { return TEXT; }

      void setText(const QString& s);
      QString getText() const { return _text; }

      double lineSpacing() const;
      virtual void resetMode();

      bool isEmpty() const;
      void setStyle(int n);
      int style() const           { return textStyle; }

      virtual void draw1(Painter&);

      virtual bool startEdit(QMatrix&);
      virtual bool edit(QKeyEvent*);
      virtual void endEdit();
      virtual bool isMovable() const { return true; }
      virtual void write(Xml& xml) const;
      void write(Xml& xml, const char*) const;
      virtual void read(QDomNode);
      virtual void layout();
      virtual const QRectF& bbox() const;
      };

#endif
