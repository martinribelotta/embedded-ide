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
    QStringList include;
    QStringList defines;
    QString cc_cflags;
    QString cflags;
};

Q_DECLARE_METATYPE(MakefileInfo)

#endif // MAKEFILEINFO_H

