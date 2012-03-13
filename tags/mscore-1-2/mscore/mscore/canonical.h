//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __CANONICAL_H__
#define __CANONICAL_H__

//---------------------------------------------------------
//   CScore
//    canonical (partial) score representation
//
//   buffer contains something like:
//
//    <Track no="0">
//       <Chord>
//          <len>1/4</len>
//          </Chord>
//       </Track>
//---------------------------------------------------------

class CScore {
      QBuffer buffer;
      Score* score;

      Segment* write(Xml&, DurationElement*);

   public:
      CScore(Score* s, int track, Segment*);
      const QByteArray& data() const { return buffer.data(); }
      };


#endif

