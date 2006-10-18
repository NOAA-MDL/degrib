#*****************************************************************************
# <atdir3.tcl> :: Tcl/Tk Windows 3.1/95 & Hp-Unix
#
# Purpose:
#     This is another attempt at generating a generic file/directory selecter
#   that is powerful, easy to use, and efficient
#
# Files Needed:
#   Source: None
#   Input: None
#   Output: None
#
# Procedures: (P=Public) (S=private)
#  () AT_DirGetDir {df dir_name}
#  () AT_DirChangeDir {df dir_name flag}
#  () AT_Dir_Cget {df dir_name option}
#  () AT_Dir_Configure {df dir_name args}
#  () AT_Dir {df dir_name args}
#  -----------------------------
#  () AT_Demo1 {}
#  () AT_Demo2 {{path [pwd]} {ray_name demo2_ray} {tl .foo2}}
#  () AT_Demo3 {{path [pwd]} {file ""} {ray_name demo3_ray} {tl .foo3}}
#  () AT_Demo4 {{path [pwd]} {file ""} {ray_name demo3_ray} {tl .foo3}}
#
# History:
#    8/1998 Arthur Taylor (RDC/TDL) Commented
#
# Notes:
#*****************************************************************************
# Global Variables:
#   $dir_name (Used to store global data associated with a directory entry)
#     $df same index as listbox frame.
#     ($df,-dir)    Current desired directory for the widget (may not be a
#                   directory)
#     ($df,-secondary) Secondary directory to auto check (doesn't change,
#                   so look first in -dir and then in -secondary.  If file
#                   names match use -dir copy.)
#     ($df,seclist) current list of files actually located on secondary drive.
#     ($df,cur_dir) Current actual directory for widget (is a directory)
#     ($df,-filter) Current filter for the widget
#     ($df,-filterCmd) Current filter Command for the widget
#     ($df,-sortCmd) Current sort Command for the widget
#     ($df,-CB)     Current data to display (0 files only), (1 directories
#                   only) (2 both files and directories)
#     ($df,-show)   show dir entry (top), filter entry(mid), or both (both)
#     ($df,-slave)  A widget to update when this widget changes directories.
#     ($df,-select_cmd) A command to call when a file selection is made.
#     ($df,-command) A command to call when this widget changes directories.
#                   the command is called using:
#                   eval <command> -dir <dirlist> -files <filelist>
#                   dirlist: all subdirectories of -dir (not fullpath)
#                   filelist: all files of -dir
#     ($df,is_slave) 0 if not updated by a different widget.
#                   else frame of widget it relies on for updates.
#     ($df,-fullpath) 1 show full path of subdirectories, 0 just subdirectory
#     ($df,-secCaseSensitive) 0 secondary drive is not case sensitive.
#                             1 secondary drive is case sensitive.
#                           (during matches to see if it is in main directory)
#####

#*****************************************************************************
#  <AT_DirGetDir>
#
# Purpose:
#     Update the dir listing.
#
# Variables:(I=input)(O=output)(G=global)
#   df         (I) Tcl/Tk pathname of frame around dir-widget
#   dir_name   (I) The name of a global array to store dir info in.
#
# Returns: NULL
#
# FILE:
#
# History:
#   11/1997 Arthur Taylor (RDC/TDL) Created
#    8/1998 Arthur Taylor (RDC/TDL) Re-visited
#
# Notes:
#   Do we want a DIR($df,current_dir)?
#   Find cleaner way than cd pwd cd?
#   Files/Directories ~* are ignored (Thinks this is a refrence to unix's
#     user ~*, but in DOS we are allowed ~* files, so I just ignore them)
#*****************************************************************************
proc AT_DirGetDir {df dir_name} {
  upvar #0 $dir_name DIR

  if {[info exists DIR($df,-dir)] != 1} {
    tk_messageBox -message "Error: No variable $DIR($df,-dir) when calling\
          AT_DirGetDir.\n Probably means problems using AT_Dir widget"
    return 0
  }

# Check if this is a slave of a different widget, if so call its master.
  if {$DIR($df,is_slave) != 0} {
    AT_DirGetDir $DIR($df,is_slave) $dir_name
    return 0
  }

# Make sure the directory is valid and reduced ie c:/foo/.. == c:/
  if {[file isdirectory $DIR($df,-dir)] != 1} {
    if {[file exist $DIR($df,-dir)]} {
      set DIR($df,-dir) $DIR($df,cur_dir)
      return -1
    } else {
      set ans [tk_messageBox -message "$DIR($df,-dir) does not exist. Create it?" \
            -type yesno]
      if {$ans == "yes"} {
        file mkdir $DIR($df,-dir)
      } else {
        set DIR($df,-dir) $DIR($df,cur_dir)
        return -1
      }
    }
  }
  set current [pwd]
  set last [string index $DIR($df,-dir) [expr {[string length $DIR($df,-dir)] -1}]]
  if {$last == ":"} {
#    cd [file join $DIR($df,-dir) /]
    cd $DIR($df,-dir)\\
  } else {
    cd $DIR($df,-dir)
  }
  set DIR($df,-dir) [pwd]
  cd $current
  set DIR($df,cur_dir) $DIR($df,-dir)
  $df.top.entry xview end

  set dirlist "<..>"
  set filelist ""
  set filter $DIR($df,-filter)
  set filterCmd $DIR($df,-filterCmd)
  set sortCmd $DIR($df,-sortCmd)
  set direct $DIR($df,-dir)
  set secdir $DIR($df,-secondary)
  set seclist ""

#Check if widget has slave.  If so use slave's filter, master's direct.
  if {[winfo exists $DIR($df,-slave)]} {
    set has_slave 1
    set df2 $DIR($df,-slave)
    if {[info exist DIR($df2,-dir)] != 1} {
      tk_messageBox -message "Error: No variable $DIR($df2,-dir) when calling\
            AT_DirGetDir.\n Probably means problems linking to AT_Dir widgets"
      return 0
    }
    set filter $DIR($df2,-filter)
    set filterCmd $DIR($df,-filterCmd)
    set sortCmd $DIR($df,-sortCmd)
  } else {
    set has_slave 0
# If we don't display any files, and there isn't any associated command,
# set filter to null, which gives nothing that the file can match against.
    if {($DIR($df,-CB) == 1) && ($DIR($df,-command) == "")} {
      set filter ""
    }
  }

  foreach i [glob -nocomplain $direct/*] {
    if {[file isdirectory $i]} {
      if {$DIR($df,-fullpath)==1} {
        lappend dirlist <$i>
      } else {
        lappend dirlist <[file tail $i]>
      }
    } else {
# Basically if it is found in the filter search, then we check if
#  if it passes all of the filterCmd, if so we keep it, otherwise we don't.
      foreach j $filter {
        if {[string match $j [file tail $i]]} {
          set f_keep 1
          foreach j $filterCmd {
            if {[llength $filterCmd] == 2} {
              set cmd [lindex $filterCmd 0]
              set arg [lindex $filterCmd 1]
              if {! [$cmd $i $arg]} {
                set f_keep 0
                break
              }
            } else {
              if {! [$filterCmd $i]} {
                set f_keep 0
                break
              }
            }
          }
          if {$f_keep == 1} {
            lappend filelist [file tail $i]
          }
          break
        }
      }
    }
  }

  if {$secdir != "NULL"} {
    foreach i [glob -nocomplain $secdir/*] {
      if {![file isdirectory $i]} {
# Basically if it is found in the filter search, then we check if
#  if it passes all of the filterCmd, if so we keep it, otherwise we don't.
        foreach j $filter {
          if {[string match $j [file tail $i]]} {
            set f_keep 1
            foreach j $filterCmd {
              if {[llength $filterCmd] == 2} {
                set cmd [lindex $filterCmd 0]
                set arg [lindex $filterCmd 1]
                if {! [$cmd $i $arg]} {
                  set f_keep 0
                  break
                }
              } else {
                if {! [$filterCmd $i]} {
                  set f_keep 0
                  break
                }
              }
            }
            if {$f_keep == 1} {
# Now we check if it is already in filelist... if not we store it there,
# and in seclist
              if {[lsearch $filelist [file tail $i]] == -1} {
                if {$DIR($df,-secCaseSensitive)!=1} {
                  if {[lsearch $filelist [string tolower [file tail $i]]] == -1} {
                    if {[lsearch $filelist [string toupper [file tail $i]]] == -1} {
                      lappend filelist [file tail $i]
                      lappend seclist [file tail $i]
                    }
                  }
                } else {
                  lappend filelist [file tail $i]
                  lappend seclist [file tail $i]
                }
              }
            }
            break
          }
        }
      }
    }
  }
  set DIR($df,seclist) $seclist

# Insert it into the widget's listbox.
  set f_sort_dirlist 0
  set f_sort_filelist 0
  set dirlist2 ""
  $df.bot.lb delete 0 end
  set j 0

  if {$DIR($df,-CB) != 0} {
    if {$sortCmd != ""} {
      if {[llength $sortCmd] == 2} {
        set cmd [lindex $sortCmd 0]
        set arg [lindex $sortCmd 1]
        set dirlist [$cmd $arg $dirlist 0]
      } else {
        set dirlist [$sortCmd $dirlist 0]
      }
    } else {
      set dirlist [lsort -increasing -dictionary $dirlist]
    }
    set f_sort_dirlist 1
    foreach i $dirlist {
      $df.bot.lb insert $j $i
      incr j 1
    }
  }
  if {$DIR($df,-CB) != 1} {
    if {$sortCmd != ""} {
      if {[llength $sortCmd] == 2} {
        set cmd [lindex $sortCmd 0]
        set arg [lindex $sortCmd 1]
        set filelist [$cmd $arg $filelist 1]
      } else {
        set filelist [$sortCmd $filelist 1]
      }
    } else {
      set filelist [lsort -increasing -dictionary $filelist]
    }
    set f_sort_filelist 1
    foreach i $filelist {
      $df.bot.lb insert $j $i
      incr j 1
    }
  }

# Call master's associated command.
  if {$DIR($df,-command) != ""} {
    if {$f_sort_dirlist != 1} {
      set dirlist [lsort -increasing -dictionary $dirlist]
      set f_sort_dirlist 1
    }
    if {$DIR($df,-fullpath) == 0} {
      foreach i $dirlist {
        lappend dirlist2 [string range $i 1 [expr {[string length $i] -2}]]
      }
    } else {
      foreach i $dirlist {
        set tmp [string range $i 1 [expr {[string length $i] -2}]]
        lappend dirlist2 [lindex [split $tmp /] end]
      }
    }

    if {$f_sort_filelist != 1} {
      set filelist [lsort -increasing -dictionary $filelist]
      set f_sort_filelist 1
    }
# Create filelist2 from filelist (using dir for directory, unless file
# is also in secondary list, in which case use secondary)
    set filelist2 ""
    foreach i $filelist {
      if {[lsearch $seclist $i] == -1} {
        lappend filelist2 [file join $direct $i]
      } else {
        lappend filelist2 [file join $secdir $i]
      }
    }
    set temp $DIR($df,-command)
    set temp [concat $temp [list -dir $dirlist2 -files $filelist2]]
    eval $temp
  }

# If widget doesn't have slave, done.
  if {$has_slave == 0} {
    return 0
  }

# Otherwise update slave's listbox.
  $df2.bot.lb delete 0 end
  set j 0
  if {$DIR($df2,-CB) != 0} {
    if {$f_sort_dirlist != 1} {
      set dirlist [lsort -increasing -dictionary $dirlist]
      set f_sort_dirlist 1
    }
    foreach i $dirlist {
      $df2.bot.lb insert $j $i
      incr j 1
    }
  }
  if {$DIR($df2,-CB) != 1} {
    if {$f_sort_filelist != 1} {
      set filelist [lsort -increasing -dictionary $filelist]
      set f_sort_filelist 1
    }
    foreach i $filelist {
      $df2.bot.lb insert $j $i
      incr j 1
    }
  }

# Call slave's associated command?
  if {$DIR($df2,-command) != ""} {
    if {$dirlist2 == ""} {
      if {$f_sort_dirlist != 1} {
        set dirlist [lsort -increasing -dictionary $dirlist]
        set f_sort_dirlist 1
      }
      if {$DIR($df,-fullpath) == 0} {
        foreach i $dirlist {
          lappend dirlist2 [string range $i 1 [expr {[string length $i] -2}]]
        }
      } else {
        foreach i $dirlist {
          set tmp [string range $i 1 [expr {[string length $i] -2}]]
          lappend dirlist2 [lindex [split $tmp /] end]
        }
      }
    }

    if {$f_sort_filelist != 1} {
      set filelist [lsort -increasing -dictionary $filelist]
      set f_sort_filelist 1
    }
# Create filelist2 from filelist (using dir for directory, unless file
# is also in secondary list, in which case use secondary)
    set filelist2 ""
    foreach i $filelist {
      if {[lsearch $seclist $i] == -1} {
        lappend filelist2 [file join $direct $i]
      } else {
        lappend filelist2 [file join $secdir $i]
      }
    }
    set temp $DIR($df2,-command)
    set temp [concat $temp [list -dir $dirlist2 -files $filelist2]]
    eval $temp
  }
  return 0
}

#*****************************************************************************
#  <AT_DirChangeDir>
#
# Purpose:
#     Handles listbox selections (changing to new directories) and making
#   sure the current selection is hightlighted.
#
# Variables:(I=input)(O=output)(G=global)
#   df         (I) Tcl/Tk pathname of frame around dir-widget
#   dir_name   (I) The name of a global array to store dir info in.
#   flag       (I) 0 if return, -1 if up arrow, 1 if down arrow
#
# Returns: NULL
#
# FILE:
#
# History:
#   11/1997 Arthur Taylor (RDC/TDL) Created
#    8/1998 Arthur Taylor (RDC/TDL) Re-visited
#
# Notes:
#*****************************************************************************
proc AT_DirChangeDir {df dir_name flag} {
  upvar #0 $dir_name DIR

  if {[info exists DIR($df,-dir)] != 1} {
    tk_messageBox -message "Error: No variable $DIR($df,-dir) when calling\
          AT_DirChangeDir.\n Probably means problems using AT_Dir widget"
    return
  }
#  set temp [$df.bot.lb index anchor]
  set temp [$df.bot.lb curselection]
  if {$temp == ""} {
    # User selected NULL line.
    return
  }
  if {$flag == 0} {
    if {[llength $temp] == 1} {
#      set val [$df.bot.lb get [lindex $temp end]]
      set val [$df.bot.lb get $temp]
      set last [expr {[string length $val] -1}]
 # Check if the selected item is thought to be a directory.
      if {([string index $val 0] == "<") && ([string index $val $last] == ">")} {
        set val [string range $val 1 [expr {$last -1}]]
        if {$DIR($df,-fullpath) == 1} {
          set val [lindex [split $val /] end]
        }
        if {$DIR($df,is_slave) == 0} {
          set DIR($df,-dir) [file join $DIR($df,-dir) $val]
          AT_DirGetDir $df $dir_name
        } else {
          set df2 $DIR($df,is_slave)
          set DIR($df2,-dir) [file join $DIR($df2,-dir) $val]
          AT_DirGetDir $df2 $dir_name
        }
      } else {
        if {$DIR($df,-select_cmd) != ""} {
          eval "$DIR($df,-select_cmd)"
        }
      }
    }
  } else {
    $df.bot.lb selection set [expr {$temp + $flag}]
  }
}

#*****************************************************************************
#  <AT_Dir_Cget>
#
# Purpose:
#     Get an options associated with an AT_Dir widget
#
# Variables:(I=input)(O=output)(G=global)
#   df         (I) Tcl/Tk pathname of frame around dir-widget
#   dir_name   (I) The name of a global array to store dir info in.
#   option     (I) Allowed option to look at: (any of the following)
#     -dir     (I) The current directory (is a valid directory).
#     -filter  (I) The initial file-filter
#     -filterCmd (I) The initial list of tcl/tk commands to use as addition
#                  filters.
#     Files    (I) Resultant files
#     Subs     (I) Resultant Subdirectories (not fullpath)
#     curselect(I) Returns anything currently selected
#     curfiles (I) Returns only files that are currently selected
#     CURfiles (I) Returns full path of currently selected files.
#     listpath (I) Return the path of the listbox (for more binds)
#
# Returns: value of option.
#
# FILE:
#
# History:
#   11/1997 Arthur Taylor (RDC/TDL) Created
#    8/1998 Arthur Taylor (RDC/TDL) Re-visited
#
# Notes:
#   may want flag for browse vs extended
#*****************************************************************************
proc AT_Dir_Cget {df dir_name option} {
  upvar #0 $dir_name DIR

  if {[info exists DIR($df,-dir)] != 1} {
    tk_messageBox -message "Error: No variable $DIR($df,-dir) when calling\
          AT_Dir_Cget.\n Probably means problems using AT_Dir widget"
    return
  }

  if {$option == "listpath"} {
    return $df.bot.lb
  }

  if {$option == "-dir"} {
    if {$DIR($df,is_slave) != 0} {
      return $DIR($DIR($df,is_slave),cur_dir)
    } else {
      return $DIR($df,cur_dir)
    }

  } elseif {$option == "-filter"} {
    if {($DIR($df,-slave) != "") && [winfo exists $DIR($df,-slave)]} {
      return $DIR($DIR($df,-slave),-filter)
    } else {
      return $DIR($df,-filter)
    }

  } elseif {$option == "-filterCmd"} {
    if {($DIR($df,-slave) != "") && [winfo exists $DIR($df,-slave)]} {
      return $DIR($DIR($df,-slave),-filterCmd)
    } else {
      return $DIR($df,-filterCmd)
    }

  } elseif {$option == "-sortCmd"} {
    if {($DIR($df,-slave) != "") && [winfo exists $DIR($df,-slave)]} {
      return $DIR($DIR($df,-slave),-sortCmd)
    } else {
      return $DIR($df,-sortCmd)
    }

  } elseif {$option == "Files"} {
    if {$DIR($df,-CB) == 1} {
      tk_messageBox -message "Error: can't AT_Dir_Cget ... Files from a\
            subdirectory only AT_Dir widget."
      return -1
    }
    set ans1 [$df.bot.lb get 0 end]
    set ans2 ""
    foreach val $ans1 {
      set last [expr {[string length $val] -1}]
   # Check if the selected item is thought to be not a directory.
      if {([string index $val 0] != "<") || \
          ([string index $val $last] != ">")} {
        lappend ans2 $val
      }
    }
    return $ans2

  } elseif {($option == "curfiles") || ($option == "CURfiles")} {
    set ans1 [$df.bot.lb curselection]
    set ans2 ""
    foreach i $ans1 {
      set val [$df.bot.lb get $i]
      set last [expr {[string length $val] -1}]
   # Check if the selected item is thought to be not a directory.
      if {([string index $val 0] != "<") || \
          ([string index $val $last] != ">")} {
        lappend ans2 $val
      }
    }
    if {$option == "curfiles"} {
      return $ans2
    } else {
      set ans3 ""
      foreach file $ans2 {
        if {[lsearch $DIR($df,seclist) $file] == -1} {
          if {$DIR($df,is_slave) != 0} {
            lappend ans3 [file join $DIR($DIR($df,is_slave),cur_dir) $file]
          } else {
            lappend ans3 [file join $DIR($df,cur_dir) $file]
          }
        } else {
          lappend ans3 [file join $DIR($df,-secondary) $file]
        }
      }
      if {[llength $ans2] == 1} {
        set ans3 [lindex $ans3 0]
      }
      return $ans3
    }

  } elseif {$option == "curselect"} {
    set ans1 [$df.bot.lb curselection]
    set ans2 ""
    foreach i $ans1 {
      set val [$df.bot.lb get $i]
      set last [expr {[string length $val] -1}]
   # Check if the selected item is thought to be a directory.
      if {([string index $val 0] == "<") && \
          ([string index $val $last] == ">")} {
        set val [string range $val 1 [expr {$last -1}]]
        if {$DIR($df,-fullpath) == 0} {
          lappend ans2 $val
        } else {
          lappend ans2 [lindex [split $val /] end]
        }
      } else {
        lappend ans2 $val
      }
    }
    return $ans2

  } elseif {$option == "Subs"} {
    if {$DIR($df,-CB) == 0} {
      tk_messageBox -message "Error: can't AT_Dir_Cget ... Subs from a\
            filelist only AT_Dir widget."
      return -1
    }
    set ans1 [$df.bot.lb get 0 end]
    set ans2 ""
    foreach val $ans1 {
      set last [expr {[string length $val] -1}]
   # Check if the selected item is thought to be a directory.
      if {([string index $val 0] == "<") && \
          ([string index $val $last] == ">")} {
        set val [string range $val 1 [expr {$last -1}]]
        if {$DIR($df,-fullpath) == 0} {
          lappend ans2 $val
        } else {
          lappend ans2 [lindex [split $val /] end]
        }
      }
    }
    return $ans2
  }
  tk_messageBox -message "AT_Dir_Cget: Unknown option: $option "
  return -1
}

#*****************************************************************************
#  <AT_Dir_Configure>
#
# Purpose:
#     Configure options associated with a AT_Dir widget
#
# Variables:(I=input)(O=output)(G=global)
#   df         (I) Tcl/Tk pathname of frame around dir-widget
#   dir_name   (I) The name of a global array to store dir info in.
#   args       (I) Extra arguments for the main frame, or
#     -dir     (I) The initial directory
#     -filter  (I) The initial file-filter
#     -filterCmd (I) The initial list of tcl/tk commands to use as addition
#                  filters.
#     -CB      (I) 0 list files, 1 list dirs, 2 list both with <>
#     -show    (I) top=Show top frame, mid=Show mid frame, both=show both
#     -slave   (I) The frame of a slave (only applicable if -show = top
#                  Used for linking a filelist with a directory list
#     -select_cmd (I) A command to call when a file selection is made.
#     -command (I) A command to call each time a new dir is made.
#                  "eval [$command] -dir DirList -files FileList"
#     -lbHeight(I) The height of the listbox
#     -fullpath (I) 1 show full path of subdirectories, 0 just subdirectory
#     -secondary (I) New dir to treat as secondary directory.
#
# Returns: NULL
#
# FILE:
#
# History:
#   11/1997 Arthur Taylor (RDC/TDL) Created
#    8/1998 Arthur Taylor (RDC/TDL) Re-visited
#
# Notes:
#   may want flag for browse vs extended
#*****************************************************************************
proc AT_Dir_Configure {df dir_name args} {
  upvar #0 $dir_name DIR

  if {[info exists DIR($df,-dir)] != 1} {
    tk_messageBox -message "Error: No variable $DIR($df,-dir) when calling\
          AT_DirChangeDir.\n Probably means problems using AT_Dir widget"
    return
  }
  array set Local [array get DIR]
  foreach {i j} $args {
    set Local($df,$i) $j
  }

  set adjust 0
  if {$Local($df,-dir) != $DIR($df,-dir)} {
    if {[file isdirectory $Local($df,-dir)]} {
      set DIR($df,-dir) $Local($df,-dir)
      set adjust 1
    }
  }
  if {$Local($df,-secondary) != $DIR($df,-secondary)} {
    if {[file isdirectory $Local($df,-secondary)] || ($Local($df,-secondary) == "NULL")} {
      set DIR($df,-secondary) $Local($df,-secondary)
      set adjust 1
    }
  }
  if {$Local($df,-secCaseSensitive) != $DIR($df,-secCaseSensitive)} {
    set DIR($df,-secCaseSensitive) $Local($df,-secCaseSensitive)
    set adjust 1
  }
  if {$Local($df,-fullpath) != $DIR($df,-fullpath)} {
    if {($Local($df,-fullpath) >= 0) && ($Local($df,-fullpath) <= 1)} {
      set DIR($df,-fullpath) $Local($df,-fullpath)
      set adjust 1
    }
  }
  if {$Local($df,-filter) != $DIR($df,-filter)} {
    set DIR($df,-filter) $Local($df,-filter)
    set adjust 1
  }
  if {$Local($df,-filterCmd) != $DIR($df,-filterCmd)} {
    set DIR($df,-filterCmd) $Local($df,-filterCmd)
    set adjust 1
  }
  if {$Local($df,-sortCmd) != $DIR($df,-sortCmd)} {
    set DIR($df,-sortCmd) $Local($df,-sortCmd)
    set adjust 1
  }
  if {$Local($df,-CB) != $DIR($df,-CB)} {
    if {($Local($df,-CB) >= 0) && ($Local($df,-CB) <= 2)} {
      set DIR($df,-CB) $Local($df,-CB)
      set adjust 1
    }
  }
  if {$Local($df,-show) != $DIR($df,-show)} {
    if {$Local($df,-show) == "both"} {
      if {$DIR($df,-show) == "top"} {
        pack $df.mid -fill x -before $df.bot
      } else {
        pack $df.top -fill x -before $df.mid
      }
      set DIR($df,-show) "both"
    } elseif {$Local($df,-show) == "top"} {
      if {$DIR($df,-show) == "both"} {
        pack forget $df.mid
      } else {
        pack $df.top -fill x -before $df.mid
        pack forget $df.mid
      }
      set DIR($df,-show) "top"
    } elseif {$Local($df,-show) == "mid"} {
      if {$DIR($df,-show) == "both"} {
        pack forget $df.top
      } else {
        pack forget $df.top
        pack $df.mid -fill x -before $df.bot
      }
      set DIR($df,-show) "mid"
    }
  }
  if {$Local($df,-slave) != $DIR($df,-slave)} {
    set DIR($DIR($df,-slave),is_slave) 0
    if {$Local($df,-slave) == ""} {
  # Do we want to change -dir of slave and update it prior to releasing it?
      set DIR($df,-slave) ""
    } elseif {[winfo exists $Local($df,-slave)]} {
      set DIR($df,-slave) $Local($df,-slave)
      set DIR($DIR($df,-slave),is_slave) $df
      set adjust 1
    }
  }
  if {[info exists Local($df,-lbHeight)]} {
    $df.bot.lb configure -height $Local($df,-lbHeight)
  }
  if {$Local($df,-select_cmd) != $DIR($df,-select_cmd)} {
    set DIR($df,-select_cmd) $Local($df,-select_cmd)
  }
  if {$Local($df,-command) != $DIR($df,-command)} {
    set DIR($df,-command) $Local($df,-command)
    if {$DIR($df,-command) != ""} {
      set adjust 1
    }
  }
  if {$adjust == 1} {
    AT_DirGetDir $df $dir_name
  }
}

#*****************************************************************************
#  <AT_Dir>
#
# Purpose:
#     Set up a new AT_Dir widget
#
# Variables:(I=input)(O=output)(G=global)
#   df         (I) Tcl/Tk pathname of frame around dir-widget
#   dir_name   (I) The name of a global array to store dir info in.
#   args       (I) Extra arguments for the main frame, or
#     -dir     (I) The initial directory
#     -filter  (I) The initial file-filter
#     -filterCmd (I) The initial list of tcl/tk commands to use as addition
#                  filters.
#     -CB      (I) 0 list files, 1 list dirs, 2 list both with <>
#     -show    (I) top=Show top frame, mid=Show mid frame, both=show both
#     -slave   (I) The frame of a slave (only applicable if -show = top
#                  Used for linking a filelist with a directory list
#     -select_cmd (I) A command to call when a file selection is made.
#     -command (I) A command to call each time a new dir is made.
#                  "eval [$command] -dir DirList -files FileList"
#     -lbHeight(I) The height of the listbox (in lines)
#     -entWidth(I) The width of the entry (in character '0's)
#     -Update  (I) 0=don't get listing, 1=get listing
#     -fullpath (I) 1 show full path of subdirectories, 0 just subdirectory
#
# Returns: NULL
#
# FILE:
#
# History:
#   11/1997 Arthur Taylor (RDC/TDL) Created
#    8/1998 Arthur Taylor (RDC/TDL) Re-visited
#
# Notes:
#   may want flag for browse vs extended
#*****************************************************************************
proc AT_Dir {df dir_name args} {
  upvar #0 $dir_name DIR

  array set DIR [list $df,-filter * $df,-filterCmd "" $df,-dir [pwd] $df,-CB 2 $df,-show "both" \
        $df,-slave "" $df,-lbHeight 10 $df,-Update 1 $df,-command "" \
        $df,-fullpath 0 $df,-entWidth 25 $df,-select_cmd "" $df,-sortCmd "" \
        $df,-secondary "NULL" $df,-secCaseSensitive 0]
  foreach {i j} $args {
    set DIR($df,$i) $j
  }
  set DIR($df,is_slave) 0

  if {[file isdirectory $DIR($df,-dir)] != 1} {
    if {[file exists $DIR($df,-dir)]} {
      set DIR($df,-dir) [pwd]
    } else {
      set ans [tk_messageBox -message "$DIR($df,-dir) does not exist. Create it?" \
            -type yesno]
      if {$ans == "yes"} {
        file mkdir $DIR($df,-dir)
      } else {
        set DIR($df,-dir) [pwd]
      }
    }
  }
  set DIR($df,cur_dir) $DIR($df,-dir)
  set DIR($df,seclist) ""
  if {($DIR($df,-fullpath) < 0) || ($DIR($df,-fullpath) > 1)} {
    set DIR($df,-fullpath) 0
  }
  if {($DIR($df,-CB) < 0) || ($DIR($df,-CB) > 2)} {
    set DIR($df,-CB) 2
  }
  if {($DIR($df,-show) != "both") && ($DIR($df,-show) != "mid") && \
      ($DIR($df,-show) != "top")} {
    set DIR($df,-show) "both"
  }
  if {$DIR($df,-slave) != ""} {
    if {([winfo exists $DIR($df,-slave)]) && \
        [info exists DIR($DIR($df,-slave),-dir)]} {
      set DIR($DIR($df,-slave),is_slave) $df
    } else {
      set DIR($df,-slave) ""
    }
  }

# DIR($df,-Update) and DIR($df,-lbHeight) are both set do not need to keep.
  set loc_update $DIR($df,-Update)
  set loc_lbHeight $DIR($df,-lbHeight)
  set loc_entWidth $DIR($df,-entWidth)
  unset DIR($df,-Update)
  unset DIR($df,-lbHeight)
  unset DIR($df,-entWidth)

# ------------Creating new window---------------------------------------------
# $df                  (f) frame surrounding the directory
# $df.top.label        (l) Label for which directory
# $df.top.entry        (e) Entry for which directory
# $df.mid.label        (l) Label for file filter
# $df.mid.entry        (e) Entry for file filter
# $df.bot.lb          (lb) List box of files
# $df.bot.yscroll     (sb) Scroll bar for list box.
# ------------Creating new window---------------------------------------------
  if {($df != "") && ([winfo exists $df] == 0)} {
    frame $df
  }
  frame $df.top
    label $df.top.label -text "Dir:" -width 6
    entry $df.top.entry -textvariable [join "$dir_name ($df,-dir)" ""] \
          -width $loc_entWidth
    $df.top.entry xview moveto 1
    pack $df.top.label -side left
    pack $df.top.entry -side right -fill x -expand yes
  frame $df.mid
    label $df.mid.label -text "Filter" -width 6
    entry $df.mid.entry -textvariable [join "$dir_name ($df,-filter)" ""] \
          -width $loc_entWidth
    $df.mid.entry xview moveto 1
    pack $df.mid.label -side left
    pack $df.mid.entry -side right -fill x -expand yes
  frame $df.bot
    scrollbar $df.bot.yscroll -orient vertical -command "$df.bot.lb yview"
    listbox $df.bot.lb -yscrollcommand [list $df.bot.yscroll set] \
          -height $loc_lbHeight
    pack $df.bot.yscroll -side right -fill y
    pack $df.bot.lb -side left -fill both -expand yes
  if {$DIR($df,-show) != "mid"} {
    pack $df.top -side top -fill x
  }
  if {$DIR($df,-show) != "top"} {
    pack $df.mid -side top -fill x
  }
  pack $df.bot -side bottom -fill both -expand yes

  bind $df.top.entry <Return> "AT_DirGetDir $df $dir_name"
  bind $df.mid.entry <Return> "AT_DirGetDir $df $dir_name"
  bind $df.bot.lb <Button-1> "focus $df.bot.lb"
  bind $df.bot.lb <Return> "AT_DirChangeDir $df $dir_name 0"
  bind $df.bot.lb <ButtonRelease-1> "AT_DirChangeDir $df $dir_name 0"
  bind $df.bot.lb <Up> "AT_DirChangeDir $df $dir_name -1"
  bind $df.bot.lb <Down> "AT_DirChangeDir $df $dir_name 1"

  if {$loc_update == 1} {
    AT_DirGetDir $df $dir_name
  }
}

#*****************************************************************************
#  Demos...
#*****************************************************************************

#*****************************************************************************
# Simple demo.
#*****************************************************************************
proc AT_Demo1 {{tl .atdemo1}} {
  catch {destroy $tl}
  global demo1_ray
  catch {unset demo1_ray}

  toplevel $tl
  focus $tl
  AT_Dir $tl demo1_ray
  puts [array get demo1_ray]
}

#*****************************************************************************
# A directory browse selecter using AT_Dir Widget.
#
# This is similar to tk_getOpenFile, except we get directories.
#
# Note: use of tkwait is so I can return selected directory to calling
#   routine.
#*****************************************************************************
proc AT_Demo2 {{path pwd} {title "Directory List"} {tl .atdemo2} \
               {ray_name demo2_ray} {X -1} {Y -1}} {
  upvar #0 $ray_name RAY
  global f_wait2
  catch {destroy $tl}
  catch {unset RAY}
  if {$path == "pwd"} {
    set path [pwd]
  }

# ------------Creating new window---------------------------------------------
# $tl                  (f) Top level for browse selecter
# $tl.direct           (f) Frame for the AT_Dir widget
# $tl.bframe           (f) Bottom frame
# $tl.bframe.done      (b) Button for finishing selecting directory
# $tl.bframe.cancel    (b) Button for canceling selecting directory
# ------------Creating new window---------------------------------------------
  toplevel $tl
  focus $tl
  wm title $tl $title
  frame $tl.direct
  AT_Dir $tl.direct $ray_name -dir $path -show top -fullpath 0 \
        -entWidth 35 -lbHeight 15

  frame $tl.bframe
    button $tl.bframe.done -text "Done" -command "set f_wait2 0"
    button $tl.bframe.cancel -text "Cancel" -command "set f_wait2 1"
    pack $tl.bframe.done $tl.bframe.cancel -side left -fill x -expand yes
  pack $tl.bframe -side bottom -expand yes -fill x -anchor s
  pack $tl.direct -side top -expand yes -fill both
  wm protocol $tl WM_DELETE_WINDOW {set f_wait2 1}
  if {($X != -1) && ($Y != -1)} {
    wm geometry $tl "+$X+$Y"
  }

  tkwait variable f_wait2
  if {[winfo exists $tl]} {
    set out [AT_Dir_Cget $tl.direct $ray_name -dir]
    catch {destroy $tl}
    catch {unset RAY}
    if {$f_wait2 == 0} {
      return $out
    } else {
      return
    }
  } else {
    return
  }
}

#*****************************************************************************
# A file browse selecter using AT_Dir Widget.
#
# This is similar to tk_getOpenFile.
#
# Note: use of tkwait is so I can return selected directory to calling
#   routine.
#*****************************************************************************
proc AT_Demo3 {{path [pwd]} {file ""} {filter *} {filterCmd ""} {title Open} \
               {tl .atdemo3} {ray_name demo3_ray} {X -1} {Y -1} {sortCmd ""}} {
  upvar #0 $ray_name RAY
  global f_wait3
  catch {destroy $tl}
  catch {unset RAY}

# ------------Creating new window---------------------------------------------
# $tl                  (f) Top level for browse selecter
# $tl.direct           (f) Frame for the AT_Dir widget
# $tl.bframe           (f) Bottom frame
# $tl.bframe.done      (b) Button for finishing selecting file
# $tl.bframe.cancel    (b) Button for canceling selecting file
# $tl.top              (f) Frame for File name
# $tl.top.lf           (l) Label "File:"
# $tl.top.rt           (l) Current selected file.
# ------------Creating new window---------------------------------------------
  toplevel $tl
  focus $tl
  wm title $tl $title
  frame $tl.direct
  AT_Dir $tl.direct $ray_name -dir $path -filter $filter -filterCmd $filterCmd -fullpath 0 \
        -entWidth 30 -lbHeight 12 -sortCmd $sortCmd \
        -select_cmd "$tl.top.rt delete 0 end; $tl.top.rt insert \
        0 \[AT_Dir_Cget $tl.direct $ray_name -dir\]/\[AT_Dir_Cget $tl.direct\
        $ray_name curfiles\]; $tl.top.rt xview end"
  frame $tl.bframe
    button $tl.bframe.done -text "Done" -command "if {\[file isfile \[$tl.top.rt get\]\]} \
        {set f_wait3 0} else {$tl.top.rt delete 0 end}"
    button $tl.bframe.cancel -text "Cancel" -command "set f_wait3 1"
    pack $tl.bframe.done $tl.bframe.cancel -side left -fill x -expand yes
  frame $tl.top
    label $tl.top.lf -text "File: " -bd 5 -relief ridge -width 6
    entry $tl.top.rt -bd 5 -relief ridge
    pack $tl.top.lf -side left
    pack $tl.top.rt -side left -expand yes -fill x
  if {[file isfile [file join $path $file]]} {
    $tl.top.rt delete 0 end
    $tl.top.rt insert 0 [file join [AT_Dir_Cget $tl.direct $ray_name -dir] $file]
    $tl.top.rt xview end
  }
  pack $tl.bframe -side bottom -expand yes -fill x -anchor s
  pack $tl.top -side bottom -expand yes -fill both
  pack $tl.direct -side top -expand yes -fill both
  wm protocol $tl WM_DELETE_WINDOW {set f_wait3 1}
  bind $tl.top.rt <Return> "+ if {\[file isfile \[$tl.top.rt get\]\]} \
        {set f_wait3 0} else {$tl.top.rt delete 0 end}"
  if {($X != -1) && ($Y != -1)} {
    wm geometry $tl "+$X+$Y"
  }

  tkwait variable f_wait3
  if {[winfo exists $tl]} {
    set out [$tl.top.rt get]
    if {$out == ""} {
      set out [AT_Dir_Cget $tl.direct $ray_name curfiles]
    }
    if {$out != ""} {
      if {[file dirname $out] == "."} {
        set dir [AT_Dir_Cget $tl.direct $ray_name -dir]
        set out [file join $dir $out]
#        set last [string index $dir [expr {[string length $dir] -1}]]
#        if {($last == "\\") || ($last == "/")} {
#          set out "$dir$out"
#        } else {
#          set out "$dir/$out"
#        }
      }
    }
    catch {destroy $tl}
    catch {unset RAY}
    if {$f_wait3 == 0} {
      return $out
    } else {
      return
    }
  } else {
    return
  }
}

#*****************************************************************************
# A file browse selecter using AT_Dir Widget.
#
# This is similar to tk_getSaveFile.
#
# Note: use of tkwait is so I can return selected directory to calling
#   routine.
#*****************************************************************************
proc AT_Demo4 {{path [pwd]} {file ""} {filter *} {filterCmd ""} {title "Save As"} \
               {tl .atdemo4} {ray_name demo4_ray} {X -1} {Y -1}} {
  upvar #0 $ray_name RAY
  global f_wait3
  catch {destroy $tl}
  catch {unset RAY}

# ------------Creating new window---------------------------------------------
# $tl                  (f) Top level for browse selecter
# $tl.direct           (f) Frame for the AT_Dir widget
# $tl.bframe           (f) Bottom frame
# $tl.bframe.done      (b) Button for finishing selecting file
# $tl.bframe.cancel    (b) Button for canceling selecting file
# $tl.top              (f) Frame for File name
# $tl.top.lf           (l) Label "File:"
# $tl.top.rt           (l) Current selected file.
# ------------Creating new window---------------------------------------------
  toplevel $tl
  focus $tl
  wm title $tl $title
  frame $tl.direct
  AT_Dir $tl.direct $ray_name -dir $path -filter $filter -filterCmd $filterCmd -fullpath 0 \
        -entWidth 30 -lbHeight 12 -select_cmd "$tl.top.rt delete 0 end; \
        $tl.top.rt insert 0 \
        \[AT_Dir_Cget $tl.direct $ray_name -dir\]/\[AT_Dir_Cget $tl.direct\
        $ray_name curfiles\]; $tl.top.rt xview end"
  frame $tl.bframe
    button $tl.bframe.done -text "Done" -command "set f_wait3 0"
    button $tl.bframe.cancel -text "Cancel" -command "set f_wait3 1"
    pack $tl.bframe.done $tl.bframe.cancel -side left -fill x -expand yes
  frame $tl.top
    label $tl.top.lf -text "File: " -bd 5 -relief ridge -width 6
    entry $tl.top.rt -bd 5 -relief ridge
    pack $tl.top.lf -side left
    pack $tl.top.rt -side left -expand yes -fill x
  $tl.top.rt delete 0 end
  $tl.top.rt insert 0 "$file"
  $tl.top.rt xview end
  pack $tl.bframe -side bottom -expand yes -fill x -anchor s
  pack $tl.top -side bottom -expand yes -fill both
  pack $tl.direct -side top -expand yes -fill both
  wm protocol $tl WM_DELETE_WINDOW {set f_wait3 1}
  bind $tl.top.rt <Return> "+ set f_wait3 0"
  if {($X != -1) && ($Y != -1)} {
    wm geometry $tl "+$X+$Y"
  }
  focus $tl.bframe.done

  tkwait variable f_wait3
  set val [AT_Demo4_Resolve $ray_name $tl]
  catch {destroy $tl}
  catch {unset RAY}
  return $val
}

proc AT_Demo4_Resolve {ray_name tl} {
  global f_wait3

  if {! [winfo exists $tl]} {
    return
  }
  if {$f_wait3 != 0} {
    return
  }
  set out [$tl.top.rt get]
  if {$out == ""} {
    set out [AT_Dir_Cget $tl.direct $ray_name curfiles]
  }
  if {$out != ""} {
    if {[file dirname $out] == "."} {
      set dir [AT_Dir_Cget $tl.direct $ray_name -dir]
      set out [file join $dir $out]
#      set last [string index $dir [expr {[string length $dir] -1}]]
#      if {($last == "\\") || ($last == "/")} {
#        set out "$dir$out"
#      } else {
#        set out "$dir/$out"
#      }
    }
  }
  if {[file exists $out]} {
    set ans [tk_messageBox -message "$out already exists... Overwrite?" \
             -type yesno -icon question]
    if {$ans == "no"} {
      tkwait variable f_wait3
      return [AT_Demo4_Resolve $ray_name $tl]
    }
  }
  return $out
}

#*****************************************************************************
# A file browse selecter using AT_Dir Widget.
#
# This allows user to either select an existing or a non-existing file.
#
# Note: use of tkwait is so I can return selected directory to calling
#   routine.
#*****************************************************************************
proc AT_Demo7 {{path [pwd]} {file ""} {filter *} {filterCmd ""} {title "Save As"} \
               {tl .atdemo7} {ray_name demo7_ray} {X -1} {Y -1}} {
  upvar #0 $ray_name RAY
  global f_wait3
  catch {destroy $tl}
  catch {unset RAY}

# ------------Creating new window---------------------------------------------
# $tl                  (f) Top level for browse selecter
# $tl.direct           (f) Frame for the AT_Dir widget
# $tl.bframe           (f) Bottom frame
# $tl.bframe.done      (b) Button for finishing selecting file
# $tl.bframe.cancel    (b) Button for canceling selecting file
# $tl.top              (f) Frame for File name
# $tl.top.lf           (l) Label "File:"
# $tl.top.rt           (l) Current selected file.
# ------------Creating new window---------------------------------------------
  toplevel $tl
  focus $tl
  wm title $tl $title
  frame $tl.direct
  AT_Dir $tl.direct $ray_name -dir $path -filter $filter -filterCmd $filterCmd -fullpath 0 \
        -entWidth 30 -lbHeight 12 -select_cmd "$tl.top.rt delete 0 end; \
        $tl.top.rt insert 0 \
        \[AT_Dir_Cget $tl.direct $ray_name -dir\]/\[AT_Dir_Cget $tl.direct\
        $ray_name curfiles\]; $tl.top.rt xview end"
  frame $tl.bframe
    button $tl.bframe.done -text "Done" -command "set f_wait3 0"
    button $tl.bframe.cancel -text "Cancel" -command "set f_wait3 1"
    pack $tl.bframe.done $tl.bframe.cancel -side left -fill x -expand yes
  frame $tl.top
    label $tl.top.lf -text "File: " -bd 5 -relief ridge -width 6
    entry $tl.top.rt -bd 5 -relief ridge
    pack $tl.top.lf -side left
    pack $tl.top.rt -side left -expand yes -fill x
  $tl.top.rt delete 0 end
  $tl.top.rt insert 0 "$file"
  $tl.top.rt xview end
  pack $tl.bframe -side bottom -expand yes -fill x -anchor s
  pack $tl.top -side bottom -expand yes -fill both
  pack $tl.direct -side top -expand yes -fill both
  wm protocol $tl WM_DELETE_WINDOW {set f_wait3 1}
  bind $tl.top.rt <Return> "+ set f_wait3 0"
  if {($X != -1) && ($Y != -1)} {
    wm geometry $tl "+$X+$Y"
  }

  tkwait variable f_wait3

  if {$f_wait3 == 1} {
    catch {destroy $tl}
    catch {unset RAY}
    return ""
  }
  set out [$tl.top.rt get]
  if {$out == ""} {
    set out [AT_Dir_Cget $tl.direct $ray_name curfiles]
  }
  if {$out != ""} {
    if {[file dirname $out] == "."} {
      set dir [AT_Dir_Cget $tl.direct $ray_name -dir]
      set out [file join $dir $out]
#      set last [string index $dir [expr {[string length $dir] -1}]]
#      if {($last == "\\") || ($last == "/")} {
#        set out "$dir$out"
#      } else {
#        set out "$dir/$out"
#      }
    }
  }
  catch {destroy $tl}
  catch {unset RAY}
  return $out
}

#*****************************************************************************
# Simple demo for slave/master code.
#*****************************************************************************
proc AT_Demo5 {{tl .atdemo5}} {
  catch {destroy $tl}
  global demo5_ray
  catch {unset demo5_ray}

  toplevel $tl
  AT_Dir $tl.f demo5_ray
  AT_Dir $tl.f2 demo5_ray
  AT_Dir_Configure $tl.f2 demo5_ray -filter "*.pcx *.c"
  set foo1 [AT_Dir_Cget $tl.f demo5_ray listpath]
  set foo2 [AT_Dir_Cget $tl.f2 demo5_ray listpath]
  bind $foo2 <Left> "focus $foo1"
  bind $foo2 <Right> "focus $foo1"
  bind $foo1 <Left> "focus $foo2"
  bind $foo1 <Right> "focus $foo2"
  bind $tl <Button-3> "puts \[AT_Dir_Cget $tl.f demo5_ray curselect\]"
  bind $tl <Control-a> "AT_Dir_Configure $tl.f demo5_ray -show top -CB 1\
        -slave $tl.f2; puts \"Configuring mid\"; \
        AT_Dir_Configure $tl.f2 demo5_ray -show mid -CB 0"
  bind $tl <Control-b> "AT_Dir_Configure $tl.f demo5_ray -show both -CB 2 \
        -slave {}; AT_Dir_Configure $tl.f2 demo5_ray -show both -CB 2"
  pack $tl.f $tl.f2 -side left -expand true -fill both
  update
  pack propagate $tl.f off
  pack propagate $tl.f2 off
  upvar #0 demo5_ray DIR
  puts [array get DIR]
}

#*****************************************************************************
# A file browse selecter using AT_Dir Widget.
#
# This is similar to tk_getOpenFile but with multiple files.
#
# Note: use of tkwait is so I can return selected directory to calling
#   routine.
#*****************************************************************************
proc AT_Demo6 {{path [pwd]} {file ""} {filter *} {filterCmd ""} {title Open} \
               {tl .atdemo6} {ray_name demo6_ray}} {
  upvar #0 $ray_name RAY
  global f_wait6
  catch {destroy $tl}
  catch {unset RAY}

# ------------Creating new window---------------------------------------------
# $tl                  (f) Top level for browse selecter
# $tl.direct           (f) Frame for the AT_Dir widget
# $tl.bframe           (f) Bottom frame
# $tl.bframe.done      (b) Button for finishing selecting file
# $tl.bframe.cancel    (b) Button for canceling selecting file
# $tl.top              (f) Frame for File name
# $tl.top.lf           (l) Label "File:"
# $tl.top.rt           (l) Current selected file.
# ------------Creating new window---------------------------------------------
  toplevel $tl
  focus $tl
  wm title $tl $title
  frame $tl.direct
  AT_Dir $tl.direct $ray_name -dir $path -filter $filter -filterCmd $filterCmd -fullpath 0 \
        -entWidth 30 -lbHeight 12\
        -select_cmd "$tl.top.rt delete 0 end; $tl.top.rt insert \
        0 \[AT_Dir_Cget $tl.direct $ray_name curfiles\]"
  set path [AT_Dir_Cget $tl.direct $ray_name listpath]
  $path configure -selectmode extended
  frame $tl.bframe
    button $tl.bframe.done -text "Done" -command "set f_wait6 0"
    button $tl.bframe.cancel -text "Cancel" -command "set f_wait6 1"
    pack $tl.bframe.done $tl.bframe.cancel -side left -fill x -expand yes
  frame $tl.top
    label $tl.top.lf -text "File: " -bd 5 -relief ridge -width 6
    entry $tl.top.rt -bd 5 -relief ridge
    pack $tl.top.lf -side left
    pack $tl.top.rt -side left -expand yes -fill x
  if {[file isfile [file join $path $file]]} {
    $tl.top.rt delete 0 end
    $tl.top.rt insert 0 [file join [AT_Dir_Cget $tl.direct $ray_name -dir] $file]
  }
  pack $tl.bframe -side bottom -expand yes -fill x -anchor s
  pack $tl.top -side bottom -expand yes -fill both
  pack $tl.direct -side top -expand yes -fill both
  wm protocol $tl WM_DELETE_WINDOW {set f_wait6 1}
  bind $tl.top.rt <Return> "set f_wait6 0"
  bind $path <Return> "set f_wait6 0"
  tkwait variable f_wait6

  if {[winfo exists $tl]} {
    set out [$tl.top.rt get]
    if {$out == ""} {
      set out [AT_Dir_Cget $tl.direct $ray_name curfiles]
    }
    if {$out != ""} {
      set dir [AT_Dir_Cget $tl.direct $ray_name -dir]
      set last [string index $dir [expr {[string length $dir] -1}]]
      if {($last == "\\") || ($last == "/")} {
        set dir [string range $dir 0 [expr {[string length $dir] -2}]]
      }
      set out "$dir $out"
    }
    catch {destroy $tl}
    catch {unset RAY}
    if {$f_wait6 == 0} {
      return $out
    } else {
      return
    }
  } else {
    return
  }
}
