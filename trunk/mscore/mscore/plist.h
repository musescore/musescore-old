//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: plist.h,v 1.4 2006/03/02 17:08:41 wschweer Exp $
//
//    templates for stl void* container
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

#ifndef __PSTL_H__
#define __PSTL_H__

namespace pstl
{

//---------------------------------------------------------
//   plist
//---------------------------------------------------------

template<class T> class plist : public std::list<void*> {
      typedef std::list<void*> vlist;

   public:
      class iterator : public vlist::iterator {
         public:
            iterator() : vlist::iterator() {}
            iterator(vlist::iterator i) : vlist::iterator(i) {}

            T operator*() {
                  return (T)(**((vlist::iterator*)this));
                  }
            iterator operator++(int) {
                  return iterator ((*(vlist::iterator*)this).operator++(0));
                  }
            iterator& operator++() {
                  return (iterator&) ((*(vlist::iterator*)this).operator++());
                  }
            };

      class const_iterator : public vlist::const_iterator {
         public:
            const_iterator() : vlist::const_iterator() {}
            const_iterator(vlist::const_iterator i) : vlist::const_iterator(i) {}
            const_iterator(vlist::iterator i) : vlist::const_iterator(i) {}

            const T operator*() const {
                  return (T)(**((vlist::const_iterator*)this));
                  }
            };

      class reverse_iterator : public vlist::reverse_iterator {
         public:
            reverse_iterator() : vlist::reverse_iterator() {}
            reverse_iterator(vlist::reverse_iterator i) : vlist::reverse_iterator(i) {}

            T operator*() {
                  return (T)(**((vlist::reverse_iterator*)this));
                  }
            };

      class const_reverse_iterator : public vlist::const_reverse_iterator {
         public:
            const_reverse_iterator() : vlist::const_reverse_iterator() {}
            const_reverse_iterator(vlist::reverse_iterator i) : vlist::const_reverse_iterator(i) {}

            T operator*() const {
                  return (T)(**((vlist::const_reverse_iterator*)this));
                  }
            };

      plist() : vlist() {}
      virtual ~plist() {}

      void push_back(T v)             { vlist::push_back((void*)v); }
      iterator begin()                { return vlist::begin(); }
      iterator end()                  { return vlist::end(); }
      const_iterator begin() const    { return vlist::begin(); }
      const_iterator end() const      { return vlist::end(); }
      reverse_iterator rbegin()       { return vlist::rbegin(); }
      reverse_iterator rend()         { return vlist::rend(); }
      const_reverse_iterator rbegin() const { return vlist::rbegin(); }
      const_reverse_iterator rend() const   { return vlist::rend(); }
      T& back() const                 { return (T&)(vlist::back()); }
      T& front() const                { return (T&)(vlist::front()); }
      iterator find(const T& t)       {
            return std::find(begin(), end(), t);
            }
      const_iterator find(const T& t) const {
            return std::find(begin(), end(), t);
            }
      };
}


#endif
