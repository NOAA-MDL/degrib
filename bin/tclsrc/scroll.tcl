proc Scrolled_Text { c text args } {
  set index [lsearch $args "-keepScroll"]
  if {$index != -1} {
    set args [lreplace $args $index $index]
    eval {text $text \
          -xscrollcommand [list $c.xscroll set]  \
          -yscrollcommand [list $c.yscroll set] \
          -highlightthickness 0 \
          -borderwidth 0} $args
  } else {
    eval {text $text \
          -xscrollcommand [list $c.xscroll set]  \
          -yscrollcommand [list Scroll_Set $c.yscroll \
              [list grid $c.yscroll -row 0 -column 1 -sticky ns]] \
          -highlightthickness 0 \
          -borderwidth 0} $args
  }
  scrollbar $c.xscroll -orient horizontal \
       -command [list $text xview]
  scrollbar $c.yscroll -orient vertical \
       -command [list $text yview]
  grid $text $c.yscroll -sticky news
  grid $c.xscroll -sticky ew
  grid rowconfigure $c 0 -weight 1
  grid columnconfigure $c 0 -weight 1
  return $text
}

# Welch p 347
proc Scroll_Set {scrollbar geoCmd offset size} {
  if {$offset != 0.0 || $size != 1.0} {
    eval $geoCmd
    $scrollbar set $offset $size
  } else {
    set manager [lindex $geoCmd 0]
    $manager forget $scrollbar
  }
}

# Welch p 392
# c is the frame that will contain just the canvas (and scrollbars)
# canv is the name of the canvas (ie $c.canvas)
proc Scrolled_Canvas { c canvas args } {
#  frame $c
  eval {canvas $canvas \
       -xscrollcommand [list Scroll_Set $c.xscroll \
            [list grid $c.xscroll -row 1 -column 0 -sticky we]] \
       -yscrollcommand [list Scroll_Set $c.yscroll \
            [list grid $c.yscroll -row 0 -column 1 -sticky ns]] \
       -highlightthickness 0 \
       -borderwidth 0} $args
  scrollbar $c.xscroll -orient horizontal \
       -command [list $canvas xview]
  scrollbar $c.yscroll -orient vertical \
       -command [list $canvas yview]
  grid $canvas $c.yscroll -sticky news
  grid $c.xscroll -sticky ew
  grid rowconfigure $c 0 -weight 1
  grid columnconfigure $c 0 -weight 1
  return $canvas
}

#*****************************************************************************
# <multi_scroll>
#
# Purpose
#     Scrollbar instigated a yscrollcommand.  We deal with scrolling the
#   numerous listboxes associated with this scrollbar.
#
# Variables (I=Input) (G=global) (O=output)
#   scroll_list(I)=list of listboxes linked to the scrollbar
#   args       (I)=List specifying scroll design options.
#
# Returns: NULL
#
# History:
#
# Notes:
#     Welch (Practical programming in TCL ed. 2 p. 514)
#*****************************************************************************
proc multi_scroll {scroll_list args} {
	foreach item $scroll_list {
		 eval {$item yview} $args
	}
}

#*****************************************************************************
# <multi_scroll2>
#
# Purpose
#     Listbox instigated a yscrollcommand.  We deal scrolling the scrollbar
#   and any other listboxes that are associated with that scrollbar
#
# Variables (I=Input) (G=global) (O=output)
#   scroll_path(I) Tk path of scrollbar
#   box_list   (I) list of listboxes linked to the scrollbar
#   args       (I) List specifying scroll design options.
#
# Returns: NULL
#
# History:
#    8/1998 Howard Berger (NWS) Commented
#
# Notes:
#     Modified from Welch (Practical programming in TCL p.514)
#*****************************************************************************
proc multi_scroll2 {scroll_path box_list args} {
  eval {$scroll_path set} $args
  foreach i $box_list {
    eval {$i yview moveto} [lindex $args 0]
  }
}

proc Scroll_Test {} {
  catch {destroy .c}
  frame .c
  Scrolled_Canvas .c .c.canv -width 601 -height 601 \
    -scrollregion {0 0 601 601} -bd 6 -relief sunken
  set pad [expr {[.c.canv cget -bd] + [.c.canv cget -highlightthickness]}]
  .c.canv create line 0 0 $pad $pad
  pack .c -fill both -expand true
}
