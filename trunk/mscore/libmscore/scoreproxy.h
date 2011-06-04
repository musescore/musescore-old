//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#ifndef __SCOREPROXY_H__
#define __SCOREPROXY_H__

#include <QtCore/QString>
#include <QtCore/QRectF>

class Score;

//---------------------------------------------------------
//   ScoreProxy
//---------------------------------------------------------

class ScoreProxy {
      Score* s;
      qreal _scale;

   public:
      ScoreProxy();
      ~ScoreProxy();
      bool read(const QString&);

      void expose(void* gc, int page, qreal x, qreal y, qreal w, qreal h);
      Score* score() const { return s; }
      qreal pageWidth() const;
      qreal pageHeight() const;
      int pages() const;
      qreal scale() const      { return _scale; }
      void setScale(qreal val) { _scale = val; }
      };

#endif

