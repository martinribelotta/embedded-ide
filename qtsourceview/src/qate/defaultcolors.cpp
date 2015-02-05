/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#include "qate/defaultcolors.h"

#include <QtCore/Qt>
#include "highlighter.h"

using namespace Qate;

DefaultColors::DefaultColors()
{
	m_keywordFormat.setForeground(Qt::black);
	m_keywordFormat.setFontWeight(75);
	
	m_dataTypeFormat.setForeground(Qt::blue);
	
	m_decimalFormat.setForeground(Qt::darkYellow);

	m_baseNFormat.setForeground(Qt::darkYellow);
	
	m_floatFormat.setForeground(Qt::darkYellow);
	
	m_charFormat.setForeground(Qt::red);
	
	m_stringFormat.setForeground(Qt::red);
	
	m_commentFormat.setForeground(QColor(0x60,0x60,0x60));
	
	m_alertFormat.setForeground(Qt::red);
	m_alertFormat.setFontUnderline(true);
	m_alertFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
	
	m_errorFormat.setForeground(Qt::red);
	m_errorFormat.setFontUnderline(true);
	m_errorFormat.setUnderlineStyle(QTextCharFormat::DashUnderline);
	
	m_functionFormat.setForeground(Qt::cyan);
	
	m_regionMarkerFormat.setForeground(Qt::green);
	
        m_othersFormat.setForeground(Qt::darkBlue);
}

DefaultColors &DefaultColors::instance()
{
    static DefaultColors DefaultColors;
    return DefaultColors;
}

QString DefaultColors::name(const QTextCharFormat &format) const
{
    if (format == QTextCharFormat())
        return "Default format";
    else if (format == m_keywordFormat)
        return "Keyword";
    else if (format == m_dataTypeFormat)
        return "Data type format";
    else if (format == m_decimalFormat)
        return "Decimal format";
    else if (format == m_baseNFormat)
        return "Base N format";
    else if (format == m_floatFormat)
        return "Float format";
    else if (format == m_charFormat)
        return "Char format";
    else if (format == m_stringFormat)
        return "String format";
    else if (format == m_commentFormat)
        return "Comment format";
    else if (format == m_alertFormat)
        return "Alert format";
    else if (format == m_errorFormat)
        return "Error format";
    else if (format == m_functionFormat)
        return "Function format";
    else if (format == m_regionMarkerFormat)
        return "Region Marker format";
    else if (format == m_othersFormat)
        return "Others format";
    else
        return "Unidentified format";
}

using namespace TextEditor::Internal;
void DefaultColors::ApplyToHighlighter(TextEditor::Internal::Highlighter *hl)
{
#if 0
//	hl->configureFormat(Highlighter::Normal,           instance().m_othersFormat       );
//	hl->configureFormat(Highlighter::VisualWhitespace, instance().m_othersFormat       );
	hl->configureFormat(Highlighter::Keyword,          instance().m_keywordFormat      );
	hl->configureFormat(Highlighter::DataType,         instance().m_dataTypeFormat     );
	hl->configureFormat(Highlighter::Decimal,          instance().m_decimalFormat      );
	hl->configureFormat(Highlighter::BaseN,            instance().m_baseNFormat        );
	hl->configureFormat(Highlighter::Float,            instance().m_floatFormat        );
	hl->configureFormat(Highlighter::Char,             instance().m_charFormat         );
	hl->configureFormat(Highlighter::String,           instance().m_stringFormat       );
	hl->configureFormat(Highlighter::Comment,          instance().m_commentFormat      );
	hl->configureFormat(Highlighter::Alert,            instance().m_alertFormat        );
	hl->configureFormat(Highlighter::Error,            instance().m_errorFormat        );
	hl->configureFormat(Highlighter::Function,         instance().m_functionFormat     );
	hl->configureFormat(Highlighter::RegionMarker,     instance().m_regionMarkerFormat );
	hl->configureFormat(Highlighter::Others,           instance().m_othersFormat       );
#endif
}

