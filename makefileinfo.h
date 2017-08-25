#ifndef MAKEFILEINFO_H
#define MAKEFILEINFO_H

#include <QMetaType>
#include <QStringList>
#include <QHash>

#include "etags.h"

struct MakefileInfo {
    ETags tags;
    QString workingDir;
    QHash<QString, QString> allTargets;
    QStringList targets;
};

Q_DECLARE_METATYPE(MakefileInfo)

#endif // MAKEFILEINFO_H

