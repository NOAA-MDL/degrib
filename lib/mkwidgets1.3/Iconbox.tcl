package require Metawidget

# name: Iconbox
# args: args: option-value pairs
# tries to find a font as column separator, creates all widgets,
# preconfigures tags, sets all bindings for the selection and inits
# private variables.
metawidget create Iconbox {
  if { ! [ourinfo exists bInit] } {
    # bindings for selection, here for the icon widgets (-view large/small)
    foreach { sBind sStep iDir sAction sMode } {
      <Button-1>            m  0 n s   <Control-1>           m  0 t s
      <Shift-1>             m  0 n r   <Shift-Control-1>     m  0 a r
    } {
      bind Iconboxicon $sBind "catch {\[winfo parent \[winfo parent %W\]\] _select2 $sStep $iDir $sAction $sMode %W}"
    }

    # bindings for selection for the text window itself
    foreach { sBind sStep iDir sAction sMode } {
      <Button-1>            m  0 n s   <Control-1>           m  0 t s
      <Shift-1>             m  0 n r   <Shift-Control-1>     m  0 a r

      <Left>                i -1 n s   <Right>               i  1 n s
      <Shift-Left>          i -1 n r   <Shift-Right>         i  1 n r
      <Control-Left>        i -1 c c   <Control-Right>       i  1 c c
      <Shift-Control-Left>  i -1 a r   <Shift-Control-Right> i  1 a r

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
      bind Iconboxtext $sBind "catch {\[winfo parent %W\] _select $sStep $iDir $sAction $sMode}"
    }

    # jump to start or end, should be improved one day
    bind Iconboxtext <Home>        "%W see 1.0"
    bind Iconboxtext <End>         "%W see end"

    # take over some useful Text class bindings
    foreach sBind {<MouseWheel> <B2-Motion> <Button-2>} {
      bind Iconboxtext $sBind [bind Text $sBind]
    }

    # find a font with one pixel width for column separation
    _findColFonts

    our bInit 1
  }

  # internal widgets, gridded later
  scrollbar $this.scrh -command "$this _dragX" -orient horiz
  scrollbar $this.scrv -command "$this.text yview"

  text $this.cols -wrap none -cursor arrow -height 1 -spacing1 1 -spacing3 1 \
                   -border 0 -bg gray -padx 1 -pady 0

  text $this.text -wrap none -cursor arrow -spacing1 0 -spacing3 0 \
                   -border 0 -bg white -padx 1 -pady 1 -insertofftime 0 -insertwidth 0 \
                   -xscroll "$this _moveX" -yscroll "$this.scrv set"

  grid $this.text -column 0 -row 1 -sticky news
  grid $this.scrv -column 1 -row 0 -sticky ns -rowspan 3
  grid columnconf $this 0 -weight 1
  grid rowconf    $this 1 -weight 1

  # tags for column header and column separator
  $this.cols tag config tCols -border 1 -relief raised
  $this.cols tag config tCsep -back gray -font [our sColFont]
  $this.text tag config cursor -border 1 -relief solid

  # take Text class bindings out, no insertion cursor etc.
  bindtags $this.cols [mkw.lshrink [bindtags $this.cols] Text]
  bindtags $this.text [mkw.lchange [bindtags $this.text] Text Iconboxtext]

  # bindings to resize a column
  $this.cols tag bind tDrag <Button-1>        "$this _grab"
  $this.cols tag bind tDrag <B1-Motion>       "$this _drag"
  $this.cols tag bind tDrag <ButtonRelease-1> "$this _drop"
  $this.cols tag bind tText <1>               "$this _sort"

  my lSel {}          ;# list of selected icons
  my lCols {}         ;# column names
  my lIcons {}        ;# list of all icons

  my iAnchor {}       ;# selection anchor row (for -view list)
  my iCursor {}       ;# cursor row (for -view list)
  my sAnchor {}       ;# selection anchor icon
  my sCursor {}       ;# cursor icon

  my -view large
  my -columns {}
  my -onselect {}
  my -update full
  my -selectmode multiple
  my -grid 70

  -font [$this.text cget -font]
} {} -default text

# name: _findColFonts
# args: -
# columns are separated by a 1 pixel line, which is in fact a space character
# with a special font. the loops search for a font on that system, where the
# space char is 1 pixel wide.
metawidget proc Iconbox _findColFonts {} {
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

# name: _sort
# args: -
# sorts the column that has been clicked.
metawidget proc Iconbox _sort {} {
  set sColumn [lindex [$this.cols tag names current] 0]
  set iDecr [lindex [my aCols($sColumn)] 3]
  sort [mkw.decode $iDecr {0 -incr 1 -decr}] -dict -column $sColumn
  my aCols($sColumn) [lreplace [my aCols($sColumn)] 3 3 [mkw.decode $iDecr {1 0 0 1}]]
}

# name: _dragX
# args: args: from "scrollbar -command"
# called on horizontal scrollbar dragging. sets the view for column and
# main text widget simultaneously.
metawidget proc Iconbox _dragX { args } {
  eval $this.cols xview $args
  eval $this.text xview $args
}

# name: _moveX
# args: args: from "text -scrollx"
# called when main text widgets scrolls horizontally. adjusts the column
# text widget and the scrollbar.
metawidget proc Iconbox _moveX { args } {
  eval $this.scrh set $args
  eval $this.cols xview moveto [lindex $args 0]
}

# name: _setTabs
# args: bAll: if false, only tabs in column widget are set
# called when a column is resized. calculates the tabs for column and main
# widget. each column is separated by a tab. the text position inside a column
# is also based on a tab.
metawidget proc Iconbox _setTabs { {bAll 1} } {
  set lTabs {}
  set iX 1

  foreach sColumn [my lCols] {
    mkw.lassign [my aCols($sColumn)] iWidth sAlign

    # create a tab based on column alignment. For right align, the tab must
    # be 1 space left of the column separating tab (it's the text widget!)
    switch $sAlign {
      l { lappend lTabs [expr $iX]         }
      r { lappend lTabs [expr $iX+$iWidth] }
      c { lappend lTabs [expr $iX+$iWidth/2-[my iOneSpace]] }
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
metawidget proc Iconbox _getValues { sColumn } {
  set iPos [_findColumn $sColumn]
  set lValues {}
  foreach sIcon [my lIcons] {
    lappend lValues [lindex [lindex [my aIcons($sIcon)] 2] $iPos]
  }

  return $lValues
}

# name: _clipText
# args: sText: text to clip
#       sColumn: column to make sText fit into
# returns a portion of sText that is not wider then the width of sColumn.
metawidget proc Iconbox _clipText { sText sColumn {iExtra 0} } {
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
metawidget proc Iconbox _clipHeader { sColumn } {
  mkw.lassign [$this.cols tag ranges $sColumn] sI1 sI2
  $this.cols delete $sI1+1ch $sI2-2ch
  $this.cols insert $sI1+1ch [_clipText $sColumn $sColumn]

  mkw.lassign [$this.cols tag ranges $sColumn] sI1 sI2
#  $this.cols tag add tText $sI1 $sI2-2ch
  $this.cols tag add tText $sI1 $sI2-1ch
}

# name: _clipColumn
# args: sColumn: valid column name
#       vValues: name of private variable that holds column values
# called when a column is dragged. clips the values in the main widget.
# vValues must contain the values, or they are retrieved. clips each
# cell by replacing the current value with the clipped value.
metawidget proc Iconbox _clipColumn { sColumn {vValues {}} } {
  if { $vValues == {} } {
    set lValues [_getValues $sColumn]
  } else {
    set lValues [my $vValues]
  }

  set iRow 0
  foreach { sI1 sI2 } [$this.text tag ranges $sColumn] {
    set sValue [lindex $lValues $iRow]

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

# name: _clipIcon
# args: sIcon: icon name
# clips the icon's caption so that it will take up no more than 2 lines
# when displayed as a large icon.
metawidget proc Iconbox _clipIcon { sIcon {bDoit 1} } {
  set sText [lindex [lindex [my aIcons($sIcon)] 2] 0]

  if { $bDoit } {
    set iWidth [expr 2*[my -grid]-5]

    for { set i [expr [string length $sText]-2] } { $i >= -1 } { incr i -1 } {
      if { [font measure [my -font] $sText] <= $iWidth } break;
      set sText [string range $sText 0 $i]
    }
  }

  [lindex [my aIcons($sIcon)] 3].l configure -text $sText
}

# name: _grab
# args: {}
# called before column resize. stores some values in private variables for
# quicker access during actual dragging: column name into sCol, update mode
# in numeric form in iUpd, mouse x coord in iX0, column values in lValues.
metawidget proc Iconbox _grab {} {
  my sCol [lindex [$this.cols tag names current] 0]
  my iMin [lindex [my aCols([my sCol])] 2]
  my iUpd [mkw.decode [my -update] {none 0 partial 1 full 2}]
  my iX0 [expr [lindex [my aCols([my sCol])] 0]-[winfo pointerx .]]
  my lValues [_getValues [my sCol]]
}

# name: _drag
# args: -
# called during dragging. calculates new width and udpates column data
# with it. sets tabs and clips column and, if -update == full, text.
metawidget proc Iconbox _drag {} {
  set iX [expr [my iX0]+[winfo pointerx .]]
  if { $iX < [my iMin] } {
    set iX [my iMin]
  }

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
metawidget proc Iconbox _drop {} {
  _setTabs
  _clipHeader [my sCol]
  _clipColumn [my sCol] lValues

  if { [my -view] == "list" && [my sCol] == [lindex [my lCols] 0] } {
    _setListmodeTags
  }
}

# name: _findIcon
# args: sIcon: icon name
# checks if given icon exists.
metawidget proc Iconbox _findIcon { sIcon } {
  set iPos [lsearch [my lIcons] $sIcon]

  if { $iPos == -1 } {
    error "Iconbox $this: Icon $sIcon does not exist."
  }

  return $iPos
}

# name: _findColumn
# args: sColumn: column name
# checks if given column is valid and returns its position.
metawidget proc Iconbox _findColumn { sColumn } {
  set iPos [lsearch [my lCols] $sColumn]

  if { $iPos == -1 } {
    error "Iconbox $this: Unknown column '$sColumn'."
  }

  return $iPos
}

# name: _selIcon
# args: sIcon: icon to set or remove selection for
#       bSelected: 0: remove, 1: set selection
# sets or removes the selection for an item (or all items, if sIcon == *).
# changes the look of the item as well as the selection list lSel.
metawidget proc Iconbox _selIcon { sIcon {bSelected 0} } {
  if { $sIcon == "*" } {
    foreach sIcon [my lIcons] {
      set wIcon [lindex [my aIcons($sIcon)] 3]
      $wIcon.l config -bg [$this.text cget -back] -fg [$this.text cget -fore]
    }
    my lSel {}
  } elseif { $bSelected } {
    set wIcon [lindex [my aIcons($sIcon)] 3]
    $wIcon.l config -bg [$this.text cget -selectback] -fg [$this.text cget -selectfore]
    my lSel [mkw.lextend [my lSel] $sIcon]
  } else {
    set wIcon [lindex [my aIcons($sIcon)] 3]
    $wIcon.l config -bg [$this.text cget -back] -fg [$this.text cget -fore]
    my lSel [mkw.lshrink [my lSel] $sIcon]
  }
}

# name: _select
# args: sStep  : m: mouse, i: icon, r: row, p: page
#       iDir   : 0 for mouse, 1 or -1 otherwise
#       sAction: n: new selection, a: append, c: cursor only, t: toggle
#       sMode  : s: single cell, r: range of cells, c: cursor only
# calls one of the two members that really set the selection.
metawidget proc Iconbox _select { sStep iDir sAction sMode } {
  if { [my -view] == "list" } {
    _select1 $sStep $iDir $sAction $sMode
  } else {
    _select2 $sStep $iDir $sAction $sMode
  }
}

# name: _select1
# args: see _select
# called when view mode is "list". uses the standard sel tag for selection.
metawidget proc Iconbox _select1 { sStep iDir sAction sMode } {
  focus $this.text

  # step 1: determine the new cursor position
  switch $sStep {
    m {
      set iY [expr [winfo pointery $this.text]-[winfo rooty $this.text]]
      set iRow [lindex [split [$this.text index @0,$iY] .] 0]
      if { $iRow > [llength [my lIcons]] } return
    }
    r {
      set iRows [llength [my lIcons]]
      set iRow [expr [my iCursor]+$iDir]
      if { $iRow < 1 } { set iRow 1 }
      if { $iRow > $iRows } { set iRow $iRows }
    }
    p {
      set iRows [llength [my lIcons]]
      set iRowH [expr [font metrics [my -font] -linespace]+1]
      set iStep [expr [winfo height $this.text]/$iRowH]
      set iRow [expr [my iCursor]+$iDir*$iStep]
      if { $iRow < 1 } { set iRow 1 }
      if { $iRow > $iRows } { set iRow $iRows }
    }
    i {
      return
    }
  }

  # step 2: get indexes of new cursor cell and set cursor tag there
  $this.text tag remove cursor 1.0 end
  mkw.lassign [$this.text tag nextrange tIcon $iRow.0] sI1 sI2
  $this.text tag add cursor $sI1 $sI2
  my iCursor $iRow

  # step 3: determine if sel tag will be removed or added to cells
  switch $sAction {
    n - a {
      set sMethod add
    }
    t {
      if { [lsearch [$this.text tag names $iRow.2] sel] != -1 } {
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
  if { [info exists sMethod] } {
    if { ! ( $sMode == "r" && [my -selectmode] == "multiple" ) } {
      my iAnchor [my iCursor]
    }

    set iR1 [expr ([my iCursor]<[my iAnchor])?[my iCursor]:[my iAnchor]]
    set iR2 [expr ([my iCursor]>[my iAnchor])?[my iCursor]:[my iAnchor]]

    for { set i $iR1 } { $i <= $iR2 } { incr i } {
      mkw.lassign [$this.text tag nextrange tIcon $i.0] sI1 sI2
      $this.text tag $sMethod sel $sI1 $sI2
    }
  }

  $this.text tag raise sel
  $this.text tag raise cursor
  $this.text see $iRow.0

  my lSel {}
  foreach { sI1 sI2 } [$this.text tag ranges sel] {
    my lSel [linsert [my lSel] end [lindex [$this.text tag names $sI1] 0]]
  }

  eval [my -onselect]
}

# name: _select2
# args: see _select
#       wWindow: icon that was clicked
# called when view mode is "large" or "list". can't use sel tag, because the
# icons are real widgets. rather modifies there background color etc.
metawidget proc Iconbox _select2 { sStep iDir sAction sMode {wWindow {}} } {
  focus $this.text

  # try to unset cursor icon
  catch {
    set wIcon [lindex [my aIcons([my sCursor])] 3]
    $wIcon.l config -relief flat
  }

  # step 1: determine icon to set cursor for
  if { $sStep == "m" } {
    if { $wWindow == {} } return

    # search icon name. something to improve!
    set wParent [winfo parent $wWindow]
    foreach sIcon [my lIcons] {
      if { ! [string compare [lindex [my aIcons($sIcon)] 3] $wParent] } break
    }

    my sCursor $sIcon
  } else {
    set iPos [lsearch [my lIcons] [my sCursor]]

    if { $sStep == "i" } {
      set iStep 1
    } else {
      mkw.lassign [$this.text bbox 1.$iPos] iX iY iW iH
      set iStep [expr [winfo width $this.text]/$iW]

      if { $sStep == "p" } {
        set iStep [expr $iStep * [winfo height $this.text]/$iH]
      }
    }

    #set sIcon [myindex lIcons [expr $iPos+$iDir*$iStep]]
    set sIcon [lindex [my lIcons] [expr $iPos+$iDir*$iStep]]
    if { $sIcon != {} } {
      my sCursor $sIcon
    }
  }

  # step 2: determine if selection is set or removed
  switch $sAction {
    n - a {
      set iSetsel 1
    }
    t {
      if { [lsearch [my lSel] [my sCursor]] == -1 } {
        set iSetsel 1
      } else {
        set iSetsel 0
      }
    }
  }

  # step 3: remove existing selection, if new selection will be set
  if { $sAction == "n" || [my -selectmode] == "single" } {
    _selIcon * 0
  }

  # step 4: actually set selection
  if { [info exists iSetsel] } {
    if { $sMode != "r" || [my -selectmode] == "single" } {
      my sAnchor [my sCursor]
    }

    set iPos1 [lsearch [my lIcons] [my sAnchor]]
    set iPos2 [lsearch [my lIcons] [my sCursor]]
    set iP1 [expr ($iPos1<$iPos2)?$iPos1:$iPos2]
    set iP2 [expr ($iPos1>$iPos2)?$iPos1:$iPos2]
    set lIcons [lrange [my lIcons] $iP1 $iP2]
    foreach sIcon $lIcons {
      _selIcon $sIcon $iSetsel
    }
  }

  # set cursor icon
  catch {
    set wIcon [lindex [my aIcons([my sCursor])] 3]
    $wIcon.l config -relief solid
  }

  $this.text see 1.[lsearch [my lIcons] [my sCursor]]
}

# name: _setListmodeTags
# args: -
# for -view == list, the frame widgets are removed and plain text is used,
# plus the small image on the left side. in order for selection and column
# dragging to work, each line needs some tags.
metawidget proc Iconbox _setListmodeTags {} {
  # set special tags for the icon itself (left column). tIcon is needed to
  # set the selection. each icon gets a tag with the icon's name for
  # identification, e.g. to attach bindings or get the selection.
  foreach { sI1 sI2 } [$this.text tag ranges [lindex [my lCols] 0]] sIcon [my lIcons] {
    $this.text tag add tIcon $sI1+2ch $sI2-1ch
    $this.text tag add $sIcon $sI1+1ch $sI2-1ch
    $this.text tag lower $sIcon
  }

  # take over selection from lSel list.
  foreach sIcon [my lSel] {
    mkw.lassign [$this.text tag ranges $sIcon] sI1 sI2
    $this.text tag add sel $sI1+1ch $sI2
  }

  # add all the individual bindings to the icons.
  foreach lBinds [myinfo vars lBind.*] {
#    mkw.lassign [split $lBinds .] x sIcon sBind
    set lst [split $lBinds .]
    set sBind [lindex $lst [expr [llength $lst] -1]]
    set sIcon [join [lrange $lst 1 [expr [llength $lst] -2]] .]
    foreach sScript [my $lBinds] {
      $this.text tag bind $sIcon $sBind $sScript
    }
  }

  # set cursor tag
  my iCursor [expr [lsearch [my lIcons] [my sCursor]]+1]
  if { [my iCursor] } {
    mkw.lassign [$this.text tag nextrange tIcon [my iCursor].0] sI1 sI2
    $this.text tag add cursor $sI1 $sI2
  }
}

# name: _redraw
# args: -
# draws the metawidget, depending on -view. for -view == large or small,
# the icon frames are inserted into the text widget with "text window create".
# for -view == list, the frames are not used. instead, the small image is
# inserted at the beginning of each line, followed by the icon's -values.
metawidget proc Iconbox _redraw {} {
  # get the widgets out the text widget and unpack them.
  foreach wWindow [$this.text window names] {
    pack $wWindow
    pack forget $wWindow
  }

  if { [my -view] == "list" } {
    # clear contents and show column row and scrollbar
    $this.text delete 1.0 end
    grid $this.cols -column 0 -row 0 -sticky news
    grid $this.scrh -column 0 -row 2 -sticky we

    $this.text config -wrap none

    # create rows
    set iRow 0
    foreach sIcon [my lIcons] {
      incr iRow
      $this.text insert $iRow.end "\n"

      # first tab is the text anchor, second is the column separator tab.
      # each field gets a tag with its column's name.
      foreach sValue [lindex [my aIcons($sIcon)] 2] sColumn [my lCols] {
        $this.text insert $iRow.end "\t\t"
        $this.text tag add $sColumn $iRow.end-2ch $iRow.end
        $this.text insert $iRow.end-1ch $sValue
      }

      $this.text image create $iRow.1 -image [lindex [my aIcons($sIcon)] 1] -align center -padx 2
    }

    # clip all columns and create tags
    foreach sCol [my lCols] {
      _clipColumn $sCol
    }

    _setListmodeTags
  } else {
    # if we just had -view list, then clean up first
    if { [winfo ismapped $this.cols] } {
      # discard column widget and scrollbar
      grid forget $this.cols $this.scrh

      # remove selection from all icons...
      _selIcon * 0

      # ...and take over what has been selected in the list view.
      foreach { sI1 sI2 } [$this.text tag ranges sel] {
        set sIcon [lindex [$this.text tag names $sI1] 0]
        _selIcon $sIcon 1
      }

      # set the cursor icon
      catch { [lindex [my aIcons([my sCursor])] 3].l config -relief flat }
      my sCursor [lindex [my lIcons] [expr [my iCursor]-1]]
      catch { [lindex [my aIcons([my sCursor])] 3].l config -relief solid }
    }

    $this.text delete 1.0 end
    $this.text config -wrap word

    # set some variables depending on the view
    set iIcon [mkw.decode [my -view] {small 1  large 0 }]
    set iWrap [mkw.decode [my -view] "small 0  large [my -grid]"]
    set iGapx [mkw.decode [my -view] {small 15 large 5}]
    set iGapy [mkw.decode [my -view] {small 0  large 10}]
    set iMaxw 0
    set iMaxh 0

    foreach sIcon [my lIcons] {
      set wIcon [lindex [my aIcons($sIcon)] 3]
      $wIcon.i config -image [lindex [my aIcons($sIcon)] $iIcon]
      $wIcon.l config -wrap $iWrap
      set iIWidth  [winfo reqwidth  $wIcon.i]
      set iIHeight [winfo reqheight $wIcon.i]
      set iLWidth  [winfo reqwidth  $wIcon.l]
      set iLHeight [winfo reqheight $wIcon.l]

      # depending on the view, we must track the largest icon
      if { $iIcon == 0 } { ;# large icons
        # width is constant, height is sum of icon and text
        set iMaxw [my -grid]
        set iReqh [expr $iIHeight+$iLHeight]
        if { $iReqh > $iMaxh } {
          set iMaxh $iReqh
        }

        place $wIcon.i -relx .5 -rely 0 -anchor n
        place $wIcon.l -x 0 -y $iIHeight -relx .5 -rely 0 -anchor n
      } else {             ;# small icons
        # height is the greater of image and text, width is their sum
        set iReqw [expr $iIWidth+$iLWidth]
        if { $iReqw > $iMaxw } {
          set iMaxw $iReqw
        }

        set iReqh [expr ($iIHeight>$iLHeight)?$iIHeight:$iLHeight]
        if { $iReqh > $iMaxh } {
          set iMaxh $iReqh
        }

        place $wIcon.i -relx 0 -rely .5 -anchor w
        place $wIcon.l -x $iIWidth -y 0 -relx 0 -rely .5 -anchor w
      }

      $this.text window create end -align bottom -window $wIcon
    }

    # set all icons to the size of the largest icon, so they get oriented.
    incr iMaxw $iGapx
    incr iMaxh $iGapy
    foreach sIcon [my lIcons] {
      set wIcon [lindex [my aIcons($sIcon)] 3]
      $wIcon config -width $iMaxw -height $iMaxh
    }
  }

  catch { see [my sCursor] }
}

# name: -columns
# args: lColumns: list of column specifications
# sets new columns. a column spec must have the format {text alignment
# width minwidth}. text is mandatory and must be unique, the rest is optional.
metawidget proc Iconbox -columns { lColumns } {
  my lCols {}
  eval unmy aCols
  $this.cols delete 1.0 end

  foreach lColumn $lColumns {
    set sAlign l
    set iWidth 100
    set iMinwidth 10
    mkw.lassign $lColumn sText sAlign iWidth iMinwidth

    my aCols($sText) [list $iWidth $sAlign $iMinwidth 0]
    my lCols [linsert [my lCols] end $sText]

    # first tab is the text anchor, second is the column separator tab. tags:
    # tCols for the 3d look, tCsep for the gap, tDrag for resizing.
    $this.cols insert 1.end "\t\t "
    $this.cols tag add tCols 1.end-3ch 1.end-1ch
    $this.cols tag add tCsep 1.end-1ch
    $this.cols tag add tDrag 1.end-2ch 1.end
    $this.cols tag add $sText 1.end-3ch 1.end

    $this.cols tag lower $sText
    _clipHeader $sText
  }

  my -columns $lColumns
  _setTabs
}

# name: -view
# args: sView: new view to set
# sets a new view mode: small icons, large icons, or list.
metawidget proc Iconbox -view { sView } {
  my -view [mkw.complete $sView {small large list}]
  _redraw
}

# name: -update
# args: sMode: update mode for column resizing
# none resizes the column only, not the text. partial resizes the text but
# does not do clipping. full does everything.
metawidget proc Iconbox -update { sMode } {
  my -update [mkw.complete $sMode {full partial none}]
}

# name: -selectmode
# args: sMode: single, multiple
# if single, only one cell can be selected.
metawidget proc Iconbox -selectmode { sMode } {
  my -selectmode [mkw.complete $sMode {single multiple}]
}

# name: -font
# args: sFont: font specification
# sets the fonts for all two text widgets. separate fonts would have made
# geometry management too complicated.
metawidget proc Iconbox -font { sFont } {
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

  # update icons
  foreach sIcon [my lIcons] {
    [lindex [my aIcons($sIcon)] 3].l config -font $sFont
    _clipIcon $sIcon
  }
}

# name: insert
# args: iRow: position to insert icon
#       sIcon: icon name
#       args: option-value pairs
# inserts a new icon. an icon is a frame with two labels inside, one for
# the image, one for the text. packing order depends on -view and is done in
# _redraw.
metawidget proc Iconbox insert { iPos sIcon args } {
  if { [myinfo exists aIcons($sIcon)] } {
    error "Iconbox $this: Icon $sIcon already exists."
  }

  # eliminate dots and blanks, then get a unique widget name. ("read-me"
  # is ok, but "read.me" also maps to read-me. sequence will add "#1" etc.).
  regsub -all {[. ]} $sIcon {-} sNodots
  set wIcon $this._i$sNodots
  while { [winfo exists $wIcon] } {
    if { [catch { incr iSeq }] } { set iSeq 1 }
    set wIcon $this._i$sNodots#$iSeq
  }

  # create widgets. two labels for image and caption.
  frame $wIcon -border 0 -bg white
  label $wIcon.i -padx 0 -pady 0 -bg white -border 0
  label $wIcon.l -padx 0 -pady 0 -bg white -border 0 -font [my -font]

  # replace Label class bindings with selection bindings
  bindtags $wIcon.i [mkw.lchange [bindtags $wIcon.i] Label Iconboxicon]
  bindtags $wIcon.l [mkw.lchange [bindtags $wIcon.l] Label Iconboxicon]

  # make sure packing order is right (icon above text widget)
  raise $wIcon $this.text

  # icon properties: large image, small image, values for -view == list, widget, userdata
  my aIcons($sIcon) [list {} {} {} $wIcon {}]
  my lIcons [linsert [my lIcons] $iPos $sIcon]

  eval iconconfigure {$sIcon} -text {$sIcon} $args

  # queue an event for _redraw when idle. delete any old events
  # so that there will be exactly one redraw after several inserts.
  catch { after cancel [my jRedraw] }
  my jRedraw [after idle $this _redraw]
}

# name: delete
# args: args: list of icon names, nothing for all icons
# deletes the icons specified by $args. destroys the frame widget and, if
# -view == list (where the frames are not packed), deletes the associated
# rows. finally updates internal data.
metawidget proc Iconbox delete { args } {
  if { ! [llength $args] } {
    foreach sIcon [my lIcons] {
      destroy [lindex [my aIcons($sIcon)] 3]
    }

    $this.text delete 1.0 end

    my lSel {}
    my lIcons {}
    eval unmy aIcons
  } else {
    foreach sIcon $args {
      _findIcon $sIcon
      destroy [lindex [my aIcons($sIcon)] 3]

      if { [my -view] == "list" } {
        mkw.lassign [$this.text tag ranges $sIcon] sI1 sI2
        $this.text delete "$sI1 linestart" "$sI1 linestart+1line"
      }

      my lSel [mkw.lshrink [my lSel] $sIcon]
      my lIcons [mkw.lshrink [my lIcons] $sIcon]
      unmy aIcons($sIcon)
    }
  }
}

# name: iconconfigure
# args: sIcon: icon name
#       args: option-value pairs
# configures an icon. the three special options are stored in the icon's
# record, all others (like -text for the icon's caption) are applied
# to the icon's text label.
metawidget proc Iconbox iconconfigure { sIcon args } {
  set iRow  [expr [_findIcon $sIcon] + 1]
  set wIcon [lindex [my aIcons($sIcon)] 3]
  set args  [mkw.options $args * {-text *} {-image *} {-smallimage *} {-values *} {-user *}]

  if { $args != {} } {
    eval $wIcon.l configure $args
  }

  if { [info exists -text] } {
    my aIcons($sIcon) [lreplace [my aIcons($sIcon)] 2 2 [lreplace [lindex [my aIcons($sIcon)] 2] 0 0 ${-text}]]
    _clipIcon $sIcon

    if { ! [catch {$this.text index $sIcon.first}] } {
      mkw.lassign [$this.text tag ranges $sIcon] sI1 sI2
      $this.text delete $sI1 $sI2
      $this.text insert $sI1 ${-text}
      _setListmodeTags
    }
  }

  if { [info exists -image] } {
    my aIcons($sIcon) [lreplace [my aIcons($sIcon)] 0 0 ${-image}]
    if { [my -view] == "large" } {
      $wIcon.i config -image ${-image}
    }
  }

  if { [info exists -smallimage] } {
    my aIcons($sIcon) [lreplace [my aIcons($sIcon)] 1 1 ${-smallimage}]
    if { [my -view] == "small" } {
      $wIcon.i config -image ${-smallimage}
    }
  }

  if { [info exists -values] } {
    if { [llength ${-values}] >= [llength [my lCols]] } {
      error "Iconbox $this: Too many values for option -values."
    }

    set lValues [lindex [my aIcons($sIcon)] 2]
    set lValues [concat [list [lindex $lValues 0]] ${-values}]
    my aIcons($sIcon) [lreplace [my aIcons($sIcon)] 2 2 $lValues]

    if { ! [catch {$this.text index $sIcon.first}] } {
      foreach sValue ${-values} sColumn [lrange [my lCols] 1 end] {
        mkw.lassign [$this.text tag nextrange $sColumn $iRow.0] sI1 sI2
        $this.text delete $sI1+1ch $sI2-1ch
        $this.text insert $sI1+1ch $sValue
      }
    }
  }

  if { [info exists -user] } {
    my aIcons($sIcon) [lreplace [my aIcons($sIcon)] 4 4 ${-user}]
  }
}

# name: iconcget
# args: sIcon: icon name
#       args: option-value pairs
# retrieves an icon's option. the values of the three special options are
# taken from the icon's record, all others from the icon's text label.
metawidget proc Iconbox iconcget { sIcon args } {
  _findIcon $sIcon

# Arthur: Added following line.
  set sOption [lindex $args 0]
# Arthur: Modified the -icon => image amd -smallicon to -smallimage
# Arthur: Changed all sItem => sIcon.
  switch -- $sOption {
    -image      { return [lindex [my aIcons($sIcon)] 0] }
    -smallimage { return [lindex [my aIcons($sIcon)] 1] }
    -values    { return [lindex [my aIcons($sIcon)] 2] }
    -user      { return [lindex [my aIcons($sIcon)] 4] }
    default    {
      set wIcon [lindex [my aIcons($sIcon)] 3]
      eval $wIcon.l cget $args
    }
  }
}

# name: iconbind
# args: sIcon: icon name
#       args: as accepted by the bind command
# defines bindings for an icon. all bindings must be stored because for
# -view == list, the icons are displayed differently (plain text, not
# the frame and the labels).
metawidget proc Iconbox iconbind { sIcon args } {
  _findIcon $sIcon

  # for each icon, maintain a variable containing sequence-script
  # pairs. if $sScript is empy, remove it. if $sScript start with +,
  # append it. otherwise reset list with given binding.
  if { [llength $args] == 2 } {
    mkw.lassign $args sBind sScript
    if { $sScript == {} } {
      unmy lBind.$sIcon.$sBind
    } elseif { [string index $sScript 0] == "+" } {
      my lBind.$sIcon.$sBind [linsert [my lBind.$sIcon.$sBind] end $sScript]
    } else {
      my lBind.$sIcon.$sBind [list $sScript]
    }
  }

  # apply binding to image and text label
  set wIcon [lindex [my aIcons($sIcon)] 3]
  eval bind $wIcon.i $args
  eval bind $wIcon.l $args
}

# name: selection
# args: sAction: set, get or clear
#       args: depends on sAction
# modifies the selection. because icons are made with widgets and inserted
# into the text, the text's sel tag cannot be used. rather the background of
# the text label is changed.
metawidget proc Iconbox selection { sAction args } {
  switch [mkw.complete $sAction {clear get set}] {
    clear {
      $this.text tag remove sel 1.0 end
      _selIcon * 0
    }

    get {
      return [my lSel]
    }

    set {
      $this.text tag remove sel 1.0 end
      _selIcon * 0

      foreach sIcon [lindex $args 0] {
        _findIcon $sIcon
        _selIcon $sIcon 1

        if { [my -view] == "list" } {
          mkw.lassign [$this.text tag ranges $sIcon] sI1 sI2
          $this.text tag add sel $sI1 $sI2
        }
      }
    }
  }
}

# name: sort
# args: args: as accepted by lsort, plus -column
# sorts the icons, either by their name or by one of the columns.
metawidget proc Iconbox sort { args } {
  set args [mkw.options $args * {-column *}]

  # build table with two columns: one is icon name, second is sort criteria
  set lData {}
  if { ! [info exists -column] } {
    # if no column is specified, just take the icon's caption
    foreach sIcon [my lIcons] {
      set wIcon [lindex [my aIcons($sIcon)] 3]
      lappend lData [list $sIcon [$wIcon.l cget -text]]
    }
  } else {
    set iPos [lsearch [my lCols] ${-column}]
    if { $iPos == -1 } {
      error "Iconbox $this: Invalid column ${-column}"
    }

    # otherwise, extract the value specified by -column
    foreach sIcon [my lIcons] {
      lappend lData [list $sIcon [lindex [lindex [my aIcons($sIcon)] 2] $iPos]]
    }
  }

  # sort it. the first column now contains the icons in sorted order
  set lData [eval lsort $args -index 1 \$lData]

  # extract first column, rebuilt icon list, and redraw
  my lIcons {}
  foreach lRow $lData {
    my lIcons [linsert [my lIcons] end [lindex $lRow 0]]
  }

  _redraw
}

# name: see
# args: sIcon: icon names
# makes an icon visible. simply transforms the cell speci into a
# real text index and calls the "see" command.
metawidget proc Iconbox see { sIcon } {
  set iPos [_checkItem $sIcon]

  if { [my -view] == "list" } {
    $this.cols see 1.0
    $this.text see [expr $iPos+1].0
  } else {
    $this.text see 1.$iPos
  }
}

# name: bind
# args: args: as accepted by the bind command.
# applies bindings to the text widget itself.
metawidget proc Iconbox bind_ { args } {
  eval bind $this.text $args
}

# name: size
# args: -
# returns the number of rows.
metawidget proc Iconbox size {} {
  return [llength [my lIcons]]
}

# name: names
# args: -
# returns the icon names.
metawidget proc Iconbox names {} {
  return [my lIcons]
}

metawidget command Iconbox _sort       _sort
metawidget command Iconbox _dragX      _dragX
metawidget command Iconbox _moveX      _moveX
metawidget command Iconbox _grab       _grab
metawidget command Iconbox _drag       _drag
metawidget command Iconbox _drop       _drop
metawidget command Iconbox _select     _select
metawidget command Iconbox _select2    _select2
metawidget command Iconbox _redraw     _redraw

metawidget command Iconbox insert      insert
metawidget command Iconbox delete      delete
metawidget command Iconbox iconconfigure iconconfigure
metawidget command Iconbox iconcget    iconcget
metawidget command Iconbox iconbind    iconbind
metawidget command Iconbox selection   selection
metawidget command Iconbox sort        sort
metawidget command Iconbox see         see
metawidget command Iconbox bind        bind_
metawidget command Iconbox size        size
metawidget command Iconbox names       names

metawidget option  Iconbox -columns    -columns
metawidget option  Iconbox -view       -view
metawidget option  Iconbox -update     -update
metawidget option  Iconbox -selectmode -selectmode
metawidget option  Iconbox -font       -font
metawidget option  Iconbox -onselect

proc testGetFiles {} {
  global sPath
  if { $sPath == "/" } { set sPath "" }

  .ibox delete

  foreach sFile [glob -nocomplain $sPath/*] {
    set sTail [file tail $sFile]
    set iSize [file size $sFile]
    set sMod [clock format [file mtime $sFile] -format {%D %H:%M}]

    set sIcon $sFile

    if { [file isfile $sFile] } {
      .ibox insert end $sIcon -text $sTail -image pFI -smallimage pFi -values [list $iSize $sMod]
    } else {
      .ibox insert end $sIcon -text $sTail -image pFO -smallimage pFo -values [list {} $sMod]
      .ibox iconbind $sIcon <Double-1> "set sPath {$sIcon}; testGetFiles"
    }
  }

  .ibox bind <Key-Return> {
     set sPath [lindex [.ibox selection get] 0]
     testGetFiles
  }
}

proc test {} {
  image create photo pFo -file ./demos/images/Folder.gif
  image create photo pFi -file ./demos/images/File.gif
  image create photo pFO -file ./demos/images/Folder32.gif
  image create photo pFI -file ./demos/images/File32.gif

  pack [label .head -text "Enter a path below and press Browse"] -side top -anchor w
  pack [frame .ctrl] -side top -fill x
  pack [button .ctrl.list -text "List"  -command {.ibox config -view list}] -side right
  pack [button .ctrl.smll -text "Small" -command {.ibox config -view small}] -side right
  pack [button .ctrl.lrge -text "Large" -command {.ibox config -view large}] -side right
  pack [button .ctrl.back -text "Back"  -command {set sPath [file dirname $sPath]; testGetFiles}] -side right
  pack [button .ctrl.brws -text "Browse" -command testGetFiles] -side right
  pack [entry  .ctrl.path -textvariable sPath] -side left -fill x -expand 1
  pack [iconbox .ibox -columns {{Name l 100} {Size r} {Modified}}] -side bottom -fill both -expand 1
}

#test
