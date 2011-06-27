import QtQuick 1.0
import MuseScore 1.0
import Qt.labs.folderlistmodel 1.0

Row {
      spacing: 5
      Rectangle {
            width: 150; height: 400;
            color: "lightgray"
            ListView {
                  width: 150; height: 400

                  FolderListModel {
                        id: folderModel
                        nameFilters: ["*.mscx", "*.mscz"]
                        showDirs: true
                        showDotAndDotDot: true
                        folder: "file:/home"
                        }
                  Component {
                        id: fileDelegate
                        Text {
                              width: 150
                              font.pixelSize: 14
                              text: fileName
                              }
                        }
                  model:        folderModel
                  delegate:     fileDelegate
                  highlight:    Rectangle { color: "lightsteelblue"; radius: 3 }
                  focus:        true
                  MouseArea {
                        anchors.fill: parent
                        onClicked: {
                              var idx = parent.indexAt(parent.contentX+mouseX, parent.contentY+mouseY)
                              parent.currentIndex = idx
                              }
                        onDoubleClicked: {
                              var idx = parent.currentIndex;
                              if (folderModel.isFolder(idx)) {
                                    var name = parent.currentItem.text
                                    if (name == ".") {
                                          }
                                    else if (name == "..") {
                                          folderModel.folder = folderModel.parentFolder
                                          }
                                    else {
                                          var url = folderModel.folder + "/" + name
                                          folderModel.folder = url
                                          }
                                    }
                              else {
                                    console.log("file")
                                    }
                              }
                        }
                  }
            }

      ScoreView {
            width: 600; height: 400
            clip: true
            smooth: true

            MouseArea {
                  anchors.fill: parent
                  onPositionChanged: { parent.drag(mouseX, mouseY)      }
                  onPressed:         { parent.startDrag(mouseX, mouseY) }
                  }
            }
      }

