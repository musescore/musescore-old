//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2007 by Werner Schweer and others
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

#include "volknob.h"
#include "awlplugin.h"
#include "panknob.h"
#include "midipanknob.h"
#include "colorlabel.h"

#include <QtCore/QtPlugin>
#include <QtDesigner/QDesignerCustomWidgetInterface>

QWidget* KnobPlugin::createWidget(QWidget* parent)
	{
      return new Awl::Knob(parent);
      }
QWidget* VolKnobPlugin::createWidget(QWidget* parent)
	{
      return new Awl::VolKnob(parent);
      }
QWidget* PanKnobPlugin::createWidget(QWidget* parent)
	{
      return new Awl::PanKnob(parent);
      }
QWidget* MidiPanKnobPlugin::createWidget(QWidget* parent)
	{
      return new Awl::MidiPanKnob(parent);
      }
QWidget* ColorLabelPlugin::createWidget(QWidget* parent)
	{
      QWidget* w = new Awl::ColorLabel(parent);
      w->setGeometry(0, 0, 50, 50);
      return w;
      }

//---------------------------------------------------------
//   customWidgets
//---------------------------------------------------------

QList<QDesignerCustomWidgetInterface*> AwlPlugins::customWidgets() const
	{
	QList<QDesignerCustomWidgetInterface*> plugins;
            plugins
               << new VolKnobPlugin
               << new PanKnobPlugin
               << new MidiPanKnobPlugin
               << new KnobPlugin
               << new ColorLabelPlugin
               ;
      return plugins;
	}

Q_EXPORT_PLUGIN(AwlPlugins)

