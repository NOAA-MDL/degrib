package require Metawidget

# name: Spinentry
# args: args
# for the first instance defines the images for the arrow buttons.
# creates the two arrow "buttons" and inits the option variables.
metawidget create Spinentry {
  if { ! [ourinfo exists pDown] } {
    our pDown [image create bitmap -data {
      #define _width 8
      #define _height 3
      static char _bits[] = { 0x7c, 0x38, 0x10 };
    }]
    our pUp [image create bitmap -data {
      #define _width 8
      #define _height 3
      static char _bits[] = { 0x10, 0x38, 0x7c };
    }]

    foreach sBind [bind Entry] {
      bind Spinentry $sBind [bind Entry $sBind]
    }
  }

  entry $this.entr
  label $this.entr.stup -border 2 -relief raised -image [our pUp]   -cursor arrow
  label $this.entr.stdn -border 2 -relief raised -image [our pDown] -cursor arrow

  pack  $this.entr -fill both -expand 1
  place $this.entr.stup -relx 1 -rely 0 -w 16 -relh .5 -anchor ne
  place $this.entr.stdn -relx 1 -rely 1 -w 16 -relh .5 -anchor se

  bind $this.entr <Up>   "$this _step +"
  bind $this.entr <Down> "$this _step -"
  bind $this.entr.stup <Button-1> "$this _press %W +"
  bind $this.entr.stdn <Button-1> "$this _press %W -"
  bind $this.entr.stup <ButtonRelease-1> "$this _press %W"
  bind $this.entr.stdn <ButtonRelease-1> "$this _press %W"

  my -step    1
  my -speed   100
  my -maximum {}
  my -minimum {}
  my -onerror {}
  my -value   {}
} {} -default entr

# name: _spin
# args: sDir: + or 0
# called on button press. a timer will call this function after -speed ms.
# the timer job is cancelled with button release.
metawidget proc Spinentry _spin { sDir } {
  $this _step $sDir
  my iJob1 [after [my -speed] $this _spin $sDir]
}

# name: _press
# args: sWindow: arrow "button" that was pressed
#       sDir: + or -
# changes the relief of the arrow button. for button press, it calls
# _spin. for button release, it kills any pending timer jobs.
metawidget proc Spinentry _press { sWindow {sDir {}} } {
  if { $sDir == {} } {
    $sWindow config -relief raised
    catch { after cancel [my iJob1] }
  } else {
    $sWindow config -relief sunken
    $this _spin $sDir
  }
}

# name: _step
# args: sDir: + or -
# called on cursor up/down. changes the entry's value one time and updates
# the entry widget. if the expr does not work and -onerror is defined, it
# evals it, otherwise escalates the error upward.
metawidget proc Spinentry _step { sDir } {
  set fValue [$this get]

  if { [catch {expr $fValue $sDir [my -step]} fValue] } {
    if { [my -onerror] != {} } {
      eval [my -onerror]
    } else {
      error $fValue
    }
  }

  if { [my -minimum] != {} && $fValue < [my -minimum]} {
    set fValue [my -minimum]
  }
  if { [my -maximum] != {} && $fValue > [my -maximum]} {
    set fValue [my -maximum]
  }

  $this.entr delete 0 end
  $this.entr insert 0 $fValue
}

# name: -value
# args: fValue
# simply sets a value in the entry widget. also checks if that value
# can be stepped up and down. if not, it raises an exception.
metawidget proc Spinentry -value { fValue } {
  if { [catch { expr $fValue + [my -step] }] } {
    throw "Value $fValue is not numeric"
  }

  $this.entr delete 0 end
  $this.entr insert 0 $fValue

  my -value $fValue
}

metawidget command Spinentry _step    _step
metawidget command Spinentry _spin    _spin
metawidget command Spinentry _press   _press

metawidget option Spinentry  -step
metawidget option Spinentry  -speed
metawidget option Spinentry  -maximum
metawidget option Spinentry  -minimum
metawidget option Spinentry  -onerror
metawidget option Spinentry  -value   -value

proc test {} {
  pack [spinentry .spin -value 10 -step 2 -minimum 0 -maximum 20 -speed 80 -onerror {puts NoGood!}] -fill x -pady 10
}

#test

