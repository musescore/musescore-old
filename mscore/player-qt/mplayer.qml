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
//  the file LICENSE.GPL
//=============================================================================

import QtQuick 1.0
import Qt.labs.gestures 1.0
import MuseScore 1.0
import "mobile" as Mobile

Item {
      property string apiKey : "KeqJNLwjASQKMkoBF6U6qhnGiRXqgTkq"

      id: screen
      property string state: "ListView"
      width: 854
      height: 480-38

      Rectangle {
            id: background;
            anchors.fill: parent
            state: "myscores"

            color: "#343434"
            Image {
                  source: "mobile/images/stripes.png"
                  fillMode: Image.Tile
                  anchors.fill: parent
                  opacity: 0.3
                  }

            states: [
                  State {
                        name: "ScoreView"
                        when: screen.state == "ScoreView"
                        PropertyChanges {
                              target: scoreView
                              x: 0
                              }
                        PropertyChanges {
                              target: scoreListView
                              x: -width;
                              }
                        PropertyChanges {
                              target: toolBar
                              button1Visible: true
                              button2Visible: true
                              button3Visible: true
                              tempoButtonVisible: true
                              }
                        },
                  State {
                        name: "PlainScoreView"
                        when: screen.state == "PlainScoreView"
                        PropertyChanges {
                              target: scoreView
                              x: 0
                              }
                        PropertyChanges {
                              target: scoreListView
                              x: -width;
                              }
                        PropertyChanges {
                              target: toolBar
                              visible: false
                              }
                        PropertyChanges {
                              target: titleBar
                              visible: false
                              }
                        }
                  ]
            transitions: Transition {
                  NumberAnimation {
                        properties: "x"
                        easing.type: Easing.InOutQuad
                        duration: 500
                        }
                  }

            ListModel {
                  id: scorelist
                  ListElement {
                        title: "Promenade"
                        author: "Modeste Moussorgsky"
                        path: ":/scores/promenade.mscz"
                        imagePath: "qrc:///scores/promenade.png"
                        }
                  ListElement {
                        title: "Leise rieselt der Schnee"
                        author: "Traditional"
                        path: ":/scores/schnee.mscz"
                        imagePath: "qrc:///scores/schnee.png"
                        }
                  ListElement {
                        title: "Italienisches Konzert"
                        author: "J.S. Bach"
                        path: ":/scores/italian-1.mscz"
                        imagePath: "qrc:///scores/italian-1.png"
                        }
                  }

            //Mobile.ScoreListModel { id: scoreListModel }

            XmlListModel {
                id: xmlScoreListModel
                source: "http://api.musescore.com/services/rest/score.xml?oauth_consumer_key="+apiKey
                query: "/scores/score"

                XmlRole { name: "title"; query: "title/string()" }
                XmlRole { name: "composer"; query: "metadata/composer/string()" }
                XmlRole { name: "permalink"; query: "permalink/string()" }
                XmlRole { name: "description"; query: "description/string()" }
                XmlRole { name: "id"; query: "id/string()" }
                XmlRole { name: "secret"; query: "secret/string()" }
                onStatusChanged: {
                    if (status == xmlScoreListModel.Ready, progress ==1.0) {
                        for(var i = 0; i < xmlScoreListModel.count; i++){
                          var o = xmlScoreListModel.get(i);
                            print(o.id);
                           scorelist.append({"title":o.title,
                                              "author": o.composer,
                                              "path": "http://static.musescore.com/"+o.id+"/"+o.secret+"/score.mscz",
                                              imagePath: "http://static.musescore.com/"+o.id+"/"+o.secret+"/thumb.png"
                           });
                        }
                    }
                }
            }

            ListView {
                  id: scoreListView
                  anchors.top: titleBar.bottom
                  anchors.bottom: toolBar.top

                  width: parent.width
                  height: parent.height
                  model: scorelist
                  delegate: Mobile.ListDelegate { }
                  }

            ScoreView {
                  id: scoreView
                  parentWidth: screen.width
                  parentHeight: screen.height
                  x: -width;
                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              if (mouseX > parent.width * .6)
                                    scoreView.nextPage();
                              else if (mouseX < parent.width * .3)
                                    scoreView.prevPage();
                              else {
                                    if (screen.state == "ScoreView")
                                          screen.state = "PlainScoreView"
                                    else
                                          screen.state = "ScoreView";
                                    }
                              }
                        onPressAndHold: {
                              scoreView.seek(mouseX, mouseY);
                              }
                        }
                  }

            Mobile.TitleBar {
                  id: titleBar
                  height: 40
                  width: parent.width
                  opacity: .9
                  anchors.top: parent.top
                  }

            Mobile.ToolBar {
                  id: toolBar
                  height: 40;
                  width: parent.width;
                  opacity: .9
                  anchors.bottom: parent.bottom

                  button1Label: "MyScores"
                  onButton1Clicked: {
                        if (screen.state == "ScoreView")
                              screen.state = "ListView"
                        else
                              screen.state = "ScoreView"
                        }
                  onButton3Clicked: {
                        scoreView.rewind()
                        }
                  onTempoChanged: {
                        scoreView.setTempo(tempo)
                        }
                  onButton2Clicked: scoreView.play();
                  button1Visible: false
                  button2Visible: false
                  button3Visible: false
                  tempoButtonVisible: false
                  }
            }
      }

