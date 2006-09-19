//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: painter.h,v 1.8 2006/03/02 17:08:40 wschweer Exp $
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

#ifndef __PAINTER_H__
#define __PAINTER_H__

//---------------------------------------------------------
//   Painter
//---------------------------------------------------------

class Painter : public QPainter {
      bool _print;

  public:
      Painter() { _print = false; }
      Painter(QPaintDevice* d)
         : QPainter(d), _print(false) {}
      void setPrint(bool val)        { _print = val; }
      bool print() const             { return _print; }
      const QRect clipRect() const      { return clipRegion().boundingRect(); }
      };

#endif

