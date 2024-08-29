        #include <QCoreApplication>
#include <QtTest/QtTest>

#include "testsuit.h"

#define QT_NO_PROCESS

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TestSuit test_suit;

    return QTest::qExec(&test_suit, argc, argv);
}


