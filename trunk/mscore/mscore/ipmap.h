//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: ipmap.h,v 1.3 2006/03/02 17:08:34 wschweer Exp $
//
//    templates for stl map(int, void*)
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

#ifndef __IPMAP_H__
#define __IPMAP_H__

namespace pstl
{

//---------------------------------------------------------
//   plist
//---------------------------------------------------------

template<class T> class ipmap : public std::map<int, void*> {
      typedef std::map<int, void*> vmap;

   public:
      class iterator : public vmap::iterator {
         public:
            iterator() : vmap::iterator() {}
            iterator(vmap::iterator i) : vmap::iterator(i) {}

            std::pair<int, T>& operator*() const {
                  return (std::pair<int, T>&)(vmap::iterator::operator*());
                  }
            std::pair<int, T>* operator->() const {
                  return (std::pair<int, T>*)(vmap::iterator::operator->());
                  }
            };

      class const_iterator : public vmap::const_iterator {
         public:
            const_iterator() : vmap::const_iterator() {}
            const_iterator(vmap::const_iterator i) : vmap::const_iterator(i) {}
            const_iterator(vmap::iterator i) : vmap::const_iterator(i) {}

            const std::pair<int, T>& operator*() const {
                  return (const std::pair<int, T>&)(vmap::const_iterator::operator*());
                  }
            std::pair<int, T>* operator->() const {
                  return (std::pair<int, T>*)(vmap::const_iterator::operator->());
                  }
            };

      class reverse_iterator : public vmap::reverse_iterator {
         public:
            reverse_iterator() : vmap::reverse_iterator() {}
            reverse_iterator(vmap::reverse_iterator i) : vmap::reverse_iterator(i) {}

            std::pair<int, T>& operator*() const {
                  return (std::pair<int, T>&)(vmap::reverse_iterator::operator*());
                  }
            };

      class const_reverse_iterator : public vmap::const_reverse_iterator {
         public:
            const_reverse_iterator() : vmap::const_reverse_iterator() {}
            const_reverse_iterator(vmap::const_reverse_iterator i) : vmap::const_reverse_iterator(i) {}
            const_reverse_iterator(vmap::reverse_iterator i) : vmap::const_reverse_iterator(i) {}

            const std::pair<int, T>& operator*() const {
                  return (const std::pair<int, T>&)(vmap::const_reverse_iterator::operator*());
                  }
            };

      ipmap() : vmap() {}

//      void push_back(T v)             { vmap::push_back((void*)v); }
      iterator begin()                { return vmap::begin(); }
      iterator end()                  { return vmap::end(); }
      const_iterator begin() const    { return vmap::begin(); }
      const_iterator end() const      { return vmap::end(); }
      reverse_iterator rbegin()       { return vmap::rbegin(); }
      reverse_iterator rend()         { return vmap::rend(); }
      const_reverse_iterator rbegin() const { return vmap::rbegin(); }
      const_reverse_iterator rend() const   { return vmap::rend(); }
//      T& back() const                 { return (T&)(vmap::back()); }
//      T& front() const                { return (T&)(vmap::front()); }
      iterator upper_bound(int i)     { return vmap::upper_bound(i); }
      const_iterator upper_bound(int i) const { return vmap::upper_bound(i); }
      };
}


#endif
