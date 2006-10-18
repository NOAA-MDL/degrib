package require Metawidget

# name: Tooltip
# args: args: option-value pairs
# creates the label for the bubblehelp widget. the new class bindings
# are later attached to any widget where a bubblehelp shall appear.
# the nowait flag is true if the bubblehelp was just active within the
# last second. in this case no new delay would occur.
metawidget create Tooltip {
  pack [label $this.text -bg #FFC -border 1 -relief solid -padx 2] -fill x

  # hide the window and take it away from the window manager
  wm withdraw $this
  wm overrideredirect $this 1

  # define some new class bindings for that instance
  bind Tooltip-$this <Enter>  "$this _start %W"
  bind Tooltip-$this <Leave>  "$this _hide"
  bind Tooltip-$this <Button> "$this _hide"

  my bNowait 0    ;# if 1: no new timer when above new widget
  my -delay 1000  ;# default delay time 1 second
} {
}  -default text -type toplevel

# name: add
# args: sWindow: widget to assign a bubblehelp
#       sText: text to appear in the bubblehelp
# registers a widget with this bubblehelp instance.
metawidget proc Tooltip add { sWindow sText } {
  # add the class bindings to the widget
  bindtags $sWindow [mkw.lextend [bindtags $sWindow] Tooltip-$this]
  my $sWindow $sText
}

# name: forget
# args: sWindow: widget to remove bubblehelp from
# removes a bubblehelp from a widget
metawidget proc Tooltip forget { sWindow } {
  # remove the class bindings from the widget
  bindtags $sWindow [mkw.lshrink [bindtags $sWindow] Tooltip-$this]
  unmy $sWindow
}

# name: _start
# args: sWindow: widget to display help for
# triggered on the <Enter> event. starts a timer or shows the help now.
metawidget proc Tooltip _start { sWindow } {
  if { [my bNowait] } {        ;# if bubblehelp was active...
    _show $sWindow             ;# ... then no new waiting time
  } else {                     ;# otherwise start timer
    our hJobID [after [my -delay] $this _show $sWindow]
  }
}

# name: _show
# args: sWindow: widget to display help for
# actually displays the help. nowait is set to 1. if the mouse
# enters another widget then the timer won't start again.
metawidget proc Tooltip _show { sWindow } {
  catch { after cancel [our hWaitID] }
  my bNowait 1                 ;# set 'bubblehelp active' flag

  # set the text for this widget, show and position it
  $this.text config -text [my $sWindow]
  wm deiconify $this
  raise $this
  wm geometry $this +[expr 2+[winfo pointerx .]]+[expr 16+[winfo pointery .]]
}

# name: _hide
# args: -
# triggered on the <Leave> event and on any button click. hides the help
# and resets the nowait flag after 1 second.
metawidget proc Tooltip _hide {} {
  # start timer which will reset 'bubblehelp active' flag
  our hWaitID [after 500 set bNowait 0]

  # kill any pending timer and hide the widget
  catch { after cancel [our hJobID] }
  wm withdraw $this
}

metawidget command Tooltip _start _start
metawidget command Tooltip _show  _show
metawidget command Tooltip _hide  _hide

metawidget command Tooltip add    add
metawidget command Tooltip forget forget

metawidget option  Tooltip -delay

proc test {} {
  pack [label .l -text "Move the mouse over the buttons"]

  pack [frame .f1 -border 1 -relief raised]
  pack [button .f1.b1 -text B1] -side left -padx 2 -pady 2
  pack [button .f1.b2 -text B2] -side left -padx 2 -pady 2
  pack [button .f1.b3 -text B3] -side left -padx 2 -pady 2

  tooltip .ttip1 -delay 500
  .ttip1 add .l "Yes, it works also on labels and other widget types"
  .ttip1 add .f1.b1 "This is button 1 with .bhelp1"
  .ttip1 add .f1.b2 "Button 2 with .bhelp1"
  .ttip1 add .f1.b3 "Button three with .bhelp1"

  pack [frame .f2 -border 1 -relief raised] -side bottom
  pack [button .f2.b1 -text B1] -side left -padx 2 -pady 2
  pack [button .f2.b2 -text B2] -side left -padx 2 -pady 2
  pack [button .f2.b3 -text B3] -side left -padx 2 -pady 2

  tooltip .ttip2 -delay 500
  .ttip2 add .f2.b1 "This is button 1 with .bhelp2"
  .ttip2 add .f2.b2 "Button 2 with .bhelp2"
  .ttip2 add .f2.b3 "Button three with .bhelp2"

  .ttip2 configure -bg blue -fg white

  raise .
}

#test

