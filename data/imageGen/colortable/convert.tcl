proc doit {file} {
  set fp [open $file r]
  set cnt 0
  while {[gets $fp line] > 0} {
    if {$cnt > 8} {
      set num [lindex $line 0]
      set R [lindex $line 1]
      set G [lindex $line 2]
      set B [lindex $line 3]
      set label1 [lindex $line 4]
      set label2 [lindex $line 5]
      puts "$num $R $G $B $label1 $label2"
#      puts $line
    }
    incr cnt
  }
  close $fp
}

doit p:/ndfd/degrib/data/imagegen/colortable/wx_200306.colortable
#doit p:/ndfd/degrib/data/imagegen/colortable/wx_200401.colortable
#doit p:/ndfd/degrib/data/imagegen/colortable/wx_200402.colortable
#doit p:/ndfd/degrib/data/imagegen/colortable/wx_200411.colortable

