//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "profile.h"
#include "mscore.h"
#include "xml.h"
#include "preferences.h"
#include "palette.h"

static bool profilesRead = false;
static QList<Profile*> _profiles;
static Profile* defaultProfile;
Profile* profile;

//---------------------------------------------------------
//   initProfile
//---------------------------------------------------------

void initProfile()
      {
      defaultProfile = new Profile;
      defaultProfile->setName("default");

      foreach(Profile* p, Profile::profiles()) {
            if (p->name() == preferences.profile) {
                  profile = p;
                  break;
                  }
            }
      if (profile == 0)
            profile = defaultProfile;
      }

//---------------------------------------------------------
//   Profile
//---------------------------------------------------------

Profile::Profile()
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Profile::write()
      {
      QString ext(".profile");
      if (_path.isEmpty()) {
            QDir dir;
            dir.mkpath(dataPath);
            _path = dataPath + "/profiles";
            dir.mkpath(_path);
            _path += "/" + _name + ext;
            }
      QFile f(_path);
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = mscore->tr("Open Profile File\n") + f.fileName() + mscore->tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, mscore->tr("MuseScore: Open Profile file"), s);
            return;
            }
      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      xml.stag("Profile");
      xml.tag("name", _name);
      PaletteBox* pb = mscore->getPaletteBox();
      pb->write(xml);
      xml.etag();
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = mscore->tr("Write Profile failed: ") + f.errorString();
            QMessageBox::critical(0, mscore->tr("MuseScore: Write Style"), s);
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Profile::read()
      {
      if (_name == "default") {
            PaletteBox* paletteBox = mscore->getPaletteBox();
            paletteBox->clear();
            mscore->populatePalette();
            return;
            }
      QFile f(_path);
      if (f.open(QIODevice::ReadOnly)) {
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(&f, false, &err, &line, &column)) {
                  QString error = QString("error reading profile %1 at line %2 column %3: %4")
                     .arg(f.fileName()).arg(line).arg(column).arg(err);
                  QMessageBox::warning(0,
                     QWidget::tr("MuseScore: Load Style failed:"),
                     error,
                     QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
                  return;
                  }
            docName = _path;
            for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
                  if (e.tagName() == "museScore") {
                        QString version = e.attribute(QString("version"));
                        QStringList sl = version.split('.');
                        // _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                        for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                              QString tag(ee.tagName());
                              QString val(ee.text());
                              if (tag == "Profile")
                                    read(ee);
                              else
                                    domError(ee);
                              }
                        }
                  }
            }
      else {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load Profile failed:"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            }
      }

void Profile::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "name")
                  _name = val;
            else if (tag == "PaletteBox") {
                  PaletteBox* paletteBox = mscore->getPaletteBox();
                  paletteBox->clear();
                  paletteBox->read(e);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Profile::save()
      {
      PaletteBox* pb = mscore->getPaletteBox();
      bool d = pb && pb->dirty();
      if (d && (profile->name() != "default"))
            write();
      }

//---------------------------------------------------------
//   profiles
//---------------------------------------------------------

const QList<Profile*>& Profile::profiles()
      {
      if (!profilesRead) {
            QString s = dataPath + "/profiles";
            QDir dir(s);
            QStringList nameFilters;
            nameFilters << "*.profile";
            QStringList pl = dir.entryList(nameFilters, QDir::Files, QDir::Name);

            _profiles.append(defaultProfile);
            foreach (QString s, pl) {
                  Profile* p = new Profile;
                  p->setPath(dataPath + "/profiles/" + s);
                  p->setName(QFileInfo(s).baseName());
                  _profiles.append(p);
                  }
            profilesRead = true;
            }
      return _profiles;
      }

//---------------------------------------------------------
//   createNewProfile
//---------------------------------------------------------

Profile* Profile::createNewProfile(const QString& name)
      {
      profile->save();
      Profile* p = new Profile(*profile);
      p->setName(name);
      p->setPath("");
      p->setDirty(false);
      p->write();
      _profiles.append(p);
      return p;
      }

