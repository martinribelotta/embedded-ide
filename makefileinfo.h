#ifndef MAKEFILEINFO_H
#define MAKEFILEINFO_H

#include <QtGlobal>
#include <QStringList>

struct MakefileInfo {
    QStringList targets;
    QStringList include;
    QStringList defines;
};

Q_DECLARE_METATYPE(MakefileInfo)

#endif // MAKEFILEINFO_H

