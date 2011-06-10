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

#include "fifo.h"

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void FifoBase::clear()
	{
	ridx    = 0;
      widx    = 0;
      counter = 0;
      }

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void FifoBase::push()
      {
      widx = (widx + 1) % maxCount;
//      q_atomic_increment(&counter);
      ++counter;
      }

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void FifoBase::pop()
      {
      ridx = (ridx + 1) % maxCount;
      // q_atomic_decrement(&counter);
      --counter;
      }

