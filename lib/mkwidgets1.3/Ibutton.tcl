package require Metawidget

# name: Ibutton
# args: args: option-value pairs
# outside is a frame, inside a label and a button. The button is used so that
# this metawidget can take the focus. the bindings simply redirect clicks
# on the frame or the image to the inner button. all following methods
# just redirect a particular option to the appropriate widget(s).
metawidget create Ibutton {
  pack [frame $this.ibut -border 2 -relief raised] -fill both -expand 1
  grid [label $this.ibut.icon  -padx 0 -pady 0] -column 0 -row 0
  grid [button $this.ibut.text -padx 0 -pady 0 -border 0] -column 1 -row 0

  bind $this.ibut      <Button-1>        "$this config -relief sunken"
  bind $this.ibut      <ButtonRelease-1> "$this config -relief raised; $this.ibut.text invoke"
  bind $this.ibut.icon <Button-1>        "$this config -relief sunken"
  bind $this.ibut.icon <ButtonRelease-1> "$this config -relief raised; $this.ibut.text invoke"
  bind $this.ibut.text <Button-1>        "$this config -relief sunken"
  bind $this.ibut.text <ButtonRelease-1> "$this config -relief raised"

  my -text {}
  my -image {}
} {} -default ibut.text

metawidget proc Ibutton _opt { sOpt sVal } {
  switch -- $sOpt {
    -image  { $this.ibut.icon configure -image $sVal }
    -relief { $this.ibut configure -relief $sVal }
    -bd { $this.ibut configure -bd $sVal }
    -bg {
      $this.ibut      config -bg $sVal
      $this.ibut.icon config -bg $sVal
      $this.ibut.text config -bg $sVal -activeback $sVal
    }
  }
}

metawidget option Ibutton -image        {_opt -image}
metawidget option Ibutton -bd           {_opt -bd}
metawidget option Ibutton -borderwidth  {_opt -bd}
metawidget option Ibutton -relief       {_opt -relief}
metawidget option Ibutton -bg           {_opt -bg}
metawidget option Ibutton -background   {_opt -bg}

proc test {} {
  image create photo p1 -file ./demos/images/Compass.gif
  image create photo p2 -file ./demos/images/Notebook.gif

  pack [button .but0 -text Regular] -fill x
  pack [ibutton .ibt1 -border 5 -image p1 -text Compass] -fill x
  pack [ibutton .ibt2 -image p2 -text Notebook -command {after 1000}] -fill x
}

#test

