package require Metawidget

# name: Gridcontrol
# args: args: option-value pairs
# tries to find a font for column and row separation, creates all widgets,
# preconfigures tags, sets all bindings for the selection and inits
# private variables.
metawidget create Gridcontrol {
  if { ! [ourinfo exists bInit] } {
    # bindings for selection in main text widget
    foreach { sBind sStep iDir sAction sMode } {
      <Button-1>            m  0 n s   <Control-1>           m  0 t s
      <Shift-1>             m  0 n r   <Shift-Control-1>     m  0 a r
      <Button1-Motion>      m  0 n r

      <Left>                c -1 n s   <Right>               c  1 n s
      <Shift-Left>          c -1 n r   <Shift-Right>         c  1 n r
      <Control-Left>        c -1 c c   <Control-Right>       c  1 c c
      <Shift-Control-Left>  c -1 a r   <Shift-Control-Right> c  1 a r

      <Up>                  r -1 n s   <Down>                r  1 n s
      <Shift-Up>            r -1 n r   <Shift-Down>          r  1 n r
      <Control-Up>          r -1 c c   <Control-Down>        r  1 c c
      <Shift-Control-Up>    r -1 a r   <Shift-Control-Down>  r  1 a r

      <Prior>               p -1 n s   <Next>                p  1 n s
      <Shift-Prior>         p -1 n r   <Shift-Next>          p  1 n r
      <Control-Prior>       p -1 c c   <Control-Next>        p  1 c c
      <Shift-Control-Prior> p -1 a r   <Shift-Control-Next>  p  1 a r

      <Control-space>       c  0 t s
    } {
      bind Gridcontroltext $sBind "\[winfo parent %W\] _select $sStep $iDir $sAction $sMode"
    }

    # jump to start or end, should be improved one day
    bind Gridcontroltext <Home>        "%W see 1.0"
    bind Gridcontroltext <End>         "%W see end"

    # take over some useful Text class bindings
    foreach sBind {<MouseWheel> <B2-Motion> <Button-2> <<Copy>>} {
      bind Gridcontroltext $sBind [bind Text $sBind]
    }

    # find fonts for column and row separation
    _findSepFonts

    our bInit 1
  }

  # internal widgets, gridded later
  frame     $this.edge -bg gray -border 1 -relief raised
  scrollbar $this.scrh -command "$this _dragX" -orient horiz
  scrollbar $this.scrv -command "$this _dragY"

  text $this.cols -wrap none -cursor arrow -height 1 -spacing1 1 -spacing3 1 \
                   -border 0 -bg gray -padx 1 -pady 0

  text $this.rows -wrap none -cursor arrow -width 1 -spacing1 1 -spacing3 1 \
                   -border 0 -bg gray -padx 0 -pady 1

  text $this.text -wrap none -cursor arrow -spacing1 1 -spacing3 1 \
                   -border 0 -bg white -padx 1 -pady 1 -insertofftime 0 -insertwidth 0 \
                   -xscroll "$this _moveX" -yscroll "$this _moveY"

  grid $this.edge -column 0 -row 0 -sticky news
  grid $this.cols -column 1 -row 0 -sticky news
  grid $this.rows -column 0 -row 1 -sticky news
  grid $this.text -column 1 -row 1 -sticky news
  grid $this.scrv -column 2 -row 0 -sticky ns -rowspan    3
  grid $this.scrh -column 0 -row 2 -sticky we -columnspan 2
  grid columnconf $this 1 -weight 1
  grid rowconf    $this 1 -weight 1

  # tags for column header and column separator
  $this.cols tag add    end 1.end
  $this.cols tag config tCols -border 1 -relief raised
  $this.cols tag config tCsep -back gray -font [our sColFont]

  # tags for row header and row separator
  $this.rows tag config tRows -border 1 -relief raised -lmargin1 2
  $this.rows tag config tRsep -back gray -font [our sRowFont] -spacing1 0 -spacing3 0

  # tags for main text widget: column and row separator and cursor (black frame)
  $this.text tag config tCsep -back gray -font [our sColFont]
  $this.text tag config tRsep -back gray -font [our sRowFont] -spacing1 0 -spacing3 0
  $this.text tag config cursor -border 1 -relief solid

  # take Text class bindings out, no insertion cursor etc.
  bindtags $this.cols [mkw.lshrink [bindtags $this.cols] Text]
  bindtags $this.rows [mkw.lshrink [bindtags $this.rows] Text]
  bindtags $this.text [mkw.lchange [bindtags $this.text] Text Gridcontroltext]

  # bindings for selection in columns and rows
  foreach { sBind sAction sMode } { <Button-1> n s   <Control-1>       t s
                                    <Shift-1>  n r   <Control-Shift-1> a r } {
    $this.cols tag bind tCsel $sBind "$this _select2 c $sAction $sMode"
    $this.rows tag bind tRows $sBind "$this _select2 r $sAction $sMode"
  }

  # bindings to resize a column
  $this.cols tag bind tDrag <Button-1>        "$this _grab"
  $this.cols tag bind tDrag <B1-Motion>       "$this _drag"
  $this.cols tag bind tDrag <ButtonRelease-1> "$this _drop"

  # binding for top left corner, selects all
  bind $this.edge <1> "$this _select2 a a r"

  my iRows 0                          ;# number of rows
  my lCols {}                         ;# list of column names
  my lData {{}}                       ;# table of data

  my -update full                     ;# redraw method when column resized
  my -selectmode multiple             ;# one or more cells selectable?
  my -onselect {}                     ;# command to eval on selection

  -font [$this.text cget -font]
} {} -default text

# name: _findSepFonts
# args: -
# columns are separated by a 1 pixel line, which is in fact a space character
# with a special font. same goes for the rows. the loops search for two fonts
# on that system, one where the space char is 1 pixel wide, one where the
# spacing is 1 pixel high. font names are stored in class-common variables.
metawidget proc Gridcontrol _findSepFonts {} {
  for { set iBest 1 } { $iBest <= 4 } { incr iBest } {
    for { set iSize 1 } { $iSize <= 4 } { incr iSize } {
      foreach sFamily [font families] {
        set sDescr [list $sFamily $iSize]

        if { [font measure $sDescr " "] == $iBest } {
          our sColFont $sDescr
        }
        if { [font metrics $sDescr -linespace] == $iBest } {
          our sRowFont $sDescr
        }

        if { [ourinfo exists sColFont] && [ourinfo exists sRowFont] } return
      }
    }
  }
}

# name: _dragX
# args: args: from "scrollbar -command"
# called on horizontal scrollbar dragging. sets the view for column and
# main text widget simultaneously.
metawidget proc Gridcontrol _dragX { args } {
  regsub {1 units$} $args {2 units} args
  eval $this.cols xview $args
  eval $this.text xview $args
}

# name: _dragY
# args: args: from "scrollbar -command"
# called on vertical scrollbar dragging. sets the view for row and
# main text widget simultaneously. always scrolls two rows, so that
# a separator row is never on top.
metawidget proc Gridcontrol _dragY { args } {
  regsub {1 units$} $args {2 units} args
  eval $this.rows yview $args
  eval $this.text yview $args

  if { [$this.text tag names @0,0] == "tRsep" } {
    $this.rows yview scroll 1 units
    $this.text yview scroll 1 units
  }
}

# name: _moveX
# args: args: from "text -scrollx"
# called when main text widgets scrolls horizontally. adjusts the column
# text widget and the scrollbar.
metawidget proc Gridcontrol _moveX { args } {
  eval $this.scrh set $args
  eval $this.cols xview moveto [lindex $args 0]
}

# name: _moveY
# args: args: from "text -scrolly"
# called when main text widgets scrolls vertically. adjusts the row
# text widget and the scrollbar.
metawidget proc Gridcontrol _moveY { args } {
  eval $this.scrv set $args
  eval $this.rows yview moveto [lindex $args 0]
}

# name: _setTabs
# args: bAll: if false, only tabs in column widget are set
# called when a column is resized. calculates the tabs for column and main
# widget. each column is separated by a tab. the text position inside a column
# is also based on a tab.
metawidget proc Gridcontrol _setTabs { {bAll 1} } {
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
metawidget proc Gridcontrol _getValues { sColumn } {
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
metawidget proc Gridcontrol _clipText { sText sColumn } {
  # get column width and substract the width of two tab chars.
  # strange, but the text widget allocates pixels for a tab.
  set iColWidth [expr [lindex [my aCols($sColumn)] 0]-[my iTwoTabs]-1]

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
metawidget proc Gridcontrol _clipHeader { sColumn } {
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
metawidget proc Gridcontrol _clipColumn { sColumn {vValues {}} } {
  if { $vValues == {} } {
    set lValues [_getValues $sColumn]
  } else {
    set lValues [my $vValues]
  }

  foreach { sI1 sI2 } [$this.text tag ranges $sColumn] sValue $lValues {
    $this.text delete $sI1+1ch $sI2-1ch
    $this.text insert $sI1+1ch [_clipText $sValue $sColumn]
  }
}

# name: _grab
# args: {}
# called before column resize. stores some values in private variables for
# quicker access during actual dragging: column name into sCol, update mode
# in numeric form in iUpd, mouse x coord in iX0, column values in lValues.
metawidget proc Gridcontrol _grab {} {
  my sCol [lindex [$this.cols tag names current] 0]
  my iUpd [mkw.decode [my -update] {none 0 partial 1 full 2}]
  my iX0 [expr [lindex [my aCols([my sCol])] 0]-[winfo pointerx .]]
  my lValues [_getValues [my sCol]]
}

# name: _drag
# args: -
# called during dragging. calculates new width and udpates column data
# with it. sets tabs and clips column and, if -update == full, text.
metawidget proc Gridcontrol _drag {} {
  set iX [expr [my iX0]+[winfo pointerx .]]
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
metawidget proc Gridcontrol _drop {} {
  _setTabs
  _clipHeader [my sCol]
  _clipColumn [my sCol] lValues
}

# name: _findColumn
# args: sColumn: column name
# checks if given column is valid and returns its position.
metawidget proc Gridcontrol _findColumn { sColumn } {
  set iPos [lsearch [my lCols] $sColumn]
  if { $iPos == -1 } {
    error "Gridcontrol $this: Unknown column '$sColumn'."
  }

  return $iPos
}

# name: _resolveArea
# args: sCell1/2: Cell or Range definition
# resolves a cell range specified by sCell1 and sCell2 into column names,
# row numbers and column positions. sCell1/2 can have formats like *.*,
# MyCol.*, *.5, MyCol.5 and identifies a cell, a column, a row or all.
# therefore sCell1 and sCell2 define a rectangular shaped range of cells.
# the result is a list with 5 elements: the column names of that range,
# the first and last row number, and the first and last column positions.
metawidget proc Gridcontrol _resolveArea { {sCell1 *.*} {sCell2 {}} } {
  if { $sCell2 == {} } {
    set sCell2 $sCell1
  }

  mkw.lassign [split $sCell1 .] sC1 iR1
  mkw.lassign [split $sCell2 .] sC2 iR2

  if { $sC1 == "*" } {
    set sC1 [lindex [my lCols] 0]
  }
  if { $iR1 == "*" } {
    set iR1 1
  }

  if { $sC2 == "*" } {
    set sC2 [lindex [my lCols] end]
  }
  if { $iR2 == "*" } {
    set iR2 [my iRows]
  }

  set iC1 [_findColumn $sC1]
  set iC2 [_findColumn $sC2]

  set iCol1 [expr ($iC1<$iC2)?$iC1:$iC2]
  set iCol2 [expr ($iC1>$iC2)?$iC1:$iC2]
  set iRow1 [expr ($iR1<$iR2)?$iR1:$iR2]
  set iRow2 [expr ($iR1>$iR2)?$iR1:$iR2]

  return [list [lrange [my lCols] $iCol1 $iCol2] $iRow1 $iRow2 $iCol1 $iCol2]
}

# name: _select
# args: sStep  : m: mouse, c: column, r: row, p: page
#       iDir   : 0 for mouse, 1 or -1 otherwise
#       sAction: n: new selection, a: append, c: cursor only, t: toggle
#       sMode  : s: single cell, r: range of cells, c: cursor only
# sets the selection, using the standard sel tag.
metawidget proc Gridcontrol _select { sStep iDir sAction sMode } {
  focus $this.text

  # step 1: determine the new cursor position
  if { $sStep == "m" } {
    # get index at mouse position ([... index current] does not update during
    # <B1-Motion> events. cells would not get selected with dragging).
    set iX [expr [winfo pointerx $this.text]-[winfo rootx $this.text]]
    set iY [expr [winfo pointery $this.text]-[winfo rooty $this.text]]
    set sInd [$this.text index @$iX,$iY]

    # split sInd into column and row.
    set iRow [expr ([lindex [split $sInd .] 0]+1)/2]
    set sCol [lindex [$this.text tag names $sInd] 0]

    # leave, if we had hit an invalid area
    if { [lsearch "{} tCsep tRsep end" $sCol] != -1 } return
  } else {
    # get current cursor position...
    if { ! [myinfo exists lCursor] } return
    mkw.lassign [my lCursor] sCol iRow

    # ...and calculate new position
    switch $sStep {
      c {
        set iPos [expr [lsearch [my lCols] $sCol]+$iDir]
        if { $iPos >= 0 && $iPos < [llength [my lCols]]} {
          set sCol [lindex [my lCols] $iPos]
        }
      }
      r {
        incr iRow $iDir
        if { $iRow < 1 } { set iRow 1 }
        if { $iRow > [my iRows] } { set iRow [my iRows] }
      }
      p {
        # widget height divided thru line height gives number of visible rows
        set iRowH [expr [font metrics [my -font] -linespace]+1]
        set iStep [expr [winfo height $this.text]/$iRowH]

        incr iRow [expr $iDir*$iStep]
        if { $iRow < 1 } { set iRow 1 }
        if { $iRow > [my iRows] } { set iRow [my iRows] }
      }
    }
  }

  # step 2: get indexes of new cursor cell and set cursor tag there
  mkw.lassign [$this.text tag nextrange $sCol [expr $iRow*2-1].0] sIc1 sIc2
  if { ! [info exists sIc1] } return
  $this.text tag remove cursor 1.0 end
  $this.text tag add cursor $sIc1 $sIc2
  my lCursor [list $sCol $iRow]

  # step 3: determine if sel tag will be removed or added to cells
  switch $sAction {
    n - a {
      set sMethod add
    }
    t {
      # if it's already part of the column's tags, remove it
      if { [lsearch [$this.text tag names $sIc1] sel] != -1 } {
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
      $this.text tag $sMethod sel $sIc1 $sIc2
      my lAnchor [my lCursor]
    }
    r.multiple {
      if { ! [myinfo exists lAnchor] } {
        my lAnchor [my lCursor]
      }

      # for a range, loop thru columns and set sel for affected rows
      mkw.lassign [my lAnchor] sACol iARow
      set iC1 [lsearch [my lCols] $sACol]
      set iC2 [lsearch [my lCols] $sCol]
      set iI1 [expr ($iARow<$iRow?$iARow:$iRow)*2-2]
      set iI2 [expr ($iARow>$iRow?$iARow:$iRow)*2-1]

      if { $iC2 < $iC1 } {
        set iC3 $iC2
        set iC2 $iC1
        set iC1 $iC3
      }

      for { set i $iC1 } { $i <= $iC2 } { incr i } {
        set sColumn [lindex [my lCols] $i]
        eval $this.text tag $sMethod sel [lrange [$this.text tag ranges $sColumn] $iI1 $iI2]
      }
    }
  }

  $this.text tag raise sel
  $this.text tag raise cursor
  $this.text see $sIc1

  if { [$this.text tag names @0,0] == "tRsep" } {
    $this.rows yview scroll 1 units
    $this.text yview scroll 1 units
  }

  # if -onselect script is specified, evaluate it.
  eval [my -onselect]
}

# name: _select2
# args: see _select
# called for selection of entire rows or columns. modifies the anchor points
# lCursor and lAnchor and calls _select to actually set the seletion.
metawidget proc Gridcontrol _select2 { sStep sAction sMode } {
  if { [my -selectmode] == "single" } return

  switch $sStep {
    a {
      my lCursor [list [lindex [my lCols] 0] 1]
      my lAnchor [list [lindex [my lCols] end] [my iRows]]
    }
    c {
      set sInd [$this.cols index current]
      set sCol [lindex [$this.cols tag names $sInd] 0]
      my lCursor [list $sCol 1]

      if { $sMode == "s" } {
        my lAnchor [list $sCol [my iRows]]
      } else {
        my lAnchor [lreplace [my lAnchor] 1 1 [my iRows]]
      }
    }
    r {
      set sInd [$this.rows index current]
      set iRow [expr ([lindex [split $sInd .] 0]+1)/2]
      my lCursor [list [lindex [my lCols] 0] $iRow]

      if { $sMode == "s" } {
        my lAnchor [list [lindex [my lCols] end] $iRow]
      } else {
        my lAnchor [lreplace [my lAnchor] 0 0 [lindex [my lCols] end]]
      }
    }
  }

  if { [my lCursor] != "" } {
    _select c 0 $sAction r
  }
}

# name: -update
# args: sMode: update mode for column resizing
# none resizes the column only, not the text. partial resizes the text but
# does not do clipping. full does everything.
metawidget proc Gridcontrol -update { sMode } {
  my -update [mkw.complete $sMode {full partial none}]
}

# name: -selectmode
# args: sMode: single, multiple
# if single, only one cell can be selected.
metawidget proc Gridcontrol -selectmode { sMode } {
  my -selectmode [mkw.complete $sMode {single multiple}]
}

# name: -font
# args: sFont: font specification
# sets the fonts for all three text widgets. separate fonts would have made
# geometry management too complicated.
metawidget proc Gridcontrol -font { sFont } {
  # set font
  $this.text config -font $sFont
  $this.cols config -font $sFont
  $this.rows config -font $sFont

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
metawidget proc Gridcontrol column { sAction {sColumn {}} args } {
  switch [mkw.complete $sAction {insert delete configure cget fit bind names exists}] {
    insert {
      if { [lsearch [my lCols] $sColumn] != -1 } {
        error "Gridcontrol $this: Column '$sColumn' already exists."
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
      my aCols($sColumn) [list 120 l $sColumn]

      # update data table: insert empty string at new column position
      for { set i 1 } { $i <= [my iRows] } { incr i } {
        my lData [lreplace [my lData] $i $i [linsert [lindex [my lData] $i] $iPos {}]]
      }

      # update main widget: the column at the insertion position gives us the
      # indexes where to insert. inserts two tabs and a space. the first tab
      # is used for text anchor, the second is the column separator tab. the
      # space becomes the gray vertical line. the new column including the
      # two tabs gets a tag assigned with the same name as the column!
      foreach { sI1 sI2 } [$this.text tag ranges $sTag] {
        $this.text insert $sI1 "\t\t "
        $this.text tag add $sColumn $sI1 $sI1+2ch
        $this.text tag add tCsep $sI1+2ch
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
      $this.cols tag lower $sColumn    ;# make it the first in the tag list

      # set any passed option/value pairs in the rest of $args
      eval column config {$sColumn} -text {$sColumn} [lrange $args 1 end]
      _setTabs
    }

    delete {
      set iCol [$this _findColumn $sColumn]

      # update data table: delete fields of that column
      #loop i 1 [my iRows] {}
      for { set i 1 } { $i <= [my iRows] } { incr i } {
        my lData [lreplace [my lData] $i $i [lreplace [lindex [my lData] $i] $iCol $iCol]]
      }

      # update column list, forget column record
      my lCols [lreplace [my lCols] $iCol]
      unmy aCols($sColumn)

      # update main widget: delete all text with the column name's tag
      foreach { sI1 sI2 } [$this.text tag ranges $sColumn] {
        $this.text delete $sI1 $sI2+1ch
      }

      # update column widget: similar to above
      mkw.lassign [$this.cols tag ranges $sColumn] sI1 sI2
      $this.cols delete $sI1 $sI2

      _setTabs
    }

    configure {
      _findColumn $sColumn

      set args [mkw.options $args * {-width *} {-align *} {-text *}]

      # all non-special options, please proceed to the column's tag.
      if { $args != {} } {
        eval $this.cols tag configure {$sColumn} $args
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
        _setTabs
      }

      # change column title. clipping will let it appear.
      if { [info exists -text] } {
        my aCols($sColumn) [lreplace [my aCols($sColumn)] 2 2 ${-text}]
        _clipHeader $sColumn
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
        set iReqw [font measure $sFont "\t$sValue \t"]
        if { $iReqW > $iWidth } { set iWidth $iReqW }
      }

      column config $sColumn -width $iWidth
    }

    bind {      ;# bind command for column. is applied to the column's tag.
      _findColumn $sColumn
      eval $this.cols tag bind {$sColumn} $args
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
metawidget proc Gridcontrol insert { iRow args } {
  if { $iRow == "end" || $iRow > [my iRows] } {
    set iRow [expr [my iRows]+1]
  } elseif { $iRow < 1 } {
    set iRow 1
  }

  foreach lValues $args {
    # update data table and row count
    my lData [linsert [my lData] $iRow $lValues]
    my iRows [expr [my iRows]+1]

    # insert one line for the values and one for row separator
    set iLine [expr $iRow*2-1]
    $this.text insert $iLine.0 "\n\n"
    $this.text tag add tRsep $iLine.0+1lines $iLine.0+2lines

    # insert text similar to "column insert" (same technique, same tags)
    foreach sValue $lValues sColumn [my lCols] {
      $this.text insert $iLine.end "\t\t "
      $this.text tag add $sColumn $iLine.end-3ch $iLine.end-1ch
      $this.text tag add tCsep $iLine.end-1ch
      $this.text insert $iLine.end-2ch [_clipText $sValue $sColumn]
    }
    $this.text tag add end $iLine.end

    # update rows widget: one line for row number, one for separator
    set sI [$this.rows index end]
    $this.rows insert end "[my iRows]\n\n"
    $this.rows tag add tRows $sI-1lines $sI
    $this.rows tag add tRsep $sI $sI+1lines
    incr iRow
  }

  # let rows widget dynamically grow or shrink
  $this.rows config -width [expr [string length [my iRows]]+1]
}

# name: delete
# args: iRow1: first row to delete
#       iRow2: last row to delete
# deletes one or more rows. iRow1 and iRow2 specifiy a range of rows.
metawidget proc Gridcontrol delete { iRow1 {iRow2 {}} } {
  if { $iRow1 == "end" || $iRow1 > [my iRows] } {
    set iRow1 [my iRows]
  } elseif { $iRow1 < 1 } {
    set iRow1 1
  }

  if { $iRow2 == "end" || $iRow2 > [my iRows] } {
    set iRow2 [my iRows]
  } elseif { $iRow2 == {} } {
    set iRow2 $iRow1
  } elseif { $iRow2 < 1 } {
    set iRow2 1
  }

  # clean up data table and row count
  my lData [lreplace [my lData] $iRow1 $iRow2]
  my iRows [expr [my iRows]+$iRow1-$iRow2-1]

  # delete rows from main widget
  set iLine1 [expr $iRow1*2-1]
  set iLine2 [expr $iRow2*2+1]
  $this.text delete $iLine1.0 $iLine2.0

  # delete trailing rows from rows widget
  set sI [$this.rows index end]
  $this.rows delete $sI-[expr ($iRow2-$iRow1)*2+3]lines $sI-1lines
  $this.rows config -width [expr [string length [my iRows]]+1]
}

# name: set
# args: sCell: cell specifier
#       args: list of value-lists to be set
# sets new values for a cell or a range of cells. args must be a two-
# dimensional table that is embedded at the position specified by sCell.
metawidget proc Gridcontrol set_ { sCell args } {
  mkw.lassign [_resolveArea $sCell] lCols iRow1 iRow2 iCol1

  foreach lValues $args {
    set iCol2 [expr $iCol1+[llength $lValues]-1]
    set lRow  [lindex [my lData] $iRow1]
    set lRow  [eval lreplace \$lRow $iCol1 $iCol2 $lValues]
    my lData [lreplace [my lData] $iRow1 $iRow1 $lRow]
    incr iRow1
  }

  foreach sCol $lCols {
    _clipColumn $sCol
  }
}

# name: get
# args: sCell1/2: cell specifier
# returns the values of a cell or a range of cells. sCell1 specifies
# the top-left corner, sCell2 the bottom-right corner of the cell range.
# the return list is a two-dimensional table (a list of lists).
metawidget proc Gridcontrol get { {sCell1 *.*} {sCell2 {}} } {
  mkw.lassign [_resolveArea $sCell1 $sCell2] lCols iRow1 iRow2 iCol1 iCol2
  set lData {}

  for { set i $iRow1 } { $i <= $iRow2 } { incr i } {
    lappend lData [lrange [lindex [my lData] $i] $iCol1 $iCol2]
  }

  return $lData
}

# name: tag
# args: sAction: almost all commands allowed for tags
#       sTag: tag name. must not be a column name
#       args: depends on sAction
# similar to the "text tag" command. some tag commands require special
# treatment so the don't mix up the internal tags. careful with fonts:
# geometry might get out of wack.
metawidget proc Gridcontrol tag { sAction sTag args } {
  # column names are reserved, the sel tag is not.
  if { [lsearch [my lCols] $sTag] != -1 } {
    error "Gridcontrol $this: Tag name $sTag not allowed"
  }

  switch [mkw.complete $sAction {add bind cget configure delete lower names raise ranges remove}] {
    add - remove {
      # tag ranges are given by cell specifiers in the first two elems of $args.
      # they are resolved to columns and rows, then true text indexes. then the
      # user's tag is added or removed.
      mkw.lassign [_resolveArea [lindex $args 0] [lindex $args 1]] lCols iR1 iR2
      set iL1 [expr $iR1*2-2]
      set iL2 [expr $iR2*2-1]
      foreach sCol $lCols {
        eval $this.text tag $sAction {$sTag} [lrange [$this.text tag ranges $sCol] $iL1 $iL2]
      }
    }

    ranges {
      # converts the text indexes returned by [text tag ranges] into cell
      # indexes. e.g 24.5 (row 24, character pos. 5) maps to MyCol.24.
      set lCells {}
      foreach { sI1 sI2 } [$this.text tag ranges $sTag] {
        mkw.lassign [split $sI1 .] iLine iCol
        set sCol [lindex [$this.text tag names $sI1] 0]
        set iRow [expr $iLine/2+1]
        lappend lCells $sCol.$iRow
      }

      return $lCells
    }

    names {
      # takes $sTag as a cell, converts it into an index and gets its tags
      mkw.lassign [_resolveArea $sTag] lCols iR1 iR2
      set sInd [lindex [$this.text tag ranges [lindex $lCols 0]] [expr $iR1*2-2]]
      return [$this.text tag names $sInd]
    }

    lower {
      # the internal tags (those that have the column names) *must* be the first
      # one returned by [text tag names]. the loop takes care of that.
      eval $this.text tag lower {$sTag} $args

      foreach sColumn [my lCols] {
        $this.text tag lower $sColumn
      }
    }

    default {
      # all other commands are directly applied.
      eval $this.text tag $sAction {$sTag} $args
    }
  }
}

# name: see
# args: sCell: cell specifier
# makes a give cell visible. simply transforms the cell spec into a
# real text index and calls the "see" command.
metawidget proc Gridcontrol see { sCell } {
  mkw.lassign [_resolveArea $sCell] sCol iRow
  $this.text see [lindex [$this.text tag nextrange $sCol $iRow.0] 0]
}

# name: bind
# args: args: as accepted by the bind command.
# applies bindings to the main widget.
metawidget proc Gridcontrol bind_ { args } {
  eval bind $this.text $args
}

# name: rows
# args: -
# returns the number of rows.
metawidget proc Gridcontrol rows {} {
  return [my iRows]
}

metawidget command Gridcontrol _dragX   _dragX
metawidget command Gridcontrol _dragY   _dragY
metawidget command Gridcontrol _moveX   _moveX
metawidget command Gridcontrol _moveY   _moveY
metawidget command Gridcontrol _grab    _grab
metawidget command Gridcontrol _drag    _drag
metawidget command Gridcontrol _drop    _drop
metawidget command Gridcontrol _select  _select
metawidget command Gridcontrol _select2 _select2

metawidget command Gridcontrol column   column
metawidget command Gridcontrol insert   insert
metawidget command Gridcontrol delete   delete
metawidget command Gridcontrol set      set_
metawidget command Gridcontrol get      get
metawidget command Gridcontrol tag      tag
metawidget command Gridcontrol see      see
metawidget command Gridcontrol bind     bind_
metawidget command Gridcontrol rows     rows

metawidget option  Gridcontrol -update     -update
metawidget option  Gridcontrol -selectmode -selectmode
metawidget option  Gridcontrol -font       -font
metawidget option  Gridcontrol -onselect

proc testSetValue {} {
  global sValue

  foreach sCell [.grid tag ranges sel] {
    .grid set $sCell $sValue
  }
}

proc test {} {
  pack [frame .ctrl] -fill x
  pack [label .ctrl.capt -text "Select some cells, enter a number in the entry below and press Enter"]
  pack [entry .ctrl.entr -textvariable sValue] -side left -fill x -expand 1
  bind .ctrl.entr <Key-Return> {.ctrl.setv invoke}

  pack [button .ctrl.setv -text Set -command {
    foreach sCell [.grid tag ranges sel] {
      if { [lsearch [.grid tag names $sCell] tSum] != -1 } continue
      .grid set $sCell [list $sValue]
    }

    for { set i 1 } { $i <= 3 } { incr i } {
      set lValues [.grid get val$i.1 val$i.4]
      set fSum 0
      foreach sVal $lValues {
        catch { set fSum [expr $fSum+$sVal] }
      }

      .grid set val$i.5 $fSum
    }
    for { set i 1 } { $i <= 5 } { incr i } {
      set lValues [.grid get val1.$i val3.$i]
      set fSum 0
      foreach sVal [lindex $lValues 0] {
        catch { set fSum [expr $fSum+$sVal] }
      }

      .grid set vsum.$i $fSum
    }
  }] -side right


  pack [gridcontrol .grid -font {Courier 12}] -fill both -expand 1

  .grid column insert val1 end -text "Value 1" -width 80 -align c
  .grid column insert val2 end -text "Value 2" -width 80 -align c
  .grid column insert val3 end -text "Value 3" -width 80 -align c
  .grid column insert vsum end -text "Sum"     -width 80 -align c

  .grid insert end [list {} {} {} {}]
  .grid insert end [list {} {} {} {}]
  .grid insert end [list {} {} {} {}]
  .grid insert end [list {} {} {} {}]
  .grid insert end [list {} {} {} {}]

  .grid tag config tSum -back yellow;
  .grid tag add tSum vsum.*
  .grid tag add tSum *.5

  .grid tag add tTot vsum.5
  .grid tag config tTot -back red

  .grid bind <Double-1> {
     set sValue [lindex [lindex [.grid get [lindex [.grid tag ranges sel] 0]] 0] 0]
  }
}

#test
