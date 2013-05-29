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

#ifndef MERBUILDCONFIGURATIONPROJECTPATHENVIRONMENTVARIABLESETTER_H
#define MERBUILDCONFIGURATIONPROJECTPATHENVIRONMENTVARIABLESETTER_H

#include "merprojectlistener.h"

namespace Mer {
namespace Internal {

class MerBuildConfigurationProjectPathEnvironmentVariableSetter : public MerProjectListener
{
    Q_OBJECT

private slots:
    void onTargetAddedToProject(ProjectExplorer::Target *target);
    void onProjectAdded(ProjectExplorer::Project *project);

private:
    bool handleProject(Qt4ProjectManager::Qt4Project *qt4Project) { Q_UNUSED(qt4Project); return true; }
    bool forgetProject(ProjectExplorer::Project *project) { Q_UNUSED(project); return true; }
    static void addProjectPathEnvironmentVariableToMerBuildConfigurationsOfTarget(ProjectExplorer::Target *target);
};

} // Internal
} // Mer

#endif // MERBUILDCONFIGURATIONPROJECTPATHENVIRONMENTVARIABLESETTER_H
