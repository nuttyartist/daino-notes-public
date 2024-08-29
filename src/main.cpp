/*********************************************************************************************
 * Mozilla License
 * Just a meantime project to see the ability of qt, the framework that my OS might be based on
 * And for those linux users that believe in the power of notes
 *********************************************************************************************/

#include "mainwindow.h"
#include "singleinstance.h"
#include <QApplication>
#include <QFontDatabase>
// #include "flexbackend.h"
// using namespace Quite::Objects;
// using namespace Quite::Objects::Flex;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#if (defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)) || defined(Q_OS_WIN)
    qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "3000");
#endif

    // Set application information
    app.setApplicationName("Daino Notes");
    app.setApplicationVersion(APP_VERSION);

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    app.setDesktopFileName(APP_ID);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_DisableWindowContextHelpButton);
#endif

    if (QFontDatabase::addApplicationFont(":/fonts/fontawesome/fa-solid-900.ttf") < 0)
        qWarning() << "FontAwesome cannot be loaded !";

    if (QFontDatabase::addApplicationFont(":/fonts/material/material-symbols-outlined.ttf") < 0)
        qWarning() << "Material Icons cannot be loaded !";



    // Load fonts from resources
    // Sans
    // Inter
    QFontDatabase::addApplicationFont(":/fonts/inter/InterVariable.ttf");
    QFontDatabase::addApplicationFont(":/fonts/inter/InterVariable-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/inter/Inter.ttc");

    // Serif
    // Ibarra Real
    QFontDatabase::addApplicationFont(":/fonts/ibarrareal/IbarraRealNova[wght].ttf");
    QFontDatabase::addApplicationFont(":/fonts/ibarrareal/IbarraRealNova-Italic[wght].ttf");
    // Trykker
    QFontDatabase::addApplicationFont(":/fonts/trykker/Trykker-Regular.ttf");

    // Mono
    // iA Mono
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iamono/iAWriterMonoS-BoldItalic.ttf");

           // iA Duo
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaduo/iAWriterDuoS-BoldItalic.ttf");

           // iA Quattro
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-Italic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/iaquattro/iAWriterQuattroS-BoldItalic.ttf");

    // Prevent many instances of the app to be launched
    QString plumeName = "com.awsomeness.plume";
    QString notesName = "com.awsomeness.notes";
    SingleInstance instance;
    if (instance.hasPrevious(plumeName) || instance.hasPrevious(notesName)) {
        return EXIT_SUCCESS;
    }

    instance.listen(plumeName);

    // qmlRegisterType<FlexBackend>(
    //         "com.tripolskypetr.quitejs",
    //         1, 0,
    //         "FlexBackend"
    //         );

    // Create and Show the app
    MainWindow w;
    w.show();

    // Bring the window to the front
    QObject::connect(&instance, &SingleInstance::newInstance, &w,
                     [&]() { (&w)->setMainWindowVisibility(true); });

    return app.exec();
}
