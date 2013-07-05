/****************************************************************************
**
** Copyright (C) 2012 - 2013 Jolla Ltd.
** Contact: http://jolla.com/
**
** This file is part of Qt Creator.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Digia.
**
****************************************************************************/

#include "merssh.h"
#include <mer/merconstants.h>
#include <ssh/sshremoteprocessrunner.h>

#include <QFile>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>
#include <QSocketNotifier>

enum CommandType {
    CommandTypeUndefined,
    CommandTypeStandard,
    CommandTypeSb2,
    CommandTypeMb2
};

MerSSH::MerSSH(QObject *parent)
    : QObject(parent)
    , m_exitCode(0)
    , m_quietOnError(false)
    , m_runner(new QSsh::SshRemoteProcessRunner())
{
    connect(m_runner, SIGNAL(processStarted()), SLOT(onProcessStarted()));
    connect(m_runner, SIGNAL(readyReadStandardOutput()), SLOT(onStandardOutput()));
    connect(m_runner, SIGNAL(readyReadStandardError()), SLOT(onStandardError()));
    connect(m_runner, SIGNAL(processClosed(int)), SLOT(onProcessClosed(int)));
    connect(m_runner, SIGNAL(connectionError()), SLOT(onConnectionError()));
}

MerSSH::~MerSSH()
{
    delete m_runner;
}

QSsh::SshConnectionParameters connectionParameters()
{
    QSsh::SshConnectionParameters parameters;
    parameters.host = QLatin1String(Mer::Constants::MER_SDK_DEFAULTHOST);
    parameters.authenticationType = QSsh::SshConnectionParameters::AuthenticationByKey;
    parameters.timeout = 10;
    return parameters;
}

CommandType commandTypeFromString(const QString &commandType)
{
    if (commandType == QLatin1String(Mer::Constants::MER_EXECUTIONTYPE_STANDARD))
        return CommandTypeStandard;
    else if (commandType == QLatin1String(Mer::Constants::MER_EXECUTIONTYPE_SB2))
        return CommandTypeSb2;
    else if (commandType == QLatin1String(Mer::Constants::MER_EXECUTIONTYPE_MB2))
        return CommandTypeMb2;
    return CommandTypeUndefined;
}

QString cacheFile(const QString &command, const QString &wrapperDir)
{
    QString result;
    const QStringList commandElements = command.split(QLatin1Char(' '));
    if (commandElements.first() == QLatin1String("qmake")) {
        if (commandElements.contains(QLatin1String("-version")))
            result = QLatin1String(Mer::Constants::QMAKE_VERSION);
        else if (commandElements.contains(QLatin1String("-query")))
            result = QLatin1String(Mer::Constants::QMAKE_QUERY);
    } else if (commandElements.first() == QLatin1String("gcc")) {
        if (commandElements.contains(QLatin1String("-dumpmachine")))
            result = QLatin1String(Mer::Constants::GCC_DUMPMACHINE);
        else if (commandElements.contains(QLatin1String("-dumpversion")))
            result = QLatin1String(Mer::Constants::GCC_DUMPVERSION);
    }
    if (!result.isEmpty())
        result.prepend(wrapperDir);
    return result;
}

bool MerSSH::run(const QString &sdkToolsDir, const QString &merTargetName,
                 const QString &commandType, const QString &command, bool interactive)
{
    m_interactive = interactive;

    using namespace Mer::Constants;
    // HACK: Qt Creator ignores gcc's exit code, and can be confused by non-gcc-like output.
    m_quietOnError = command.startsWith(QLatin1String("gcc "));

    const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QString sharedHome = environment.value(QLatin1String(MER_SSH_SHARED_HOME));
    const QString sharedTarget = environment.value(QLatin1String(MER_SSH_SHARED_TARGET));
    const QString sharedSrc = environment.value(QLatin1String(MER_SSH_SHARED_SRC));

    m_merSysRoot = sharedTarget + QLatin1Char('/') + merTargetName;

    QSsh::SshConnectionParameters params = connectionParameters();
    params.userName = environment.value(QLatin1String(MER_SSH_USERNAME));
    params.privateKeyFile = environment.value(QLatin1String(MER_SSH_PRIVATE_KEY));
    params.authenticationType = QSsh::SshConnectionParameters::AuthenticationByKey;
    params.port = environment.value(QLatin1String(MER_SSH_PORT)).toInt();

    CommandType type = commandTypeFromString(commandType);
    if (type == CommandTypeUndefined) {
        printError(QString::fromLatin1("Invalid commandType '%1'").arg(commandType));
        return false;
    }

    const QString merTargetDir = sdkToolsDir + merTargetName + QDir::separator();
    m_currentCacheFile = cacheFile(command, merTargetDir);
    if (QFile::exists(m_currentCacheFile)) {
        QFile cacheFile(m_currentCacheFile);
        if (!cacheFile.open(QIODevice::ReadOnly)) {
            printError(QString::fromLatin1("Cannot read cached file '%1'").arg(m_currentCacheFile));
            return false;
        }
        fprintf(stdout, "%s", cacheFile.readAll().constData());
        fflush(stdout);
        startTimer(0); // Trigger immediate exit
        return true;
    }

    QString completeCommand;

    switch (type) {
    case CommandTypeSb2: {
        const QString targetName = QString::fromUtf8(qgetenv(MER_SSH_TARGET_NAME));
        completeCommand = QLatin1String("sb2 -t ") + targetName + QLatin1Char(' ') + command + QLatin1Char(' ');
        break;
    }
    case CommandTypeMb2: {
        const QString projectPath = QString::fromUtf8(qgetenv(MER_PROJECTPATH_ENVVAR_NAME));
        const QString device = QString::fromUtf8(qgetenv(MER_SSH_DEVICE_NAME));
        const QString projectPathParameter = projectPath.isEmpty() ? QString() : QLatin1String(" -p ") + projectPath;
        const QString deviceParameter = device.isEmpty() ? QString() : QLatin1String(" -d \"") + device + QLatin1String("\"");
        const QString targetName = QString::fromUtf8(qgetenv(MER_SSH_TARGET_NAME));
        completeCommand = QLatin1String("mb2") + projectPathParameter + deviceParameter + QLatin1String(" -t ") +
                targetName +  QLatin1Char(' ') + command + QLatin1Char(' ');
        break;
    }
    case CommandTypeStandard:
        completeCommand = command;
        break;
    case CommandTypeUndefined:
        printError(QString::fromLatin1("Invalid commandType '%1'").arg(commandType));
        return false;
    default:
        break;
    }

    if (m_currentCacheFile.isEmpty()) {
        const QString currentPath = QDir::currentPath();
        const QString cleanSharedHome = QDir::fromNativeSeparators(QDir::cleanPath(sharedHome));
        const QString cleanSharedSrc = QDir::fromNativeSeparators(QDir::cleanPath(sharedSrc));
        if (!currentPath.startsWith(cleanSharedHome) && (sharedSrc.isEmpty() || !currentPath.startsWith(cleanSharedSrc))) {
            printError(QString::fromLatin1("Project ERROR: Project is outside of shared home '%1' and shared src '%2'.")
                       .arg(QDir::toNativeSeparators(sharedHome)).arg(QDir::toNativeSeparators(sharedSrc)));
            return false;
        }
        completeCommand.prepend(QLatin1String("cd \"") + QDir::currentPath()
                                + QLatin1String("\" && "));
    }
    // First replace shared home and then shared src (error prone!)
    completeCommand.replace(sharedHome, QLatin1String("$HOME/"));
    if (!sharedSrc.isEmpty())
        completeCommand.replace(sharedSrc, QLatin1String("/home/src1/"));
    completeCommand = completeCommand.trimmed();
    // hack for gcc when querying pre-defined macros and header paths
    QRegExp rx(QLatin1String("\\bgcc\\b.*\\B-\\B"));
    const bool queryPredefinedMacros = rx.indexIn(completeCommand) != -1;
    if (queryPredefinedMacros)
        completeCommand.append(QLatin1String(" < /dev/null"));


    m_sshConnectionParams = params;
    m_completeCommand = completeCommand.toUtf8();
    m_runner->run(m_completeCommand, m_sshConnectionParams);
    return true;
}

QString MerSSH::shellSafeArgument(const QString &argument)
{
    QString safeArgument = argument;
    if (QFile::exists(safeArgument))
        safeArgument = QDir::fromNativeSeparators(safeArgument);
    if (safeArgument.indexOf(QLatin1Char(' ')) > -1)
        safeArgument = QLatin1Char('"') + safeArgument + QLatin1Char('"');
    return safeArgument;
}

void MerSSH::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    QCoreApplication::exit(0);
}

void MerSSH::onProcessStarted()
{
    if (m_interactive) {
        m_stdin = new QFile;
        if (!m_stdin->open(stdin, QIODevice::ReadOnly | QIODevice::Unbuffered)) {
            qDebug("Warning: Cannot read from standard input.");
            return;
        }
        QSocketNotifier * const notifier = new QSocketNotifier(0, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)), SLOT(handleStdin()));
    }
}

void MerSSH::onStandardOutput()
{
    const QByteArray output(m_runner->readAllStandardOutput());
    if (!m_currentCacheFile.isEmpty()) {
        m_currentCacheData.append(output);
    } else {
        fprintf(stdout, "%s", output.constData());
        fflush(stdout);
    }
}

void MerSSH::onStandardError()
{
    printError(QString::fromUtf8(m_runner->readAllStandardError()));
}

void MerSSH::onProcessClosed(int exitStatus)
{
    const bool ok = exitStatus == QSsh::SshRemoteProcess::NormalExit
            && m_runner->processExitCode() == 0;
    if (ok && !m_currentCacheFile.isEmpty()) {
        QFile file(m_currentCacheFile);
        if (!file.open(QIODevice::WriteOnly)) {
            printError(QString::fromLatin1("Cannot write file '%1'").arg(m_currentCacheFile));
            QCoreApplication::exit(1);
        }
        if (m_currentCacheFile.endsWith(QLatin1String(Mer::Constants::QMAKE_QUERY)))
            m_currentCacheData.replace(":/", QString(QLatin1Char(':') + m_merSysRoot
                                                     + QLatin1Char('/')).toUtf8());
        file.write(m_currentCacheData);
        fprintf(stdout, "%s", m_currentCacheData.constData());
        fflush(stdout);
    }
    QCoreApplication::exit(m_runner->processExitCode());
}

void MerSSH::onConnectionError()
{
    printError(QString::fromLatin1("Project ERROR: Could not connect to MerSDK Virtual Machine. %1")
               .arg(m_runner->lastConnectionErrorString()));
    QCoreApplication::exit(1);
}

void MerSSH::handleStdin()
{
    m_runner->writeDataToProcess(m_stdin->readLine());
}

void MerSSH::printError(const QString &message)
{
    if (m_quietOnError)
        return;
    qCritical("%s", qPrintable(message));
}