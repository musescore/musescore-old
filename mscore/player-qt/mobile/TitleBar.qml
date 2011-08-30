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
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        }
                  horizontalAlignment: Text.AlignHCenter
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

