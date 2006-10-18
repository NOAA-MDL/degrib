#!/usr/local/bin/wish

package require mkWidgets

proc metaDemo { lWidgets } {
  . config -minsize {320 200}

  menu .menu -tearoff 0
  menu .menu.file -tearoff 0
  .menu.file add command -label About... -command aboutBox
  .menu.file add separator
  .menu.file add command -label Exit -command exit
  menu .menu.view -tearoff 0
  menu .menu.wind -tearoff 0
  .menu.wind add command -label {Tile across}       -command {arrangeDocs tile}
  .menu.wind add command -label {Tile horizontally} -command {arrangeDocs horizontally}
  .menu.wind add command -label {Tile vertically}   -command {arrangeDocs vertically}
  .menu.wind add command -label {Cascade}           -command {arrangeDocs cascade}
  .menu.wind add separator
  .menu.wind add command -label {Arrange icons}     -command {arrangeDocs icons}
  .menu.wind add command -label {Minimize all}      -command {arrangeDocs minimize}
  .menu add cascade -menu .menu.file -label File
  .menu add cascade -menu .menu.view -label Metawidgets
  .menu add cascade -menu .menu.wind -label Window
  . config -menu .menu

  pack [frame .work -bg gray50] -fill both -expand 1

  tooltip   .ttip
  toolbar   .tbar
  statusbar .sbar

  foreach sFile [glob ./images/*.gif] {
    set sImage img[file rootname [file tail $sFile]]
    image create photo $sImage -file $sFile
  }

  set iCount 0
  foreach sWidget $lWidgets {
    if { $sWidget == {} } {
      .tbar add separator sep$iCount
    } else {
      set sToolb [.tbar add button tool$sWidget -image img$sWidget -command ".menu.view invoke $sWidget"]
      .ttip add $sToolb $sWidget
      .menu.view add command -label $sWidget -command test$sWidget
      incr iCount
    }
  }

  .sbar add statTime -image imgTinyclock
  .sbar push "$iCount metawidgets loaded"
  updateTime
}

proc updateTime {} {
  .sbar itemconf statTime -text [clock format [clock seconds] -format {%H:%M:%S }]
  after 1000 updateTime
}

proc aboutBox {} {
  window .about -title {About mkWidgets Demo}

  pack [label .about.icon -image imgLlama] -side left -padx 5
  pack [label .about.txt1 -text {Metawidgets with mkWidgets 1.0}] -pady 5
  pack [label .about.txt2 -text {http://ourworld.compuserve.com/homepages/mmg_kraus}]
  pack [button .about.okay -text {  Close  } -command {destroy .about} -pady 0] -pady 5
  grab .about
}

proc arrangeDocs { sHow } {
  ::Document::Arrange $sHow
}

proc testToolbar {} {
  if { ! [winfo exists .work.dtoo] } {
    document .work.dtoo -title Toolbar -image imgToolbar -width 160 -height 220
    .work.dtoo menu entryconf Close -command "destroy .work.dtoo"
    .work.dtoo pack [frame .work.dtoo.test -border 0 -bg white] -fill both -expand 1

    toolbar .work.dtoo.test.tbar
    .work.dtoo.test.tbar add button b1 -image imgFile
    .work.dtoo.test.tbar add button b2 -image imgFolder
    .work.dtoo.test.tbar add separator s1
    .work.dtoo.test.tbar add radiobutton r1 -group g1 -image imgLarge
    .work.dtoo.test.tbar add radiobutton r2 -group g1 -image imgIconbox
    .work.dtoo.test.tbar add radiobutton r3 -group g1 -image imgSmall
    .work.dtoo.test.tbar add radiobutton r4 -group g1 -image imgList
    .work.dtoo.test.tbar add separator s2
    .work.dtoo.test.tbar add checkbutton c1 -image imgNodes
    .work.dtoo.test.tbar add checkbutton c2 -image imgCompass

    pack [label .work.dtoo.test.text -bg white -wrap 200 -text {This toolbar has buttons, radiobuttons and checkbuttons. Drag it to other sides!}]
  } else {
    .work.dtoo raise
  }
}

proc testStatusbar {} {
  if { ! [winfo exists .work.dsta] } {
    document .work.dsta -title Statusbar -image imgStatusbar -width 160 -height 220
    .work.dsta menu entryconf Close -command "destroy .work.dsta"
    .work.dsta pack [frame .work.dsta.test -border 0] -fill both -expand 1

    pack [button .work.dsta.test.anim -text Animate -command {
      .work.dsta.test.tbar push {Loading something...}
      for { set f 0 } { $f <= 1 } { set f [expr $f+.2] } {
        .work.dsta.test.tbar config -progress $f
        after 100
      }
      .work.dsta.test.tbar config -progress 0
      .work.dsta.test.tbar pop
    } -pady 0] -fill x

    statusbar .work.dsta.test.tbar -text {Status Bar} -ticks 5
    .work.dsta.test.tbar add field1 -text { Any text } -image imgNotebook
  } else {
    .work.dsta raise
  }
}

proc testCalendar {} {
  if { ! [winfo exists .work.dcal] } {
    document .work.dcal -title Calendar -image imgCalendar -width 160 -height 220
    .work.dcal menu entryconf Close -command "destroy .work.dcal"
    .work.dcal pack [label .work.dcal.txt1 -text "Use the cursor keys or\nthe arrow symbols"]
    .work.dcal pack [calendar .work.dcal.cale]
    .work.dcal pack [label .work.dcal.txt2 -text "Double-click on the date\nto set today's date"]
  } else {
    .work.dcal raise
  }
}

proc testAclock {} {
  if { ! [winfo exists .work.dclk] } {
    document .work.dclk -title Clock -image imgAclock -width 160 -height 220
    .work.dclk menu entryconf Close -command "destroy .work.dclk"
    .work.dclk pack [label .work.dclk.txt1 -text "Drag the pointers or\nuse the cursor keys"]
    .work.dclk pack [label .work.dclk.txt2 -text "Double-click on the time to\nset the actual time"] -side bottom
    .work.dclk pack [aclock .work.dclk.clck] -fill both -expand 1
  } else {
    .work.dclk raise
  }
}

proc testCombobox {} {
  if { ! [winfo exists .work.dcbx] } {
    document .work.dcbx -title Combobox -image imgCombobox -width 160 -height 150
    .work.dcbx menu entryconf Close -command "destroy .work.dcbx"
    .work.dcbx pack [label .work.dcbx.txt1 -text "Below is a combobox:"]
    .work.dcbx pack [combobox .work.dcbx.cbx1 -entries {{Select a value} {from the list.} {Or just key in} {something else.}}]
    .work.dcbx pack [label .work.dcbx.txt2 -text "This one is restricted:"]
    .work.dcbx pack [combobox .work.dcbx.cbx2 -entries {{Select a value} {from the list.} {But you cannot} {enter anything.}} -state restricted]
    .work.dcbx pack [label .work.dcbx.txt3 -text "Cursor keys work, too.\nUse 'Esc' to get out."]
  } else {
    .work.dcbx raise
  }
}

proc testDocument {} {
  if { ! [winfo exists .work.ddoc] } {
    document .work.ddoc -title {Document Windows} -image imgDocument -width 250 -height 100
    .work.ddoc menu entryconf Close -command "destroy .work.ddoc"
    .work.ddoc pack [label .work.ddoc.txt1 -text "This and all the others are document windows\nAnything can be placed inside them.  " -border 1 -relief sunken -bg yellow] -fill both -expand 1
  } else {
    .work.ddoc raise
  }
}

proc testPane {} {
  if { ! [winfo exists .work.dpan] } {
    document .work.dpan -title {Paned Windows} -image imgPane -width 160 -height 220
    .work.dpan menu entryconf Close -command "destroy .work.dpan"
    .work.dpan pack [frame .work.dpan.frm0 -border 0] -fill both -expand 1
    pack [label .work.dpan.frm0.txt1 -text "This one resizes proportionally:"]
    pack [pane  .work.dpan.frm0.pan1 -width 3 -resize both] -fill both -expand 1
    .work.dpan.frm0.pan1 pack first  [text  .work.dpan.frm0.frm1 -border 1 -relief sunken -bg white -wrap word] -fill both -expand 1
    .work.dpan.frm0.pan1 pack second [frame .work.dpan.frm0.frm2 -border 1 -relief sunken -bg white] -fill both -expand 1
    .work.dpan.frm0.frm1 insert end "Use the mouse to drag the panes. Also, resize the window."

    pack [label .work.dpan.frm0.txt2 -text "The left window does not resize:"]
    pack [pane  .work.dpan.frm0.pan2 -width 3 -resize second] -fill both -expand 1
    .work.dpan.frm0.pan2 pack first  [frame .work.dpan.frm0.frm3 -border 1 -relief sunken -bg white] -fill both -expand 1
    .work.dpan.frm0.pan2 pack second [frame .work.dpan.frm0.frm4 -border 1 -relief sunken -bg white] -fill both -expand 1
  } else {
    .work.dpan raise
  }
}

proc testProgressbar {} {
  if { ! [winfo exists .work.dpro] } {
    document .work.dpro -title {Progress Bar} -image imgProgressbar -width 160 -height 120
    .work.dpro menu entryconf Close -command "destroy .work.dpro"
    .work.dpro pack [label .work.dpro.txt1 -text "Press the button below."]
    .work.dpro pack [progressbar .work.dpro.prog -width 120 -height 22] -padx 15 -pady 5
    .work.dpro pack [button .work.dpro.anim -text Animate -command {
      for { set i 0 } { $i <= 100 } { incr i 5 } {
        .work.dpro.prog set $i
        after 100
      }
      .work.dpro.prog set 0
    } -pady 0] -fill x
  } else {
    .work.dpro raise
  }
}

proc testScrollbox {} {
  if { ! [winfo exists .work.dsbx] } {
    document .work.dsbx -title {Scrollbox} -image imgScrollbox -width 160 -height 220
    .work.dsbx menu entryconf Close -command "destroy .work.dsbx"
    .work.dsbx pack [label .work.dsbx.txt1 -text "A scrollbox is a listbox\nwith automatic scrollbars"]
    .work.dsbx pack [scrollbox .work.dsbx.sbox] -fill both -expand 1
    .work.dsbx.sbox insert end {Here is some text.} {There's some more.} "It is [clock format [clock seconds]]" \
                               {Current directory:} "[glob ./*]"
    .work.dsbx.sbox config -scrollbar auto
  } else {
    .work.dsbx raise
  }
}

proc testTabcontrol {} {
  if { ! [winfo exists .work.dtab] } {
    document .work.dtab -title {Tab Control} -image imgTabcontrol -width 220 -height 160
    .work.dtab menu entryconf Close -command "destroy .work.dtab"
    .work.dtab pack [label .work.dtab.txt1 -text "These tabs look quite nice:"]
    .work.dtab pack [tabcontrol .work.dtab.tabc -width auto] -fill both -expand 1 -padx 10 -pady 10

    label .work.dtab.tabc.gnrl -bg white -border 1 -relief sunken -text "Tabs can be dynamically\ncreated and deleted."
    label .work.dtab.tabc.size -bg white -border 1 -relief sunken -text "Tabs can have a constant size\nor resize with their parent widget."
    label .work.dtab.tabc.rows -bg white -border 1 -relief sunken -text "A tab control can have\nmore than one row of tabs."
    .work.dtab.tabc insert general 0 -text " General " -window .work.dtab.tabc.gnrl
    .work.dtab.tabc insert edit    0 -text " Size "    -window .work.dtab.tabc.size
    .work.dtab.tabc insert find    0 -text " Rows "    -window .work.dtab.tabc.rows
    .work.dtab.tabc invoke general
  } else {
    .work.dtab raise
  }
}

proc testTextframe {} {
  if { ! [winfo exists .work.dtfr] } {
    document .work.dtfr -title {Text Frame} -image imgTextframe -width 180 -height 140
    .work.dtfr menu entryconf Close -command "destroy .work.dtfr"
    .work.dtfr pack [label .work.dtfr.txt1 -text "A frame with some text:"]
    .work.dtfr pack [textframe .work.dtfr.tfrm -text " Language "]

    foreach sLang {English German Spanish French} iCol {0 0 1 1} iRow {0 1 0 1} {
      radiobutton .work.dtfr.tfrm.l$sLang -value $sLang -border 0 -text $sLang
      .work.dtfr.tfrm grid .work.dtfr.tfrm.l$sLang -col $iCol -row $iRow -sticky w
    }
    .work.dtfr.tfrm.lEnglish invoke
  } else {
    .work.dtfr raise
  }
}

proc testTooltip {} {
  if { ! [winfo exists .work.dtip] } {
    document .work.dtip -title Tooltip -image imgTooltip -width 125 -height 115
    .work.dtip menu entryconf Close -command "destroy .work.dtip"
    .work.dtip pack [label .work.dtip.txt1 -text "Move the mouse over\nthe sqares and wait"] -fill x
    .work.dtip pack [label .work.dtip.txt2 -text "Tooltips are also\nactive in the toolbar"] -side bottom -fill x

    tooltip .work.dtip.ttip -delay 500
    foreach sColor {red green blue yellow black white} {
      frame .work.dtip.r$sColor -bg $sColor
      .work.dtip pack .work.dtip.r$sColor -side left -fill both -expand 1
      .work.dtip.ttip add .work.dtip.r$sColor "A $sColor rectangle"
    }
  } else {
    .work.dtip raise
  }
}

proc testGridcontrol {} {
  if { ! [winfo exists .work.dgri] } {
    document .work.dgri -title Gridcontrol -image imgGridcontrol -width 125 -height 115
    .work.dgri menu entryconf Close -command "destroy .work.dgri"
    .work.dgri pack [gridcontrol .work.dgri.grid -font {Courier 12}] -fill both -expand 1

    .work.dgri.grid column insert Col1 end -text Word   -width 60
    .work.dgri.grid column insert Col2 end -text Number -width 60 -align right
    .work.dgri.grid column insert Col3 end -text Read!  -width 60 -align center

    set i 0
    foreach sWord {The quick brown fox jumps over the lazy dog} \
            sRead {Resize the column and see some nice text clipping!} {
      incr i
      .work.dgri.grid insert end [list $sWord $i $sRead]
    }

    .work.dgri.grid tag add tYellow Col3.*
    .work.dgri.grid tag config tYellow -background yellow
  } else {
    .work.dgri raise
  }
}

proc testListcontrol {} {
  if { ! [winfo exists .work.dlis] } {
    document .work.dlis -title Listcontrol -image imgListcontrol -width 125 -height 115
    .work.dlis menu entryconf Close -command "destroy .work.dlis"
    .work.dlis pack [listcontrol .work.dlis.list] -fill both -expand 1

    .work.dlis.list column insert Col1 end -text Icon  -width 120
    .work.dlis.list column insert Col2 end -text Size  -width 60
    .work.dlis.list column insert Col3 end -text Type  -width 60

    set iRow 1
    foreach sImage [image names] \
            sRead {Drag the column and see some nice text clipping!} {
      if { [image width $sImage] != 16 } continue

      set sSize [image width $sImage]x[image height $sImage]
      set sType [image type $sImage]

      .work.dlis.list insert end [list " $sImage" $sSize $sType]
      .work.dlis.list image create Col1.$iRow -image $sImage
      incr iRow
    }
  } else {
    .work.dlis raise
  }
}

proc testIconbox {} {
  if { ! [winfo exists .work.dibo] } {
    document .work.dibo -title Iconbox -image imgIconbox -width 125 -height 115
    .work.dibo menu entryconf Close -command "destroy .work.dibo"

    .work.dibo pack [frame .work.dibo.view -border 0] -side top -anchor w
    pack [button .work.dibo.view.lrge -image imgLarge -command {.work.dibo.ibox config -view large}] -side left
    pack [button .work.dibo.view.smll -image imgSmall -command {.work.dibo.ibox config -view small}] -side left
    pack [button .work.dibo.view.list -image imgList  -command {.work.dibo.ibox config -view list}] -side left
    .work.dibo pack [iconbox .work.dibo.ibox -columns {Object {Description l 200}}] -fill both -expand 1

    .work.dibo.ibox insert end object1 -text "A Folder"   -image imgFolder32 -smallimage imgFolder -values {{These objects here are hard coded}}
    .work.dibo.ibox insert end object2 -text "Another Folder" -image imgFolder32 -smallimage imgFolder -values {{... and don't have any real meaning}}
    .work.dibo.ibox insert end object3 -text "File ABC"  -image imgFile32 -smallimage imgFile -values {{This could represent a file.}}
    .work.dibo.ibox insert end object4 -text "Compass"   -image imgCompass32 -smallimage imgCompass -values {{No fancy icons, no user acceptance...}}
    .work.dibo.ibox insert end object5 -text "Text File" -image imgNotebook32 -smallimage imgNotebook -values {{Icons can have individual bindings}}

    .work.dibo.ibox selection set object1
  } else {
    .work.dibo raise
  }
}

proc testTreecontrol {} {
  global lWidgets

  if { ! [winfo exists .work.dtre] } {
    document .work.dtre -title Treecontrol -image imgTreecontrol -width 125 -height 115
    .work.dtre menu entryconf Close -command "destroy .work.dtre"
    .work.dtre pack [treecontrol .work.dtre.tree -text {A Static Tree} -image imgNodes] -fill both -expand 1

    .work.dtre.tree insert framework -text Framework      -image imgFolder
    .work.dtre.tree insert metawids  -text Metawidgets    -image imgFolder      -parent framework
    .work.dtre.tree insert toolbars  -text Toolbars       -image imgFolder      -parent framework
    .work.dtre.tree insert toolbar1  -text CommonElements -image imgToolbar     -parent toolbars
    .work.dtre.tree insert toolbar2  -text DrawingTools   -image imgToolbar     -parent toolbars
    .work.dtre.tree insert statusbar -text Statusbar      -image imgStatusbar   -parent framework

    .work.dtre.tree insert dialogs   -text Dialogs        -image imgFolder
    .work.dtre.tree insert dialog1   -text Preferences    -image imgTabcontrol  -parent dialogs
    .work.dtre.tree insert dialog2   -text DatabaseQuery  -image imgCompass     -parent dialogs
    .work.dtre.tree insert dialog3   -text WhateverElse   -image imgNotebook    -parent dialogs

    .work.dtre.tree insert windows   -text Windows        -image imgFolder
    .work.dtre.tree insert window1   -text Spreadsheet    -image imgGridcontrol -parent windows
    .work.dtre.tree insert window2   -text Treeview       -image imgTreecontrol -parent windows
    .work.dtre.tree insert window3   -text ObjectBrowser  -image imgListcontrol -parent windows

    foreach sWidget $lWidgets {
      if { $sWidget == {} } continue
      .work.dtre.tree insert mw$sWidget -text $sWidget -image img$sWidget -parent metawids
    }
  } else {
    .work.dtre raise
  }
}

proc testIbutton {} {
  if { ! [winfo exists .work.dibu] } {
    document .work.dibu -title {Iconic Button} -image imgIbutton -width 200 -height 120
    .work.dibu menu entryconf Close -command "destroy .work.dibu"
    .work.dibu pack [label .work.dibu.txt1 -text "Ibuttons are buttons with an image"] -fill x
    .work.dibu pack [ibutton .work.dibu.ibut1 -image imgFolder -text "Create a folder..."] -fill x
    .work.dibu pack [ibutton .work.dibu.ibut2 -image imgNotebook -text "Make a note..."] -fill x
  } else {
    .work.dibu raise
  }
}

proc testSpinentry {} {
  if { ! [winfo exists .work.dspi] } {
    document .work.dspi -title {Spinentry} -image imgSpinentry -width 100 -height 100
    .work.dspi menu entryconf Close -command "destroy .work.dspi"
    .work.dspi pack [label .work.dspi.txt1 -text "A spinentry is almost\nlike an entry"] -fill x
    .work.dspi pack [spinentry .work.dspi.spi1 -value 123] -fill x
    .work.dspi pack [spinentry .work.dspi.spi2 -value 0 -minimum -10 -maximum 10 -step .2] -fill x
  } else {
    .work.dspi raise
  }
}

set lWidgets {Textframe Ibutton Scrollbox Combobox Spinentry Tabcontrol Pane {} \
              Document Toolbar Statusbar Tooltip {} \
              Gridcontrol Listcontrol Iconbox Treecontrol {} \
              Progressbar Calendar Aclock}

metaDemo $lWidgets

