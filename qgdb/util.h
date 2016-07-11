#ifndef FILE__UTIL_H
#define FILE__UTIL_H

#include <QString>

#define MIN(a,b) ((a)<(b))
#define MAX(a,b) ((a)>(b))

//#define stringToCStr(str) str.toAscii().constData()
#define stringToCStr(str) qPrintable(str)


QString getFilenamePart(QString fullPath);
void dividePath(QString fullPath, QString *filename, QString *folderPath);

long long stringToLongLong(const char* str);
QString longLongToHexString(long long num);


#endif // FILE__UTIL_H

