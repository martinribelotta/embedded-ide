#ifndef FILE__LOG_H
#define FILE__LOG_H

#include <QDebug>

//#define ENABLE_DEBUGMSG
#ifndef ENABLE_DEBUGMSG
#define debugMsg(fmt...)  do{}while(0)
#else
void debugMsg_(const char *file, int lineNo, const char *fmt,...);
#define debugMsg(fmt...)  debugMsg_(__FILE__, __LINE__, fmt)
#endif

void errorMsg(const char *fmt,...);
void infoMsg(const char *fmt,...);

#endif // FILE__LOG_H


