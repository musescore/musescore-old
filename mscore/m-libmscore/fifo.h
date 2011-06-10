//=============================================================================
//  MscorePlayer
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __FIFO_H__
#define __FIFO_H__

//---------------------------------------------------------
//   FifoBase
//    - works only for one reader/writer
//    - reader writes ridx
//    - writer writes widx
//    - reader decrements counter
//    - writer increments counter
//    - counter increment/decrement must be atomic
//---------------------------------------------------------

class FifoBase {

   protected:
      int ridx;               // read index
      int widx;               // write index
      volatile int counter;   // objects in fifo
      int maxCount;

      void push();
      void pop();

   public:
      FifoBase()              { clear(); }
      virtual ~FifoBase()     {}
      void clear();
      int count() const       { return counter; }
      bool isEmpty() const    { return counter == 0; }
      bool isFull() const     { return maxCount == counter; }
      };

#endif

