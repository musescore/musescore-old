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

class Segment;

//---------------------------------------------------------
//   PlaybackCursor
//---------------------------------------------------------

class PlaybackCursor : public QDeclarativeItem {
      Q_OBJECT

      virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

   public:
      PlaybackCursor(QDeclarativeItem* parent = 0);
      virtual ~PlaybackCursor() {}
      void setColor(const QColor&) {}
      void setTick(int) {}
      };

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

class ScoreView : public QDeclarativeItem, public MuseScoreView {
      Q_OBJECT
      Q_PROPERTY(qreal parentWidth  READ parentWidth  WRITE setParentWidth)
      Q_PROPERTY(qreal parentHeight READ parentHeight WRITE setParentHeight)

      Score* score;
      PlaybackCursor* playbackCursor;
      int _currentPage;
      qreal _parentWidth, _parentHeight;
      qreal mag;
      int playPos;
      QRectF _boundingRect;

      virtual QVariant itemChange(GraphicsItemChange, const QVariant&);

      virtual void dataChanged(const QRectF&)   { update(); }
      virtual void updateAll()                  { update(); }
      virtual void moveCursor()                 {}
      virtual void adjustCanvasPosition(const Element*, bool) {}
      virtual void setScore(Score*)             {}
      virtual void removeScore()                {}
      virtual void changeEditElement(Element*)  {}
      virtual int gripCount() const             { return 0; }
      virtual const QRectF& getGrip(int) const;
      virtual const QTransform& matrix() const;
      virtual void setDropRectangle(const QRectF&) {}
      virtual void cmdAddSlur(Note*, Note*)     {}
      virtual void startEdit()                  {}
      virtual void startEdit(Element*, int)     {}
      virtual Element* elementNear(QPointF)     { return 0; }

      virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
      virtual bool sceneEvent(QEvent*);

      virtual void setCursor(const QCursor&)    {}
      virtual QCursor cursor() const            { return QCursor(); }
      virtual QRectF boundingRect() const       { return _boundingRect; }

   public slots:
      void setScore(const QString& s);
      void play();
      void setCurrentPage(int n);
      void nextPage();
      void prevPage();
      void rewind();
      void setTempo(qreal);
      void seek(qreal x, qreal y);

   public:
      ScoreView(QDeclarativeItem* parent = 0);
      virtual ~ScoreView() {}
      qreal parentWidth() const       { return _parentWidth;  }
      void setParentWidth(qreal val)  { _parentWidth = val;   }
      qreal parentHeight() const      { return _parentHeight; }
      void setParentHeight(qreal val) { _parentHeight = val;  }
      void moveCursor(int);
      };


#endif

