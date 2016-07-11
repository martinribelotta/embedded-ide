#include "log.h"
#include <QDebug>
#include <QTime>

void debugMsg_(const char *filename, int lineNo, const char *fmt, ...)
{
	va_list ap;
	char buffer[1024];
	QTime curTime = QTime::currentTime();

	va_start(ap, fmt);

	vsnprintf(buffer, sizeof(buffer), fmt, ap);

	va_end(ap);

	printf("%2d.%03d| DEBUG | %s:%d| %s\n",
			curTime.second()%100, curTime.msec(),
			filename, lineNo, buffer);
}


void errorMsg(const char *fmt, ...)
{
	va_list ap;
	char buffer[1024];
	QTime curTime = QTime::currentTime();

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);

	va_end(ap);

	printf("%2d.%03d| ERROR | %s\n",
			curTime.second()%100, curTime.msec(),
			buffer);
}

void infoMsg(const char *fmt, ...)
{
	va_list ap;
	char buffer[1024];
	QTime curTime = QTime::currentTime();

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);

	va_end(ap);

	printf("%2d.%03d| INFO  | %s\n",
			curTime.second()%100, curTime.msec(),
			buffer);
}
