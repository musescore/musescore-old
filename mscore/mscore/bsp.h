//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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
//
//  This code is from Qt implementation of QGraphicsItem
//    Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
//=============================================================================

#ifndef __BSP_H__
#define __BSP_H__

class BspTreeVisitor;
class InsertItemBspTreeVisitor;
class RemoveItemBspTreeVisitor;
class FindItemBspTreeVisitor;

#include "element.h"

//---------------------------------------------------------
//   BspTree
//    binary space partitioning
//---------------------------------------------------------

class BspTree
      {
   public:
      struct Node {
            enum Type { Horizontal, Vertical, Leaf };
            union {
                  qreal offset;
                  int leafIndex;
                  };
            Type type;
            };
   private:
      void initialize(const QRectF& rect, int depth, int index);
      void climbTree(BspTreeVisitor* visitor, const QPointF& pos, int index = 0);
      void climbTree(BspTreeVisitor* visitor, const QRectF& rect, int index = 0);

      void findItems(ElementList* foundItems, const QRectF& rect, int index);
      void findItems(ElementList* foundItems, const QPointF& pos, int index);
      QRectF rectForIndex(int index) const;

      QVector<Node> nodes;
      QVector<ElementList > leaves;
      int leafCnt;
      QRectF rect;

      InsertItemBspTreeVisitor* insertVisitor;
      RemoveItemBspTreeVisitor* removeVisitor;
      FindItemBspTreeVisitor* findVisitor;

   public:
      BspTree();
      ~BspTree();

      void initialize(const QRectF& rect, int depth);
      void clear();

      void insert(Element* item);
      void remove(Element* item);
      void remove(const QSet<Element*>& items);

      ElementList items(const QRectF& rect);
      ElementList items(const QPointF& pos);
      int leafCount() const;

      inline int firstChildIndex(int index) const {
            return index * 2 + 1;
            }

      inline int parentIndex(int index) const {
            return index > 0 ? ((index & 1) ? ((index - 1) / 2) : ((index - 2) / 2)) : -1;
            }

      QString debug(int index) const;
      };

//---------------------------------------------------------
//   BspTreeVisitor
//---------------------------------------------------------

class BspTreeVisitor
      {
   public:
      virtual ~BspTreeVisitor() {}
      virtual void visit(ElementList* items) = 0;
      };

#endif
