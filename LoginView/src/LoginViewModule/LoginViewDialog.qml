import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Universal 2.12
import QtQuick.Window 2.12

Popup {
    id: root
    property var lognViewModel: null

    width: Math.min(100 * Screen.pixelDensity * (parent.width / parent.height),
                    parent.width * 0.9);
    height: Math.min(200 * Screen.pixelDensity, parent.height * 0.9);


    x: parent.width / 2 - width / 2
    y: parent.height / 2 - height / 2

    LoginView {
        id: source
        lognViewModel: root.lognViewModel
        anchors.fill: parent

        onLoginClicked: {
            root.close();
        }
    }

    onClosed: {
        source.clear();
    }

}