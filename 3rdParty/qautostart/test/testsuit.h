#ifndef TESTSUIT_H
#define TESTSUIT_H

#include <QtTest/QtTest>
#include <QAutostart>

class TestSuit : public QObject {
    Q_OBJECT

private slots:
    // functions executed by QtTest before and after test suite
    void initTestCase();
    void cleanupTestCase();

    // functions executed by QtTest before and after each test
    void init();
    void cleanup();

    void testAutostartConfig();

};

#endif // TESTSUIT_H
