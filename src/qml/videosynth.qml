import QtQuick 2.3
import QtQuick.Window 2.2

Window
{
    visible: true
    id: mainwindow
    width:1024
    height:768
    color: "#000000"
    visibility: "FullScreen"


    Rectangle
    {
        id:root
        color: mainwindow.color
        anchors.fill: parent

        ShaderEffect
        {
            id: shader
            z: 0
            property variant prevSource: shaderOutput

            property variant feedbackGain:    0.99
            property real freq: 0.7
            property real time
            NumberAnimation on time
            {
                running: true
                loops: Animation.Infinite
                from: 0.0
                to: 100.0
                duration: 100000
            }


            anchors.fill: parent
            visible:true
            blending:true
            smooth: true

            property real frameWidth:  parent.width
            property real frameHeight: parent.height
            antialiasing: true
            fragmentShader: "

                          #define lines 1000.0

                          varying highp vec2 qt_TexCoord0;
                          uniform highp float time;
                          uniform highp float freq;
                          uniform sampler2D prevSource;
                          uniform lowp  float qt_Opacity;
                          uniform highp float feedbackGain;
                          uniform highp float frameWidth;
                          uniform highp float frameHeight;
                          void main()
                          {
                              highp vec2 uv = qt_TexCoord0;
                              highp vec4 feedback = texture2D(prevSource,0.999*uv-vec2(0.0,0.004));
                              highp float timebase = (floor(lines * uv.y) + uv.x)/lines;
                              highp float osc1 = 0.5+0.5*sin(6.283185*(0.1*time+1.0*timebase + freq*1.0*length(feedback.rgb)));
                              highp float osc2 = 0.5+0.5*sin(6.283185*(0.01*time+4.0*lines*timebase + 0.9*freq*osc1));
                              highp float osc3 = 0.5+0.5*sin(6.283185*(0.01*time+2.999*lines*timebase + 0.9*freq*osc2));
                              gl_FragColor = mix(vec4(osc3,osc1,osc2,0.9),feedback,0.54);
                          }"
        }

        ShaderEffectSource
        {
            id: shaderOutput
            hideSource: false
            anchors.fill: parent
            sourceItem: shader
            recursive: true
            visible: false
            live: true
        }
    }
}
