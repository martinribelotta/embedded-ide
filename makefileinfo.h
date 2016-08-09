#ifndef MAKEFILEINFO_H
#define MAKEFILEINFO_H

#include <QMetaType>
#include <QStringList>

struct MakefileInfo {
    QStringList targets;
    QStringList include;
    QStringList defines;
    QString workingDir;
    QString cc_cflags;
    QString cflags;
};

Q_DECLARE_METATYPE(MakefileInfo)

#endif // MAKEFILEINFO_H

