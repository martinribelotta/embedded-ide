[![Build Status](https://travis-ci.org/robertknight/qt-mustache.svg?branch=master)](https://travis-ci.org/robertknight/qt-mustache)

# Qt Mustache

qt-mustache is a simple library for rendering [Mustache templates](http://mustache.github.com/).

### Example Usage

```cpp
#include "mustache.h"

QVariantHash contact;
contact["name"] = "John Smith";
contact["email"] = "john.smith@gmail.com";

QString contactTemplate = "<b>{{name}}</b> <a href=\"mailto:{{email}}\">{{email}}</a>";

Mustache::Renderer renderer;
Mustache::QtVariantContext context(contact);

QTextStream output(stdout);
output << renderer.render(contactTemplate, &context);
```

Outputs: `<b>John Smith</b> <a href="mailto:john.smith@gmail.com">john.smith@gmail.com</a>`

For further examples, see the tests in `test_mustache.cpp`

### Building
 * To build the tests, run `qmake` followed by `make`
 * To use qt-mustache in your project, just add the `mustache.h` and `mustache.cpp` files to your project.
  
### License
 qt-mustache is licensed under the BSD license. 

### Dependencies
 qt-mustache depends on the QtCore library.  It is compatible with Qt 4 and Qt 5.
 
## Usage

### Syntax

qt-mustache uses the standard Mustache syntax.  See the [Mustache manual](http://mustache.github.com/mustache.5.html) for details.

### Data Sources

qt-mustache expands Mustache tags using values from a `Mustache::Context`.  `Mustache::QtVariantContext` is a simple
context implementation which wraps a `QVariantHash` or `QVariantMap`.  If you want to render a template using a custom data source,
you can either create a `QVariantHash` which mirrors the data source or you can re-implement `Mustache::Context`.

### Partials

When a `{{>partial}}` Mustache tag is encountered, qt-mustache will attempt to load the partial using a `Mustache::PartialResolver`
provided by the context.  `Mustache::PartialMap` is a simple resolver which takes a `QHash<QString,QString>` map of partial names
to values and looks up partials in that map.  `Mustache::PartialFileLoader` is another simple resolver which
fetches partials from `<partial name>.mustache` files in a specified directory.

You can re-implement the `Mustache::PartialResolver` interface if you want to load partials from a custom source
(eg. a database).

### Error Handling

If an error occurs when rendering a template, `Mustache::Renderer::errorPosition()` is set to non-negative value and
template rendering stops.  If the error occurs whilst rendering a partial template, `errorPartial()` contains the name
of the partial.

### Lambdas

The [Mustache manual](http://mustache.github.com/mustache.5.html) provides a mechanism to customize rendering of
template sections by setting the value for a tag to a callable object (eg. a lambda in Ruby or Javascript),
which takes the unrendered block of text for a template section and renders it itself.  qt-mustache supports
this via the `Context::canEval()` and `Context::eval()` methods.
