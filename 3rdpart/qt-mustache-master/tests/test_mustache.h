/*
  Copyright 2012, Robert Knight

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
*/

#pragma once

#include "mustache.h"

#include <QtTest/QtTest>

class TestMustache : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void testContextLookup();
	void testErrors();
	void testPartialFile();
	void testPartials();
	void testSections();
	void testFalsiness();
	void testSetDelimiters();
	void testValues();
	void testEscaping();
	void testEval();
	void testHelpers();
	void testIncompleteTag();
	void testIncompleteSection();
	void testLambda();
	void testQStringListIteration();
	void testUnescapeHtml();
#if QT_VERSION >= 0x050000
	void testConformance();
	void testConformance_data();
#endif // QT_VERSION >= 0x050000
};

