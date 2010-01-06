//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "updatechecker.h"

UpdateChecker::UpdateChecker()
{   
    manager = new QNetworkAccessManager(this);  
}

UpdateChecker::~UpdateChecker()
{

} 
 
void UpdateChecker::onRequestFinished(QNetworkReply* reply)
{
    QByteArray data = reply->readAll();
    QXmlStreamReader reader(data);
    QString version;
    QString upgradeRevision;
    QString downloadUrl;
    QString infoUrl;
    QString description;
    QString releaseType;
    
    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if(token == QXmlStreamReader::StartDocument) {
            continue;
        }
        if(token == QXmlStreamReader::StartElement) {
            if(reader.name() == "version") {
                version = parseText(reader);
            }else if (reader.name() == "revision") {
                upgradeRevision = parseText(reader);
            }else if (reader.name() == "downloadUrl") {
                downloadUrl = parseText(reader);
            }else if (reader.name() == "infoUrl") {
                infoUrl = parseText(reader);
            }else if (reader.name() == "description") {
                description = parseText(reader);
            }            
        }
    }
    
    if (reader.error())
        qDebug() << reader.error() << reader.errorString();
    
    QString message = QString(tr("An update for MuseScore is available : <a href=\"%1\">MuseScore %2r%3</a>")).arg(downloadUrl).arg(version).arg(upgradeRevision);
    printf("revision %s\n", revision.toAscii().constData());
    if(!version.isNull() &&  upgradeRevision > revision ){
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Update available"));
        msgBox.setText(message);
        msgBox.setTextFormat(Qt::RichText);
        msgBox.exec();
    }    
}

QString UpdateChecker::parseText(QXmlStreamReader& reader){
    QString result;
    reader.readNext();  
    if(reader.tokenType() == QXmlStreamReader::Characters)
      result = reader.text().toString();
    return result;
}
 
void UpdateChecker::check(QString rev)
{   
    #if defined(Q_WS_WIN)
    os = "win";
    #endif
    #if defined(Q_WS_MAC)
    os = "mac";
    #endif
    if(qApp->applicationName() == "MuseScore"){ //avoid nightly cymbals
        #if defined(MSCORE_UNSTABLE)
        release = "pre";
        #else
        release = "stable";
        #endif
    }else{
        release = "nightly";
    }
        
    if(!os.isNull() && !release.isNull()){
        revision =  rev;   
        manager->get(QNetworkRequest(QUrl("http://update.musescore.org/update_"+os +"_" + release +".xml")));
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onRequestFinished(QNetworkReply*)));
    }
}
