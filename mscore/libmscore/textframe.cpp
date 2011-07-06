//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "box.h"
#include "text.h"
#include "score.h"
#include "barline.h"
#include "repeat.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"
#include "mscore.h"

//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

TBox::TBox(Score* score)
   : VBox(score)
      {
      Text* s = new Text(score);
      s->setTextStyle(TEXT_STYLE_FRAME);
      s->setSubtype(TEXT_FRAME);
      s->setStyled(false);
      add(s);
      }

//---------------------------------------------------------
//   layout
///   The text box layout() adjusts the frame height to text
///   height.
//---------------------------------------------------------

void TBox::layout()
      {
      setPos(QPointF());      // !?
      setbbox(QRectF(0.0, 0.0, system()->width(), point(boxHeight())));
      if (_el.size() == 1) {
            Text* text = static_cast<Text*>(_el[0]);
            if (text->type() != TEXT)
                  return;
            text->layout();
            qreal h;
            if (text->isEmpty()) {
                  QFontMetricsF fm(text->font());
                  h = fm.lineSpacing();
                  }
            else
                  h = text->height();
            text->setPos(leftMargin() * DPMM, topMargin() * DPMM);
            setbbox(QRectF(0.0, 0.0, system()->width(), h));
            }
      }

//---------------------------------------------------------
//   add
///   Add new Element \a e to text box
//---------------------------------------------------------

void TBox::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == TEXT) {
            Text* text = static_cast<Text*>(e);
            text->setLayoutToParentWidth(true);
            text->setFlag(ELEMENT_MOVABLE, false);
            }
      else {
            printf("TBox::add: element not allowed\n");
            return;
            }
      _el.append(e);
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

Text* TBox::getText()
      {
      if (_el.isEmpty())
            return 0;
      if (_el[0]->type() == TEXT)
            return static_cast<Text*>(_el[0]);
      return 0;
      }
