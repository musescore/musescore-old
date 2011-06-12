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

#include "box.h"
#include "text.h"
#include "score.h"
// #include "mscore.h"
#include "barline.h"
#include "repeat.h"
#include "scoreview.h"
#include "scoreview.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"

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

#if 0
//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool TBox::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(tr("Frame Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TBox::propertyAction(ScoreView* /*viewer*/, const QString& cmd)
      {
      if (cmd == "props") {
            BoxProperties vp(this, 0);
            vp.exec();
            }
      }
#endif

