#!/bin/tclsh

proc SplitWord {s1 word} {
   set s0 $s1
   set cnt0 0
   set ansList ""
   set fst [string index $word 0]
   set len [string length $word]
   while {[set ind [string first $fst $s1]] != -1} {
      if {$word == [string range $s1 $ind [expr $ind + $len - 1]]} {
         lappend ansList [string range $s0 0 [expr $ind + $cnt0 - 1]]
         set s0 [string range $s0 [expr $ind + $cnt0 + $len] end]
         set cnt0 0
         set s1 $s0
      } else {
         set s1 [string range $s1 [expr $ind + 1] end]
         incr cnt0 [expr $ind + 1]
      }
   }
   lappend ansList $s0
}

proc GetVal {str col} {
  set value [SplitWord $str "<td"]
  if {[llength $value] != 2} {
     puts "Problems reading $col column"
     set value 0
  }
  set value [lindex $value 1]
  set ind [string first ">" $value]
  set value [string range $value [expr $ind + 1] end]
  set value [string trim $value]
  if {[string range $value 0 7] == "<center>"} {
     set value [string range $value 8 [expr [string length $value] - 10]]
  }

  set val_font [SplitWord $value "<font"]
  if {[llength $val_font] != 1} {
    set ans [lindex $val_font 0]
    for {set i 1} {$i < [llength $val_font]} {incr i} {
      set ind [string first ">" [lindex $val_font $i]]
      set ans "$ans[string range [lindex $val_font $i] [expr $ind + 1] end]"
    }
    set val_font [SplitWord $ans "</font>"]
    if {[llength $val_font] != 1} {
      set value [join $val_font ""]
    }
  }
  set val_font [SplitWord $value "<span"]
  if {[llength $val_font] != 1} {
    set ans [lindex $val_font 0]
    for {set i 1} {$i < [llength $val_font]} {incr i} {
      set ind [string first ">" [lindex $val_font $i]]
      set ans "$ans[string range [lindex $val_font $i] [expr $ind + 1] end]"
    }
    set val_font [SplitWord $ans "</span>"]
    if {[llength $val_font] != 1} {
      set value [join $val_font ""]
    }
  }
  set val_font [SplitWord $value "<div"]
  if {[llength $val_font] != 1} {
    set ans [lindex $val_font 0]
    for {set i 1} {$i < [llength $val_font]} {incr i} {
      set ind [string first ">" [lindex $val_font $i]]
      set ans "$ans[string range [lindex $val_font $i] [expr $ind + 1] end]"
    }
    set val_font [SplitWord $ans "</div>"]
    if {[llength $val_font] != 1} {
      set value [join $val_font ""]
    }
  }

  set val_nbsp [SplitWord $value "&nbsp;"]
  set value [join $val_nbsp ""]
  set val_nbsp [SplitWord $value "&amp"]
  set value [join $val_nbsp "&"]
  set val_nbsp [SplitWord $value "&micro;"]
  set value [join $val_nbsp "µ"]
  set val_nbsp [SplitWord $value "&#956"]
  set value [join $val_nbsp "µ"]
  set val_nbsp [SplitWord $value "<br>"]
  set value [join $val_nbsp ""]
  set val_nbsp [SplitWord $value "</tr>"]
  set value [join $val_nbsp ""]
  set value [join [split $value ","] ";"]
  return $value
}

set fp [open table2.html r]
set f_cont 0
set numTR 0

# Use a stream algorithm.
set ansLine ""
while {[gets $fp line] >= 0} {
  set line [string trim $line]
  if {[string range $line 0 3] == "<h2>"} {
    incr f_cont 1
    set numTR 0
  }
  if {$f_cont <= 1} { continue }
  if {[string range $line 0 3] == "<tr>"} {
    incr numTR 1
  }
  if {$numTR < 2} { continue }
  if {[string range $line 0 3] == "<tr>"} {
#    puts "$ansLine"
    set lst [SplitWord $ansLine "</td>"]
    if {[llength $lst] < 4} { continue }

    set value [GetVal [lindex $lst 0] val]
    set value [string trimleft $value 0]
    if {$value == ""} { set value 0 }
    set value [string trim $value]

    set parm [GetVal [lindex $lst 1] parm]
    if {$parm == "Reserved"} {
      set parm "undefined"
    }
    set parm_br [SplitWord $parm " (see note)"]
    if {[llength $parm_br] != 1} {
      set parm [join $parm_br ""]
    }
    set parm_br [SplitWord $parm " (See Note)"]
    if {[llength $parm_br] != 1} {
      set parm [join $parm_br ""]
    }
    set parm [string trim $parm]

    set units [GetVal [lindex $lst 2] units]
    set units_sup [SplitWord $units "<sup>"]
    if {[llength $units_sup] != 1} {
      set units [join $units_sup "^"]
    }
    set units_sup [SplitWord $units "<sup class=\"moz-txt-sup\">"]
    if {[llength $units_sup] != 1} {
      set units [join $units_sup "^"]
    }
    set units_sup [SplitWord $units "</sup>"]
    if {[llength $units_sup] != 1} {
      set units [join $units_sup ""]
    }
    set units_sup [SplitWord $units "**"]
    set units [join $units_sup "^"]
    if {$units == "/s"} { set units "1/s" }
    if {[string tolower $units] == "degree true" } {set units deg}
    if {[string tolower $units] == "deg. true" } {set units deg}
    if {[string tolower $units] == "deg true" } {set units deg}
    if {[string tolower $units] == "non-dim" } {set units "-"}
    if {[string tolower $units] == "non-dim." } {set units "-"}
    if {[string tolower $units] == "numeric" } {set units "-"}
    if {[string tolower $units] == "percent" } {set units "%"}
    if {[string tolower $units] == "kg/kg" } {set units "kg/kg"}
    set units [string trim $units]

    set abbrev [GetVal [lindex $lst 3] abbrev]
    if {$abbrev == "-"} {
      set abbrev "var$value"
    }
    if {[llength $abbrev] != 1} {
      set abbrev [join $abbrev ""]
    }
    set abbrev [string trim $abbrev]

    if {$parm == "Missing"} {
      set parm "undefined"
    } elseif {$parm == "Reserved for future use"} {
      set parm "undefined"
    }
    if {$parm == "undefined"} {
      set units "-"
      set abbrev "var$value"
    }

    if {$units == "K"} {
      puts "   /* $value */ {\"$abbrev\", \"$parm\", \"$units\", UC_K2F},"
    } else {
      puts "   /* $value */ {\"$abbrev\", \"$parm\", \"$units\", UC_NONE},"
    }

    set ansLine ""
  }
  set ansLine "$ansLine $line"
}

close $fp


