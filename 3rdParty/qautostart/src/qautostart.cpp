// The MIT License (MIT)
//
// Copyright (C) 2016 Mostafa Sedaghat Joo (mostafa.sedaghat@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "qautostart.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>
#include <QSettings>
#include <QProcess>
#include <QString>
#include <QFile>
#include <QDir>
#include <QtGlobal>

#if defined (Q_OS_WIN)
#define REG_KEY "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

bool Autostart::isAutostart() const {
    QSettings settings(REG_KEY, QSettings::NativeFormat);

    if (settings.value(appName()).isNull()) {
        return false;
    }

    return true;
}

void Autostart::setAutostart(bool autostart) {
    QSettings settings(REG_KEY, QSettings::NativeFormat);

    if (autostart) {
        settings.setValue(appName() , appPath().replace('/','\\'));
    } else {
        settings.remove(appName());
    }
}

QString Autostart::appPath() const {
    return QCoreApplication::applicationFilePath() + " --autostart";
}

#elif defined (Q_OS_MAC)

bool Autostart::isAutostart() const {
    QProcess process;
    process.start("osascript", {
        "-e tell application \"System Events\" to get the path of every login item"
    });
    process.waitForFinished(3000);
    const auto output = QString::fromLocal8Bit(process.readAllStandardOutput());
    return output.contains(appPath());
}

void Autostart::setAutostart(bool autostart) {
    // Remove any existing login entry for this app first, in case there was one
    // from a previous installation, that may be under a different launch path.
    {
        QProcess::execute("osascript", {
            "-e tell application \"System Events\" to delete every login item whose name is \"" + appName() + "\""
        });
    }

    // Now install the login item, if needed.
    if ( autostart )
    {
        QProcess::execute("osascript", {
            "-e tell application \"System Events\" to make login item at end with properties {path:\"" + appPath() + "\", hidden:true, name: \"" + appName() + "\"}"
        });
    }
}

QString Autostart::appPath() const {
    QDir appDir = QDir(QCoreApplication::applicationDirPath());
    appDir.cdUp();
    appDir.cdUp();
    QString absolutePath = appDir.absolutePath();

    return absolutePath;
}

#elif defined (Q_OS_LINUX)
bool Autostart::isAutostart() const {
    QFileInfo desktopFile(Autostart::getAutostartDir() + "/" + appName() + ".desktop");
    return desktopFile.isFile();
}

// Spec: https://specifications.freedesktop.org/autostart-spec/autostart-spec-0.5.html#idm45071660440416
QString Autostart::getAutostartDir() const {
    QByteArray xdgEnv = qgetenv("XDG_CONFIG_HOME");
    if (!xdgEnv.isEmpty()) {
        QDir xdgConfigHome = QDir(xdgEnv);
        if (xdgConfigHome.exists()) {
            return QDir::cleanPath(xdgConfigHome.absolutePath() + "/autostart");
        }
    }
    return QDir::homePath() + "/.config/autostart";
}

bool Autostart::isFlatpak() const {
    QFileInfo flatpakInfo("/.flatpak-info");
    return flatpakInfo.isFile() && !qgetenv("FLATPAK_ID").isEmpty();
}

void Autostart::setAutostart(bool autostart) {
    QString path = Autostart::getAutostartDir();
    QString name = appName() + ".desktop";
    QFile file(path + "/" + name);

    file.remove();

    if(autostart) {
        QDir dir(path);
        if(!dir.exists()) {
            dir.mkpath(path);
        }

        if (file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << "[Desktop Entry]" << Qt::endl;
            stream << "Exec=" << appPath() << Qt::endl;
            stream << "Type=Application" << Qt::endl;
        }
    }
}

QString Autostart::appPath() const {
    QString cmd;
    if (Autostart::isFlatpak()) {
        cmd = "flatpak run " + QString::fromUtf8(qgetenv("FLATPAK_ID"));
    } else {
        cmd = QCoreApplication::applicationFilePath();
    }
    return cmd + " --autostart";
}

#else

bool Autostart::isAutostart() const {
    return false;
}

void Autostart::setAutostart(bool autostart) {
    Q_UNUSED(autostart);
}

QString Autostart::appPath() const {
    return QString();
}
#endif

QString Autostart::appName() const {
#if defined(Q_OS_LINUX)
    if (Autostart::isFlatpak()) {
        return QString::fromUtf8(qgetenv("FLATPAK_ID"));
    }
#endif
    return QCoreApplication::applicationName();
}
