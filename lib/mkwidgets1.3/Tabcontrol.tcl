package require Metawidget

# name: Tabcontrol
# args: args: option-value pairs for metawidget configuration
# defines the internal widgets and private variables. initializes the -width
# option by calling its member function.
metawidget create Tabcontrol {
  frame $this.tabs -border 2 -relief flat   ;# contains all rows and tabs
  label $this.actv -border 2 -relief raised ;# the active tab is also a frame
  frame $this.work -border 2 -relief raised ;# the work area
  frame $this.gbar -border 0                ;# a gray line to hide the lower
                                            ;# ...border of the active tab
  my iWorkY    0  ;# y-coordinate where the work area begins
  my iAutoW    0  ;# minimum width of $this.tabs, if -width in auto mode
  my lAllTabs  {} ;# list of all defined tab names
  my sActTab   {} ;# name of the currently active tab
  my wActWin   {} ;# the active tab's associated user widget

  -width 100%

  bind $this <Configure> "$this _drawTabs"  ;# to update tab geometry if resized
} {} -default tabs

# name: _drawTabs
# args: -
# redraws the tabs. called when resized or tabs are configured. basically
# calculates the new geometry of the tabs container $this.tabs, of the
# the active tab $this.actv and the line that hides its lower border.
# the work area's top pixel coordinate is also adjusted.
metawidget proc Tabcontrol _drawTabs {} {
  # position tab container depending on -width option.
  switch [my sWmode] {
    pct { place $this.tabs -x 0 -y 0 -relh 1 -relw [my iWidth] -w 0    }
    abs { place $this.tabs -x 0 -y 0 -relh 1 -w    [my iWidth] -relw 0 }
    req { place $this.tabs -x 0 -y 0 -relh 1 -w    [my iAutoW] -relw 0 }
  }

  # if an active tab is set, places the $this.actv window exactly over it,
  # but sizes it slightly bigger. places the $this.gbar frame so that
  # $this.actv and $this.work look like one window.
  if { [my sActTab] != {} } {
    set iX [winfo x [my wTab]]
    set iY [expr [winfo y [my wRow]]-2]
    set iW [expr [winfo w [my wTab]]+4]
    set iH [expr [winfo h [my wRow]]+2]

    place $this.actv -x $iX -y $iY -w $iW -h $iH
    place $this.gbar -x [expr $iX+1] -y [expr $iY+$iH-2] -w [expr $iW-2] -h 2
    place $this.work -x 0 -y [my iWorkY] -relw 1 -h [expr [winfo height $this.tabs]-[my iWorkY]]
  }
}

# name: _findTab
# args: sTab: Tab name to retrieve data for
# raises an exception, if the tab doesn't exist. otherwise, returns
# the tab's properties: widget name, row, user window and command.
metawidget proc Tabcontrol _findTab { sTab } {
  if { ! [myinfo exists aTabs($sTab)] } {
    error "Tabcontrol $this: No such tab '$sTab'"
  }

  return [my aTabs($sTab)]
}

# name: _calcGeom
# args: -
# calculates the minimum required with of the container $this.tabs, so that
# the labels of all tabs are completely visible. retrieves the 'required with'
# of all tab rows and calculates the longest one. used when -width is 'auto'.
# also calculates the top border of the work area.
metawidget proc Tabcontrol _calcGeom {} {
  update

  set iAutoW 0
  foreach wWindow [winfo children $this.tabs] {
    set iWidth [winfo reqw $wWindow]
    if { $iWidth < $iAutoW } continue
    set iAutoW $iWidth
  }
  my iAutoW [expr $iAutoW+4]

  set iWorkY 2
  foreach wWindow [winfo children $this.tabs] {
    incr iWorkY [expr [winfo reqh $wWindow]-2]
  }
  my iWorkY $iWorkY
}

# name: _updateActive
# args: -
# make the active tab look like the original tab (which stays in its row)
# remember: all tabs are simply packed into their row frame. to make a tab
# look raised, the $this.actv frame is configured to look like the
# to-be-raised tab and placed over it slighly bigger by the _drawTabs function.
metawidget proc Tabcontrol _updateActive {} {
  if { [my sActTab] == {} } return

  $this.actv config -text  [[my wTab] cget -text]  -image  [[my wTab] cget -image]  \
                    -backg [[my wTab] cget -backg] -foreg  [[my wTab] cget -foreg]  \
                    -font  [[my wTab] cget -font]  -cursor [[my wTab] cget -cursor] \
                    -under [[my wTab] cget -under] -anchor [[my wTab] cget -anchor] \
                    -padx  [[my wTab] cget -padx]  -pady   [[my wTab] cget -pady]
}

# name: -width
# args: sWidth: new width value
# option member. controls the width of the container $this.tabs and its
# behaviour when the toplevel is resized. sWidth can be 'auto', an integer,
# or an integer followed by a percent sign. sets the two private variables
# sWmode and iWidth accordingly. they are used in the _drawTabs member.
metawidget proc Tabcontrol -width { sWidth } {
  if { [regexp {^([0-9]+)%$} $sWidth sDummy iWidth] } {
    my sWmode pct
    my iWidth [expr $iWidth/100.]
  } elseif { [regexp {^[0-9]+$} $sWidth iWidth] } {
    my sWmode abs
    my iWidth $iWidth
  } elseif { $sWidth == "auto" } {
    my sWmode req
    my iWidth [my iAutoW]
  } else {
    error "Tabcontrol $this: Bad width $sWidth. Must be an integer, integer% or 'auto'"
  }

  my -width $sWidth
  _drawTabs
}

# name: insert
# args: sTab: name of the new tab
#       iRow: row to place the tab in
#       args: option-value pairs for tab configuration
# defines and displays a new tab. derives the row's widget name and eventually
# creates it if it doesn't exist. creates a label for the tab and packs it
# into its row. initializes tab properties.
metawidget proc Tabcontrol insert { sTab iRow args } {
  if { [myinfo exists aTabs($sTab)] } {
    error "Tabcontrol $this: Tab '$sTab' already exists"
  }

  set wRow $this.tabs.row$iRow    ;# wRow is child of $this.tabs
  set wTab $wRow.tab$sTab         ;# wTab is child of wRow

  if { ! [myinfo exists lTabs$iRow] } {  ;# if wRow doesn't exist
    frame $wRow -border 0         ;# then create it and
    my lTabs$iRow {}              ;# initialize a list for all its tabs
  }

  # create tab widget, simply pack it on the left, create mouse click binding
  label $wTab -border 2 -relief raised
  pack $wTab -side left -ipadx 2 -fill both -expand 1
  bind $wTab <1> "$this invoke $sTab"

  my aTabs($sTab) [list $wTab $iRow {} {}]          ;# initialize tab prop/s
  eval tabconfigure {$sTab} -text {$sTab} $args     ;# configure it a bit

  _calcGeom         ;# update required width
  invoke $sTab      ;# and raise the new tab

  # add tab to the all-tabs list and to the row-specific list
  my lAllTabs   [mkw.lextend [my lAllTabs] $sTab]
  my lTabs$iRow [mkw.lextend [my lTabs$iRow] $sTab]
}

# name: delete
# args: sTab: tab to delete
# deletes a tab. displays another tab, if there's one left.
metawidget proc Tabcontrol delete { sTab } {
  mkw.lassign [_findTab $sTab] wTab iRow wWindow

  pack forget $wWindow
  destroy $wTab
  unmy aTabs($sTab)

  # update all-tabs list and row-specific list
  my lAllTabs   [mkw.lshrink [my lAllTabs] $sTab]
  my lTabs$iRow [mkw.lshrink [my lTabs$iRow] $sTab]

  # if there is some other tab left and the deleted tab was active, then
  # raise the other tab. otherwise hide the $this.actv frame.
  if { [llength [my lAllTabs]] } {
    if { $sTab == [my sActTab] } {
      invoke [lindex [my lAllTabs] 0]
    }

    _calcGeom
    _drawTabs
  } else {
    place forget $this.actv
    place forget $this.gbar
    my sActTab {}
  }
}

# name: tabconfigure
# args: sTab: tab to be configured
#       args: option-value pairs
# configures a tab. the two special options -window and -command are stored
# in the tab's property list (no immediate action required). all other options
# are passed to the tab's widget.
metawidget proc Tabcontrol tabconfigure { sTab args } {
  mkw.lassign [_findTab $sTab] wTab

  set lArgs [mkw.options $args * {-window *} {-command *}]

  if { [info exists -window] } {
    my aTabs($sTab) [lreplace [my aTabs($sTab)] 2 2 ${-window}]
  }

  if { [info exists -command] } {
    my aTabs($sTab) [lreplace [my aTabs($sTab)] 3 3 ${-command}]
  }

  eval $wTab config $lArgs

  _calcGeom
  _drawTabs
  _updateActive
}

# name: tabcget
# args: sTab: tab to retrieve option value for
#       sOption: name of an option
# retrieves an option value of a tab. either returns a tab property value for
# one of the special options -window and -command, or tries to get it directly
# from the tab's widget.
metawidget proc Tabcontrol tabcget { sTab sOption } {
  mkw.lassign [_findTab $sTab] wTab iRow wWindow sCommand

  set wRow $this.tabs.row$iRow.tab$sTab
  set wTab $wRow.tab$sTab

  switch -- $sOption {
    -window   { return $wWindow }
    -command  { return $sCommand }
    default  { return [$wTab cget $sOption] }
  }
}

# name: invoke
# args: sTab: tab to raise
# raises a tab, i.e. makes it look like in the foreground and displays its
# associated user window. Also, the row with the active tab is always the lowest
# one. therefore the rows are rearranged.
metawidget proc Tabcontrol invoke { sTab } {
  mkw.lassign [_findTab $sTab] wTab iRow wWindow sCommand

  catch { pack forget [my wActWin ] }  ;# hide user-widget from active tab
  my wTab $wTab                        ;# update private variables...
  my wRow [winfo parent $wTab]         ;# ...needed later e.g. for _drawTabs
  my sActTab $sTab
  my wActWin $wWindow

  # rearrange rows so that the one with the new active tab is at the bottom.
  set iY 0
  set lRows [mkw.lshrink [lsort [winfo children $this.tabs]] [my wRow]]
  foreach wRow [linsert $lRows end [my wRow]] {
    raise $wRow
    place $wRow -x 0 -y $iY -relw 1
    incr iY [expr [winfo reqheight $wRow]-2]
  }

  # pack and raise the new tab's user-widget, if it exists
  if { [winfo exist $wWindow] } {
    pack $wWindow -in $this.work -padx 2 -pady 2 -fill both -expand 1
    raise $wWindow
  }

  update
  _drawTabs
  _updateActive

  if { $sCommand != {} } {
    eval $sCommand    ;# eventually execute the tab's command, if there is one
  }
}

# name: get
# args: sWhat: row number or 'active' or nothing
# returns all tabs, the active tab or the tabs of a given row.
metawidget proc Tabcontrol get { {sWhat {}} } {
  if { $sWhat == {} } {
    return [my lAllTabs]
  } elseif { $sWhat == "active" } {
    return [my sActTab]
  } elseif { [regexp {^[0-9]+$} $sWhat] } {
    return [my lTabs$sWhat]
  } else {
    error "Tabcontrol $this: Bad index $sWhat. Should be active, an integer or nothing."
  }
}

# name: bind
# args: sTab: tab to set binding for (or get from)
#       args: arguments for the bind command
# creates or retrieves bindings for the given tab.
metawidget proc Tabcontrol bind_ { sTab args } {
  mkw.lassign [_findTab $sTab] wRow wTab
  eval bind $wTab $args
}

metawidget command Tabcontrol _drawTabs _drawTabs

metawidget command Tabcontrol insert    insert
metawidget command Tabcontrol delete    delete
metawidget command Tabcontrol tabconfigure tabconfigure
metawidget command Tabcontrol tabcget   tabcget
metawidget command Tabcontrol invoke    invoke
metawidget command Tabcontrol get       get
metawidget command Tabcontrol bind      bind_

metawidget option  Tabcontrol -width    -width

proc test {} {
  global iTabno
  set iTabno 0

  wm minsize . 200 200

  [text .code] insert end [info body test]

  frame .fwid -border 2 -relief groove
  pack [radiobutton .fwid.auto -text "Set it to Auto mode"  -value 0 -command {.tabs config -width auto}] -anchor w
  pack [radiobutton .fwid.half -text "Half of window width" -value 1 -command {.tabs config -width 50%}] -anchor w
  pack [radiobutton .fwid.full -text "Full width of window" -value 2 -command {.tabs config -width 100%}] -anchor w
  pack [radiobutton .fwid.exct -text "Exactly 300 pixels"   -value 3 -command {.tabs config -width 300}] -anchor w

  frame .fcrt
  pack [button .fcrt.tcre -text "Create a tab" -command {
    incr iTabno
    label .lbl$iTabno -text "This is tab $iTabno" -font {Helvetica 24} -bg white
    .tabs insert Tab$iTabno 1 -window .lbl$iTabno
  }] -fill x
  pack [button .fcrt.tdel -text "Delete last tab" -command {
    .tabs delete Tab$iTabno
    destroy .lbl$iTabno
    incr iTabno -1
  }] -fill x

  tabcontrol .tabs -width auto
  pack .tabs -fill both -expand 1 -padx 10 -pady 10

  .tabs insert width  0 -text "Change Width" -window .fwid -underline 7
  .tabs insert edit   0 -text "Create Tabs"  -window .fcrt -underline 7
  .tabs insert source 0 -text "View Source"  -window .code -underline 5
  .tabs invoke width

  bind . <Alt-w> {.tabs invoke width}
  bind . <Alt-t> {.tabs invoke edit}
  bind . <Alt-s> {.tabs invoke source}
}

#test
