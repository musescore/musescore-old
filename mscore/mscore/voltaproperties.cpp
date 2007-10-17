//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "voltaproperties.h"

//---------------------------------------------------------
//   VoltaProperties
//---------------------------------------------------------

VoltaProperties::VoltaProperties(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   getEndings
//---------------------------------------------------------

QList<int> VoltaProperties::getEndings() const
      {
      QList<int> il;
      QString s = repeat->text();
      QStringList sl = s.split(",", QString::SkipEmptyParts);
      foreach(QString l, sl) {
            int i = l.simplified().toInt();
            il.append(i);
            }
      return il;
      }

//---------------------------------------------------------
//   setEndings
//---------------------------------------------------------

void VoltaProperties::setEndings(const QList<int>& l)
      {
      QString s;
      foreach(int i, l) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      repeat->setText(s);
      }

