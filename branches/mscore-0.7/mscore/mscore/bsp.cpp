//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: $
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "bsp.h"
#include "element.h"

//---------------------------------------------------------
//   InsertItemBspTreeVisitor
//---------------------------------------------------------

class InsertItemBspTreeVisitor : public BspTreeVisitor
      {
   public:
      Element* item;

      void visit(QList<Element*> *items) {
            items->prepend(item);
            }
      };

//---------------------------------------------------------
//   RemoveItemBspTreeVisitor
//---------------------------------------------------------

class RemoveItemBspTreeVisitor : public BspTreeVisitor
      {
   public:
      Element* item;

      void visit(QList<Element*> *items) {
            items->removeAll(item);
            }
      };

//---------------------------------------------------------
//   FindItemBspTreeVisitor
//---------------------------------------------------------

class FindItemBspTreeVisitor : public BspTreeVisitor
      {
   public:
      QList<Element*>* foundItems;

      void visit(QList<Element*>* items) {
            for (int i = 0; i < items->size(); ++i) {
                  Element* item = items->at(i);
                  if (!item->itemDiscovered) {
                        item->itemDiscovered = 1;
                        foundItems->prepend(item);
                        }
                  }
            }
      };

//---------------------------------------------------------
//   BspTree
//---------------------------------------------------------

BspTree::BspTree()
   : leafCnt(0)
      {
      insertVisitor = new InsertItemBspTreeVisitor;
      removeVisitor = new RemoveItemBspTreeVisitor;
      findVisitor   = new FindItemBspTreeVisitor;
      }

BspTree::~BspTree()
      {
      delete insertVisitor;
      delete removeVisitor;
      delete findVisitor;
      }

//---------------------------------------------------------
//   initialize
//---------------------------------------------------------

void BspTree::initialize(const QRectF& rect, int depth)
      {
      this->rect = rect;
      leafCnt = 0;
      nodes.resize((1 << (depth + 1)) - 1);
      nodes.fill(Node());
      leaves.resize(1 << depth);
      leaves.fill(QList<Element*>());

      initialize(rect, depth, 0);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void BspTree::clear()
      {
      leafCnt = 0;
      nodes.clear();
      leaves.clear();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void BspTree::insert(Element* element)
      {
      insertVisitor->item = element;
      climbTree(insertVisitor, element->abbox());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BspTree::remove(Element* element)
      {
      removeVisitor->item = element;
      climbTree(removeVisitor, element->abbox());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BspTree::remove(const QSet<Element*>& items)
      {
      for (int i = 0; i < leaves.size(); ++i) {
            QList<Element*> newItemList;
            const QList<Element*>& oldItemList = leaves[i];
            for (int j = 0; j < oldItemList.size(); ++j) {
                  Element* item = oldItemList.at(j);
                  if (!items.contains(item))
                        newItemList << item;
                  }
            leaves[i] = newItemList;
            }
      }

//---------------------------------------------------------
//   items
//---------------------------------------------------------

QList<Element*> BspTree::items(const QRectF& rect)
      {
      QList<Element*> tmp;
      findVisitor->foundItems = &tmp;
      climbTree(findVisitor, rect);
      return tmp;
      }

//---------------------------------------------------------
//   items
//---------------------------------------------------------

QList<Element*> BspTree::items(const QPointF& pos)
      {
      QList<Element*> tmp;
      findVisitor->foundItems = &tmp;
      climbTree(findVisitor, pos);

      QList<Element*> l;
      for (int i = 0; i < tmp.size(); ++i) {
            Element* e = tmp.at(i);
            e->itemDiscovered = 0;
            if (e->contains(pos))
                  l.append(e);
            }
      return l;
      }

//---------------------------------------------------------
//   leafCount
//---------------------------------------------------------

int BspTree::leafCount() const
      {
      return leafCnt;
      }

//---------------------------------------------------------
//   debug
//---------------------------------------------------------

QString BspTree::debug(int index) const
      {
      const Node* node = &nodes.at(index);

      QString tmp;
      if (node->type == Node::Leaf) {
            QRectF rect = rectForIndex(index);
            if (!leaves[node->leafIndex].isEmpty()) {
                  tmp += QString::fromLatin1("[%1, %2, %3, %4] contains %5 items\n")
                   .arg(rect.left()).arg(rect.top())
                   .arg(rect.width()).arg(rect.height())
                   .arg(leaves[node->leafIndex].size());
                  }
            }
      else {
            if (node->type == Node::Horizontal) {
                  tmp += debug(firstChildIndex(index));
                  tmp += debug(firstChildIndex(index) + 1);
                  }
            else {
                  tmp += debug(firstChildIndex(index));
                  tmp += debug(firstChildIndex(index) + 1);
                  }
            }
      return tmp;
      }

//---------------------------------------------------------
//   initialize
//---------------------------------------------------------

void BspTree::initialize(const QRectF& rect, int depth, int index)
      {
      Node* node = &nodes[index];
      if (index == 0) {
            node->type = Node::Horizontal;
            node->offset = rect.center().x();
            }

      if (depth) {
            Node::Type type;
            QRectF rect1, rect2;
            qreal offset1, offset2;

            if (node->type == Node::Horizontal) {
                  type = Node::Vertical;
                  rect1.setRect(rect.left(), rect.top(), rect.width(), rect.height() / 2);
                  rect2.setRect(rect1.left(), rect1.bottom(), rect1.width(), rect.height() - rect1.height());
                  offset1 = rect1.center().x();
                  offset2 = rect2.center().x();
                  }
            else {
                  type = Node::Horizontal;
                  rect1.setRect(rect.left(), rect.top(), rect.width() / 2, rect.height());
                  rect2.setRect(rect1.right(), rect1.top(), rect.width() - rect1.width(), rect1.height());
                  offset1 = rect1.center().y();
                  offset2 = rect2.center().y();
                  }

            int childIndex = firstChildIndex(index);

            Node* child = &nodes[childIndex];
            child->offset = offset1;
            child->type = type;

            child = &nodes[childIndex + 1];
            child->offset = offset2;
            child->type = type;

            initialize(rect1, depth - 1, childIndex);
            initialize(rect2, depth - 1, childIndex + 1);
            }
      else {
            node->type = Node::Leaf;
            node->leafIndex = leafCnt++;
            }
      }

//---------------------------------------------------------
//   climbTree
//---------------------------------------------------------

void BspTree::climbTree(BspTreeVisitor* visitor, const QPointF& pos, int index)
      {
      if (nodes.isEmpty())
            return;

      Node* node = &nodes[index];
      int childIndex = firstChildIndex(index);

      switch (node->type) {
            case Node::Leaf:
                  visitor->visit(&leaves[node->leafIndex]);
                  break;
            case Node::Vertical:
                  if (pos.x() < node->offset)
                        climbTree(visitor, pos, childIndex);
                  else
                        climbTree(visitor, pos, childIndex + 1);
                  break;
            case Node::Horizontal:
                  if (pos.y() < node->offset)
                        climbTree(visitor, pos, childIndex);
                  else
                        climbTree(visitor, pos, childIndex + 1);
                  break;
            }
      }

//---------------------------------------------------------
//   climbTree
//---------------------------------------------------------

void BspTree::climbTree(BspTreeVisitor* visitor, const QRectF& rect, int index)
      {
      if (nodes.isEmpty())
            return;

      Node* node = &nodes[index];
      int childIndex = firstChildIndex(index);

      switch (node->type) {
            case Node::Leaf:
                  visitor->visit(&leaves[node->leafIndex]);
                  break;
            case Node::Vertical:
                  if (rect.left() < node->offset) {
                        climbTree(visitor, rect, childIndex);
                        if (rect.right() >= node->offset)
                              climbTree(visitor, rect, childIndex + 1);
                        }
                  else {
                        climbTree(visitor, rect, childIndex + 1);
                        }
                  break;
            case Node::Horizontal:
                  int childIndex = firstChildIndex(index);
                  if (rect.top() < node->offset) {
                        climbTree(visitor, rect, childIndex);
                        if (rect.bottom() >= node->offset)
                              climbTree(visitor, rect, childIndex + 1);
                        }
                  else {
                        climbTree(visitor, rect, childIndex + 1);
                        }
            }
      }

//---------------------------------------------------------
//   rectForIndex
//---------------------------------------------------------

QRectF BspTree::rectForIndex(int index) const
      {
      if (index <= 0)
            return rect;

      int parentIdx = parentIndex(index);
      QRectF rect   = rectForIndex(parentIdx);
      const Node *parent = &nodes.at(parentIdx);

      if (parent->type == Node::Horizontal) {
            if (index & 1)
                  rect.setRight(parent->offset);
            else
                  rect.setLeft(parent->offset);
            }
      else {
            if (index & 1)
                  rect.setBottom(parent->offset);
            else
                  rect.setTop(parent->offset);
            }
      return rect;
      }

