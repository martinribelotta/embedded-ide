#include "ini.h"

#include <QFile>
#include <QStringList>
#include "util.h"
#include "log.h"
#include <assert.h>

Entry::Entry(const Entry &src) :
name(src.name),
value(src.value)
{
}

Ini::Ini()
{
}

Ini::Ini(const Ini &src)
{
	copy(src);
}

Ini::~Ini()
{
	for(int i = 0;i < m_entries.size();i++) {
		Entry *entry = m_entries[i];
		delete entry;
	}

}

Ini& Ini::operator=(const Ini &src)
{
	if(&src == this)
		return *this;

	copy(src);

	return *this;
}

/**
 * @brief Replaces the entries in this ini with another one.
 */
void Ini::copy(const Ini &src)
{
	removeAll();
	for(int i = 0;i < src.m_entries.size();i++) {
		Entry *entry = src.m_entries[i];
		Entry *newEntry = new Entry(*entry);
		m_entries.push_back(newEntry);
	}
}

void Ini::removeAll()
{
	for(int i = 0;i < m_entries.size();i++) {
		Entry *entry = m_entries[i];
		delete entry;
	}

	m_entries.clear();
}

Entry *Ini::findEntry(QString name)
{
	for(int i = 0;i < m_entries.size();i++) {
		Entry *entry = NULL;
		entry = m_entries[i];
		if(entry->name == name)
			return entry;
	}

	return NULL;
}

Entry *Ini::addEntry(QString name)
{
	Entry *entry = findEntry(name);
	if(!entry) {
		entry = new Entry(name);
		m_entries.push_back(entry);
	}

	return entry;
}

void Ini::setInt(QString name, int value)
{
	Entry *entry = addEntry(name);
	entry->value.sprintf("%d", value);
}

void Ini::setBool(QString name, bool value)
{
	Entry *entry = addEntry(name);
	entry->value.sprintf("%d", (int)value);
}

void Ini::setString(QString name, QString value)
{
	Entry *entry = addEntry(name);
	entry->value = value;
}

void Ini::setStringList(QString name, QStringList value)
{
	Entry *entry = addEntry(name);
	entry->value = value.join(";");
}

int Ini::getInt(QString name, int defaultValue)
{
	Entry *entry = findEntry(name);
	if(!entry) {
		setInt(name, defaultValue);
		entry = findEntry(name);
	}

	return entry->value.toInt();
}


bool Ini::getBool(QString name, bool defaultValue)
{
	Entry *entry = findEntry(name);
	if(!entry) {
		setBool(name, defaultValue);
		entry = findEntry(name);
	}

	return entry->value.toInt();
}

QString Ini::getString(QString name, QString defaultValue)
{
	Entry *entry = findEntry(name);
	if(!entry) {
		setString(name, defaultValue);
		entry = findEntry(name);
	}

	return entry->value;
}

QStringList Ini::getStringList(QString name, QStringList defaultValue)
{
	QString list = getString(name, defaultValue.join(";"));

	return list.split(";");
}

QColor Ini::getColor(QString name, QColor defaultValue)
{
	Entry *entry = findEntry(name);
	if(!entry) {
		setColor(name, defaultValue);
		entry = findEntry(name);
	}
	return QColor(entry->value);

}

void Ini::setColor(QString name, QColor value)
{
	Entry *entry = addEntry(name);
	entry->value.sprintf("#%02x%02x%02x", value.red(), value.green(), value.blue());
}

void Ini::dump()
{

	for(int i = 0;i < m_entries.size();i++) {
		Entry *entry = m_entries[i];
		printf("_%s_%s_\n", stringToCStr(entry->name), stringToCStr(entry->value));
	}

}

/**
 * @brief Saves the content to a ini file.
 * @return 0 on success.
 */
int Ini::save(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::Truncate | QIODevice::ReadWrite | QIODevice::Text))
		return 1;
	for(int i = 0;i < m_entries.size();i++) {
		Entry *entry = m_entries[i];
		file.write(stringToCStr(entry->name));
		file.write("=\"");
		file.write(stringToCStr(entry->value));
		file.write("\"\r\n");
	}

	return 0;
}

/**
 * @brief Loads the content of a ini file.
 * @return 0 on success.
 */
int Ini::appendLoad(QString filename)
{
	int lineNo = 1;
	QString str;
	QString name;
	QString value;

	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return 1;

	QString allContent(file.readAll ());

	enum { IDLE, SKIP_LINE, KEY, VALUE, VALUE_STR } state = IDLE;
	for(int i = 0;i < allContent.size();i++) {

		QChar c = allContent[i];

		switch(state) {
			case IDLE:
				if(c == QChar('=')) {
					errorMsg("Empty key at L%d", lineNo);
					state = SKIP_LINE;
				} else if(c == QChar('\n') || c == QChar('\r'))
					lineNo++;
				else if(c.isSpace()) {}
				else if(c == QChar('#'))
					state = SKIP_LINE;
				else {
					str = c;
					state = KEY;
				}
				break;
			case KEY:
				if(c == QChar('\n') || c == QChar('\r')) {
					errorMsg("Parse error at L%d", lineNo);
					lineNo++;
					state = IDLE;
				} else if(c == QChar('=')) {
					name = str;
					value = "";
					state = VALUE;
				} else
					str += c;
				break;
			case VALUE:
				if(c == QChar('\n') || c == QChar('\r')) {
					lineNo++;

					Entry *entry = addEntry(name.trimmed());
					entry->value = value.trimmed();

					state = IDLE;
				} else {
					if(value.isEmpty()) {
						if(c == '"')
							state = VALUE_STR;
						else if(!c.isSpace())
							value += c;
					} else
						value += c;
				}
				break;
			case VALUE_STR:
				if(c == QChar('"')) {
					lineNo++;

					Entry *entry = addEntry(name.trimmed());
					entry->value = value.trimmed();

					state = IDLE;
				} else
					value += c;
				break;
			case SKIP_LINE:
				if(c == QChar('\n') || c == QChar('\r')) {
					lineNo++;
					state = IDLE;
				}
				break;
		}
	}

	return 0;
}




