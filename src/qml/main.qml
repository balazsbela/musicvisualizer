import QtQuick 2.3
import QtQuick.Window 2.2

Window
{
    visible: true
    id: mainwindow
    color: "#000000"
    visibility: "FullScreen"

    Rectangle
    {
        id:root
        color: mainwindow.color
        width: parent.width
        height: parent.height / 2

        property variant previousdata : []
        property variant previousDataAge: []

        property bool interpolate : true
        property real t : 0.0

        Connections
        {
            target: dataModel
            onModelReset:
            {
                if (root.interpolate)
                {
                    root.t = 0.0;
                }
            }
        }

        Timer
        {
            running: true
            interval: 15
            repeat: true
            onTriggered:
            {
                if (root.interpolate)
                {
                   root.t = root.t >= 1.00 ?  1.00 : root.t + 0.15;
                }

               canvas.requestPaint();
            }
        }

        Canvas
        {
            id: canvas
            anchors.fill: parent
            renderTarget: Canvas.FramebufferObject
            renderStrategy: Canvas.Cooperative
            antialiasing: true
            onPaint:
            {
                var ctx = canvas.getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.lineWidth = 3;
                ctx.globalAlpha = 1.0;

                var t = root.t;

                for (var i = 0; i < dataModel.count; ++i)
                {
                    var x = i * 4;
                    if (x > mainwindow.width)
                    {
                        return;
                    }

                    var item = dataModel.get(i);
                    ctx.strokeStyle = Qt.hsla(i / dataModel.count, 0.85, 0.45, 1.0);

                    var current = 0;

                    if (root.interpolate)
                    {
                        var previous = root.previousdata[i] ? root.previousdata[i] : 0;
                        current = (1 - t) * item.val + t * previous;

                        if (Math.abs(item.val - previous) > 0.1 || root.previousDataAge[i] > 10)
                        {
                            root.previousdata[i] = item.val;
                            root.previousDataAge[i] = 0;
                        }
                        else
                        {
                            root.previousDataAge[i]++;
                        }
                    }
                    else
                    {
                        current = item.val;
                    }

                    ctx.beginPath();
                    ctx.moveTo(x, parent.height);
                    ctx.lineTo(x, parent.height - 300 * current);
                    ctx.stroke();
                }
            }

        }

        ShaderEffect
        {
            id: shaderItem
            property variant source: ShaderEffectSource { sourceItem: canvas; smooth: true; hideSource: true}
            property variant recursiveSource: recursiveSource
            anchors.fill: parent
            visible: true

            property real interpolationFactor;

            ParallelAnimation {
                running: true
                loops: Animation.Infinite

                UniformAnimator {
                    uniform: "interpolationFactor"
                    target: shaderItem
                    from: 0.85
                    to: 0.99
                    duration: 100
                    easing.type: Easing.Linear
                }

                UniformAnimator {
                    uniform: "interpolationFactor"
                    target: shaderItem
                    from: 0.99
                    to: 0.85
                    duration: 100
                    easing.type: Easing.Linear
                }
            }

            fragmentShader: "
                varying mediump vec2 qt_TexCoord0;
                uniform highp float qt_Opacity;
                uniform lowp sampler2D source;
                uniform lowp sampler2D recursiveSource;
                uniform lowp float interpolationFactor;
                void main() {
                    vec4 current = texture2D(source, qt_TexCoord0) * 1.75;
                    vec4 previous = texture2D(recursiveSource, qt_TexCoord0);
                    gl_FragColor = qt_Opacity * mix(current, previous, interpolationFactor);
                    //gl_FragColor = qt_Opacity * current;
                }"
        }

        ShaderEffectSource
        {
            id: recursiveSource
            sourceItem: shaderItem
            visible: false
            live: true
            smooth: true
            recursive: true
        }

    }

    // Add mirror effect

    ShaderEffect {
        property variant source: ShaderEffectSource { sourceItem: root; hideSource: false }

        anchors.top: root.bottom
        width: root.width
        height: root.height

        fragmentShader: "
            varying highp vec2 qt_TexCoord0;
            uniform highp float qt_Opacity;
            uniform sampler2D source;
            void main(void) {
                highp vec2 pos = vec2(qt_TexCoord0.x, (1.0 - qt_TexCoord0.y*0.8));
                pos.x += (qt_TexCoord0.y*0.2) * (pos.x * -1.0 + 1.0);
                highp vec4 pix = texture2D(source, pos);
                pix *= (0.4 - qt_TexCoord0.y*0.5) * min(qt_TexCoord0.y*5.0, 1.0);
                gl_FragColor = pix * qt_Opacity;
            }"
    }

}
