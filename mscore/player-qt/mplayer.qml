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
import MuseScore 1.0
import "mobile" as Mobile

Item {
      id: screen
//      anchors.fill: parent;

      width: 1024; height: 768
      state: "myscores"
      property bool inScoreView: false;

      Rectangle {
            id: background;
            anchors.fill: parent;

            color: "#343434"
            Image {
                  source: "mobile/images/stripes.png"
                  fillMode: Image.Tile
                  anchors.fill: parent
                  opacity: 0.3
                  }

            states: State {
                  name: "ScoreView"
                  when: screen.inScoreView == true
                  PropertyChanges {
                        target: scoreViewFlick
                        x: 0
                        }
                  PropertyChanges {
                        target: scoreListView
                        x: -(parent.width * 1.5)
                        }
                  PropertyChanges {
                        target: toolBar
                        button1Visible: true
                        button2Visible: true
                        }
                  }
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
                        path: ":/scores/promenade.mscz"
                        }
                  ListElement {
                        title: "Leise rieselt der Schnee"
                        path: ":/scores/schnee.mscz"
                        }
                  ListElement {
                        title: "Italienisches Konzert"
                        path: ":/scores/italian-1.mscz"
                        }
                  }

            Component {
                  id: scorelistdelegate
                  Text {
                        id: label
                        font.pixelSize: 18
                        text: type
                        }
                  }

            ListView {
                  id: scoreListView
                  width: parent.width
                  height: parent.height
                  model: scorelist
                  delegate: Mobile.ListDelegate { }

                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              var idx = scoreListView.indexAt(mouseX, mouseY)
                              if (idx >= 0) {
                                    scoreView.setScore(scorelist.get(idx).path)
                                    screen.inScoreView = true
                                    }
                              }
                        }
                  }

            Flickable {
                  id: scoreViewFlick
                  width: parent.width;
                  height: parent.height;
                  x: -(parent.width * 1.5);

                  contentWidth: scoreView.width
                  contentHeight: scoreView.height

                  ScoreView {
                        id: scoreView
                        MouseArea {
                              anchors.fill: parent
                              onClicked: {
                                    if (mouseX < width * .3)
                                          parent.prevPage()
                                    else if (mouseX > width * .6)
                                          parent.nextPage()
                                    }
                              }
                        }
                  }

            Mobile.ToolBar {
                  id: toolBar
                  height: 40; anchors.bottom: parent.bottom;
                  width: parent.width; opacity: 0.9
                  button1Label: "MyScores"; button2Label: "Play"
                  onButton1Clicked: {
                        if (screen.inScoreView == true)
                              screen.inScoreView = false;
                        else
                              screen.inScoreView = true;
                        }
                  onButton2Clicked: scoreView.play();
                  button1Visible: false
                  button2Visible: false
                  }
            }
      }

