package require Metawidget

# name: Combobox
# args: args
# for the first instance defines the image for the arrow button.
# creates all widgets including the popup list and its toplevel window.
# the toplevel is withdrawn. defines bindings for keys and mouse clicks.
# initializes option values by calling the respective option members.
metawidget create Combobox {
  # initialize the icon and try to find out if KDE is running
  if { ! [ourinfo exists pDown] } {
    our pDown [image create bitmap -data {
      #define _width 8
      #define _height 4
      static char _bits[] = { 0xfe, 0x7c, 0x38, 0x10 };
    }]

    # on Linux and KDE, the <FocusOut> binding (see below) doesn't work
    our bKde [expr { $::tcl_platform(os) == "Linux" && ! [catch { exec ps -C kde }] }]
    our wToplevel [winfo toplevel $this]
  }

  entry     $this.entr      -bg white -selectborder 0 -selectback black -selectfore white
  button    $this.entr.arrb -padx 0 -pady 0 -image [our pDown] -command "$this _showList" -cursor arrow
  toplevel  $this.topl      -border 0
  scrollbar $this.topl.scrv -command "$this.topl.list yview"
  listbox   $this.topl.list -bg white -border 0 -yscroll "$this.topl.scrv set" \
                             -selectborder 0 -selectback {dark blue} -selectfore white

  # the list is in a window without border. the binding is supposed to let
  # it disappear when some other window is clicked. does not quite work on
  # KDE: when the window to which the combobox belongs is clicked, the focus
  # doesn't get out, and the bind script doesn't get triggered. there's a
  # workaround in _showList and _hideList.
  wm overrideredirect $this.topl 1
  wm withdraw $this.topl
  bind $this.topl <FocusOut> "$this _hideList"

  pack  $this.entr      -fill x -expand 1
  place $this.entr.arrb -relx 1 -y 0 -relh 1 -anchor ne
  pack  $this.topl.scrv -side right -fill y
  pack  $this.topl.list -side left -fill both -expand 1

  bind $this.entr <Up>   "$this see -"
  bind $this.entr <Down> "$this see +"

  bind $this.topl.list <ButtonRelease-1> "$this _hideList; $this see ?"
  bind $this.topl.list <Key-Return>      "$this _hideList; $this see ?"
  bind $this.topl.list <Key-Escape>      "$this _hideList"

  my iIndex -1                         ;# currently displayed
  my iListH 40                         ;# required height for list in pixels
  my -command {}                       ;# evaluated on value change

  -lines 5                             ;# set box to 5 lines
} {} -default entr

# name: _showList
# args: -
# displays the toplevel with listbox and scrollbar at the appropriate
# position. the <FocusOut> binding and the <Escape> key lets it
# disappear by calling _hideList. the toplevel window is positioned
# either above or below the entry widget, depending on the space
# left below of it.
metawidget proc Combobox _showList {} {
  set iX [winfo rootx  $this]
  set iY [winfo rooty  $this]
  set iW [winfo width  $this]
  set iH [winfo height $this]

  # show listbox above the entry, if there's not enough space below
  if { [expr [winfo screenheight $this] - ( $iY + $iH )] >= [my iListH] } {
    incr iY $iH
  } else {
    incr iY -[my iListH]
  }

  # position window, then show it
  wm geometry  $this.topl ${iW}x[my iListH]+$iX+$iY
  wm deiconify $this.topl

  $this.topl.list selection clear 0 end
  $this.topl.list selection set [my iIndex]

  raise $this.topl
  focus $this.topl.list

  # workaround for KDE: the listbox doesn't disappear when the application
  # window is clicked outside of a widget that can receive the focus. it
  # would even stay on the screen when the app. window is moved. to prevent
  # this, a <Configure> binding helps. we need to save any previous script
  # for this binding, though.
  if { [our bKde] } {
    our sConfScript [bind [our wToplevel] <Configure>]
    bind [our wToplevel] <Configure> "$this _hideList"
  }
}

# name: _hideList
# args: -
# releases the grab. lets the listbox toplevel disappear.
metawidget proc Combobox _hideList {} {
  wm withdraw  $this.topl
  focus        $this.entr

  # restore the original <Configure> bind script again!
  if { [our bKde] } {
    bind [our wToplevel] <Configure> [our sConfScript]
  }
}

# name: -lines
# args: iLines: number of listbox lines
# set the number of lines for the listbox. converts it into pixels by
# multiplying it with the font height. stores pixel height in iListH.
metawidget proc Combobox -lines { iLines } {
  $this.topl.list configure -height $iLines
  my -lines $iLines

  set iFontH [font metrics [$this.topl.list cget -font] -linespace]
  my iListH [expr (1+$iFontH)*[my -lines]+2]
}

# name: -entries
# args: lEntries: list of entries to set for the Combobox
# repopulates the list with the given values in lEntries.
metawidget proc Combobox -entries { args } {
  if { ! [llength $args] } {
    $this.topl.list get 0 end
  } else {
    $this.topl.list delete 0 end
    eval $this.topl.list insert 0 [lindex $args 0]
  }
}

# name: -state
# args: sState: state to change the Combobox to
# applies normal and disabled state simply to the entry widget and the button.
# for restricted state, the two bindings prevent input to the entry field.
# only values from the list can be set with cursor up/down.
metawidget proc Combobox -state { sState } {
  my -state [mkw.complete $sState {normal disabled restricted}]

  switch [my -state] {
    normal - disabled {
      bind $this.entr <Key> {}
      bind $this.entr <KeyRelease> {}
      $this.entr config -state $sState
      $this.entr.arrb config -state $sState
    }
    restricted {
      bind $this.entr <Key> "$this.entr config -state disabled"
      bind $this.entr <KeyRelease> "$this.entr config -state normal"
      $this.entr config -state normal
      $this.entr.arrb config -state normal
      see 0
    }
  }
}

# name: see
# args: iIndex: index of the value list to display
# if index is not given, returns the index of the currently displayed
# value, or -1 if the value is not part of the value list.
# if index is + or -, the next resp. previous value in the value list is
# set. for index ?, the listbox' currently selected element is taken.
# if index is an integer, the corresponding element from the value list
# is displayed. any defined command is evaluated, if the value in
# the entry field has changed.
metawidget proc Combobox see { {iIndex {}} } {
  if { $iIndex == {} } {
    return [lsearch -exact [$this.topl.list get 0 end] [$this.entr get]]
  }

  set iLen [llength [$this.topl.list get 0 end]]
  set iOldIndex [my iIndex]

  if { $iIndex == "?" } {
    my iIndex [$this.topl.list curselection]
  } elseif { $iIndex == "+" && [my iIndex] < [expr $iLen-1] } {
    my iIndex [expr [my iIndex]+1]
  } elseif { $iIndex == "-" && [my iIndex] > 0 } {
    my iIndex [expr [my iIndex]-1]
  } elseif { $iIndex >= 0 && $iIndex < $iLen } {
    my iIndex $iIndex
  }

  if { $iOldIndex != [my iIndex] } {
    $this.entr delete 0 end
    $this.entr insert 0 [$this.topl.list get [my iIndex]]
    eval [my -command]
  }
}

metawidget command Combobox _showList _showList
metawidget command Combobox _hideList _hideList

metawidget command Combobox see       see

metawidget option  Combobox -lines   -lines
metawidget option  Combobox -entries -entries -entries
metawidget option  Combobox -state   -state
metawidget option  Combobox -command

proc test {} {
  pack [frame .upfr -border 0] -fill both
  pack [label .upfr.text -text "Enter values and press Set!"] -anchor w
  pack [entry .upfr.vals] -side left -fill x -expand 1
  pack [button .upfr.setb -pady 0 -text Set! -command {.cbox config -entries [.upfr.vals get]} ] -side right

  .upfr.vals insert end {red green blue yellow cyan magenta}

  pack [combobox .cbox -command {.outp config -text [.cbox get]}] -fill x -pady 10

  pack [label .outp -border 1 -relief sunken] -fill x
  pack [radiobutton .mod1 -text "Normal"     -value 1 -command {.cbox config -state normal}] -fill x -side left
  pack [radiobutton .mod2 -text "Restricted" -value 2 -command {.cbox config -state restricted}] -fill x -side left
  pack [radiobutton .mod3 -text "Disabled"   -value 3 -command {.cbox config -state disabled}] -fill x -side left
}

#test


