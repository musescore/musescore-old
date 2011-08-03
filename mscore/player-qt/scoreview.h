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

#ifndef __SCOREVIEW_H__
#define __SCOREVIEW_H__

#include "libmscore/mscoreview.h"

#include <QtGui/QTransform>
#include <QtCore/QRectF>
#include <QtDeclarative/QDeclarativeItem>

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

class ScoreView : public QDeclarativeItem, public MuseScoreView {
      Q_OBJECT

      Score* score;
      int _currentPage;

      virtual void dataChanged(const QRectF&);
      virtual void updateAll();
      virtual void moveCursor();
      virtual void adjustCanvasPosition(const Element* el, bool playBack);
      virtual void setScore(Score*) { printf("setScore\n");}
      virtual void removeScore() {printf("removeScore\n");}
      virtual void changeEditElement(Element*);
      virtual int gripCount() const;
      virtual const QRectF& getGrip(int) const;
      virtual const QTransform& matrix() const;
      virtual void setDropRectangle(const QRectF&);
      virtual void cmdAddSlur(Note* firstNote, Note* lastNote);
      virtual void startEdit() { printf("startEdit\n");}
      virtual void startEdit(Element*, int /*startGrip*/) { printf("startEdit\n");}
      virtual Element* elementNear(QPointF) { printf("elementNear\n"); return 0; }

      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

      virtual void setCursor(const QCursor&) {} // { QWidget::setCursor(c); }
      virtual QCursor cursor() const { return QCursor(); } // { return QWidget::cursor(); }
      virtual QRectF boundingRect() const;

   public slots:
      void setScore(const QString& s);
      void play();
      void setCurrentPage(int n);
      void nextPage();
      void prevPage();

   public:
      ScoreView(QDeclarativeItem* parent = 0);
      virtual ~ScoreView() {}
      };


#endif

