/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSHADERGRAPHLOADER_P_H
#define QSHADERGRAPHLOADER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/private/qtguiglobal_p.h>

#include <QtGui/private/qshadergraph_p.h>

QT_BEGIN_NAMESPACE

class QIODevice;

class QShaderGraphLoader
{
public:
    enum Status : char {
        Null,
        Waiting,
        Ready,
        Error
    };

    Q_GUI_EXPORT QShaderGraphLoader() noexcept;

    Q_GUI_EXPORT Status status() const noexcept;
    Q_GUI_EXPORT QShaderGraph graph() const noexcept;

    Q_GUI_EXPORT QIODevice *device() const noexcept;
    Q_GUI_EXPORT void setDevice(QIODevice *device) noexcept;

    Q_GUI_EXPORT QHash<QString, QShaderNode> prototypes() const noexcept;
    Q_GUI_EXPORT void setPrototypes(const QHash<QString, QShaderNode> &prototypes) noexcept;

    Q_GUI_EXPORT void load();

private:
    Status m_status;
    QIODevice *m_device;
    QHash<QString, QShaderNode> m_prototypes;
    QShaderGraph m_graph;
};

Q_DECLARE_TYPEINFO(QShaderGraphLoader, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QShaderGraphLoader)
Q_DECLARE_METATYPE(QShaderGraphLoader::Status)

#endif // QSHADERGRAPHLOADER_P_H
