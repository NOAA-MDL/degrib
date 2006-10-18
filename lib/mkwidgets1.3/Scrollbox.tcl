package require Metawidget

# name: Scrollbox
# args: args
# creates the widgets. the widget option -scrollbar is defined
# by calling its respective member functions.
metawidget create Scrollbox {
  scrollbar $this.scrv -command "$this.list yview"
  scrollbar $this.scrh -command "$this.list xview" -orient horiz
  listbox   $this.list -bg white -border 1 -selectborder 0 -selectback black \
                        -selectfore white -yscroll "$this.scrv set" -xscroll "$this.scrh set"

  grid columnconf $this 0 -weight 1
  grid rowconf    $this 0 -weight 1
  grid $this.list -column 0 -row 0 -sticky news

  bind $this.list <1> { focus %W }

  -scrollbar off
} {} -default list

# name: _setscroll
# args: -
# if -scrollbar is auto, displays or hides the two scrollbars.
# uses the fraction values the scrollbars would be set to.
# anything > 0 and < 1 would indicate that the scrollbar should
# be displayed.
metawidget proc Scrollbox _setscroll {} {
  if { [my -scrollbar] != "auto" } return

  mkw.lassign [$this.scrv get] fFrac1 fFrac2
  if { $fFrac1 > 0 || $fFrac2 < 1 } {
    grid $this.scrv -column 1 -row 0 -sticky ns
  } else {
    grid forget $this.scrv
  }

  mkw.lassign [$this.scrh get] fFrac1 fFrac2
  if { $fFrac1 > 0 || $fFrac2 < 1 } {
    grid $this.scrh -column 0 -row 1 -sticky we
  } else {
    grid forget $this.scrh
  }
}

# name: _redraw
# args: -
# updates the scrollbars after becoming idle. any
# old job is deleted, so exactly one redraw is done.
metawidget proc Scrollbox _redraw {} {
  catch { cancel [my hJob] }
  my hJob [after idle $this _setscroll]
}

# name: -scrollbar
# args: sScrollbar: on, off or auto
# option member. lets the scrollbars either appear (on) or disappear (off).
# for auto, _setscroll takes care of that. the binding is needed to
# update the scrollbars when the window is resized.
metawidget proc Scrollbox -scrollbar { sScrollbar } {
  my -scrollbar [mkw.complete $sScrollbar {auto on off}]

  switch [my -scrollbar] {
    auto {
      bind $this.list <Configure> "$this _setscroll"
      _redraw
    }
    on {
      bind $this.list <Configure> {}
      grid $this.scrv -column 1 -row 0 -sticky ns
      grid $this.scrh -column 0 -row 1 -sticky we
    }
    off {
      bind $this.list <Configure> {}
      grid forget $this.scrv $this.scrh
    }
  }
}

# name: find
# args: args: listbox entries (not indexes) to search for
# the position for each entry is determined and appended to a list. the
# list is returned. a -1 indicates that the element is not in the list.
metawidget proc Scrollbox find { args } {
  set lEntries [$this.list get 0 end]
  set lHits {}

  foreach sText $args {
    lappend lHits [lsearch -exact $lEntries $sText]
  }

  return $lHits
}

# name: selection
# args: sOption: option to set or get the selection
#       args: depends on sOption
# extends the 'listbox selection' command by the two options setbyval
# and get. 'setbyval' sets the selection for all listbox entries in args.
# (whereas 'set' sets them by index). get is the opposite of setbyval.
# all other options are directly handed over to the listbox.
metawidget proc Scrollbox selection_ { sOption args } {
  switch $sOption {
    setbyval {
      set lEntries [$this.list get 0 end]

      foreach sText $args {
        set iPos [lsearch -exact $lEntries $sText]
        if { $iPos == -1 } continue
        $this.list selection set $iPos
      }
    }
    get {
      set lResult {}

      foreach iIndex [$this.list curselection] {
        lappend lResult [$this.list get $iIndex]
      }

      return $lResult
    }
    default {
      eval $this.list selection $sOption $args
    }
  }
}

# name: set
# args: args: listbox entries
# deletes all entries in the listbox and adds the ones defined in args.
metawidget proc Scrollbox set_ { args } {
  $this.list delete 0 end
  eval $this.list insert 0 $args
  _redraw
}

# name: removes
# args: args: listbox entries
# removes all entries specified in args from the listbox.
metawidget proc Scrollbox remove { args } {
  set lEntries [$this.list get 0 end]
  eval set_ [eval mkw.lshrink \$lEntries $args]
}

# name: sort
# args: args: passed to the lsort command
# sorts the listbox using lsort. args is passed to the lsort command.
metawidget proc Scrollbox sort { args } {
  set lEntries [$this.list get 0 end]
  eval set_ [eval lsort $args \$lEntries]
}

# name: insert
# args: args: passed to 'listbox insert'
# wrapper around 'listbox insert'. redraws the scrollbars.
metawidget proc Scrollbox insert { args } {
  eval $this.list insert $args
  _redraw
}

# name: delete
# args: args: passed to 'listbox delete'
# wrapper around 'listbox delete'. redraws the scrollbars.
metawidget proc Scrollbox delete { args } {
  eval $this.list delete $args
  _redraw
}

metawidget command Scrollbox _setscroll _setscroll

metawidget command Scrollbox delete     delete
metawidget command Scrollbox insert     insert
metawidget command Scrollbox sort       sort
metawidget command Scrollbox remove     remove
metawidget command Scrollbox set        set_
metawidget command Scrollbox selection  selection
metawidget command Scrollbox find       find

metawidget option  Scrollbox -scrollbar -scrollbar

proc test {} {
  global iLine

  set iLine 0

  pack [radiobutton .rad1 -value 0 -text "Scrollbars always on"    -command {.sbox config -scrollbar on}] -anchor w
  pack [radiobutton .rad2 -value 1 -text "Scrollbars always off"   -command {.sbox config -scrollbar off}] -anchor w
  pack [radiobutton .rad3 -value 2 -text "Scrollbars in auto mode" -command {.sbox config -scrollbar auto}] -anchor w

  pack [button .addl -text "Add a line" -command {
    incr iLine
    .sbox insert end "It is [clock format [clock seconds] -format %H:%M:%S]. This is line $iLine."
    .sbox see end
  }] -fill x

  pack [button .reml -text "Remove first line" -command {.sbox delete 0}] -fill x
  pack [scrollbox .sbox] -fill both -expand 1
}

#test

