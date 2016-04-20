#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "audioengine.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AudioEngine audioEngine(&app);
    audioEngine.setup();

    audioEngine.startPlayback();
    //audioEngine.startToneGenerator();

    QQmlApplicationEngine engine;
    engine.addImportPath(":/qml/");
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    return app.exec();
}
