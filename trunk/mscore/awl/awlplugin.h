//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

#ifndef __AWLPLUGIN_H__
#define __AWLPLUGIN_H__

#include <QtDesigner/QDesignerCustomWidgetInterface>

//---------------------------------------------------------
//   AwlPlugin
//---------------------------------------------------------

class AwlPlugin : public QDesignerCustomWidgetInterface {
	Q_INTERFACES(QDesignerCustomWidgetInterface)
      bool m_initialized;

   public:
    	AwlPlugin() : m_initialized(false) { }
	bool isContainer() const     { return false;         }
    	bool isInitialized() const   { return m_initialized; }
    	QIcon icon() const           { return QIcon();       }
    	virtual QString codeTemplate() const { return QString();     }
    	QString whatsThis() const    { return QString();     }
    	QString toolTip() const      { return QString();     }
    	QString group() const        { return "MusE Awl Widgets"; }
	void initialize(QDesignerFormEditorInterface *) {
		if (m_initialized)
			return;
		m_initialized = true;
		}
      };

//---------------------------------------------------------
//   KnobPlugin
//---------------------------------------------------------

class KnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	KnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return QString("awl/knob.h"); }
      QString name() const        { return "Awl::Knob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   VolKnobPlugin
//---------------------------------------------------------

class VolKnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	VolKnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/volknob.h"; }
      QString name() const { return "Awl::VolKnob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   PanKnobPlugin
//---------------------------------------------------------

class PanKnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	PanKnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/panknob.h"; }
      QString name() const { return "Awl::PanKnob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   MidiPanKnobPlugin
//---------------------------------------------------------

class MidiPanKnobPlugin : public QObject, public AwlPlugin {
      Q_OBJECT

   public:
     	MidiPanKnobPlugin(QObject* parent = 0) : QObject(parent) {}
      QString includeFile() const { return "awl/midipanknob.h"; }
      QString name() const { return "Awl::MidiPanKnob"; }
      QWidget* createWidget(QWidget* parent);
      };

//---------------------------------------------------------
//   AwlPlugins
//---------------------------------------------------------

class AwlPlugins : public QObject, public QDesignerCustomWidgetCollectionInterface {
      Q_OBJECT
      Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

   public:
      QList<QDesignerCustomWidgetInterface*> customWidgets() const;
      };

#endif

