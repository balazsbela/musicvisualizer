#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>

#include <memory>

#include "audioengine.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.addImportPath(":/qml/");
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    AudioEngine audioEngine;

    QThread audioThread;
    audioEngine.moveToThread(&audioThread);
    audioThread.start();

    QObject::connect(&app, &QGuiApplication::aboutToQuit, [&]()
    {
        audioEngine.stop();
        audioThread.exit(0);
    });

    audioEngine.setup();
    audioEngine.startPlayback();

    int result = app.exec();

    audioThread.wait(5000);
    return result;
}
