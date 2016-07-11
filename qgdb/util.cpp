#include "util.h"

#include <assert.h>
#include <QString>
#include <stdio.h>


/**
 * @brief Divides a path into a filename and a path.
 *
 * Example: dividePath("/dir/filename.ext") => "/dir", "filename.ext".
 */
void dividePath(QString fullPath, QString *filename, QString *folderPath)
{
	int divPos = fullPath.lastIndexOf('/');
	if(divPos> 0) {
		if(filename)
			*filename = fullPath.mid(divPos+1);
		if(folderPath)
			*folderPath = fullPath.left(divPos);
	} else {
		if(filename)
			*filename = fullPath;
	}
}

/**
 * @brief Returns the filename of a path.
 *
 * Example: getFilenamePart("/dir/filename.ext") => "filename.ext".
 */
QString getFilenamePart(QString fullPath)
{
	QString filename;
	dividePath(fullPath, &filename, NULL);

	return filename;
}


long long stringToLongLong(const char* str)
{
	unsigned long long num = 0;
	QString str2 = str;
	str2.replace('_', "");
	num = str2.toLongLong();

	return num;
}

QString longLongToHexString(long long num)
{
	QString newStr;
	QString str;
	str.sprintf("%llx", num);
	if(num != 0) {
		while(str.length()%4 != 0)
			str = "0" + str;

		for(int i = str.length()-1;i >= 0;i--) {
			newStr += str[str.length()-i-1];
			if(i%4 == 0 && i != 0)
				newStr += "_";
		}
		str = newStr;
	}

	return "0x" + str;
}

#ifdef NEVER
void testFuncs()
{
	printf("%12x -> '%s'\n", 0, stringToCStr(longLongToHexString(0)));
	printf("%12x -> '%s'\n", 0x1, stringToCStr(longLongToHexString(0x1)));
	printf("%12x -> '%s'\n", 0x123, stringToCStr(longLongToHexString(0x123)));
	printf("%12x -> '%s'\n", 0x1234, stringToCStr(longLongToHexString(0x1234)));
	printf("%12x -> '%s'\n", 0x1234567, stringToCStr(longLongToHexString(0x1234567)));
	printf("%12llx -> '%s'\n", 0x12345678ULL, stringToCStr(longLongToHexString(0x12345678ULL)));
	printf("%12llx -> '%s'\n", 0x123456789abcULL, stringToCStr(longLongToHexString(0x123456789abcULL)));
}
#endif
