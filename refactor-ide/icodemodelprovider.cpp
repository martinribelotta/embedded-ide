#include "icodemodelprovider.h"

#include <QApplication>
#include <QFileInfo>
#include <QMimeDatabase>


CodeModelFactory::CodeModelFactory() : QObject(QApplication::instance())
{

}

CodeModelFactory::~CodeModelFactory()
{

}

CodeModelFactory &CodeModelFactory::instance()
{
    static CodeModelFactory *ptr = nullptr;
    if (!ptr)
        ptr = new CodeModelFactory;
    return *ptr;
}

ICodeModelProvider *CodeModelFactory::modelForSuffix(const QString &suffix)
{
    for(const auto& iface: providerList)
        if (iface->canHandleSuffix(suffix))
            return iface;
    return nullptr;
}

ICodeModelProvider *CodeModelFactory::modelForMime(const QMimeType &type)
{
    for(const auto& iface: providerList)
        if (iface->canHandleMimeType(type))
            return iface;
    return nullptr;
}

ICodeModelProvider *CodeModelFactory::modelForFile(const QString &filePath)
{
#define RETURN_IF_NNULL(expr) do { auto p = expr; if (p) return p; } while(0)
    RETURN_IF_NNULL(modelForSuffix(QFileInfo(filePath).suffix()));
    RETURN_IF_NNULL(modelForMime(QMimeDatabase().mimeTypeForFile(filePath)));
#undef RETURN_IF_NNULL
    return nullptr;
}
