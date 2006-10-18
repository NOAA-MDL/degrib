package require Metawidget

# name: Window
# args: option-value pairs
# this class merges the commands wm and toplevel (that is, the most
# common functions of wm and those that apply to the Windows platform),
# making the control of toplevel Windows easier and more consistent.
# the Window metawidget is basically identical with the toplevel widget,
# except that it defines some additional methods and options that use
# wm functions.
metawidget create Window {} {} -type toplevel

# name: resize
# args: iWidth, iHeight: new size of the Window
metawidget proc Window resize { iWidth iHeight } {
  wm geometry $this =${iWidth}x$iHeight
}

# name: move
# args: iX, iY: new position of the Window
metawidget proc Window move { iX iY } {
  wm geometry $this +${iX}+$iY
}

# name: protocol
# args: args: passed directly to 'wm protocol'
metawidget proc Window protocol { args } {
  eval wm protocol $this $args
}

# name: -geometry
# args: sArg: geometry a la 'wm geometry'
metawidget proc Window -geometry { {sArg {}} } {
  eval wm geometry $this $sArg
}

# name: -minsize
# args: sArg: width and height as a two-element list
metawidget proc Window -minsize { {sArg {}} } {
  eval wm minsize $this $sArg
}

# name: -maxsize
# args: sArg: width and height as a two-element list
metawidget proc Window -maxsize { {sArg {}} } {
  eval wm maxsize $this $sArg
}

# name: -resizable
# args: sArg: flags a la 'wm resizable' in a two-element list
metawidget proc Window -resizable { {sArg {}} } {
  eval wm resizable $this $sArg
}

# name: -override
# args: sArg: flag for 'wm overrideredirect'
metawidget proc Window -override { {sArg {}} } {
  eval wm overrideredirect $this $sArg
}

# name: -ondelete
# args: sArg: command for 'wm protocol ... WM_DELETE_WINDOW' or nothing
metawidget proc Window -ondelete { {sArg ?} } {
  if { $sArg == "?" } {
    wm protocol $this WM_DELETE_WINDOW
  } elseif { $sArg == {} } {
    wm protocol $this WM_DELETE_WINDOW {}
  } else {
    wm protocol $this WM_DELETE_WINDOW $sArg
  }
}

# name: -title
# args: sArg: new Window title
metawidget proc Window -title { {sArg {}} } {
  if { $sArg == {} } {
    wm title $this
  } else {
    wm title $this $sArg
  }
}

# name: -state
# args: sArg: option value as returned by 'wm state', except 'icon'
metawidget proc Window -state { {sArg {}} } {
  if { $sArg == {} } {
    return [wm state $this]
  }

  switch [complete $sArg {normal iconic withdrawn}] {
    normal    { wm deiconify $this }
    iconic    { wm iconify   $this }
    withdrawn { wm withdraw  $this }
  }
}

metawidget command Window resize     resize
metawidget command Window move       move
metawidget command Window protocol   protocol

metawidget option  Window -geometry  -geometry  -geometry
metawidget option  Window -minsize   -minsize   -minsize
metawidget option  Window -maxsize   -maxsize   -maxsize
metawidget option  Window -resizable -resizable -resizable
metawidget option  Window -override  -override  -override
metawidget option  Window -ondelete  -ondelete  -ondelete
metawidget option  Window -title     -title     -title
metawidget option  Window -state     -state     -state

# maps the "." toplevel to a Window object. a bit tricky.
bindtags . [lreplace [bindtags .] 1 1 Window]
rename ::. ::.::.         ;# ain't that funny...

proc . { sCmd args } {
  ::Window::__dispatch . $sCmd $args
}


proc test {} {
  . config -title {Main Window}

  window .w
  .w config -title {New Window} -minsize {320 200}
  .w config -ondelete {
    if { [tk_messageBox -icon question -message "Close it?" -type yesno] == "yes" } {
      destroy .w
    }
  }
}

#test
