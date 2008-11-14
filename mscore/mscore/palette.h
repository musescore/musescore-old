//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: palette.h,v 1.10 2006/03/02 17:08:40 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __PALETTE_H__
#define __PALETTE_H__

class Element;
class Sym;
class Xml;

//---------------------------------------------------------
//   PaletteBoxButton
//---------------------------------------------------------

class PaletteBoxButton : public QPushButton {
      Q_OBJECT

      virtual void paintEvent(QPaintEvent*);
      virtual void changeEvent(QEvent*);

   public:
      PaletteBoxButton(QWidget*, QWidget* parent = 0);
      };

//---------------------------------------------------------
//   PaletteBox
//---------------------------------------------------------

class PaletteBox : public QDockWidget {
      Q_OBJECT

      QList<QWidget*> widgets;
      QVBoxLayout* vbox;
      virtual QSize sizeHint() const;
      virtual void closeEvent(QCloseEvent*);

   signals:
      void paletteVisible(bool);

   public:
      PaletteBox(QWidget* parent = 0);
      void addPalette(const QString& s, QWidget*);
      };


//---------------------------------------------------------
//   PaletteCell
//---------------------------------------------------------

struct PaletteCell {
      Element* element;
      QString name;
      };


//---------------------------------------------------------
//   PaletteScrollArea
//---------------------------------------------------------

class PaletteScrollArea : public QScrollArea {
      Q_OBJECT

      virtual void resizeEvent(QResizeEvent*);

   public:
      PaletteScrollArea(QWidget* w, QWidget* parent = 0);
      };

//---------------------------------------------------------
//   Palette
//---------------------------------------------------------

class Palette : public QWidget {
      Q_OBJECT

      QList<PaletteCell*> cells;

      int hgrid, vgrid;
      int currentIdx;
      int selectedIdx;
      QPoint dragStartPosition;
      qreal extraMag;
      bool _drawGrid;
      bool _selectable;
      bool _readOnly;
      qreal _yOffset;

      bool staff;

      void redraw(const QRect&);
      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseDoubleClickEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void leaveEvent(QEvent*);
      virtual bool event(QEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);

      int idx(const QPoint&) const;
      QRect idxRect(int);

   private slots:
      void actionToggled(bool val);

   signals:
      void droppedElement(Element*);
      void startDragElement(Element*);
      void boxClicked(int);

   public:
      Palette(QWidget* parent = 0);
      Palette(qreal mag);
      ~Palette();
      void addObject(int idx, Element*, const QString& name);
      void addObject(int idx, int sym);
      void setGrid(int, int);
      void showStaff(bool val)     { staff = val; }
      Element* element(int idx)    { return cells[idx]->element; }
      void setDrawGrid(bool val)   { _drawGrid = val; }
      void write(Xml&, const char*) const;
      void read(QDomElement);
      void clear();
      void setSelectable(bool val) { _selectable = val;  }
      bool selectable() const      { return _selectable; }
      int getSelectedIdx() const   { return selectedIdx; }
      void setSelected(int idx)    { selectedIdx = idx;  }
      bool readOnly() const        { return _readOnly;   }
      void setReadOnly(bool val)   { _readOnly = val;    }
      void setMag(qreal val)       { extraMag = val;     }
      void setYOffset(qreal val)   { _yOffset = val;     }
      int columns() const { return width() / hgrid; }
      int rows() const;
      int resizeWidth(int);
      };

#endif
