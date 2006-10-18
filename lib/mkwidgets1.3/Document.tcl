package require Metawidget

# name: Document
# args: args: option-value pairs
# defines icons for the window title bar. defines a default menu and creates
# all standard window elements (MS Windows look&feel). declares option values.
metawidget create Document {
  # if first object is being created, define some icons etc.
  if { ! [ourinfo exists lAllDocs] } {
    our lAllDocs {}          ;# list of all document windows
    our wFocus {}            ;# topmost window (drawn active)

    # set platform dependend colors and cursor shapes
    if { $::tcl_platform(platform) == "windows" } {
      ourarray set aColors {gray50 SystemDisabledText white SystemWindow darkblue SystemHighlight gray SystemButtonFace}
      ourarray set aCursors {{} {} nw size_nw_se ne size_ne_sw sw size_ne_sw se size_nw_se n size_ns s size_ns w size_we e size_we}
    } else {
      ourarray set aColors {gray50 gray50 white white darkblue darkblue gray gray}
      ourarray set aCursors {{} {} nw top_left_corner ne top_right_corner sw bottom_left_corner se bottom_right_corner n sb_v_double_arrow s sb_v_double_arrow w sb_h_double_arrow e sb_h_double_arrow}
    }

    # x/y-offset from top left corner
    our iOffset 0

    our pIconMin [image create bitmap -data {
      #define _width 8
      #define _height 8
      static char _bits[] = { 0x00,0x00,0x00,0x00,0x00,0x3e,0x3e,0x00 };
    }]
    our pIconMax [image create bitmap -data {
      #define _width 8
      #define _height 8
      static char _bits[] = { 0x7f,0x7f,0x41,0x41,0x41,0x41,0x7f,0x00 };
    }]
    our pIconRestore [image create bitmap -data {
      #define _width 8
      #define _height 8
      static char _bits[] = { 0x7c,0x7c,0x44,0x5f,0x5f,0x71,0x11,0x1f };
    }]
    our pIconClose [image create bitmap -data {
      #define _width 8
      #define _height 8
      static char _bits[] = { 0x00,0x63,0x36,0x1c,0x1c,0x36,0x63,0x00 };
    }]
  }

  # define popup menu
  menu $this.menu -tearoff 0
#  $this.menu add command -label Restore  -command "$this _restore"
#  $this.menu add command -label Minimize -command "$this _minimize"
#  $this.menu add command -label Maximize -command "$this _maximize"
#  $this.menu add command -label Restore  -command "::${this}::_restore"
  $this.menu add command -label Minimize -command "::${this}::_minimize"
  $this.menu add command -label Maximize -command "::${this}::_maximize"
  $this.menu add separator
  $this.menu add command -label Close -command "destroy $this"

  # create title bar elements
  pack [frame $this.head -border 0] -padx 2 -pady 2 -fill x
  pack [label $this.head.icon -border 0] -side left -padx 1
  pack [label $this.head.clob -border 2 -relief raised -image [our pIconClose]] -side right -pady 2 -padx 2
  pack [label $this.head.maxb -border 2 -relief raised -image [our pIconMax]] -side right -pady 2
  pack [label $this.head.minb -border 2 -relief raised -image [our pIconMin]] -side right -pady 2
  pack [label $this.head.text -border 0 -anchor w] -side left -fill x -expand 1

  # work area where all inner widgets will be go
  pack [frame $this.work -border 0] -fill both -expand 1 -padx 2 -pady 2

  # make sure mouse cursor resets when not over the boarder
  bind $this.work <Enter> "$this config -cursor {}"
  bind $this.head <Enter> "$this config -cursor {}"

  # option values, self explaining
  my -title $this
  my -foreground [our aColors(white)]
  my -background [our aColors(darkblue)]
  my -font [$this.head.text cget -font]
  my -minsize {100 30}
  my -icontext {}
  my -state normal
  my -image {}

  # position and size of the window. note that later on, -x and -y contain
  # either the icon or the window position, depending on the window's state.
  my iNormX [our iOffset]
  my iNormY [our iOffset]
  my -width  250
  my -height 150

  # bind menu to icon and close button
  bind $this.head.icon <1>        "$this _showMenu"
  bind $this.head.icon <Double-1> "$this.menu invoke Close"
  bind $this.head.clob <1>        "$this.menu invoke Close"

  # increase offset for next window
  our iOffset [expr [our iOffset]+16]
  if { [expr [our iOffset]+64] > [winfo height [winfo parent $this]] } {
    our iOffset 0
  }

  # show in normal mode and put it on top
  _restore
  raise_

  # add it to the list of all documents
  _register
} {
  if { [our wFocus] == $this } {
    our wFocus {}
  }

  # remove it from all documents
  _unregister
} -border 2 -relief raised

# name: _register
# args: o Document object to register
# adds the current object to a list. required for geometry management.
metawidget proc Document _register {} {
  our lAllDocs [linsert [our lAllDocs] end $this]
}

# name: _unregister
# args: o Document object to unregister
# removes the object from this list.
metawidget proc Document _unregister {} {
  our lAllDocs [mkw.lshrink [our lAllDocs] $this]
}

# name: _showMenu
# args: -
# displays the popup menu right under the document's icon
metawidget proc Document _showMenu {} {
  $this.menu post [winfo rootx $this.head.icon] \
    [expr [winfo height $this.head.icon] + [winfo rooty $this.head.icon]]

  bind $this.menu <FocusOut> "$this.menu unpost"
}

# name: _getEdge
# args: iXp, iYp: mouse position
# called only when the mouse is over the document's border. returns an area
# indicator of the mouse position, e.g. sw for south-west = bottom left.
# used to set the mouse cursor and to determine the resize mode.
metawidget proc Document _getEdge { iXp iYp } {
  incr iXp -[winfo rootx $this]
  incr iYp -[winfo rooty $this]

  # allow max. 16 pixel for edge detection
  set iXEdge [expr ([my -width ] < 32)?[my -width ]>>1:16]
  set iYEdge [expr ([my -height] < 32)?[my -height]>>1:16]

  # detect if mouse is on one of the edges (x coordinate)
  if { $iXp < $iXEdge } {
    set sPosH w
  } elseif { $iXp > [expr [my -width]-$iXEdge] } {
    set sPosH e
  } else {
    set sPosH {}
  }

  # detect if mouse is on one of the edges (y coordinate)
  if { $iYp < $iYEdge } {
    set sPosV n
  } elseif { $iYp > [expr [my -height]-$iYEdge] } {
    set sPosV s
  } else {
    set sPosV {}
  }

  return $sPosV$sPosH
}

# name: _motion
# args: iXp, iYp: mouse position (from the %X and %Y fields of an event)
# called by a <Motion> binding over the document frame. Sets the mouse cursor
# to show arrows depending on the region of the mouse. _getEdge returns an
# area (e.g. ne for north-east = upper right corner), the aCursor array
# maps this to a valid cursor name (depending on platform).
metawidget proc Document _motion { iXp iYp } {
  $this config -cursor [our aCursors([_getEdge $iXp $iYp])]
}

# name: _move1
# args: iXp, iYp: mouse position (from the %X and %Y fields of an event)
# called on a <Button-1> event on the title bar. stores the current mouse
# pointer position. raises the document.
metawidget proc Document _move1 { iXp iYp } {
  raise_

  set wParent [winfo parent $this]
  my iXr1 [winfo rootx $wParent]
  my iYr1 [winfo rooty $wParent]
  my iXr2 [expr [my iXr1]+[winfo width  $wParent]]
  my iYr2 [expr [my iYr1]+[winfo height $wParent]]

  my iXp0 $iXp
  my iYp0 $iYp
}

# name: _move2
# args: iXp, iYp: mouse position (from the %X and %Y fields of an event)
# called on a <B1-Motion> event on the title bar. calculates the new
# window position and places it there.
metawidget proc Document _move2 { iXp iYp } {
  if { $iXp < [my iXr1] } { set iXp [my iXr1] }
  if { $iYp < [my iYr1] } { set iYp [my iYr1] }
  if { $iXp > [my iXr2] } { set iXp [my iXr2] }
  if { $iYp > [my iYr2] } { set iYp [my iYr2] }

  place $this -x [expr [my -x]-[my iXp0]+$iXp] -y [expr [my -y]-[my iYp0]+$iYp]
}

# name: _move3
# args: -
# called on a <ButtonRelease-1> event on the title bar. stores the actual
# window position in some private variables.
metawidget proc Document _move3 {} {
  my -x [winfo x $this]
  my -y [winfo y $this]
}

# name: _resize1
# args: iXp, iYp: mouse position (from the %X and %Y fields of an event)
# called on a <1> event on one of the borders. stores the current mouse
# position. depending on the mouse position (at an edge or not), the
# resize mode is determined.
metawidget proc Document _resize1 { iXp iYp } {
  raise_

  my iWMin [lindex [my -minsize] 0]
  my iHMin [lindex [my -minsize] 1]

  my iXMax [expr [my -x]+[my -width] -[my iWMin]]
  my iYMax [expr [my -y]+[my -height]-[my iHMin]]

  my iXp0 $iXp
  my iYp0 $iYp

  set sPos [_getEdge $iXp $iYp]
  set sXPos {}; regexp {[we]} $sPos sXPos; my sXPos $sXPos
  set sYPos {}; regexp {[ns]} $sPos sYPos; my sYPos $sYPos
}

# name: _resize2
# args: iXp, iYp: mouse position (from the %X and %Y fields of an event)
# called on a <B1-Motion> event on the borders. calculates the new
# Document sizes and position and places it depending on the resize mode.
metawidget proc Document _resize2 { iXp iYp } {
  # sometimes the <1> event and _resize1 didn't precede this one...
  if { ! [myinfo exists sXPos] } {
    return
  }

  if { [my sXPos] == "w" } {
    set iX [expr [my -x]-[my iXp0]+$iXp]
    if { $iX > [my iXMax] } { set iX [my iXMax] }

    set iW [expr [my -width]+[my iXp0]-$iXp]
    if { $iW < [my iWMin] } { set iW [my iWMin] }

    place $this -x $iX -width $iW
  } elseif { [my sXPos] == "e" } {
    set iW [expr [my -width]-[my iXp0]+$iXp]
    if { $iW < [my iWMin] } { set iW [my iWMin] }

    place $this -width $iW
  }

  if { [my sYPos] == "n" } {
    set iY [expr [my -y]-[my iYp0]+$iYp]
    if { $iY > [my iYMax] } { set iY [my iYMax] }

    set iH [expr [my -height]+[my iYp0]-$iYp]
    if { $iH < [my iHMin] } { set iH [my iHMin] }

    place $this -y $iY -height $iH
  } elseif { [my sYPos] == "s" } {
    set iH [expr [my -height]-[my iYp0]+$iYp]
    if { $iH < [my iHMin] } { set iH [my iHMin] }

    place $this -height $iH
  }
}

# name: _resize3
# args: -
# called on a <ButtonRelease-1> event on the borders. stores the actual
# window size and position in some private variables.
metawidget proc Document _resize3 {} {
  my -x      [winfo x      $this]
  my -y      [winfo y      $this]
  my -width  [winfo width  $this]
  my -height [winfo height $this]
}

# name: _withdraw
# args: -
# simply lets the Document disappear.
metawidget proc Document _withdraw {} {
  place forget $this
  my -state withdrawn
}

# name: _maximize
# args: -
# maximizes the document. exchanges the icon for the maximize button.
# deletes bindings for resize and move.
metawidget proc Document _maximize {} {
  # if it was minimized or withdrawn: set to default
  # if we come from normal state, store the window position
  if { [my -state] == "normal" } {
    my iNormX [my -x]
    my iNormY [my -y]
  } else {
    _restore
  }

  # place it to fill entirely its master
  place $this -x 0 -y 0 -width 0 -height 0 -relwidth 1 -relheight 1
  $this.head.maxb configure -image [our pIconRestore]
  $this.menu entryconf Maximize -state disabled
  $this configure -border 0

  # delete bindings for resizing
  bind $this <1> {}
  bind $this <B1-Motion> {}
  bind $this <ButtonRelease-1> {}

  # modify/delete bindings for moving
  bind $this.head.text <1> "$this raise"
  bind $this.head.text <B1-Motion> {}
  bind $this.head.text <ButtonRelease-1> {}

  # delete binding for mouse cursor changes over borders
  bind $this <Motion> {}

  # change bindings for maximize button and title bar
  bind $this.head.text <Double-1> "$this _restore"
  bind $this.head.maxb <1> "$this _restore"

  my -state maximized
}

# name: _minimize
# args: -
# minimizes the document. exchanges the icon for the minimize button.
# deletes bindings for resize.
metawidget proc Document _minimize {} {
  # if it was maximized or withdrawn: set to default
  # if we come from normal state, store the window position
  if { [my -state] == "normal" } {
    my iNormX [my -x]
    my iNormY [my -y]
  } else {
    _restore
  }

  # morph -x and -y to contain the icon position
  if { [myinfo exists iIconX] } {
    my -x [my iIconX]
    my -y [my iIconY]
  } else {
    my -x [my iNormX]
    my -y [my iNormY]
  }

  # withdraw the document's working area
  pack forget $this.work
  $this.head.minb configure -image [our pIconRestore]
  $this.menu entryconf Minimize -state disabled

  # eventually change title text
  if { [my -icontext] != {} } {
    $this.head.text configure -text [my -icontext]
  }

  place $this -x [my -x] -y [my -y] -width 128 -height [expr [winfo reqheight $this.head] + 8] -relwidth 0 -relheight 0

  # delete bindings for resizing
  bind $this <1> {}
  bind $this <B1-Motion> {}
  bind $this <ButtonRelease-1> {}

  # delete binding for mouse cursor changes over borders
  bind $this <Motion> {}

  # change bindings for minimize button and title bar
  bind $this.head.text <Double-1> "$this _restore"
  bind $this.head.minb <1> "$this _restore"

  my -state minimized
}

# name: _restore
# args: -
# restores a Document from any of the other states. assigns the regular
# bindings to all widgets.
metawidget proc Document _restore {} {
  # if we come from iconized state, store the icon position
  if { [my -state] == "minimized" } {
    my iIconX [my -x]
    my iIconY [my -y]
  }

  # morph -x and -y to contain the window position
  my -x [my iNormX]
  my -y [my iNormY]

  # show the working area and place Document on last known position
  pack  $this.work -fill both -expand 1 -padx 2 -pady 2
  place $this -x [my -x] -y [my -y] -width [my -width] -height [my -height] -relwidth 0 -relheight 0

  # restore icons and menu entries for all title bar elements
  $this.head.minb configure -image [our pIconMin]
  $this.head.maxb configure -image [our pIconMax]
  $this.head.text configure -text [my -title]
  $this.menu entryconf Minimize -state normal
  $this.menu entryconf Maximize -state normal
  ::${this}::$this config -border 2

  # restore resize bindings
  bind $this <1>               "$this _resize1 %X %Y"
  bind $this <B1-Motion>       "$this _resize2 %X %Y"
  bind $this <ButtonRelease-1> "$this _resize3"

  # restore move bindings
  bind $this.head.text <1>               "$this _move1 %X %Y"
  bind $this.head.text <B1-Motion>       "$this _move2 %X %Y"
  bind $this.head.text <ButtonRelease-1> "$this _move3"

  # restore maximize and minimize bindings
  bind $this.head.maxb <1>        "$this _maximize"
  bind $this.head.minb <1>        "$this _minimize"
  bind $this.head.text <Double-1> "$this _maximize"

  # restore binding for mouse cursor changes over borders
  bind $this <Motion> "$this _motion %X %Y"

  my -state normal
}

# name: -x et al.
# args: iX, iY, iWidth iHeight: geometry values
# sets a new size or position of the Document window. -x and -y apply
# regardless if the window is iconified or not, but -width and -height
# only affect the window in normal state.
metawidget proc Document -x { iX } {
  my -x $iX
  place $this -x [my -x]
}
metawidget proc Document -y { iY } {
  my -y $iY
  place $this -y [my -y]
}
metawidget proc Document -width { iWidth } {
  my -width $iWidth

  if { [my -state] == "normal" } {
    place $this -width [my -width]
  }
}
metawidget proc Document -height { iHeight } {
  my -height $iHeight

  if { [my -state] == "normal" } {
    place $this -height [my -height]
  }
}

# name: -font
# args: sFont: font to set
# option handler. sets the title font.
metawidget proc Document -font { sFont } {
  my -font $sFont
  $this.head.text configure -font $sFont
}

# name: -image
# args: sImage: image to set
# option handler. sets the image in the title bar.
metawidget proc Document -image { sImage } {
  my -image $sImage
  $this.head.icon configure -image $sImage
}

# name: -title
# args: sTitle: title to set
# option handler. sets title text.
metawidget proc Document -title { sTitle } {
  my -title $sTitle
  $this.head.text configure -text $sTitle
}

# name: -state
# args: sState: new Document state
# option handler. sets the state for the document. the variable -state
# is set in the respective (private) member functions.
metawidget proc Document -state { sState } {
  switch -- [mkw.complete $sState {normal minimized maximized withdrawn}] {
    normal    { _restore  }
    minimized { _minimize }
    maximized { _maximize }
    withdrawn { _withdraw }
  }
}

# name: menu
# args: args: arguments for the regular menu command.
# simply passes all arguments to the popup menu.
metawidget proc Document menu_ { args } {
  eval $this.menu $args
}

# name: lower
# args: -
# draws the title bar in grey.
metawidget proc Document lower_ {} {
  $this.head configure -bg [our aColors(gray50)]
  $this.head.icon configure -bg [our aColors(gray50)]
  $this.head.text configure -bg [our aColors(gray50)] -fg [our aColors(gray)]
}

# name: raise
# args: -
# puts the Document on top and draws the title bar in different colors.
metawidget proc Document raise_ {} {
  catch { [our wFocus] lower }
  our wFocus $this

  $this.head configure -bg [my -background]
  $this.head.icon configure -bg [my -background]
  $this.head.text configure -bg [my -background] -fg [my -foreground]

  raise $this
}

# name: pack
# args: sWindow: widget to be packed
#       args: option-value pairs
# redirects packing to the work area widget. the binding is required
# to reset the mouse cursor when it enters the work area. this is
# only required if the packed widget is a logical child.
metawidget proc Document pack_ { sWindow args } {
  eval pack $sWindow -in $this.work $args

  if { [winfo parent $sWindow] == $this } {
    bind $sWindow <Enter> "+$this config -cursor {}"
  }
}

# name: grid
# args: sWindow: widget to be gridded
#       args: option-value pairs
# redirects gridding to the work area widget.
metawidget proc Document grid_ { sWindow args } {
  eval grid $sWindow -in $this.work $args

  if { [winfo parent $sWindow] == $this } {
    bind $sWindow <Enter> "+$this config -cursor {}"
  }
}

# name: place
# args: sWindow: widget to be placed
#       args: option-value pairs
# redirects placing to the work area widget.
metawidget proc Document place_ { sWindow args } {
  eval place $sWindow -in $this.work $args

  if { [winfo parent $sWindow] == $this } {
    bind $sWindow <Enter> "+$this config -cursor {}"
  }
}

# name: Arrange
# args: sHow: arrange mode
#       sParent: a valid parent widget of document windows(s).
# static member function. resizes and repositions all Document objects
# in one of several ways. it always arrange iconfied windows before
# it repositions the non-iconfied windows.
metawidget proc Document Arrange { sHow } {
  variable lAllDocs

  set sHow [mkw.complete $sHow {horizontally vertically cascade tile icons maximize minimize}]

  # first check if at least one document window exists
  if { ! [info exists lAllDocs] || ! [llength $lAllDocs] } return

  # get a list of all iconified windows and a list of all others
  set lDocs {}
  set lIcon {}
  foreach sDoc $lAllDocs {
    if { [$sDoc cget -state] == "minimized" } {
      lappend lIcon $sDoc
    } else {
      lappend lDocs $sDoc
    }
  }

  # get parent window's dimensions for later calculation
  set sParent [winfo parent [lindex $lAllDocs 0]]
  set iW [winfo width  $sParent]
  set iH [winfo height $sParent]

  # if there are iconfied windows, arrange them first and reduce iH
  set iXd 0
  foreach sDoc $lIcon {
    set iWd [winfo width $sDoc]
    if { [expr $iXd+$iWd] > $iW } {
      set iXd 0
    }
    if { $iXd == 0 } {
      incr iH -[winfo height $sDoc]
    }
    $sDoc configure -x $iXd -y $iH
    incr iXd $iWd
  }

  # how many non-iconified windows? just exit when none
  set iDocs [llength $lDocs]
  if { $iDocs == 0 } return

  switch $sHow {
    horizontally {
      set iStep [expr $iH/$iDocs]
      for { set i 0 } { $i < $iDocs } { incr i } {
        set sDoc [lindex $lDocs $i]
        $sDoc configure -x 0 -y [expr $i*$iStep] -width $iW -height $iStep
      }
    }

    vertically {
      set iStep [expr $iW/$iDocs]
      for { set i 0 } { $i < $iDocs } { incr i } {
        set sDoc [lindex $lDocs $i]
        $sDoc configure -x [expr $i*$iStep] -y 0 -width $iStep -height $iH
      }
    }

    cascade {
      set iStep 24
      for { set i 0 } { $i < $iDocs } { incr i } {
        set sDoc [lindex $lDocs $i]
        $sDoc configure -x [expr $i*$iStep] -y [expr $i*$iStep] -width [expr $iW*2/3] -height [expr $iH*2/3]
        $sDoc raise
      }
    }

    tile {
      for { set i 0 } { $i <= 100 } { incr i } {
        set iHor [expr ($i+3)/2]
        set iVer [expr ($i+2)/2]
        if { [expr $iHor*$iVer] >= $iDocs } break
      }

      set iXStep [expr $iW/$iHor]
      set iYStep [expr $iH/$iVer]
      for { set i 0 } { $i < $iDocs } { incr i } {
        set sDoc [lindex $lDocs $i]
        $sDoc configure -x [expr ($i%$iHor)*$iXStep] -y [expr ($i/$iHor)*$iYStep] -width $iXStep -height $iYStep
      }
    }

    maximize {
      foreach sDoc $lDocs {
        $sDoc _maximize
      }
    }

    minimize {
      foreach sDoc $lDocs {
        $sDoc _minimize
      }
    }
  }
}

metawidget command Document _showMenu _showMenu
metawidget command Document _move1    _move1
metawidget command Document _move2    _move2
metawidget command Document _move3    _move3
metawidget command Document _resize1  _resize1
metawidget command Document _resize2  _resize2
metawidget command Document _resize3  _resize3
metawidget command Document _withdraw _withdraw
metawidget command Document _maximize _maximize
metawidget command Document _minimize _minimize
metawidget command Document _restore  _restore
metawidget command Document _motion   _motion

metawidget command Document menu      menu_
metawidget command Document lower     lower_
metawidget command Document raise     raise_
metawidget command Document pack      pack_
metawidget command Document grid      grid_
metawidget command Document place     place_
metawidget command Document Arrange   Arrange

metawidget option  Document -x          -x
metawidget option  Document -y          -y
metawidget option  Document -width      -width
metawidget option  Document -height     -height
metawidget option  Document -font       -font
metawidget option  Document -image      -image
metawidget option  Document -title      -title
metawidget option  Document -state      -state
metawidget option  Document -foreground
metawidget option  Document -background
metawidget option  Document -minsize
metawidget option  Document -icontext

proc test {} {
  pack [frame .w -back darkgray] -fill both -expand 1
  document .w.d1
  document .w.d2
  document .w.d3

  .w.d3 pack [label .w.d3.f -border 2 -relief sunken -text "1"] -fill both -expand 1
  pack [label .w.d3.f.f -border 2 -relief raised -text "2"] -fill both -expand 1
}

#test