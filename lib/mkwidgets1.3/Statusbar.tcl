package require Metawidget

# name: Statusbar
# args: args: option-value pairs
# creates inner widgets, initializes variables.
metawidget create Statusbar {
  # two pixel gap on top for a better look
  pack [frame $this.gap -border 0 -height 2] -fill x
  pack [label $this.sta -border 1 -relief sunken -anchor w] -padx 1 -side left -fill both -expand 1

  # progress bar properties
  my -ticks 0
  my -progress 0

  my iVisible 0
  my lStack {}

  -state normal
} {} -default sta

# name: push
# args: sText: text to push
# overlays the current text with sText. can be removed with pop.
metawidget proc Statusbar push { sText } {
  my lStack [linsert [my lStack] end [$this.sta cget -text]]
  $this.sta config -text $sText
  update
}

# name: pop
# args: -
# removes the last text defined with push.
metawidget proc Statusbar pop {} {
  $this.sta config -text [lindex [my lStack] end]
  my lStack [lreplace [my lStack] end end]
  update
}

# name: add
# args: sTag: name for new field
#       args: option-value pairs
# installs a new field that can be assigned with an icon and a text.
# sTag must be a unique identifier, args are passed to itemconf
metawidget proc Statusbar add { sTag args } {
  if { [winfo exists $this._$sTag] } {
    error "Statusbar $this: Item $sTag already exists."
  }

  pack [frame $this._$sTag -border 1 -relief sunken] -side left -fill y -padx 1
  pack [label $this._$sTag.icon -border 0 -pady 0] -side left -fill y
  pack [label $this._$sTag.text -border 0 -pady 0] -side left -fill y

  eval itemconf $sTag $args

  return $this._$sTag
}

# name: delete
# args: args: list of fields as created by add
# deletes all fields specified by the elements of args.
metawidget proc Statusbar delete { args } {
  foreach sTag $args {
    destroy $this._$sTag
  }
}

# name: itemconf
# args: sTag: field id (see add)
#       args: option-value pairs
# configures a field. most options are passed to the inner icon as well as to
# the label. some options are passed only to the widget where they make sense.
metawidget proc Statusbar itemconf { sTag args } {
  if { ! [winfo exists $this._$sTag] } {
    error "Statusbar $this: Item $sTag does not exist."
  }

  foreach { sOption sValue } $args {
    switch -- $sOption {
      -width {
        $this._$sTag config -width $sValue
        pack propagate $this._$sTag [expr !$sValue]
      }
      -border | -relief {
        $this._$sTag config $sOption $sValue
      }
      -image {
        $this._$sTag.icon config $sOption $sValue
      }
      -text {
        $this._$sTag.text config $sOption $sValue
      }
      default {
        $this._$sTag.icon config $sOption $sValue
        $this._$sTag.text config $sOption $sValue
      }
    }
  }
}

# name: itemcget
# args: sTag: field id (see add)
#       sOption: option name
# returns the value of a field option.
metawidget proc Statusbar itemcget { sTag sOption } {
  if { ! [winfo exists $this._$sTag] } {
    error "Statusbar $this: Item $sTag does not exist."
  }

  switch -- $sOption {
    -border | -relief | -width { return [$this._$sTag cget $sOption] }
    -image  { return [$this._$sTag.icon cget $sOption] }
    default { return [$this._$sTag.text cget $sOption] }
  }
}

# name: -ticks
# args: iTicks: number of "lights" for the progress bar
# displays a progress bar with iTicks gray rectangles. a progress value is set
# with -progress. if iTicks is 0, the progress bar is deleted.
metawidget proc Statusbar -ticks { iTicks } {
  my -ticks $iTicks

  catch { destroy $this.pro }
  if { ! $iTicks } return

  pack [frame $this.pro -border 1 -relief sunken] -side right -fill y -padx 1

  for { set i 0 } { $i < $iTicks } { incr i } {
    pack [frame $this.pro.pc$i -width 8 -bg gray] -side left -fill y -padx 1 -pady 1
  }
}

# name: -progress
# args: fProg: progress (0-1)
# displays a progress in the progress bar. fProg must be between 0 and 1.
metawidget proc Statusbar -progress { fProg } {
  my -progress $fProg

  if { $fProg < 0 } { set fProg 0 }
  if { $fProg > 1 } { set fProg 1 }

  set iVisible [expr round( $fProg * [my -ticks] )]
  if { $iVisible == [my iVisible] } return
  my iVisible $iVisible

  for { set i 0 } { $i < $iVisible } { incr i } {
    $this.pro.pc$i config -bg indianred
  }

  for { set i $iVisible } { $i < [my -ticks] } { incr i } {
    $this.pro.pc$i config -bg gray
  }

  update
}

# name: -state
# args: sState: normal or withdrawn
# either displays the Statusbar or withdraws if from the screen. ensures that
# the Statusbar is packed at the bottom as the first element in the order.
metawidget proc Statusbar -state { sState } {
  my -state [mkw.complete $sState {normal withdrawn}]

  if { [my -state] == "normal" } {
    set sFirst [lindex [pack slaves [winfo parent $this]] 0]
    if { $sFirst != {} } {
      pack $this -side bottom -fill x -before $sFirst
    } else {
      pack $this -side bottom -fill x
    }
  } else {
    pack forget $this
  }
}

metawidget command Statusbar push     push
metawidget command Statusbar pop      pop
metawidget command Statusbar add      add
metawidget command Statusbar delete   delete
metawidget command Statusbar itemconf itemconf
metawidget command Statusbar itemcget itemcget

metawidget option  Statusbar -ticks    -ticks
metawidget option  Statusbar -progress -progress
metawidget option  Statusbar -state    -state

proc test {} {
  . config -bg gray50
  wm minsize . 400 150

  image create photo p1 -file ./demos/images/Notebook.gif
  image create photo p2 -file ./demos/images/Compass.gif

  pack [button .b1 -text {Show Progress} -command {
    .s push {Loading something...}
    foreach fProg {0 .1 .2 .3 .4 .5 .6 .7 .8 .9 1 0} {
      .s config -progress $fProg
      after 100
    }
    .s pop
  }] -fill x

  pack [button .b2 -text {Add Field} -command {
    if { [catch {incr iSeq}] } {
      set iSeq 0
    }

    .s add field$iSeq -text "[lindex {Tcl Tk is fun!} $iSeq] "
  }] -fill x

  pack [button .b3 -text {Change Icon} -command {
    .s itemconf myField -image p2
  }] -fill x

  statusbar .s -ticks 10

  .s config -text {I am a Statusbar!}
  .s add myField -text { Field } -image p1
}

#test
