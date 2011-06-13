//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "scoreview.h"
#include "mscore.h"
#include "undo.h"

#include "score.h"
#include "libmscore/element.h"

#include "articulationprop.h"
#include "bendproperties.h"
#include "boxproperties.h"
#include "tupletproperties.h"

#include "libmscore/bend.h"
#include "libmscore/box.h"
#include "libmscore/text.h"
#include "libmscore/articulation.h"
#include "libmscore/tuplet.h"

//---------------------------------------------------------
//   createElementPropertyMenu
//---------------------------------------------------------

void ScoreView::createElementPropertyMenu(Element* e, QMenu* popup)
      {
      QAction* a;
      if (e->type() == ARTICULATION) {
            e->Element::genPropertyMenu(popup);
            a = popup->addAction(tr("Articulation Properties..."));
            a->setData("a-props");
            }
      else if (e->type() == BEND) {
            e->Element::genPropertyMenu(popup);
            a = popup->addAction(tr("Bend Properties..."));
            a->setData("b-props");
            }
      else if (e->type() == HBOX) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            textMenu->addAction(getAction("frame-text"));
            textMenu->addAction(getAction("picture"));
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == VBOX) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            textMenu->addAction(getAction("frame-text"));
            textMenu->addAction(getAction("title-text"));
            textMenu->addAction(getAction("subtitle-text"));
            textMenu->addAction(getAction("composer-text"));
            textMenu->addAction(getAction("poet-text"));
            textMenu->addAction(getAction("insert-hbox"));
            textMenu->addAction(getAction("picture"));
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == TUPLET) {
            e->Element::genPropertyMenu(popup);
            a = popup->addAction(tr("Tuplet Properties..."));
            a->setData("t-props");
            }
      }

//---------------------------------------------------------
//   elementPropertyAction
//---------------------------------------------------------

void ScoreView::elementPropertyAction(const QString& cmd, Element* e)
      {
      if (cmd == "a-props") {
            ArticulationProperties rp(static_cast<Articulation*>(e));
            rp.exec();
            }
      else if (cmd == "b-props") {
            Bend* bend = static_cast<Bend*>(e);
            BendProperties bp(bend, 0);
            if (bp.exec())
                  score()->undo()->push(new ChangeBend(bend, bp.points()));
            }
      else if (cmd == "f-props") {
            BoxProperties vp(static_cast<Box*>(e), 0);
            vp.exec();
            }
      else if (cmd == "frame-text") {
            Text* s = new Text(score());
            s->setSubtype(TEXT_FRAME);
            s->setTextStyle(TEXT_STYLE_FRAME);
            s->setParent(e);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            startEdit(s);
            score()->setLayoutAll(true);
            }
      else if (cmd == "picture") {
            score()->addImage(static_cast<HBox*>(e));
            }
      else if (cmd == "frame-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_FRAME);
            t->setTextStyle(TEXT_STYLE_FRAME);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "title-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_TITLE);
            t->setTextStyle(TEXT_STYLE_TITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "subtitle-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_SUBTITLE);
            t->setTextStyle(TEXT_STYLE_SUBTITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "composer-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_COMPOSER);
            t->setTextStyle(TEXT_STYLE_COMPOSER);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "poet-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_POET);
            t->setTextStyle(TEXT_STYLE_POET);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "insert-hbox") {
            HBox* s = new HBox(score());
            double w = width() - s->leftMargin() * DPMM - s->rightMargin() * DPMM;
            s->setBoxWidth(Spatium(w / s->spatium()));
            s->setParent(e);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            startEdit(s);
            }
      else if (cmd == "picture")
            score()->addImage(e);
      else if (cmd == "t-props") {
            TupletProperties vp(static_cast<Tuplet*>(e));
            if (vp.exec()) {
                  //
                  // apply changes to all selected tuplets
                  //
                  int bracketType = vp.bracketType();
                  int numberType  = vp.numberType();
                  foreach(Element* e, score()->selection().elements()) {
                        if (e->type() == TUPLET) {
                              Tuplet* tuplet = static_cast<Tuplet*>(e);
                              if ((bracketType != tuplet->bracketType()) || (numberType != tuplet->numberType()))
                                    score()->undo()->push(new ChangeTupletProperties(tuplet, numberType, bracketType));
                              }
                        }
                  }
            }

      }

