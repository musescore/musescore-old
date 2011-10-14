import QtQuick 1.1

Item {
      id: titleBar
      BorderImage {
            source: "images/titlebar.sci"
            width: parent.width
            height: parent.height + 14
            y: -7
            }
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

