import QtQuick 2.9

Item {
    id: root

    property int size: 150
    property int lineWidth: 5
    property real value: 0

    property color primaryColor: "#29b6f6"
    property color secondaryColor: "#e0e0e0"

    property int animationDuration: 1000

    width: size
    height: size

    onValueChanged: {
        canvas.degree = value * 360;
    }

    onPrimaryColorChanged: {
        canvas.requestPaint();
    }

    onSecondaryColorChanged: {
        canvas.requestPaint();
    }

    Canvas {
        id: canvas

        property real degree: 0

        anchors.fill: parent
        antialiasing: true

        onDegreeChanged: {
            requestPaint();
        }

        onPaint: {
            var ctx = getContext("2d");

            var x = root.width / 2;
            var y = root.height / 2;

            var outerRadius = root.size / 2 - root.lineWidth;
            var progressRadius = outerRadius - root.lineWidth;
            // var progressRadius = outerRadius - root.lineWidth * 2 ; // Smaller

            var startAngle = (Math.PI / 180) * 270;

            var progressAngle = (Math.PI / 180) * (270 + degree);

            ctx.reset();

            ctx.lineCap = 'round';
            ctx.lineWidth = root.lineWidth;

            // Draw the background circle
            ctx.beginPath();
            ctx.arc(x, y, outerRadius, startAngle, startAngle + Math.PI * 2);
            ctx.strokeStyle = root.secondaryColor;
            ctx.stroke();

            if (degree > 0) {
                ctx.beginPath();
                ctx.moveTo(x, y);
                ctx.arc(x, y, progressRadius, startAngle, progressAngle);
                ctx.lineTo(x, y);
                ctx.fillStyle = root.primaryColor;
                ctx.fill();
            }
        }

        Behavior on degree {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutExpo
            }
        }
    }
}
