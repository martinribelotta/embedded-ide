# Diff project documentation

Embedded-IDE base export/import project on diff/patch utils.

## Export

For project export, the IDE call `diff` utility with two directories, the project directory and emptry temporal directory without any contents.

The process is equivalent to:
```bash
diff -Naur $(BASE) $(TEMP_DIR) > my_project.template
```
When `$(BASE)` is directory contains Makefile file and $(TEMP_DIR) is temporal directory created by IDE without contents.

The `-Naur` extention build recursive difference with unified format to produce a patch format output.

Reciselly, the command options to diff are:

```bash
diff -aur --unidirectional-new-file <current dir> <temporal dir>
```

By convention, the Embedded-IDE template extention is `*.template` but anny extention can be used.

## Import

To import project, the inverse process to diff, `patch` is invoked.

The patch utility is invoked with selected *.template contents and -p0 parameter to create entire directory structure.

### Template support

The import process, support a templated-based replace mechanism.

#### Format

Every text with `${{...}}` format is replaced by data edited in "New Project" dialog or this default value before to send to `patch` utility.

Into `${{...}}` brackets, the importer expect this string format:

> ${{field_name type:values}}

  - **field_name** is the name of entry with underscores are replaced by spaces in visualization.
  - **type** is the type of options. The supported options are:
     - `string`: Text entry. The `values` is any character except the final `}}`.
     This field is shown as input-box on "New Project dialog
     - `items`: Multiple items separated with `|` character. The items can contains any characters except `|` and `}}` but is recomended the use of
     `[a-zA-Z_][a-zA-Z0-9_]*`
     convention (common languages indentifier rules)
     This field is shown as combo-box on "New Project" dialog.
