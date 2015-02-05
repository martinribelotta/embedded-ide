/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** No Commercial Usage
**
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#include <qate/highlightdefinitionmanager.h>
#include "highlightdefinition.h"
#include <qate/highlightdefinitionhandler.h>
#include "highlighterexception.h"
#include "definitiondownloader.h"
#include "highlightersettings.h"
//#include "plaintexteditorfactory.h"
//#include "texteditorconstants.h"
//#include "texteditorplugin.h"
//#include "texteditorsettings.h"

//#include <coreplugin/icore.h>
//#include <utils/qtcassert.h>
//#include <coreplugin/progressmanager/progressmanager.h>
//#include <qtconcurrent/QtConcurrentTools>
#include "qtconcurrent/QtConcurrentTools"

#include <QtAlgorithms>
#include <QtPlugin>
#include <QString>
#include <QLatin1Char>
#include <QLatin1String>
#include <QStringList>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegExp>
#include <QFuture>
#include <QtConcurrentRun>
#include <QtConcurrentMap>
#include <QUrl>
#include <QSet>
//#include <DesktopServices>
#include <QMessageBox>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtAlgorithms>

using namespace TextEditor;
using namespace Internal;
using namespace Qate;

HighlightDefinitionManager::HighlightDefinitionManager() :
    m_downloadingDefinitions(false),
    m_registeringMimeTypes(false),
    m_queuedMimeTypeRegistrations(0)
{
    connect(&m_mimeTypeWatcher, SIGNAL(resultReadyAt(int)), this, SLOT(registerMimeType(int)));
    connect(&m_mimeTypeWatcher, SIGNAL(finished()), this, SLOT(registerMimeTypesFinished()));
    connect(&m_downloadWatcher, SIGNAL(finished()), this, SLOT(downloadDefinitionsFinished()));
}

HighlightDefinitionManager::~HighlightDefinitionManager()
{}

HighlightDefinitionManager *HighlightDefinitionManager::instance()
{
    static HighlightDefinitionManager manager;
    return &manager;
}

QString HighlightDefinitionManager::definitionIdByName(const QString &name) const
{
	return m_idByName.contains(name) ? m_idByName.value(name) : QString();
}

QString HighlightDefinitionManager::definitionIdByMimeType(const QString &mimeType) const
{
	return m_idByMimeType.contains(mimeType) ? m_idByMimeType.value(mimeType) : QString();
}

QString HighlightDefinitionManager::definitionIdByAnyMimeType(const QStringList &mimeTypes) const
{
    QString definitionId;
    foreach (const QString &mimeType, mimeTypes) {
        definitionId = definitionIdByMimeType(mimeType);
        if (!definitionId.isEmpty())
            break;
    }
    return definitionId;
}

QStringList HighlightDefinitionManager::definitionsPaths() const
{
	QStringList l;
	// TODO - user proper directories:
	// - use the platform directories for $HOME
	// - read the KDE home dir from the env
	// - add API for application developers to add a new directory
#ifdef WIN32
        l.append("C:\\QtSDK\\QtCreator\\share\\qtcreator\\generic-highlighter\\");
        l.append(QDir::homePath() + "\\AppData\\Roaming\\Nokia\\qtcreator\\generic-highlighter\\");
#else
	// ubuntu/debian
        l.append("/usr/share/kde4/apps/katepart/syntax/");
	// arch
	l.append("/usr/share/apps/katepart/syntax/");
	// user instaled dirs
        l.append("~/.kde/share/apps/katepart/syntax/");
        l.append("~/.kde4/share/apps/katepart/syntax/");
#endif
	return l;
}

QSharedPointer<HighlightDefinition> HighlightDefinitionManager::definition(const QString &id)
{
    if (!id.isEmpty() && !m_definitions.contains(id)) {
        QFile definitionFile(id);
        if (!definitionFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return QSharedPointer<HighlightDefinition>();

        QSharedPointer<HighlightDefinition> definition(new HighlightDefinition);
        Qate::HighlightDefinitionHandler handler(definition);

        QXmlInputSource source(&definitionFile);
        QXmlSimpleReader reader;
        reader.setContentHandler(&handler);
        m_isBuilding.insert(id);
        try {
            reader.parse(source);
        } catch (HighlighterException &) {
            definition.clear();
        }
        m_isBuilding.remove(id);
        definitionFile.close();

        m_definitions.insert(id, definition);
    }

    return m_definitions.value(id);
}

QSharedPointer<HighlightDefinitionMetaData> HighlightDefinitionManager::definitionMetaData(const QString &id) const
{ return m_definitionsMetaData.value(id); }

bool HighlightDefinitionManager::isBuildingDefinition(const QString &id) const
{
	return m_isBuilding.contains(id);
}

void HighlightDefinitionManager::registerMimeTypes()
{
    if (!m_registeringMimeTypes) {
        m_registeringMimeTypes = true;
        clear();
        QFuture<Qate::MimeType> future =
            QtConcurrent::run(&HighlightDefinitionManager::gatherDefinitionsMimeTypes, this);
        m_mimeTypeWatcher.setFuture(future);
        /*Core::ICore::instance()->progressManager()->addTask(future,
                                                            tr("Registering definitions"),
                                                            Constants::TASK_REGISTER_DEFINITIONS);*/
    } else {
        // QFutures returned from QConcurrent::run cannot be cancelled. So the queue.
        ++m_queuedMimeTypeRegistrations;
    }
}

void HighlightDefinitionManager::gatherDefinitionsMimeTypes(QFutureInterface<Qate::MimeType> &future)
{
    // Please be aware of the following limitation in the current implementation.
    // The generic highlighter only register its types after all other plugins
    // have populated Creator's MIME database (so it does not override anything).
    // When the generic highlighter settings change only its internal data is cleaned-up
    // and rebuilt. Creator's MIME database is not touched. So depending on how the
    // user plays around with the generic highlighter file definitions (changing
    // duplicated patterns, for example), some changes might not be reflected.
    // A definitive implementation would require some kind of re-load or update
    // (considering hierarchies, aliases, etc) of the MIME database whenever there
    // is a change in the generic highlighter settings.

    QStringList definitionsPaths = this->definitionsPaths();
    /*const HighlighterSettings &settings = TextEditorSettings::instance()->highlighterSettings();
    definitionsPaths.append(settings.definitionFilesPath());
    if (settings.useFallbackLocation())
        definitionsPaths.append(settings.fallbackDefinitionFilesPath());*/

//    Core::MimeDatabase *mimeDatabase = Core::ICore::instance()->mimeDatabase();
    Qate::MimeDatabase *mimeDatabase = this->mimeDatabase();
    QSet<QString> knownSuffixes = QSet<QString>::fromList(mimeDatabase->suffixes());

    QHash<QString, Qate::MimeType> userModified;
    const QList<Qate::MimeType> &userMimeTypes = mimeDatabase->readUserModifiedMimeTypes();
    foreach (const Qate::MimeType &userMimeType, userMimeTypes)
        userModified.insert(userMimeType.type(), userMimeType);

    foreach (const QString &path, definitionsPaths) {
        if (path.isEmpty())
            continue;

        QDir definitionsDir(path);
        QStringList filter(QLatin1String("*.xml"));
        definitionsDir.setNameFilters(filter);

        QList<QSharedPointer<HighlightDefinitionMetaData> > allMetaData;
        const QFileInfoList &filesInfo = definitionsDir.entryInfoList();
        foreach (const QFileInfo &fileInfo, filesInfo) {
            const QSharedPointer<HighlightDefinitionMetaData> &metaData = parseMetadata(fileInfo);
            if (!metaData.isNull())
                allMetaData.append(metaData);
        }

        // Consider definitions with higher priority first.
        qSort(allMetaData.begin(), allMetaData.end(), PriorityComp());

        foreach (const QSharedPointer<HighlightDefinitionMetaData> &metaData, allMetaData) {
            if (m_idByName.contains(metaData->name()))
                // Name already exists... This is a fallback item, do not consider it.
                continue;

            const QString &id = metaData->id();
            m_idByName.insert(metaData->name(), id);
            m_definitionsMetaData.insert(id, metaData);

            static const QStringList textPlain(QLatin1String("text/plain"));

            // A definition can specify multiple MIME types and file extensions/patterns.
            // However, each thing is done with a single string. There is no direct way to
            // tell which patterns belong to which MIME types nor whether a MIME type is just
            // an alias for the other. Currently, I associate all patterns with all MIME
            // types from a definition.
            QList<Qate::MimeGlobPattern> globPatterns;
            foreach (const QString &type, metaData->mimeTypes()) {
                if (m_idByMimeType.contains(type))
                    continue;

                m_idByMimeType.insert(type, id);
                Qate::MimeType mimeType = mimeDatabase->findByType(type);
                if (mimeType.isNull()) {
                    mimeType.setType(type);
                    mimeType.setSubClassesOf(textPlain);
                    mimeType.setComment(metaData->name());

                    // If there's a user modification for this mime type, we want to use the
                    // modified patterns and rule-based matchers. If not, just consider what
                    // is specified in the definition file.
                    QHash<QString, Qate::MimeType>::const_iterator it =
                        userModified.find(mimeType.type());
                    if (it == userModified.end()) {
                        if (globPatterns.isEmpty()) {
                            foreach (const QString &pattern, metaData->patterns()) {
                                static const QLatin1String mark("*.");
                                if (pattern.startsWith(mark)) {
                                    const QString &suffix = pattern.right(pattern.length() - 2);
                                    if (!knownSuffixes.contains(suffix))
                                        knownSuffixes.insert(suffix);
                                    else
                                        continue;
                                }
                                QRegExp regExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
                                globPatterns.append(Qate::MimeGlobPattern(regExp, 50));
                            }
                        }
                        mimeType.setGlobPatterns(globPatterns);
                    } else {
                        mimeType.setGlobPatterns(it.value().globPatterns());
                        mimeType.setMagicRuleMatchers(it.value().magicRuleMatchers());
                    }

                    mimeDatabase->addMimeType(mimeType);
                    future.reportResult(mimeType);
                }
            }
        }
    }
}

void HighlightDefinitionManager::registerMimeType(int index) const
{
    const Qate::MimeType &mimeType = m_mimeTypeWatcher.resultAt(index);
//    TextEditorPlugin::instance()->editorFactory()->addMimeType(mimeType.type());
//	TODO
//    mimeDatabase()->addMimeType(mimeType.type());
}

void HighlightDefinitionManager::registerMimeTypesFinished()
{
    m_registeringMimeTypes = false;
    if (m_queuedMimeTypeRegistrations > 0) {
        --m_queuedMimeTypeRegistrations;
        registerMimeTypes();
    } else {
        emit mimeTypesRegistered();
    }
}

QSharedPointer<HighlightDefinitionMetaData> HighlightDefinitionManager::parseMetadata(const QFileInfo &fileInfo)
{
    static const QLatin1Char kSemiColon(';');
    static const QLatin1Char kSpace(' ');
    static const QLatin1Char kDash('-');
    static const QLatin1String kLanguage("language");
    static const QLatin1String kArtificial("text/x-artificial-");

    QFile definitionFile(fileInfo.absoluteFilePath());
    if (!definitionFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return QSharedPointer<HighlightDefinitionMetaData>();

    QSharedPointer<HighlightDefinitionMetaData> metaData(new HighlightDefinitionMetaData);

    QXmlStreamReader reader(&definitionFile);
    while (!reader.atEnd() && !reader.hasError()) {
        if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == kLanguage) {
            const QXmlStreamAttributes &atts = reader.attributes();

            metaData->setFileName(fileInfo.fileName());
            metaData->setId(fileInfo.absoluteFilePath());
            metaData->setName(atts.value(HighlightDefinitionMetaData::kName).toString());
            metaData->setVersion(atts.value(HighlightDefinitionMetaData::kVersion).toString());
            metaData->setPriority(atts.value(HighlightDefinitionMetaData::kPriority).toString()
                                  .toInt());
            metaData->setPatterns(atts.value(HighlightDefinitionMetaData::kExtensions)
                                  .toString().split(kSemiColon, QString::SkipEmptyParts));

            QStringList mimeTypes = atts.value(HighlightDefinitionMetaData::kMimeType).
                                    toString().split(kSemiColon, QString::SkipEmptyParts);
            if (mimeTypes.isEmpty()) {
                // There are definitions which do not specify a MIME type, but specify file
                // patterns. Creating an artificial MIME type is a workaround.
                QString artificialType(kArtificial);
                artificialType.append(metaData->name().trimmed().replace(kSpace, kDash));
                mimeTypes.append(artificialType);
            }
            metaData->setMimeTypes(mimeTypes);

            break;
        }
    }
    reader.clear();
    definitionFile.close();

    return metaData;
}

QList<HighlightDefinitionMetaData> HighlightDefinitionManager::parseAvailableDefinitionsList(QIODevice *device) const
{
    static const QLatin1Char kSlash('/');
    static const QLatin1String kDefinition("Definition");

    QList<HighlightDefinitionMetaData> metaDataList;
    QXmlStreamReader reader(device);
    while (!reader.atEnd() && !reader.hasError()) {
        if (reader.readNext() == QXmlStreamReader::StartElement &&
            reader.name() == kDefinition) {
            const QXmlStreamAttributes &atts = reader.attributes();

            HighlightDefinitionMetaData metaData;
            metaData.setName(atts.value(HighlightDefinitionMetaData::kName).toString());
            metaData.setVersion(atts.value(HighlightDefinitionMetaData::kVersion).toString());
            QString url(atts.value(HighlightDefinitionMetaData::kUrl).toString());
            metaData.setUrl(QUrl(url));
            const int slash = url.lastIndexOf(kSlash);
            if (slash != -1)
                metaData.setFileName(url.right(url.length() - slash - 1));

            metaDataList.append(metaData);
        }
    }
    reader.clear();

    return metaDataList;
}

void HighlightDefinitionManager::downloadAvailableDefinitionsMetaData()
{
    QUrl url(QLatin1String("http://www.kate-editor.org/syntax/update-3.2.xml"));
    QNetworkRequest request(url);
    // Currently this takes a couple of seconds on Windows 7: QTBUG-10106.
    QNetworkReply *reply = m_networkManager.get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(downloadAvailableDefinitionsListFinished()));
}

void HighlightDefinitionManager::downloadAvailableDefinitionsListFinished()
{
    if (QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender())) {
        if (reply->error() == QNetworkReply::NoError)
            emit definitionsMetaDataReady(parseAvailableDefinitionsList(reply));
        else
            emit errorDownloadingDefinitionsMetaData();
        reply->deleteLater();
    }
}

void HighlightDefinitionManager::downloadDefinitions(const QList<QUrl> &urls, const QString &savePath)
{
    m_downloaders.clear();
    foreach (const QUrl &url, urls)
        m_downloaders.append(new DefinitionDownloader(url, savePath));

    m_downloadingDefinitions = true;
    QFuture<void> future = QtConcurrent::map(m_downloaders, DownloaderStarter());
    m_downloadWatcher.setFuture(future);
    /*Core::ICore::instance()->progressManager()->addTask(future,
                                                        tr("Downloading definitions"),
                                                        Constants::TASK_DOWNLOAD_DEFINITIONS);*/
}

void HighlightDefinitionManager::downloadDefinitionsFinished()
{
    int errors = 0;
    bool writeError = false;
    foreach (DefinitionDownloader *downloader, m_downloaders) {
        DefinitionDownloader::Status status = downloader->status();
        if (status != DefinitionDownloader::Ok) {
            ++errors;
            if (status == DefinitionDownloader::WriteError && !writeError)
                writeError = true;
        }
        delete downloader;
    }

    if (errors > 0) {
        QString text;
        if (errors == m_downloaders.size())
            text = tr("Error downloading selected definition(s).");
        else
            text = tr("Error downloading one or more definitions.");
        if (writeError)
            text.append(tr("\nPlease check the directory's access rights."));
        QMessageBox::critical(0, tr("Download Error"), text);
    }

    m_downloadingDefinitions = false;
}

bool HighlightDefinitionManager::isDownloadingDefinitions() const
{
    return m_downloadingDefinitions;
}

Qate::MimeDatabase* HighlightDefinitionManager::mimeDatabase()
{
	return m_mimeDatabase;
}

void HighlightDefinitionManager::setMimeDatabase(Qate::MimeDatabase* db)
{
	m_mimeDatabase = db;
}

void HighlightDefinitionManager::clear()
{
    m_idByName.clear();
    m_idByMimeType.clear();
    m_definitions.clear();
    m_definitionsMetaData.clear();
}
