/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef ADDKITOPERATION_H
#define ADDKITOPERATION_H

#include "operation.h"

#include <QString>

class AddKitOperation : public Operation
{
public:
    AddKitOperation();

    QString name() const;
    QString helpText() const;
    QString argumentsHelpText() const;

    bool setArguments(const QStringList &args);

    int execute() const;

#ifdef WITH_TESTS
    bool test() const;
#endif

    static QVariantMap addKit(const QVariantMap &map, const QString &id, const QString &displayName,
                              const QString &icon, const QString &debuggerId,
                              const quint32 &debuggerType, const QString &debugger,
                              const QByteArray &deviceType, const QString &device,
                              const QString &sysRoot, const QString &tc, const QString &qt,
                              const QString &mkspec, const KeyValuePairList &extra);

    static QVariantMap initializeKits();

    // internal:
    static QVariantMap addKit(const QVariantMap &map, const QVariantMap &tcMap,
                              const QVariantMap &qtMap, const QVariantMap &devMap,
                              const QString &id, const QString &displayName,
                              const QString &icon, const QString &debuggerId,
                              const quint32 &debuggerType, const QString &debugger,
                              const QByteArray &deviceType, const QString &device,
                              const QString &sysRoot, const QString &tc, const QString &qt,
                              const QString &mkspec, const KeyValuePairList &extra);

private:
    QString m_id;
    QString m_displayName;
    QString m_icon;
    QString m_debuggerId;
    quint32 m_debuggerEngine;
    QString m_debugger;
    QString m_deviceType;
    QString m_device;
    QString m_sysRoot;
    QString m_tc;
    QString m_qt;
    QString m_mkspec;
    KeyValuePairList m_extra;
};

#endif // ADDKITOPERATION_H
