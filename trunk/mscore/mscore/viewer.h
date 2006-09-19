//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: viewer.h,v 1.6 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __VIEWER_H__
#define __VIEWER_H__

class Score;

//---------------------------------------------------------
//   Viewer
//---------------------------------------------------------

class Viewer {
   public:
      virtual ~Viewer() {}
      virtual void dataChanged(Score* cp, const QRectF&) = 0;
      virtual QRectF moveCursor() = 0;
      virtual void updateAll(Score*) = 0;
      };

#endif
