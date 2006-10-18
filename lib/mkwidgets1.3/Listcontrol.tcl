package require Metawidget

# name: Listcontrol
# args: args: option-value pairs
# tries to find a font as column separator, creates all widgets,
# preconfigures tags, sets all bindings for the selection and inits
# private variables.
metawidget create Listcontrol {
  if { ! [ourinfo exists bInit] } {
    foreach { sBind sStep iDir sAction sMode } {
      <Button-1>            m  0 n s   <Control-1>           m  0 t s
      <Shift-1>             m  0 n r   <Shift-Control-1>     m  0 a r

      <Up>                  r -1 n s   <Down>                r  1 n s
      <Shift-Up>            r -1 n r   <Shift-Down>          r  1 n r
      <Control-Up>          r -1 c c   <Control-Down>        r  1 c c
      <Shift-Control-Up>    r -1 a r   <Shift-Control-Down>  r  1 a r

      <Prior>               p -1 n s   <Next>                p  1 n s
      <Shift-Prior>         p -1 n r   <Shift-Next>          p  1 n r
      <Control-Prior>       p -1 c c   <Control-Next>        p  1 c c
      <Shift-Control-Prior> p -1 a r   <Shift-Control-Next>  p  1 a r

      <Control-space>       r  0 t s
    } {
      bind Listcontroltext $sBind "\[winfo  parent %W\] _select $sStep $iDir $sAction $sMode"
    }

    # jump to start or end, should be improved one day
    bind Listcontroltext <Home>        "%W see 1.0"
    bind Listcontroltext <End>         "%W see end"

    # take over some useful Text class bindings
    foreach sBind {<MouseWheel> <B2-Motion> <Button-2> <<Copy>>} {
      bind Listcontroltext $sBind [bind Text $sBind]
    }

    # find a font with one pixel width for column separation
    _findColFonts

    our bInit 1
  }

  # internal widgets, gridded later
  scrollbar $this.scrh -command "$this _dragX" -orient horiz
  scrollbar $this.scrv -command "$this.text yview"

  text $this.cols -wrap none -cursor arrow -height 1 -spacing1 1 -spacing3 1 \
                  -border 0 -bg gray -padx 1 -pady 0 -width 0

  text $this.text -wrap none -cursor arrow -spacing1 1 -spacing3 0 \
                  -border 0 -bg white -padx 1 -pady 1 -insertofftime 0 -insertwidth 0 \
                  -xscroll "$this _moveX" -yscroll "$this.scrv set"

  grid $this.cols -column 0 -row 0 -sticky news
  grid $this.text -column 0 -row 1 -sticky news
  grid $this.scrv -column 1 -row 0 -sticky ns -rowspan 3
  grid $this.scrh -column 0 -row 2 -sticky we
  grid columnconf $this 0 -weight 1
  grid rowconf    $this 1 -weight 1

  # tags for column header and column separator
  $this.cols tag add    end 1.end
  $this.cols tag config tCols -border 1 -relief raised
  $this.cols tag config tCsep -back gray -font [our sColFont]
  $this.text tag config cursor -border 1 -relief solid

  # take Text class bindings out, no insertion cursor etc.
  bindtags $this.cols [mkw.lshrink [bindtags $this.cols] Text]
  bindtags $this.text [mkw.lchange [bindtags $this.text] Text Listcontroltext]

  # bindings to resize a column
  $this.cols tag bind tDrag <Button-1>        "$this _grab"
  $this.cols tag bind tDrag <B1-Motion>       "$this _drag"
  $this.cols tag bind tDrag <ButtonRelease-1> "$this _drop"

  my iAnchor 1                     ;# selection anchor
  my iCursor 1                     ;# cursor row

  my iRows 0                       ;# number of rows
  my lCols {}                      ;# list of column names
  my lData {{}}                    ;# table of data

  my -onselect {}                  ;# redraw method when column resized
  my -update full                  ;# one or more cells selectable?
  my -selectmode multiple          ;# command to eval on selection

  -font [$this.text cget -font]
} {} -default text

# name: _findSepFonts
# args: -
# columns are separated by a 1 pixel line, which is in fact a space character
# with a special font. the loops search for a font on that system, where the
# space char is 1 pixel wide.
metawidget proc Listcontrol _findColFonts {} {
  for { set iBest 1 } { $iBest <= 4 } { incr iBest } {
    for { set iSize 1 } { $iSize <= 4 } { incr iSize } {
      foreach sFamily [font families] {
        set sDescr [list $sFamily $iSize]
        if { [font measure $sDescr " "] == $iBest } {
          our sColFont $sDescr
          return
        }
      }
    }
  }
}

# name: _dragX
# args: args: from "scrollbar -command"
# called on horizontal scrollbar dragging. sets the view for column and
# main text widget simultaneously.
metawidget proc Listcontrol _dragX { args } {
  eval $this.cols xview $args
  eval $this.text xview $args
}

# name: _moveX
# args: args: from "text -scrollx"
# called when main text widgets scrolls horizontally. adjusts the column
# text widget and the scrollbar.
metawidget proc Listcontrol _moveX { args } {
  eval $this.scrh set $args
  eval $this.cols xview moveto [lindex $args 0]
}

# name: _setTabs
# args: bAll: if false, only tabs in column widget are set
# called when a column is resized. calculates the tabs for column and main
# widget. each column is separated by a tab. the text position inside a column
# is also based on a tab.
metawidget proc Listcontrol _setTabs { {bAll 1} } {
  set lTabs {}
  set iX 0

  foreach sColumn [my lCols] {
    mkw.lassign [my aCols($sColumn)] iWidth sAlign

    # create a tab based on column alignment. For right align, the tab must
    # be 1 space left of the column separating tab (it's the text widget!)
    switch $sAlign {
      l { lappend lTabs [expr $iX]         }
      r { lappend lTabs [expr $iX+$iWidth-[my iOneSpace]] }
      c { lappend lTabs [expr $iX+$iWidth/2] }
      n { lappend lTabs [expr $iX+$iWidth/2] }
    }

    # create tab for column separation
    lappend lTabs $sAlign [expr $iX+$iWidth] c
    incr iX $iWidth
  }

  $this.cols config -tabs $lTabs

  # set for main window, if indicated (depends on -update)
  if { $bAll } {
    $this.text config -tabs $lTabs
  }
}

# name: _getValues
# args: sColumn: valid column name
# called before dragging a column. extracts the values of the given column
# out of the lData table and returns them as a list. speeds up text clipping!
metawidget proc Listcontrol _getValues { sColumn } {
  set iPos [_findColumn $sColumn]
  set lValues {}
  foreach lRow [lrange [my lData] 1 end] {
    lappend lValues [lindex $lRow $iPos]
  }

  return $lValues
}

# name: _clipText
# args: sText: text to clip
#       sColumn: column to make sText fit into
# returns a portion of sText that is not wider then the width of sColumn.
metawidget proc Listcontrol _clipText { sText sColumn {iExtra 0} } {
  # get column width and substract the width of two tab chars.
  # strange, but the text widget allocates pixels for a tab.
  set iColWidth [expr [lindex [my aCols($sColumn)] 0]-[my iTwoTabs]-$iExtra-1]

  # for each cycle, cut off one character at the end and see
  # if required space is less then available one. if so, get out.
  for { set i [expr [string length $sText]-2] } { $i >= -1 } { incr i -1 } {
    if { [font measure [my -font] $sText] <= $iColWidth } break;
    set sText [string range $sText 0 $i]
  }

  return $sText
}

# name: _clipHeader
# args: sColumn: valid column name
# called when a column is dragged. clips the column header by replacing it.
# therefore the tCsel tag needs to be added again. tCsel is the left portion
# of a column, its bindings cause the column to be selected. (the right side
# is tDrag, to drag a column).
metawidget proc Listcontrol _clipHeader { sColumn } {
  mkw.lassign [$this.cols tag ranges $sColumn] sI1 sI2
  $this.cols delete $sI1+1ch $sI2-2ch
  $this.cols insert $sI1+1ch [_clipText [lindex [my aCols($sColumn)] 2] $sColumn]

  mkw.lassign [$this.cols tag ranges $sColumn] sI1 sI2
  $this.cols tag add tCsel $sI1 $sI2-2ch
}

# name: _clipColumn
# args: sColumn: valid column name
#       vValues: name of private variable that holds column values
# called when a column is dragged. clips the values in the main widget.
# vValues must contain the values, or they are retrieved. clips each
# cell by replacing the current value with the clipped value.
metawidget proc Listcontrol _clipColumn { sColumn {vValues {}} } {
  if { $vValues == {} } {
    set lValues [_getValues $sColumn]
  } else {
    set lValues [my $vValues]
  }

  set iRow 0
  foreach { sI1 sI2 } [$this.text tag ranges $sColumn] {
    set sValue [lindex $lValues $iRow]

    # if this cell has an image, adjust the index 2 chars to the right,
    # otherwise 1. This is the beginning of the text.
    if { [catch {
      set iExtra [image width [$this.text image cget $sI1+1ch -image]]
      set sI1 $sI1+2ch
    }] } {
      set iExtra 0
      set sI1 $sI1+1ch
    }

    $this.text delete $sI1 $sI2-1ch
    $this.text insert $sI1 [_clipText $sValue $sColumn $iExtra]
    incr iRow
  }
}

# name: _grab
# args: {}
# called before column resize. stores some values in private variables for
# quicker access during actual dragging: column name into sCol, update mode
# in numeric form in iUpd, mouse x coord in iX0, column values in lValues.
metawidget proc Listcontrol _grab {} {
  my sCol [lindex [$this.cols tag names current] 0]
  my iMin [lindex [my aCols([my sCol])] 3]
  my iUpd [mkw.decode [my -update] {none 0 partial 1 full 2}]
  my iX0 [expr [lindex [my aCols([my sCol])] 0]-[winfo pointerx .]]
  my lValues [_getValues [my sCol]]
}

# name: _drag
# args: -
# called during dragging. calculates new width and udpates column data
# with it. sets tabs and clips column and, if -update == full, text.
metawidget proc Listcontrol _drag {} {
  set iX [expr [my iX0]+[winfo pointerx .]]
  if { $iX < [my iMin] } { set iX [my iMin] }

  my aCols([my sCol]) [lreplace [my aCols([my sCol])] 0 0 $iX]

  _setTabs [my iUpd]
  _clipHeader [my sCol]

  if { [my iUpd] == 2 } {
    _clipColumn [my sCol] lValues
  }
}

# name: _drop
# args: -
# called after column resize. sets tabs and clips one last time.
metawidget proc Listcontrol _drop {} {
  _setTabs
  _clipHeader [my sCol]
  _clipColumn [my sCol] lValues
}

# name: _findColumn
# args: sColumn: column name
# checks if given column is valid and returns its position.
metawidget proc Listcontrol _findColumn { sColumn } {
  set iPos [lsearch [my lCols] $sColumn]

  if { $iPos != -1 } {
    return $iPos
  } else {
    error "Listcontrol $this: Unknown column '$sColumn'."
  }
}

# name: _resolveRow
# args: sRow: row number or "end"
# resolves a row specifier like 5 or end-5 into an absolute row.
metawidget proc Listcontrol _resolveRow { sRow } {
  if { $sRow == "end" } {
    set iRow [expr [my iRows]-1]
  } elseif { [regexp {^end-([0-9]+)$} $sRow sDummy iOffset] } {
    set iRow [expr [my iRows]-$iOffset]
  } elseif { [regexp {^[0-9]+$} $sRow] } {
    set iRow $sRow
  } else {
    error "Listcontrol $this: Bad row index '$sRow'. Must be integer, end, or end-integer."
  }

  if { $iRow <= 1 } {
    return 1
  } elseif { $iRow > [my iRows] } {
    return [my iRows]
  } else {
    return $iRow
  }
}

# name: _resolveIndex
# args: sIndex: cell specifier
# resolves cell specifier sIndex into real text indexes. allowed is *.*,
# MyCol.*, *.5, MyCol.5, it identifies a cell, a column, a row or all.
# the result is list of index pairs.
metawidget proc Listcontrol _resolveIndex { sIndex } {
  mkw.lassign [split $sIndex .] sColumn sRow

  if { ! [info exists sRow] || ! [info exists sColumn] } {
    error "Listcontrol $this: Bad index $sIndex. Must be Column.Row"
  }

  if { $sRow == "*" } {
    set iR1 1
    set iR2 [my iRows]
  } else {
    set iR1 [_resolveRow $sRow]
    set iR2 $iR1
  }

  if { $sColumn == "*" } {
    set lColumns [my lCols]
  } else {
    set lColumns [list $sColumn]
  }

  set lRanges {}

  for { set iR $iR1 } { $iR <= $iR2 } { incr iR } {
    foreach sC $lColumns {
      eval lappend lRanges [$this.text tag nextrange $sC $iR.0]
    }
  }

  return $lRanges
}

# name: _select
# args: sStep  : m: mouse, c: column, r: row, p: page
#       iDir   : 0 for mouse, 1 or -1 otherwise
#       sAction: n: new selection, a: append, c: cursor only, t: toggle
#       sMode  : s: single cell, r: range of cells, c: cursor only
# sets the selection, using the standard sel tag.
metawidget proc Listcontrol _select { sStep iDir sAction sMode } {
  focus $this.text

  # step 1: determine the new cursor position
  switch $sStep {
    m {
      set iRow  [lindex [split [$this.text index current] .] 0]
    }
    r {
      set iRow [expr [my iCursor]+$iDir]
      if { $iRow < 1 }          { set iRow 1 }
      if { $iRow > [my iRows] } { set iRow [my iRows] }
    }
    p {
      # widget height divided thru line height gives number of visible rows
      set iRowH [expr [font metrics [my -font] -linespace]+1]
      set iStep [expr [winfo height $this.text]/$iRowH]

      set iRow [expr [my iCursor]+$iDir*$iStep]
      if { $iRow < 1 }          { set iRow 1 }
      if { $iRow > [my iRows] } { set iRow [my iRows] }
    }
  }

  # step 2: get indexes of new cursor cell and set cursor tag there
  $this.text tag remove cursor 1.0 end
  $this.text tag add cursor $iRow.0 $iRow.end
  my iCursor $iRow

  # step 3: determine if sel tag will be removed or added to cells
  switch $sAction {
    n - a {
      set sMethod add
    }
    t {
      # if it's already part of the column's tags, remove it
      if { [lsearch [$this.text tag names $iRow.0] sel] != -1 } {
        set sMethod remove
      } else {
        set sMethod add
      }
    }
  }

  # step 4: remove existing sel tags, if a new selection is set
  if { $sAction == "n" || [my -selectmode] == "single" } {
    $this.text tag remove sel 1.0 end
  }

  # step 5: actually set the sel tag
  switch $sMode.[my -selectmode] {
    s.single - s.multiple - r.single {
      # for single cells, add or remove sel tag
      $this.text tag $sMethod sel $iRow.0 $iRow.end
      my iAnchor [my iCursor]
    }
    r.multiple {
      # for a range, loop thru columns and set sel for affected rows
      set iR1 [expr ([my iAnchor]<$iRow)?[my iAnchor]:$iRow]
      set iR2 [expr ([my iAnchor]>$iRow)?[my iAnchor]:$iRow]

      for { set i $iR1 } { $i <= $iR2 } { incr i } {
        $this.text tag $sMethod sel $i.0 $i.end
      }
    }
  }

  $this.text tag raise sel
  $this.text tag raise cursor
  $this.text see $iRow.0

  # if -onselect script is specified, evaluate it.
  eval [my -onselect]
}

# name: -update
# args: sMode: update mode for column resizing
# none resizes the column only, not the text. partial resizes the text but
# does not do clipping. full does everything.
metawidget proc Listcontrol -update { sMode } {
  my -update [mkw.complete $sMode {full partial none}]
}

# name: -selectmode
# args: sMode: single, multiple
# if single, only one cell can be selected.
metawidget proc Listcontrol -selectmode { sMode } {
  my -selectmode [mkw.complete $sMode {single multiple}]
}

# name: -font
# args: sFont: font specification
# sets the fonts for all two text widgets. separate fonts would have made
# geometry management too complicated.
metawidget proc Listcontrol -font { sFont } {
  $this.text config -font $sFont
  $this.cols config -font $sFont

  my -font $sFont

  # calculate new width of those strange placeholders
  my iOneSpace [font measure $sFont " "]
  my iTwoTabs  [font measure $sFont "\t\t"]

  # clip everything, in case font is larger then old one
  foreach sCol [my lCols] {
    _clipHeader $sCol
    _clipColumn $sCol
  }
}

# name: column
# args: sAction: insert, delete etc.
#       sColumn: column to work on
#       args: depends on sAction
# everything regarding columns. see comments below.
metawidget proc Listcontrol column { sAction {sColumn {}} args } {
  switch [mkw.complete $sAction {insert delete configure cget fit bind names exists}] {
    insert {
      if { [lsearch [my lCols] $sColumn] != -1 } {
        error "Listcontrol $this: Column '$sColumn' already exists."
      }

      # first elem in $args is desired column position. the name of the column
      # currently at this position is retrieved.
      set iPos [lindex $args 0]
      if { $iPos == "end" || $iPos == "" } {
        set sTag end
      } else {
        set sTag [lindex [my lCols] $iPos]
      }

      # update column list, create record for new column with defaults
      my lCols [linsert [my lCols] $iPos $sColumn]
      my aCols($sColumn) [list 120 l $sColumn 0]

      # update data table: insert empty string at new column position
      for { set i 1 } { $i <= [my iRows] } { incr i } {
        my lData [lreplace [my lData] $i $i [linsert [lindex [my lData] $i] $iPos {x}]]
      }

      # update main widget: the column at the insertion position gives us the
      # indexes where to insert. inserts two tabs. the first tab
      # is used for text anchor, the second is the column separator tab.
      # the new column including the two tabs gets a tag assigned with the
      # same name as the column!
      foreach { sI1 sI2 } [$this.text tag ranges $sTag] {
        $this.text insert $sI1 "\t\t"
        $this.text tag add $sColumn $sI1 $sI1+2ch
      }

      # update column widget: similar to above. tags are: tCols spans the
      # new column and gives it 3d look. tCsep is the gap in between. tDrag
      # covers the right tab and has bindings for column resizing.
      mkw.lassign [$this.cols tag ranges $sTag] sI1
      $this.cols insert $sI1 "\t\t "
      $this.cols tag add tCols $sI1 $sI1+2ch
      $this.cols tag add tCsep $sI1+2ch
      $this.cols tag add tDrag $sI1+1ch $sI1+3ch
      $this.cols tag add $sColumn $sI1 $sI1+3ch
      $this.cols tag lower $sColumn

      # set any passed option/value pairs in the rest of $args
      eval column config $sColumn -text $sColumn [lrange $args 1 end]
      _setTabs
    }

    delete {
      set iCol [_findColumn $sColumn]

      # update data table: delete fields of that column
      for { set i 1 } { $i <= [my iRows] } { incr i } {
        my lData [lreplace [my lData] $i $i [lreplace [lindex [my lData] $i] $iCol $iCol]]
      }

      # update column list, forget column record
      my lCols [lreplace [my lCols] $iCol $iCol]
      unmy aCols($sColumn)

      # update main widget: delete all text with the column name's tag
      foreach { sI1 sI2 } [$this.text tag ranges $sColumn] {
        $this.text delete $sI1 $sI2
      }

      # update column widget: similar to above
      mkw.lassign [$this.cols tag ranges $sColumn] sI1 sI2
      $this.cols delete $sI1 $sI2

      _setTabs
    }

    configure {
      _findColumn $sColumn

      set args [mkw.options $args * {-width *} {-align *} {-text *} {-minsize *}]

      # all non-special options, please proceed to the column's tag.
      if { $args != {} } {
        eval $this.cols tag configure $sColumn $args
      }

      # change width of column. tabs likely to change, clipping required.
      if { [info exists -width] } {
        my aCols($sColumn) [lreplace [my aCols($sColumn)] 0 0 ${-width}]
        _setTabs
        _clipHeader $sColumn
        _clipColumn $sColumn
      }

      # change alignment. update tabs.
      if { [info exists -align] } {
        set sAlign [mkw.complete ${-align} {left right center numeric}]
        my aCols($sColumn) [lreplace [my aCols($sColumn)] 1 1 [string index $sAlign 0]]
      }

      # change column title. clipping will let it appear.
      if { [info exists -text] } {
        my aCols($sColumn) [lreplace [my aCols($sColumn)] 2 2 ${-text}]
        _clipHeader $sColumn
      }

      if { [info exists -minsize] } {
        my aCols($sColumn) [lreplace [my aCols($sColumn)] 3 3 ${-minsize}]
        _clipHeader $sColumn
        _clipColumn $sColumn
      }
    }

    cget {
      _findColumn $sColumn

      # if it's not a special option, try to get value from the column's tag
      if { [catch {
        set sOption [mkw.complete $args {-width -align -text}]
      }] } {
        return [$this.cols tag cget $sColumn $args]
      }

      # otherwise return value from column record.
      switch -- $sOption {
        -width  {
          return [lindex [my aCols($sColumn)] 0]
        }
        -align {
          return [mkw.decode [lindex [my aCols($sColumn)] 1] {l left r right c center n numeric}]
        }
        -text {
          return [lindex [my aCols($sColumn)] 2]
        }
      }
    }

    fit {       ;# autosizes a column
      set iCol [_findColumn $sColumn]
      set iWidth 0
      set sFont [$this.text cget -font]
      foreach lValues [my lData] {
        set sValue [lindex $lValues $iCol]
        set iReqw [font measure $sFont "\t$sValue\t"]
        if { $iReqW > $iWidth } { set iWidth $iReqW }
      }

      column config $sColumn -width $iWidth
    }

    bind {     ;# bind command for column. is applied to the column's tag.
      _findColumn $sColumn
      eval $this.cols tag bind $sColumn $args
    }

    names {
      return [my lCols]
    }

    exists {
      return [myinfo exists aCols($sColumn)]
    }
  }
}

# name: insert
# args: iRow: position to insert new row
#       args: row values.
# inserts one or more rows at the position specified by iRow. each element
# in args must contain the values for an entire row.
metawidget proc Listcontrol insert { sRow args } {
  if { $sRow == "end" } {
    set iRow [expr [my iRows]+1]
  } else {
    set iRow [_resolveRow $sRow]
  }

  foreach lValues $args {
    # update data table and row count
    my lData [linsert [my lData] $iRow $lValues]
    my iRows [expr [my iRows]+1]

    # insert one line for the values and one for row separator
    $this.text insert $iRow.0 "\n"

    # insert text similar to "column insert" (same technique, same tags)
    foreach sValue $lValues sColumn [my lCols] {
      $this.text insert $iRow.end "\t\t"
      $this.text tag add $sColumn $iRow.end-2ch $iRow.end
      $this.text insert $iRow.end-1ch [_clipText $sValue $sColumn]
    }
    $this.text tag add end $iRow.end
    incr iRow
  }
}

# name: delete
# args: iRow1: first row to delete
#       iRow2: last row to delete
# deletes one or more rows. iRow1 and iRow2 specify a range of rows.
metawidget proc Listcontrol delete { sRow1 {sRow2 {}} } {
  set iRow1 [_resolveRow $sRow1]

  if { $sRow2 == "end" } {
    set iRow2 [my iRows]
  } elseif { $sRow2 == {} } {
    set iRow2 $iRow1
  } else {
    set iRow2 [_resolveRow $sRow2]
  }

  # clean up data table and row count
  catch { my lData [lreplace [my lData] $iRow1 $iRow2] }
  my iRows [expr [my iRows]+$iRow1-$iRow2-1]

  $this.text delete $iRow1.0 $iRow2.0+1line
}

# name: set
# args: args: ?-column ColumnList? values ?values ...?
# sets new values for a row or for certain columns only. args' first element
# is the topmost row to start insertion. all remaining elements must be lists
# containing values.
metawidget proc Listcontrol set_ { args } {
  # extract the -columns option
  set args [mkw.options $args * {-columns *}]

  # extract row and values
  mkw.lassign $args sRow lValues
  set iRow [_resolveRow $sRow]

  # what columns shall be set?
  if { [info exists -columns] } {
    set lColumns ${-columns}
  } else {
    set lColumns [my lCols]
  }

  if { [llength $lValues] != [llength $lColumns] } {
    error "Listcontrol $this: Wrong number of values: $lValues"
  }

  # update data table
  if { ! [info exists -columns] } {
    # replace an entire row in the data table
    my lData [lreplace [my lData] $iRow $iRow $lValues]
  } else {
    # do it column by column
    set lRow [lindex [my lData] $iRow]
    foreach sValue $lValues sColumn $lColumns {
      set iPos [_findColumn $sColumn]
      set lRow [lreplace $lRow $iPos $iPos $sValue]
    }

    my lData [lreplace [my lData] $iRow $iRow $lRow]
  }

  # update main widget
  foreach sValue $lValues sColumn $lColumns {
    mkw.lassign [$this.text tag nextrange $sColumn $iRow.0] sI1 sI2

    # calculate start index depending on embedded image
    if { [catch {
      $this.text image cget $sI1+1ch -image
      set sI1 $sI1+2ch
    }] } {
      set sI1 $sI1+1ch
    }

    $this.text delete $sI1 $sI2-1ch
    $this.text insert $sI1 $sValue
  }

  foreach sCol $lColumns {
    _clipColumn $sCol
  }
}

# name: get
# args: args: ?-columns ColumnList? firstRow ?lastRow?
# returns values for a range of rows, either for all columns or for
# those specified by the -columns option.
metawidget proc Listcontrol get { args } {
  # extract column option and rows
  set args [mkw.options $args * {-columns *}]
  mkw.lassign $args sRow1 sRow2

  set iRow1 [_resolveRow $sRow1]
  if { [info exists sRow2] } {
    set iRow2 [_resolveRow $sRow2]
  } else {
    set iRow2 $iRow1
  }

  # get data from data table
  if { ! [info exists -columns] } {
    # get entire rows
    set lData [lrange [my lData] $iRow1 $iRow2]
  } else {
    # here it's cumbersome
    set lIndexes {}

    # get the indexes of the columns for later "lindex"
    foreach sColumn ${-columns} {
      lappend lIndexes [_findColumn $sColumn]
    }

    # row by row, column by column...
    set lData {}
    foreach lRow [lrange [my lData] $iRow1 $iRow2] {
      set lCut {}
      foreach iIndex $lIndexes {
        lappend lCut [lindex $lRow $iIndex]
      }
      lappend lData $lCut
    }
  }

  return $lData
}

# name: image
# args: sAction: commands for images control
#       sIndex: cell specifier
# allows to attach images to each field, and other things.
metawidget proc Listcontrol image_ { sAction {sIndex {}} args } {
  set sAction [mkw.complete $sAction {cget configure create delete names}]

  if { $sAction == "names" } {
    return [$this.text image names]
  }

  foreach { sI1 sI2 } [_resolveIndex $sIndex] {
    if { $sAction == "delete" || $sAction == "create" } {
      catch {
        $this.text image cget $sI1+1ch -image
        $this.text delete $sI1+1ch
      }
    }

    if { $sAction != "delete" } {
      eval $this.text image $sAction $sI1+1ch $args
    }
  }
}

# name: tag
# args: sAction: almost all commands allowed for tags
#       sTag: tag name. must not be a column name
#       args: depends on sAction
# similar to the "text tag" command. some tag commands require special
# treatment so the don't mix up the internal tags. careful with fonts:
# geometry might get out of wack.
metawidget proc Listcontrol tag { sAction sTag args } {
  # reject column names and "sel"
  if { [lsearch [linsert [my lCols] end "sel"] $sTag] != -1 } {
    error "Listcontrol $this: Tag name $sTag not allowed"
  }

  switch [mkw.complete $sAction {add bind cget configure delete lower names lower raise ranges remove}] {
    add - remove {
      # the first element of $args specifies the cell(s) for the tag. it is
      # resolved to real text indexes, then the tag is added or removed.
      foreach { sI1 sI2 } [_resolveIndex [lindex $args 0]] {
        eval $this.text tag $sAction $sTag $sI1 $sI2
      }
    }

    ranges {
      # converts the text indexes returned by [text tag ranges] into cell
      # indexes. e.g 24.5 (row 24, character pos. 5) maps to MyCol.24.
      set lCells {}
      foreach { sI1 sI2 } [$this.text tag ranges $sTag] {
        mkw.lassign [split $sI1 .] iRow iCol
        set sCol [lindex [$this.text tag names $sI1] 0]
        lappend lCells $sCol.$iRow
      }

      return $lCells
    }

    lower {
      # the internal tags (those that have the column names) *must* be the first
      # one returned by [text tag names]. the loop takes care of that.
      eval $this.text tag lower $sTag $args

      foreach sColumn [my lCols] {
        $this.text tag lower $sColumn
      }
    }

    default {
      eval $this.text tag $sAction $sTag $args
    }
  }
}

# name: selection
# args: sWhat: what to do with selection
#       args: depends on sWhat
# clears, sets or gets selection. get returns a list of rows. set accepts
# a list where each element can have one or two values. in case of one
# value it specifies a row, for two values it specifies a range of rows.
metawidget proc Listcontrol selection { sWhat args } {
  switch [mkw.complete $sWhat {clear get set}] {
    clear {
      $this.text tag remove sel 1.0 end
    }
    get {
      set lRows {}
      foreach { sI1 sI2 } [$this.text tag ranges sel] {
        lappend lRows [lindex [split $sI1 .] 0]
      }

      return $lRows
    }
    set {
      $this.text tag remove sel 1.0 end

      foreach lRange $args {
        set iArgs [llength $lRange]

        if { $iArgs == 1 } {
          $this.text tag add sel $lRange.0 $lRange.end
        } elseif { $iArgs == 2 } {
          mkw.lassign $lRange iRow1 iRow2
          for { set i $iRow1 } { $i <= $iRow2 } { incr i } {
            $this.text tag add sel $i.0 $i.end
          }
        }
      }
    }
  }
}

# name: see
# args: sCell: cell specifier
# makes a given cell visible. simply transforms the cell into a
# real text index and calls the "see" command.
metawidget proc Listcontrol see { iRow } {
  $this.text see $iRow.0
}

# name: bind
# args: args: as accepted by the bind command.
# applies bindings to the main widget.
metawidget proc Listcontrol bind_ { args } {
  eval bind $this.text $args
}

# name: rows
# args: -
# returns the number of rows.
metawidget proc Listcontrol size {} {
  return [my iRows]
}

metawidget command Listcontrol _dragX     _dragX
metawidget command Listcontrol _moveX     _moveX
metawidget command Listcontrol _grab      _grab
metawidget command Listcontrol _drag      _drag
metawidget command Listcontrol _drop      _drop
metawidget command Listcontrol _select    _select

metawidget command Listcontrol column     column
metawidget command Listcontrol insert     insert
metawidget command Listcontrol delete     delete
metawidget command Listcontrol set        set_
metawidget command Listcontrol get        get
metawidget command Listcontrol image      image_
metawidget command Listcontrol tag        tag
metawidget command Listcontrol selection  selection
metawidget command Listcontrol see        see
metawidget command Listcontrol bind       bind_
metawidget command Listcontrol size       size

metawidget option  Listcontrol -update     -update
metawidget option  Listcontrol -selectmode -selectmode
metawidget option  Listcontrol -font       -font
metawidget option  Listcontrol -onselect

proc testGetFiles {} {
  global sPath
  if { $sPath == "/" } { set sPath "" }

  .list delete 1 end

  set iRow 1
  foreach sFile [glob -nocomplain $sPath/*] {
    set sTail [file tail $sFile]
    set iSize [file size $sFile]
    set sMod [clock format [file mtime $sFile] -format {%D %H:%M}]

    .list insert end [list $sTail $iSize $sMod]
    .list image create modt.$iRow -image pCl

    if { [expr rand()] > .5 } {
      .list image create note.$iRow -image pNb
      .list tag add tNote note.$iRow
    }

    if { [file isfile $sFile] } {
      .list image create name.$iRow -image pFi
    } else {
      .list image create name.$iRow -image pFo
      .list tag add tFolder name.$iRow
    }
    incr iRow
  }
}

proc test {} {
  image create photo pFo -file ./demos/images/Folder.gif
  image create photo pFi -file ./demos/images/File.gif
  image create photo pCl -file ./demos/images/TinyClock.gif
  image create photo pNb -file ./demos/images/Notebook.gif

  pack [label .head -text "Enter a path below and press Browse"] -side top -anchor w
  pack [frame .ctrl] -side top -fill x
  pack [button .ctrl.back -text "Back"  -command {set sPath [file dirname $sPath]; testGetFiles}] -side right
  pack [button .ctrl.brws -text "Browse" -command testGetFiles] -side right
  pack [entry  .ctrl.path -textvariable sPath] -side left -fill x -expand 1
  pack [listcontrol .list] -side bottom -fill both -expand 1

  .list column insert name end -text Name     -width 150 -minsize 32
  .list column insert size end -text Size     -width 60  -minsize 16 -align r
  .list column insert modt end -text Modified -width 100 -minsize 16
  .list column insert note end -text Note     -width 40 -minsize 16

  .list column bind name <Double-1> {.list column fit name}
  .list column bind size <Double-1> {.list column fit size}
  .list column bind modt <Double-1> {.list column fit modt}

  .list tag bind tFolder <Double-1> {
     set iRow [lindex [.list selection get] 0]
     set sFile [lindex [lindex [.list get $iRow] 0] 0]
     set sPath $sPath/$sFile
     testGetFiles
  }

  .list tag bind tNote <Double-1> {
     set iRow [lindex [.list selection get] 0]
     set sFile [lindex [lindex [.list get $iRow] 0] 0]
     toplevel .t
     grid [label .t.h -text "Here could be a note for file $sFile" -back white] -sticky news
     grab .t
     break
  }
}

#test
