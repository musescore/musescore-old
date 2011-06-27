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
      QPointF _startDrag;
      QPointF _offset;

      virtual void dataChanged(const QRectF&);
      virtual void updateAll();
      virtual void moveCursor();
      virtual void adjustCanvasPosition(const Element* el, bool playBack);
      virtual void setScore(Score*);
      virtual void removeScore();
      virtual void changeEditElement(Element*);
      virtual int gripCount() const;
      virtual const QRectF& getGrip(int) const;
      virtual const QTransform& matrix() const;
      virtual void setDropRectangle(const QRectF&);
      virtual void cmdAddSlur(Note* firstNote, Note* lastNote);
      virtual void startEdit();
      virtual void startEdit(Element*, int startGrip);
      virtual Element* elementNear(const QPointF&);

      virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

      virtual void setCursor(const QCursor&) {} // { QWidget::setCursor(c); }
      virtual QCursor cursor() const { return QCursor(); } // { return QWidget::cursor(); }

      void zoom(int step, const QPoint&);

   public slots:
      void drag(qreal x, qreal y);
      void startDrag(qreal x, qreal y);

   public:
      ScoreView(QDeclarativeItem* parent = 0);
      void loadFile(const QString& s);
      };


#endif

