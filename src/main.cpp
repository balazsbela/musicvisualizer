#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <QtQml>
#include <QSurfaceFormat>

#include <memory>

#include "constants.h"
#include "audioengine.h"
#include "fftwrapper.h"
#include "fftvisualizationdata.h"
#include "samplevisualizationdata.h"
#include "buffertextureprovider.h"


int main(int argc, char *argv[])
{
    // Disable vsync

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSwapInterval(0);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setOptions(QSurfaceFormat::StereoBuffers);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(2, 9);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);

    // Set up communication through event queue

    Visualizer::Common::fft_queue_t bufferQueue(0);
    Visualizer::Common::fft_result_queue_t fftResultQueue(0);
    Visualizer::Common::sample_queue_t sampleQueue(0);

    Q_ASSERT(bufferQueue.is_lock_free());
    Q_ASSERT(fftResultQueue.is_lock_free());
    Q_ASSERT(sampleQueue.is_lock_free());

    // Start audio thread

    QThread audioThread;
    AudioEngine audioEngine(bufferQueue, sampleQueue);
    audioEngine.moveToThread(&audioThread);
    QObject::connect(&audioThread, &QThread::started, &audioEngine, [&]()
    {
        //audioEngine.startToneGenerator();

        const auto& args = QGuiApplication::arguments();
        if (args.count() > 1)
        {
            audioEngine.startPlayback(args[1]);
        }
        else
        {
            audioEngine.startPlayback();
        }
    });

    audioThread.start();

    // Start FFT Thread

    QThread fftThread;
    FFTWrapper fft(bufferQueue, fftResultQueue);
    fft.moveToThread(&fftThread);
    QObject::connect(&fftThread, &QThread::started, &fft, &FFTWrapper::start);
    fftThread.start();


    // Ensure clean exit

    QObject::connect(&app, &QGuiApplication::aboutToQuit, &audioEngine, &AudioEngine::stop);
    QObject::connect(&app, &QGuiApplication::aboutToQuit, &fft, &FFTWrapper::stop);

    QObject::connect(&audioEngine, &AudioEngine::stopped, [&]()
    {
        audioThread.exit(0);
    });

    QObject::connect(&fft, &FFTWrapper::stopped, [&]()
    {
        fftThread.exit(0);
    });


    FFTVisualizationData fftVisualization(fftResultQueue);
    SampleVisualizationData sampleVisualization(sampleQueue);

    QSharedPointer<QImage> image(new QImage(QSize(Visualizer::Common::nrSamples, 1), QImage::Format_ARGB32));
    sampleVisualization.getBufferTextureProvider()->setImage(image);

    qmlRegisterType<QQmlVariantListModel>("visualizer.models", 1, 0, "QQmlVariantListModel");
    qmlRegisterUncreatableType<SampleVisualizationData>("visualizer.models", 1, 0, "SampleVisualizationData", "Reasons");


    QQmlApplicationEngine engine;
    engine.addImportPath(":/qml/");
    engine.rootContext()->setContextProperty(QStringLiteral("fftData"), fftVisualization.getModel());
    engine.addImageProvider(QStringLiteral("buffer"), sampleVisualization.getBufferTextureProvider());
    engine.rootContext()->setContextProperty(QStringLiteral("sampleData"), &sampleVisualization);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    int result = app.exec();

    audioThread.wait(5000);
    fftThread.wait(5000);
    return result;
}
