package require Metawidget

# name: Toolbar
# args: args: option-value pairs
# creates inner widgets and bindings. initializes variables.
metawidget create Toolbar {
  # padding of 2 pixel between outer frame and the buttons
  pack [frame $this.frm -border 2 -relief flat] -side top -fill both -expand 1

  # popup menu for right mouse key
  menu $this.menu -tearoff 0
  $this.menu add command -label Top    -command "$this config -side top"
  $this.menu add command -label Bottom -command "$this config -side bottom"
  $this.menu add command -label Left   -command "$this config -side left"
  $this.menu add command -label Right  -command "$this config -side right"

  # this is for X11 only. it's a noop on Windows and Mac
  bind $this.menu <FocusOut> "$this.menu unpost"

  # new class bindings make Toolbar draggable
  bind Toolbar$this <1>         "$this _click"
  bind Toolbar$this <B1-Motion> "$this _drag %X %Y"
  bind Toolbar$this <3>         "$this.menu post %X %Y; focus $this.menu"

  # buttons get a nice roll-over effect
  bind Toolbutton$this <Enter> {%W config -relief raised}
  bind Toolbutton$this <Leave> {%W config -relief flat}

  my wParent [winfo parent $this]

  # set options by calling their member functions
  -side top
  -state normal
} {} -default frm -border 1 -relief raised

# name: _click
# args: -
# called when clicked. stores the coordinates of the parent window.
metawidget proc Toolbar _click {} {
  my iX1 [expr [winfo rootx [my wParent]]+20]
  my iY1 [expr [winfo rooty [my wParent]]+20]
  my iX2 [expr [my iX1]+[winfo width  [my wParent]]-40]
  my iY2 [expr [my iY1]+[winfo height [my wParent]]-40]
}

# name: _drag
# args: iPx, iPy: mouse coordinates
# called when dragged. detects if the mouse is outside of any of the four
# sides of the parent window and places the Toolbar accordingly.
metawidget proc Toolbar _drag { iPx iPy } {
  if { $iPx < [my iX1] && [my -side] != "left" } {
    -side left
  } elseif { $iPx > [my iX2] && [my -side] != "right" } {
    -side right
  } elseif { $iPy < [my iY1] && [my -side] != "top" } {
    -side top
  } elseif { $iPy > [my iY2] && [my -side] != "bottom" } {
    -side bottom
  }
}

# name: _checkItem
# args: sTag: item tag
# throws an exception if the given tag does not exists. tests on
# existence of a private variable.
metawidget proc Toolbar _checkItem { sTag } {
  if { ! [myinfo exists _$sTag.wWidget] } {
    error "Toolbar $this: Item $sTag does not exist."
  }
}

# name: _redraw
# args: -
# redraws the Toolbar. computes the values for the -fill and -side options
# and the first child of the parent. packs the Toolbar before that child.
metawidget proc Toolbar _redraw {} {
  set sFill  [mkw.decode [my -side] "top x bottom x left y right y"]
  set sInner [mkw.decode [my -side] "top left bottom left left top right top"]
  set sFirst [lindex [pack slaves [my wParent]] 0]

  # pack Toolbar *before* first child (if any)
  if { $sFirst == {} } {
    pack $this -side [my -side] -fill $sFill
  } elseif { [lsearch [bindtags $sFirst] "Statusbar"] != -1 } {
    pack $this -side [my -side] -fill $sFill -after $sFirst
  } else {
    pack $this -side [my -side] -fill $sFill -before $sFirst
  }

  # pack the buttons inside the Toolbar.
  foreach sWidget [winfo children $this.frm] {
    if { [winfo class $sWidget] == "Frame" } {
      # special treatment for separator frames
      if { $sFill == "x" } {
        pack $sWidget -side $sInner -fill y -padx 4 -pady 1
      } else {
        pack $sWidget -side $sInner -fill x -padx 1 -pady 4
      }
    } else {
      # all other buttons
      pack $sWidget -side $sInner -fill both
    }
  }
}

# name: _bindTag
# args: sWhat: operation (extend or shrink)
#       wWidget: widget to modify
#       sBindTag: bindtag to add or remove
# adds or removes a bindtag to a widget.
metawidget proc Toolbar _bindTag { sWhat wWidget sBindTag } {
  bindtags $wWidget [mkw.l$sWhat [bindtags $wWidget] $sBindTag]
}

# name: _checkButton
# args: sTag: item tag
#       iState: optional new state (boolean)
# handles checkbuttons. makes them look "checked" (=sunken) respectively
# normal again.
metawidget proc Toolbar _checkButton { sTag {iState {}} } {
  # either toggle state or set new state from iState
  if { $iState == {} } {
    my _$sTag.iState [expr ! [my _$sTag.iState]]
  } else {
    my _$sTag.iState [mkw.complete $iState {on off yes no 1 0}]
  }

  # change relief and add resp. remove bind tag for roll-over effect
  if { [my _$sTag.iState] } {
    [my _$sTag.wWidget] config -relief sunken
    _bindTag shrink [my _$sTag.wWidget] Toolbutton$this
  } else {
    [my _$sTag.wWidget] config -relief flat
    _bindTag extend [my _$sTag.wWidget] Toolbutton$this
  }
}

# name: _radioButton
# args: sTag: item tag
# handles radiobuttons. sets all buttons of a group to normal state and
# then makes the clicked one look checked.
metawidget proc Toolbar _radioButton { sTag } {
  set wButton $this.frm._$sTag

  # reset all radiobuttons of the group
  foreach sGroupTag [my __[my _$sTag.sGroup]] {
    $this.frm._$sGroupTag config -relief flat
    _bindTag extend $this.frm._$sGroupTag Toolbutton$this
  }

  # then check the activated button
  [my _$sTag.wWidget] config -relief sunken
  _bindTag shrink [my _$sTag.wWidget] Toolbutton$this
}

# name: -side
# args: sSide: side to place Toolbar on
# option member. repositions the Toolbar.
metawidget proc Toolbar -side { sSide } {
  my -side [mkw.complete $sSide {top bottom left right}]
  _redraw
}

# name: -state
# args: sState: new state for the Toolbar
# option member. lets a Toolbar disappear or disables dragging.
metawidget proc Toolbar -state { sState } {
  my -state [mkw.complete $sState {normal fixed withdrawn}]

  switch [my -state] {
    normal {
      _bindTag extend $this.frm Toolbar$this
      -side [my -side]
    }
    fixed {
      _bindTag shrink $this.frm Toolbar$this
      -side [my -side]
    }
    withdrawn {
      pack forget $this
    }
  }
}

# name: add
# args: sType: type of new object
#       sTag: unique item tag
#       args: option-value pairs
# adds a new button, checkbutton, radiobutton or separator.
metawidget proc Toolbar add { sType sTag args } {
  if { [myinfo exists _$sTag.wWidget] } {
    error "Toolbar $this: Item $sTag already exists."
  }

  set sType   [mkw.complete $sType {button checkbutton radiobutton separator}]
  set wWidget $this.frm._$sTag

  # store item attributes in private variables
  my _$sTag.sTag    $sTag
  my _$sTag.sType   $sType
  my _$sTag.wWidget $wWidget
  my _$sTag.iState  0

  if { $sType == "separator" } {
    frame $wWidget -border 1 -relief sunken -width 2 -height 2
    _bindTag extend $wWidget Toolbar$this
  } else {
    button $wWidget -border 1 -relief flat -padx 0 -pady 0 -takefocus 0
    _bindTag extend $wWidget Toolbutton$this

    if { $sType == "checkbutton" } {
      bind $wWidget <1> "$this _checkButton $sTag"
    } elseif { $sType == "radiobutton" } {
      bind $wWidget <1> "$this _radioButton $sTag"

      # extract -group option and append it to a list
      set args [mkw.options $args * {-group *}]
      if { ! [info exists -group] } {
        set -group default
      }

      my _$sTag.sGroup ${-group}
      if { [myinfo exists __${-group}] } {
        my __${-group} [linsert [my __${-group}] end $sTag]
      } else {
        my __${-group} $sTag
      }
    }
  }

  eval itemconf {$sTag} $args

  if { [my -state] != "withdrawn" } {
    _redraw
  }

  return $wWidget
}

# name: delete
# args: args: list of item tags
# deletes Toolbar objects.
metawidget proc Toolbar delete { args } {
  foreach sTag $args {
    if { [my _$sTag.sType] == "radiobutton" } {
      my __[my _$sTag.sGroup] [mkw.lshrink [my __[my _$sTag.sGroup]] $sTag]
    }

    destroy [my _$sTag.wWidget]
    unmy _$sTag.sTag _$sTag.sType _$sTag.wWidget _$sTag.iState _$sTag.sGroup
  }

  _redraw
}

# name: itemconf
# args: sTag: item tag
#       args: option-value pairs
# configures a Toolbar object.
metawidget proc Toolbar itemconf { sTag args } {
  _checkItem $sTag
  eval {$this.frm._$sTag} config $args
}

# name: itemcget
# args: sTag: item tag
#       sOption: option to query
# returns an option value of a Toolbar object.
metawidget proc Toolbar itemcget { sTag sOption } {
  _checkItem $sTag
  $this.frm._$sTag cget $sOption
}

# name: set
# args: sTag: item tag
#       iState: new state for checkbuttons
# similar to invoke, but the button's command is not invoked,
# just the toolbutton is set.
metawidget proc Toolbar set_ { sTag {iState {}} } {
  _checkItem $sTag

  if { [my _$sTag.sType] == "checkbutton" } {
    _checkButton $sTag $iState
  } elseif { [my _$sTag.sType] == "radiobutton" } {
    _radioButton $sTag
  }
}

# name: get
# args: sTag: item tag
# returns the state of a button. always 0 for regular buttons.
# 0 or 1 for check- and radiobuttons.
metawidget proc Toolbar get { sTag } {
  _checkItem $sTag
  return [my _$sTag.iState]
}

# name: invoke
# args: sTag: item tag
#       iState: new state for checkbuttons
# invokes a button. if iState is specified, it sets a checkbutton to the
# corresponding value. does not apply for buttons and radiobuttons, since
# there state is always known.
metawidget proc Toolbar invoke { sTag {iState {}} } {
  _checkItem $sTag
  set_ $sTag $iState
  $this.frm._$sTag invoke
}

# name: menu
# args: args: arguments for the regular menu command.
# simply passes all arguments to the popup menu.
metawidget proc Toolbar menu_ { args } {
  eval $this.menu $args
}

# name: names
# args: -
# returns the tags of all toolbar buttons
metawidget proc Toolbar names { {sPattern *} } {
  set lTags {}

  foreach sVar [myinfo vars _*.sTag] {
    if { ! [string match $sPattern [my $sVar]] } continue
    lappend lTags [my $sVar]
  }

  return $lTags
}

metawidget command Toolbar _click       _click
metawidget command Toolbar _drag        _drag
metawidget command Toolbar _checkButton _checkButton
metawidget command Toolbar _radioButton _radioButton

metawidget command Toolbar add          add
metawidget command Toolbar delete       delete
metawidget command Toolbar itemconf     itemconf
metawidget command Toolbar itemcget     itemcget
metawidget command Toolbar set          set_
metawidget command Toolbar get          get
metawidget command Toolbar invoke       invoke
metawidget command Toolbar menu         menu_
metawidget command Toolbar names        names

metawidget option Toolbar -side  -side
metawidget option Toolbar -state -state

proc test {} {
  wm minsize . 320 250

  image create photo p1 -file ./demos/images/Notebook.gif
  image create photo p2 -file ./demos/images/Compass.gif

  pack [frame .work -bg gray50] -fill both -expand 1
  grid [frame .test] -in .work

  pack [text .test.text -width 40 -height 10 -wrap word]
  toolbar .test.tbar -state fixed -side bottom -relief sunken
  .test.text insert end "Above are regular buttons, checkbuttons and radiobuttons."
  .test.text insert end "The Toolbars can be dragged to the four window sides."
  .test.text insert end "The right mouse button will invoke a menu.\n"

  .test.tbar add button b1 -image p1 -command {.test.text delete 1.0 end }
  .test.tbar add check  b2 -image p2 -command {
    if { [.test.tbar get b2] } {
      .test.text config -bg black -fg white
    } else {
      .test.text config -bg white -fg black
    }
  }

  toolbar .tbar1
  .tbar1 add button button1 -image p2 -command {.test.text insert end "button1 clicked\n"; .test.text see end }
  .tbar1 add button button2 -image p2 -command {.test.text insert end "button2 clicked\n"; .test.text see end }
  .tbar1 add separator s1
  .tbar1 add checkbutton check1 -image p1 -command {.test.text insert end "check1 clicked\n"; .test.text see end }
  .tbar1 add checkbutton check2 -image p1 -command {.test.text insert end "check2 clicked\n"; .test.text see end }
  .tbar1 add separator s2
  .tbar1 add radiobutton radio1 -image p1 -command {.test.text insert end "radio1 clicked\n"; .test.text see end }
  .tbar1 add radiobutton radio2 -image p2 -command {.test.text insert end "radio2 clicked\n"; .test.text see end }
  .tbar1 add radiobutton radio3 -image p1 -command {.test.text insert end "radio3 clicked\n"; .test.text see end }
  .tbar1 add radiobutton radio4 -image p2 -command {.test.text insert end "radio4 clicked\n"; .test.text see end }

  toolbar .t2 -side bottom
  .t2 add button b1 -image p1 -bg red
  .t2 add button b2 -image p2 -bg green
  .t2 add button b3 -image p1 -bg yellow
  .t2 add button b4 -image p2 -bg blue
  .t2 add button b5 -image p1 -bg white
  .t2 add button b6 -image p2 -bg black
}

#test

