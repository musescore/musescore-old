//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: palette.h,v 1.10 2006/03/02 17:08:40 wschweer Exp $
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

#ifndef __PALETTE_H__
#define __PALETTE_H__

class Element;
class Sym;

//---------------------------------------------------------
//   SymbolPalette
//---------------------------------------------------------

class SymbolPalette : public QWidget {
      Q_OBJECT

      Element** symbols;
      QString* names;
      int rows, columns;
      int hgrid, vgrid;
      int currentSymbol;
      QPoint dragStartPosition;

      bool staff;

      void redraw(const QRect&);
      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual bool event(QEvent*);

   public:
      SymbolPalette(int rows, int columns);
      ~SymbolPalette();
      void addObject(int idx, Element*, const QString& name);
      void addObject(int idx, int sym);
      void setGrid(int, int);
      void showStaff(bool);
      int getRows() const { return rows; }
      int getColumns() const { return columns; }
      void setRowsColumns(int r, int c);
      Element* element(int idx) { return symbols[idx]; }
      };

#endif
