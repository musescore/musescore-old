import QtQuick 1.0

Item {
      id: titleBar
      BorderImage {
            source: "images/titlebar.sci"
            width: parent.width;
            height: parent.height + 14; y: -7
            }

      Item {
            id: container
            width: (parent.width * 2) - 55
            height: parent.height

            Text {
                  id: categoryText
                  anchors {
                        leftMargin: 10
                        rightMargin: 10
                        verticalCenter: parent.verticalCenter
                        horizontalCenter: parent.horizontalCenter
                        }
                  elide: Text.ElideLeft
                  text: "MyScores"
                  font.bold: true
                  font.pixelSize: 15
                  color: "White"
                  style: Text.Raised
                  styleColor: "Black"
                  }
            }
      }

