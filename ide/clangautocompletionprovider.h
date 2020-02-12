/*
 * This file is part of Embedded-IDE
 * 
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CLANGAUTOCOMPLETIONPROVIDER_H
#define CLANGAUTOCOMPLETIONPROVIDER_H

#include <QObject>
#include <icodemodelprovider.h>

#include <memory>

class ProjectManager;

class ClangAutocompletionProvider: public QObject, public ICodeModelProvider
{
    Q_OBJECT
public:
    explicit ClangAutocompletionProvider(ProjectManager *proj, QObject *parent);
    virtual ~ClangAutocompletionProvider() override;

    void startIndexingProject(const QString& path, FinishIndexProjectCallback_t cb) override;
    void startIndexingFile(const QString& path, FinishIndexFileCallback_t cb) override;

    void referenceOf(const QString& entity, FindReferenceCallback_t cb) override;
    void completionAt(const FileReference& ref, const QString& unsaved, CompletionCallback_t cb) override;
    void requestSymbolForFile(const QString& path, SymbolRequestCallback_t cb) override;

private:
    class Priv_t;
    std::unique_ptr<Priv_t> priv;
};

#endif // CLANGAUTOCOMPLETIONPROVIDER_H
