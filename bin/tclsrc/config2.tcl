#*****************************************************************************
#  ConfigCancel -- Arthur Taylor / MDL
#
# PURPOSE
#    Configure the program choices
#
# ARGUMENTS
#      rayName = Global array (structure) containing the main variables for
#                this program.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Copied from "config.tcl"
#
# NOTES
#*****************************************************************************
proc ConfigCancel {rayName} {

}

#*****************************************************************************
#  buildVariable -- Arthur Taylor / MDL
#
# PURPOSE
#    build the variable frame of the configuration window.
#
# ARGUMENTS
#   rayName = Global array (structure) containing the main variables for this
#             program.
#      cur2 = The frame to add the widget to.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Copied from "config.tcl"
#
# NOTES
#*****************************************************************************
proc buildVariable {rayName cur2} {
  upvar #0 $rayName ray

  set cur3 [frame $cur2.top]
   set cur3a [frame $cur3.lf]
    set cur4 [frame $cur3a.dat -relief ridge -bd 5]
      label $cur4.lab -text "Data Set"
      set dataLB $cur4.lb
      listbox $cur4.lb -exportselection false -yscrollcommand [list $cur4.ys set] \
              -xscrollcommand [list $cur4.xs set] -height 3 -width 10
      scrollbar $cur4.xs -orient horizontal -command [list $cur4.lb xview]
      scrollbar $cur4.ys -orient vertical -command [list $cur4.lb yview]
      grid $cur4.lab -column 0 -row 0 -columnspan 2 -sticky news
      grid $cur4.lb -column 0 -row 1 -sticky news
      grid $cur4.ys -column 1 -row 1 -sticky ns
      grid $cur4.xs -column 0 -row 2 -sticky ew
      grid rowconfigure $cur4 1 -weight 1
      grid columnconfigure $cur4 0 -weight 1
    set cur4 [frame $cur3a.grd -relief ridge -bd 5]
      label $cur4.lab -text "Grid Type"
      set gridLB $cur4.lb
      listbox $cur4.lb -exportselection false -yscrollcommand [list $cur4.ys set] \
              -xscrollcommand [list $cur4.xs set] -height 3 -width 10
      scrollbar $cur4.xs -orient horizontal -command [list $cur4.lb xview]
      scrollbar $cur4.ys -orient vertical -command [list $cur4.lb yview]
      grid $cur4.lab -column 0 -row 0 -columnspan 2 -sticky news
      grid $cur4.lb -column 0 -row 1 -sticky news
      grid $cur4.ys -column 1 -row 1 -sticky ns
      grid $cur4.xs -column 0 -row 2 -sticky ew
      grid rowconfigure $cur4 1 -weight 1
      grid columnconfigure $cur4 0 -weight 1
   set cur3b [frame $cur3.rt]
    set cur4 [frame $cur3b.sec -relief ridge -bd 5]
      label $cur4.lab -text "Sector"
      set sectLB $cur4.lb
      listbox $cur4.lb -exportselection false -yscrollcommand [list $cur4.ys set] \
              -xscrollcommand [list $cur4.xs set] -height 3 -width 20
      scrollbar $cur4.xs -orient horizontal -command [list $cur4.lb xview]
      scrollbar $cur4.ys -orient vertical -command [list $cur4.lb yview]
      grid $cur4.lab -column 0 -row 0 -columnspan 2 -sticky news
      grid $cur4.lb -column 0 -row 1 -sticky news
      grid $cur4.ys -column 1 -row 1 -sticky ns
      grid $cur4.xs -column 0 -row 2 -sticky ew
      grid rowconfigure $cur4 1 -weight 1
      grid columnconfigure $cur4 0 -weight 1
    set cur4 [frame $cur3b.var -relief ridge -bd 5]
      label $cur4.lab1 -text "Variable"
      label $cur4.lab2 -text "Status"
      set varLB $cur4.lb1
      set statLB $cur4.lb2
      set lbList [list $cur4.lb1 $cur4.lb2]
      listbox $cur4.lb1 -exportselection false \
              -yscrollcommand [list multi_scroll2 $cur4.ys $lbList] \
              -xscrollcommand [list $cur4.xs1 set] -height 3
      listbox $cur4.lb2 -exportselection false \
              -yscrollcommand [list multi_scroll2 $cur4.ys $lbList] \
              -xscrollcommand [list $cur4.xs2 set] -height 3 -width 15
      scrollbar $cur4.xs1 -orient horizontal -command [list $cur4.lb1 xview]
      scrollbar $cur4.xs2 -orient horizontal -command [list $cur4.lb2 xview]
      scrollbar $cur4.ys -orient vertical -command [list multi_scroll $lbList]
      grid $cur4.lab1 -column 0 -row 0 -sticky news
      grid $cur4.lab2 -column 1 -row 0 -sticky news
      grid $cur4.lb1 -column 0 -row 1 -sticky news
      grid $cur4.lb2 -column 1 -row 1 -sticky news
      grid $cur4.ys -column 2 -row 1 -sticky ns
      grid $cur4.xs1 -column 0 -row 2 -sticky ew
      grid $cur4.xs2 -column 1 -row 2 -sticky ew
      grid rowconfigure $cur4 1 -weight 1
      grid columnconfigure $cur4 0 -weight 1
      grid columnconfigure $cur4 1 -weight 1

    Pane_Create $cur3b.sec $cur3b.var -in $cur3b -orient horizontal -percent .4
    Pane_Create $cur3a.dat $cur3a.grd -in $cur3a -orient horizontal -percent .5
    Pane_Create $cur3a $cur3b -in $cur3 -orient horizontal -percent .3
#    pack $cur3.var $cur3.sec $cur3.grd $cur3.dat -side right -expand yes -fill both
  set cur3 [frame $cur2.bot -relief ridge -bd 5]
    set cur4 [frame $cur3.url]
      label $cur4.lab -text "URL:" -width 10
      entry $cur4.fixed -width 50 -textvariable $rayName\(config,URL,fixed) -state disabled -bg grey -justify right
      entry $cur4.dynam -width 50 -textvariable $rayName\(config,URL,dynamic)
      pack $cur4.lab $cur4.fixed $cur4.dynam -side left
    set cur4 [frame $cur3.local]
      label $cur4.lab -text "Local File:" -width 10
      entry $cur4.fixed -width 50 -textvariable $rayName\(config,local,fixed) -state disabled -bg grey -justify right
      entry $cur4.dynam -width 50 -textvariable $rayName\(config,local,dynamic)
      pack $cur4.lab $cur4.fixed $cur4.dynam -side left
    set cur4 [frame $cur3.label]
      label $cur4.lab -text "Label:" -width 10
      entry $cur4.fixed -width 50 -textvariable $rayName\(config,label,fixed) -state disabled -bg grey -justify right
      entry $cur4.dynam -width 50 -textvariable $rayName\(config,label,dynamic)
      pack $cur4.lab $cur4.fixed $cur4.dynam -side left
    set cur4 [frame $cur3.stat]
      label $cur4.lab -text "Status:" -width 10
      menubutton $cur4.opt -text "opnl" -direction right -menu $cur4.opt.m -relief raised -indicatoron true
      menu $cur4.opt.m -tearoff 0
      $cur4.opt.m add command -label "opnl" -command ""
      $cur4.opt.m add command -label "opnl-split" -command ""
      $cur4.opt.m add command -label "expr" -command ""
      $cur4.opt.m add command -label "expr-splt" -command ""
      label $cur4.lab2 -text "no '-split' means try to join 'VP.001-003' with 'VP.004-007' to get VP.001-007"
      pack $cur4.lab $cur4.opt $cur4.lab2 -side left
    set cur4 [frame $cur3.button]
      button $cur4.but -text "Accept"
      pack $cur4.but -side top
    pack $cur3.url $cur3.local $cur3.label $cur3.stat -side top -expand yes -fill both
    pack $cur3.button -side top -expand yes

  Pane_Create $cur2.top $cur2.bot -in $cur2 -orient vertical -percent .75
#  pack $cur2.top -side top -expand yes -fill both
#  pack $cur2.bot -side top -expand no

  set ray(config,URL,fixed) "/pub/SL.us008001/ST.opnl/DF.gr2/DC.ndfd/AR.conus/"
  set ray(config,URL,dynamic) "ds.maxt.bin"
  set ray(config,local,fixed) "e:/prj/ndfd/degrib/data/ndfd/conus/"
  set ray(config,local,dynamic) "maxt.bin"
  set ray(config,label,fixed) "ndfd/CONUS/"
  set ray(config,label,dynamic) "Days1-3/Maximum Temperature"

  $dataLB insert end NDFD
  $dataLB insert end NDGD

  $gridLB insert end NDFD

  $sectLB insert end CONUS
  $sectLB insert end "Puerto Rico"
  $sectLB insert end Hawaii
  $sectLB insert end Guam
  $sectLB insert end Alaska
  $sectLB insert end "North Hemi"


}

#*****************************************************************************
#  buildCustom -- Arthur Taylor / MDL
#
# PURPOSE
#    build the custom frame of the configuration window.
#
# ARGUMENTS
#   rayName = Global array (structure) containing the main variables for this
#             program.
#      cur2 = The frame to add the widget to.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Copied from "config.tcl"
#
# NOTES
#*****************************************************************************
proc buildCustom {rayName cur2} {
  upvar #0 $rayName ray

  set box [split $ray(custom,box) ,]
  set ray(custom,lat1) [lindex $box 0]
  set ray(custom,lon1) [lindex $box 1]
  set ray(custom,lat2) [lindex $box 2]
  set ray(custom,lon2) [lindex $box 3]

  set cur3 [frame $cur2.cgi]
    label $cur3.lab -text "URL of the CGI program"
    entry $cur3.ent -textvariable $rayName\(custom,URL)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.bk1 -height 20]
    label $cur2.lab -text "Currently the CGI program restricts to \
                           ranges of 20 degrees lat/lon"
  set cur3 [frame $cur2.bk2 -height 20]
  set cur3 [frame $cur2.lat2 -relief ridge -bd 5]
    label $cur3.lab -text "Upper (right) Latitude"
    entry $cur3.ent -textvariable $rayName\(custom,lat2)
    pack $cur3.lab -side top
    pack $cur3.ent -side top -expand yes -fill x
  set cur3 [frame $cur2.lon]
    set cur4 [frame $cur3.lf -bd 5 -relief ridge]
      label $cur4.lab -text "Left (lower) Longitude (+ east, -180..180) "
      entry $cur4.ent -textvariable $rayName\(custom,lon1)
      pack $cur4.lab -side top
      pack $cur4.ent -side top -expand yes -fill x
    set cur4 [frame $cur3.rt -bd 5 -relief ridge]
      label $cur4.lab -text "Right (upper) Longitude (+ east, -180..180) "
      entry $cur4.ent -textvariable $rayName\(custom,lon2)
      set ray(safeCustom,lon2) $ray(custom,lon2)
      pack $cur4.lab -side top
      pack $cur4.ent -side top -expand yes -fill x
    pack $cur3.lf $cur3.rt -side left -expand yes -fill both
  set cur3 [frame $cur2.lat1 -relief ridge -bd 5]
    label $cur3.lab -text "Lower (left) Latitude"
    entry $cur3.ent -textvariable $rayName\(custom,lat1)
    pack $cur3.lab -side top
    pack $cur3.ent -side top -expand yes -fill x
  pack $cur2.cgi $cur2.bk1 $cur2.lab $cur2.bk2 -side top \
       -expand no -fill x
  pack $cur2.lat2 -side top -expand no
  pack $cur2.lon -side top -expand no
  pack $cur2.lat1 -side top -expand no
}

#*****************************************************************************
#  buildServer -- Arthur Taylor / MDL
#
# PURPOSE
#    build the server frame of the configuration window.
#
# ARGUMENTS
#   rayName = Global array (structure) containing the main variables for this
#             program.
#      cur2 = The frame to add the widget to.
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Copied from "config.tcl"
#
# NOTES
#*****************************************************************************
proc buildServer {rayName cur2} {
  upvar #0 $rayName ray

  set cur3 [frame $cur2.ftpsite]
    label $cur3.lab -text "FTP Server"
    entry $cur3.ent -textvariable $rayName\(ftp,Server)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.user]
    label $cur3.lab -text "FTP User Name"
    entry $cur3.ent -textvariable $rayName\(ftp,User)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.pass]
    label $cur3.lab -text "FTP Password"
    entry $cur3.ent -textvariable $rayName\(ftp,Pass)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.ftpforeopnldir]
    label $cur3.lab -text "FTP NDFD Operational Root Dir"
    entry $cur3.ent -textvariable $rayName\(ftp,foreOpnlDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.ftpforeexprdir]
    label $cur3.lab -text "FTP NDFD Experimental Root Dir"
    entry $cur3.ent -textvariable $rayName\(ftp,foreExprDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.ftpguidopnldir]
    label $cur3.lab -text "FTP NDGD Operational Root Dir"
    entry $cur3.ent -textvariable $rayName\(ftp,guidOpnlDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.ftpguidexprdir]
    label $cur3.lab -text "FTP NDGD Experimental Root Dir"
    entry $cur3.ent -textvariable $rayName\(ftp,guidExprDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.bk -height 20]

  set cur3 [frame $cur2.httpsite]
    label $cur3.lab -text "HTTP Data Server"
    entry $cur3.ent -textvariable $rayName\(http,Server)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.httpproxy]
    label $cur3.lab -text "HTTP Proxy Server (blank if none)"
    entry $cur3.ent -textvariable $rayName\(http,Proxy)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.httpport]
    label $cur3.lab -text "HTTP Proxy Port (blank if none, example: 6588)"
    entry $cur3.ent -textvariable $rayName\(http,ProxyPort)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.httpforeopnldir]
    label $cur3.lab -text "HTTP NDFD Operational Root Dir"
    entry $cur3.ent -textvariable $rayName\(http,foreOpnlDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.httpforeexprdir]
    label $cur3.lab -text "HTTP NDFD Experimental Root Dir"
    entry $cur3.ent -textvariable $rayName\(http,foreExprDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.httpguidopnldir]
    label $cur3.lab -text "HTTP NDGD Operational Root Dir"
    entry $cur3.ent -textvariable $rayName\(http,guidOpnlDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.httpguidexprdir]
    label $cur3.lab -text "HTTP NDGD Experimental Root Dir"
    entry $cur3.ent -textvariable $rayName\(http,guidExprDir)
    pack $cur3.lab -side left
    pack $cur3.ent -side left -expand yes -fill x
  set cur3 [frame $cur2.bk2 -height 20]

  pack $cur2.ftpsite $cur2.user $cur2.pass $cur2.ftpforeopnldir $cur2.ftpforeexprdir \
       $cur2.ftpguidopnldir $cur2.ftpguidexprdir $cur2.bk \
       $cur2.httpsite $cur2.httpproxy $cur2.httpport \
       $cur2.httpforeopnldir $cur2.httpforeexprdir $cur2.httpguidopnldir $cur2.httpguidexprdir \
       $cur2.bk2 -side top -expand no -fill x
}

#*****************************************************************************
#  Configure -- Arthur Taylor / MDL
#
# PURPOSE
#    Configure the program choices
#
# ARGUMENTS
#      rayName = Global array (structure) containing the main variables for
#                this program.
#      (path,) = "FULL_PATH" section
#       (dir,) = "Directories" section
#    (imgGen,) = "ImageGen" section
#    (custom,) = "Custom" section
#       (ftp,) = "FTPSite" section
#      (http,) = "HTTPSite" section
#       (opt,) = "Options" section
# (subSector,) = "CONUS_SubSectors" section
#   (foreVar,) = "NDFD_Variables" section
#   (guidVar,) = "NDGD_Variables" section
#
# RETURNS void
#
# HISTORY
#  8/2006 Arthur Taylor (MDL): Copied from "config.tcl"
#
# NOTES
#*****************************************************************************
proc Configure {rayName} {
  upvar #0 $rayName ray

  set tl .work
  catch {destroy $tl}
  toplevel $tl
  wm protocol $tl WM_DELETE_WINDOW "ConfigCancel $rayName"
  set initWid 640
  set initHei 550
  wm minsize $tl $initWid $initHei
  wm title $tl "tkdegrib Configuration"

##### Build Main Window Tabs #####
  set cur [frame $tl.fr -width $initWid -height $initHei]
    set tabc [tabcontrol $cur.tc -width auto]
    ###### Next Line is because of a glitch in tabcontrol #####
    bind $tabc <Configure> "update; $tabc _drawTabs"

      ##### Build the variable Tab #####
#      set cur2 [frame $tabc.var -relief ridge -bd 5]
      set cur2 [frame $tabc.var]
      buildVariable $rayName $cur2
      $tabc insert variables 0 -text " Variables " -window $cur2

      ##### Build the sector Tab #####
#      set cur2 [frame $tabc.sct -relief ridge -bd 5]
#      $tabc insert sectors 0 -text " Sectors " -window $cur2

      ##### Build the "Server" Tab #####
      set cur2 [frame $tabc.etc -relief ridge -bd 5]
      buildServer $rayName $cur2
      $tabc insert extra 0 -text " Server " -window $cur2

      ##### Build the custom Tab #####
      set cur2 [frame $tabc.cust -relief ridge -bd 5]
      buildCustom $rayName $cur2
      $tabc insert custom 0 -text " Custom " -window $cur2

    set cur1 [frame $cur.bot]
      button $cur1.done -text "Ok" -command "ConfigValidate $rayName"
      button $cur1.cancel -text "Cancel" -command "ConfigCancel $rayName"
      pack $cur1.done $cur1.cancel -side left
    pack $cur.bot -anchor c -side bottom -expand no
    pack $cur.tc -side top -fill both -expand 1
  pack $tl.fr -fill both -expand 1
  update
  $tabc invoke variables
  wm minsize $tl 0 0
}
