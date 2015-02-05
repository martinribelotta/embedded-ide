


demo - no syntax-highlighter
-----------------------------

This is a small text editor, single file, single window. Very keyboard
oriented, which puts the document you are editing in the front. This is a small
replacement for Notepad.exe, or Notepad on steroids. This application
just shows how to use the text editor widget, which does not include a 
syntax highlighter.

[NIY] = not implemented yet

Shortcuts:
 - control + u
   * change current word or selection to upper case
 - control + shift + u
   * change current word or selection to lower case case
 - control + b
   * toggle bookmark
 - control + page up
   * goto previous bookmark (usually up, but also wraps around to the end)
 - control + page down
   * goto next bookmark (usually down, but also wraps around to the start)
 - control + {,},6 (any one of those keys)
   * navigate to the matching braket
 - control + f
   Find text. While in find mode:
      * search is incrementally
      * background will change if text is not found
      * enter will search again
      * enter + (alt,control,shift) - will search again backwards
      * esc, control+f will close the find control
 - control + O
   * open a new file
 - control + r
   Replace text. While in replace more:
      * the text to search, will change background telling you if the text is found
      * cursor position is not changed
      * esc, control+r will close the replace control
 - control + down,up
   * Scroll document without moving the cursor key

Extra features:
 - Smart home/end
   * Pressing home will not bring to to the start to the line, but 
     to the first non blank char in the line
   * Pressing end will not bring to to the end to the line, but 
     to the last non blank char in the line
   * If you are in the smart home/end position and you press the corresponding
     key again, you get to the real home/end position.
 - If a line is modified since the editor is opened, the line number will be colored
 - Non intrusive motifications
   * If the opened file is read only, you get a banner telling you that
   * The banner will contain a link to make the document read-write
   * If the file is deleted outside the editor, you will get a banner telling you that
   * Same if the file is modified outside the editor, but banner will also include
     a link to reload the file
   * All banners auto close after 15 seconds (some less)
