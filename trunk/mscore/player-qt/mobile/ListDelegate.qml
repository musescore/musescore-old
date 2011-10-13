import QtQuick 1.0

Component {
      Item {
            id: wrapper
            width: wrapper.ListView.view.width
            height: 70

            Item {
                  id: moveMe
                  Rectangle {
                        color: "black";
                        opacity: index % 2 ? 0.2 : 0.4;
                        height: 68;
                        width: wrapper.width;
                        y: 1
                        }
                  Rectangle {
                        x: 6;
                        y: 4;
                        width: 100;
                        height: 60;
                        color: "black";
                        smooth: true

                        Image { source: imagePath; x: 0; y: 0 }
                        }

                  Column {
                        x: 115;
                        width: wrapper.ListView.view.width - 95;
                        y: 15;
                        spacing: 2
                        Text {
                              text: title;
                              color: "white";
                              width: parent.width;
                              font.pixelSize: 14;
                              font.bold: true;
                              elide: Text.ElideRight;
                              style: Text.Raised;
                              styleColor: "black"
                              }
                        Text {
                              text: author;
                              width: parent.width;
                              font.pixelSize: 14;
                              elide: Text.ElideLeft;
                              color: "#cccccc";
                              style: Text.Raised;
                              styleColor: "black"
                              }
                        }
                  }


            MouseArea{
                   anchors.fill: parent
                   onClicked: {
                         scoreView.loadUrl(scorelist.get(index).path)
                         screen.state = "ScoreView"
                         }
                   }
            }
      }
