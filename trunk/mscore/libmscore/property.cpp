//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "property.h"
#include "mscore.h"
#include "layoutbreak.h"

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void setProperty(P_DATA_TYPE type, void* data, const QString& value)
      {
      switch(type) {
            case T_BOOL:
                  *(bool*)data = value.toInt();
                  break;
            case T_SUBTYPE:
            case T_INT:
                  *(int*)data = value.toInt();
                  break;
            case T_REAL:
                  *(qreal*)data = value.toDouble();
                  break;
            case T_COLOR:
                  *(QColor*)data = QColor();    // TODO value.toBool();
                  break;
            case T_DIRECTION:
                  {
                  if (value == "up")
                        *(Direction*)data = UP;
                  else if (value == "down")
                        *(Direction*)data = DOWN;
                  else if (value == "auto")
                        *(Direction*)data = AUTO;
                  }
                  break;
            case T_DIRECTION_H:
                  {
                  if (value == "left")
                        *(DirectionH*)data = DH_LEFT;
                  else if (value == "right")
                        *(DirectionH*)data = DH_RIGHT;
                  else if (value == "auto")
                        *(DirectionH*)data = DH_AUTO;
                  }
                  break;
            case T_LAYOUT_BREAK:
                  if (value == "line")
                        *(LayoutBreakType*)data = LAYOUT_BREAK_LINE;
                  else if (value == "page")
                        *(LayoutBreakType*)data = LAYOUT_BREAK_PAGE;
                  else if (value == "section")
                        *(LayoutBreakType*)data = LAYOUT_BREAK_SECTION;
                  break;
            case T_POINT:
                  abort();
            }
      }

void setProperty(P_DATA_TYPE type, void* data, const QVariant& value)
      {
      switch(type) {
            case T_BOOL:
                  *(bool*)data = value.toBool();
                  break;
            case T_SUBTYPE:
            case T_INT:
                  *(int*)data = value.toInt();
                  break;
            case T_REAL:
                  *(qreal*)data = value.toDouble();
                  break;
            case T_POINT:
                  *(QPointF*)data = value.toPointF();
                  break;
            case T_COLOR:
                  *(QColor*)data = value.value<QColor>();
                  break;
            case T_DIRECTION:
            case T_DIRECTION_H:
            case T_LAYOUT_BREAK:
                  setProperty(type, data, value.toString());
                  break;
            }
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant getProperty(P_DATA_TYPE type, void* data)
      {
      switch(type) {
            case T_BOOL:
                  return QVariant(*(bool*)data);
            case T_SUBTYPE:
            case T_INT:
            case T_DIRECTION:
            case T_DIRECTION_H:
            case T_LAYOUT_BREAK:
                  return QVariant(*(int*)data);
            case T_REAL:
                  return QVariant(*(qreal*)data);
            case T_COLOR:
                  return QVariant(*(QColor*)data);
            case T_POINT:
                  return QVariant(*(QPointF*)data);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant getProperty(P_DATA_TYPE type, const QDomElement& e)
      {
      switch(type) {
            case T_BOOL:
                  return QVariant(bool(e.text().toInt()));
            case T_SUBTYPE:
            case T_INT:
            case T_DIRECTION:
            case T_DIRECTION_H:
            case T_LAYOUT_BREAK:
                  return QVariant(e.text().toInt());
            case T_REAL:
                  return QVariant(e.text().toDouble());
            case T_COLOR:
                  return QVariant(readColor(e));
            case T_POINT:
                  return QVariant(readPoint(e));
            }
      return QVariant();
      }


