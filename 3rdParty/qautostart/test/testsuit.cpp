#include "testsuit.h"


void TestSuit::initTestCase()
{
    // This function is being executed at the beginning of each test suite
    // That is - before other tests from this class run
    qDebug() << "testsuit::initTestCase()";
}

void TestSuit::cleanupTestCase()
{
    // Similarly to initTestCase(), this function is executed at the end of test suite
    qDebug() << "TestSuit::cleanupTestCase()";
}

void TestSuit::init()
{
    // This function is executed before each test
    qDebug() << "TestSuit::init()";
}

void TestSuit::cleanup()
{
    // This function is executed after each test
    qDebug() << "TestSuit::cleanup()";
}

void TestSuit::testAutostartConfig()
{
    Autostart as;

    as.setAutostart(true);
    QVERIFY(as.isAutostart() == true);

    as.setAutostart(false);
    QVERIFY(as.isAutostart() == false);

}
