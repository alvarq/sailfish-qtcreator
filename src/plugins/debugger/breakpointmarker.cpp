/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#include "breakpointmarker.h"
#include "breakhandler.h"
#include "debuggercore.h"

#include <QIcon>

//////////////////////////////////////////////////////////////////
//
// BreakpointMarker
//
//////////////////////////////////////////////////////////////////


namespace Debugger {
namespace Internal {

BreakpointMarker::BreakpointMarker(BreakpointModelId id,
        const QString &fileName, int lineNumber)
    : BaseTextMark(fileName, lineNumber), m_id(id)
{
    setIcon(breakHandler()->icon(m_id));
    setPriority(TextEditor::ITextMark::NormalPriority);
    //qDebug() << "CREATE MARKER " << fileName << lineNumber;
}

BreakpointMarker::~BreakpointMarker()
{
    //qDebug() << "REMOVE MARKER ";
}

void BreakpointMarker::removedFromEditor()
{
    breakHandler()->removeBreakpoint(m_id);
}

void BreakpointMarker::updateLineNumber(int lineNumber)
{
    BaseTextMark::updateLineNumber(lineNumber);
    breakHandler()->updateLineNumberFromMarker(m_id, lineNumber);
}

void BreakpointMarker::dragToLine(int lineNumber)
{
    breakHandler()->changeLineNumberFromMarker(m_id, lineNumber);
}

void BreakpointMarker::updateFileName(const QString &fileName)
{
    BaseTextMark::updateFileName(fileName);
    breakHandler()->updateFileNameFromMarker(m_id, fileName);
}

} // namespace Internal
} // namespace Debugger

