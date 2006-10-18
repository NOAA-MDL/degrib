package require Metawidget

# name: Treecontrol
# args: args: option-value pairs
# for the first instance, defines three images for an expanded, a collapsed
# and a leaf node, and defines class bindings. then creates internal widgets,
# finally inits private variables and widget options.
metawidget create Treecontrol {
  if { ! [ourinfo exists pCol] } {
    our pCol [image create bitmap -back black -fore gray60 -data {
      #define _width 16
      #define _height 9
      static char _bits[] = {
        0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0x01
      }
    } -maskdata {
      #define _width 16
      #define _height 9
      static char _bits[] = {
        0xff, 0x01, 0x01, 0x01, 0x11, 0x01, 0x11, 0x01, 0x7d, 0x01,
        0x11, 0x01, 0x11, 0x01, 0x01, 0x01, 0xff, 0x01
      }
    }]
    our pExp [image create bitmap -back black -fore gray60 -data {
      #define _width 16
      #define _height 9
      static char _bits[] = {
        0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0x01
      }
    } -maskdata {
      #define _width 16
      #define _height 9
      static char _bits[] = {
        0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7d, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0x01
      }
    }]
    our pLeaf [image create bitmap -fore gray60 -data {
      #define _width 16
      #define _height 9
      static char _bits[] = {
        0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xff, 0x01
      }
    }]

    # Arthur : Thought about adding, " <ButtonRelease-1>     m  0 n r"
    # But the '+' doesn't work properly then.

    foreach { sBind sStep iDir sAction iMode } {
      <Button-1>            m  0 n s   <Control-1>           m  0 t s
      <Shift-1>             m  0 n r   <Shift-Control-1>     m  0 a r

      <Up>                  r -1 n s   <Down>                r  1 n s
      <Shift-Up>            r -1 n r   <Shift-Down>          r  1 n r
      <Control-Up>          r -1 c c   <Control-Down>        r  1 c c
      <Shift-Control-Up>    r -1 a r   <Shift-Control-Down>  r  1 a r

      <Prior>               p -1 n s   <Next>                p  1 n s
      <Shift-Prior>         p -1 n r   <Shift-Next>          p  1 n r
      <Control-Prior>       p -1 c c   <Control-Next>        p  1 c c
      <Shift-Control-Prior> p -1 a r   <Shift-Control-Next>  p  1 a r

      <Control-space>       r  0 t s
    } {
      bind Treecontroltext $sBind "\[winfo parent %W\] _select $sStep $iDir $sAction $iMode"
    }

    # expands and collapses on left/right cursor keys
    bind Treecontroltext <Left>        "\[winfo parent %W\] _leftright left"
    bind Treecontroltext <Right>       "\[winfo parent %W\] _leftright right"
    bind Treecontroltext <Shift-Left>  "\[winfo parent %W\] _leftright left 1"
    bind Treecontroltext <Shift-Right> "\[winfo parent %W\] _leftright right 1"

    # jump to start or end, should be improved one day
    bind Treecontroltext <Home>        "%W see 1.0"
    bind Treecontroltext <End>         "%W see end"

    # take over some useful Text class bindings
    foreach sBind {<MouseWheel> <B2-Motion> <Button-2> <<Copy>>} {
      bind Treecontroltext $sBind [bind Text $sBind]
    }

    # create string of tabs for indention. 8.0 doesn't know "string repeat"!
    for { set i 0 } { $i < 10 } { incr i } { append lTabs "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t" }
    our lTabs $lTabs
  }

  scrollbar $this.scrh -command "$this.text xview" -orient horiz
  scrollbar $this.scrv -command "$this.text yview"

  # nodes will be 22 pixels indented, ideal for 16x16 icons
  text $this.text -wrap none -cursor arrow -spacing1 1 -tabs {3 22} \
                  -border 0 -bg white -padx 1 -pady 1 -insertofftime 0 -insertwidth 0 \
                  -xscroll "$this.scrh set" -yscroll "$this.scrv set"

  pack $this.scrv -side right -fill y
  pack $this.scrh -side bottom -fill x
  pack $this.text -fill both -expand 1

  # change Text to Treecontrol class bindings
  bindtags $this.text [mkw.lchange [bindtags $this.text] Text Treecontroltext]

  $this.text tag config cursor -border 1 -relief solid

  my lNodes {}            ;# list of all nodes (unordered)
  my iCursor 0            ;# row where cursor is
  my iAnchor 0            ;# row for selection anchor

  # define root node properties. will never change. fields are:
  # text, image, parent, children, depth, expanded, final, tags, userdata
  my aNodes() [list $this {} {} {} 0 1 1 {} {}]

  -image {}       ;# image for root node
  -text $this     ;# text for root node

  my -onexpand {}         ;# eval cmd when any node is expanded
  my -onselect {}         ;# eval cmd when any node is selected
  my -selectmode multiple ;# selectmode: single or multiple
  my -fullexpand 1        ;# allows recursive expanding of nodes
} {} -default text

# name: _dump
# args: sNode: node to print tree for
#       iIndent: indention. don't specify!
# for debug purposes. prints a tree starting with node $sNode.
metawidget proc Treecontrol _dump { {sNode {}} {iIndent 0} } {
  puts "[string range [our lTabs] 3 $iIndent]$sNode               [lindex [my aNodes($sNode)] 2]   [lrange [my aNodes($sNode)] 4 end]"

  foreach sChild [lindex [my aNodes($sNode)] 3] {
    _dump $sChild [expr $iIndent+2]
  }
}

# name: _checkNode
# args: sNode: node noame
# throws an exception if the given node doesn't exist.
metawidget proc Treecontrol _checkNode { sNode } {
  if { [myinfo exists aNodes($sNode)] } return
  error "Treecontrol $this: Node $sNode does not exist."
}

# name: _toggle
# args: bAll: flag if recursive or not
# expands or collapses the node where the +/- symbol was clicked.
metawidget proc Treecontrol _toggle { bAll } {
  # get node name. is part of tag list behind the node's text.
  set sNode [lindex [$this.text tag names {current lineend-1ch}] 0]

  # no recursive expand if disallowed by user
  set bAll [expr $bAll && [my -fullexpand]]

  # look in node's properties. expand if it was collapsed and vice versa.
  switch -- [lindex [my aNodes($sNode)] 5] {
     1 { collapse $sNode $bAll }
    -1 { expand   $sNode $bAll }
  }
}

# name: _leftright
# args: sDir: left or right
#       bAll: flag if recursive or not
# expands or collapses selected nodes, triggered by cursor keys.
# if bAll is true, does it for all children as well.
metawidget proc Treecontrol _leftright { sDir {bAll 0} } {
  # no recursive expand if disallowed by user
  set bAll [expr $bAll && [my -fullexpand]]

  foreach sNode [selection get] {
    switch $sDir {
      left  { collapse $sNode $bAll }
      right { expand   $sNode $bAll }
    }
  }
}

# name: _select
# args: sStep  : m: mouse, r: row, p: page
#       iDir   : 0 for mouse, 1 or -1 otherwise
#       sAction: n: new selection, a: append, c: cursor only, t: toggle
#       sMode  : s: single cell, r: range of cells, c: cursor only
#
# sets the selection, using the standard sel tag.
metawidget proc Treecontrol _select { sStep iDir sAction sMode } {
  focus $this.text

  if { [lsearch [$this.text tag names current] _tSign] != -1 } {
    _toggle [mkw.decode $sMode {s 0 r 1}]
    return
  }

  # step 1: determine the cursor new row
  switch $sStep {
    m {
      set iRow [lindex [split [$this.text index current] .] 0]
    }
    r {
      set iMaxRow [expr [lindex [split [$this.text index end] .] 0] - 2]
      set iRow [expr [my iCursor]+$iDir]
      if { $iRow < 1 }        { set iRow 1 }
      if { $iRow > $iMaxRow } { set iRow $iMaxRow }
    }
    p {
      # widget height divided thru line height gives number of visible rows
      set iRowH [expr [font metrics [$this.text cget -font] -linespace]+1]
      set iStep [expr [winfo height $this.text]/$iRowH]
      set iMaxRow [expr [lindex [split [$this.text index end] .] 0] - 2]
      set iRow [expr [my iCursor]+$iDir*$iStep]
      if { $iRow < 1 }        { set iRow 1 }
      if { $iRow > $iMaxRow } { set iRow $iMaxRow }
    }
  }

  # step 2: get indexes of that row's node and set cursor tag there
  $this.text tag remove cursor 1.0 end
  mkw.lassign [$this.text tag nextrange _tNode $iRow.0] sC1 sC2
  if { ! [info exists sC1] || ! [info exists sC2] } return
  $this.text tag add cursor $sC1 $sC2
  my iCursor $iRow

  # step 3: determine if sel tag will be removed or added to cells
  switch $sAction {
    n - a {
      set sMethod add
    }
    t {
      if { [lsearch [$this.text tag names $sC1] sel] != -1 } {
        set sMethod remove
      } else {
        set sMethod add
      }
    }
  }

  # step 4: remove existing sel tags, if a new selection is set
  if { $sAction == "n" || [my -selectmode] == "single" } {
    $this.text tag remove sel 1.0 end
  }

  # step 5: actually set the sel tag
  switch $sMode.[my -selectmode] {
    s.single - s.multiple - r.single {
      # for single cells, add or remove sel tag
      $this.text tag $sMethod sel $sC1 $sC2
      my iAnchor [my iCursor]
    }
    r.multiple {
      # for a range, do all from anchor to cursor
      set iR1 [expr ($iRow<[my iAnchor])?$iRow:[my iAnchor]]
      set iR2 [expr ($iRow>[my iAnchor])?$iRow:[my iAnchor]]
      for { set i $iR1 } { $i <= $iR2 } { incr i } {
        mkw.lassign [$this.text tag nextrange _tNode $i.0] sI1 sI2
        $this.text tag add sel $sI1 $sI2
      }
    }
  }

  $this.text tag raise sel
  $this.text tag raise cursor
  $this.text see $iRow.0

  # if -onselect script is specified, evaluate it.
  eval [my -onselect]
}

# name: _visible
# args: sNode: node name
# returns 1 if the node is visible, i.e. if it's part of the text widget.
# can still be physically invisible because the viewport is somewhere else.
metawidget proc Treecontrol _visible { sNode } {
  if { [$this.text tag range $sNode] == {} } {
    return 0
  } else {
    return 1
  }
}

# name: _drawNode
# args: sNode: node name
#       iRow: row to draw node
# draws a node at the specified row. if row is not given, it calculates it.
# this will draw the plain node but no children.
metawidget proc Treecontrol _drawNode { sNode {iRow {}} } {
  # get row if not calculated already
  if { $iRow == {} } {
    set iRow [_calcRow $sNode]
  }

  # extract some node properties
  mkw.lassign [my aNodes($sNode)] sText pImage x x iIndent bExp

  # create new line and tabs for indention
  $this.text insert $iRow.0 "\n"
  $this.text insert $iRow.0 [string range [our lTabs] 1 $iIndent]

  # create +/-/. symbol, mark it with tag to add mouse click bindinds
  $this.text image create $iRow.end -image [mkw.decode $bExp "-1 [our pCol] 1 [our pExp] 0 [our pLeaf]"]
  $this.text tag add _tSign $iRow.end-1char

  # add node image, if specified, and add tag for selection bindings.
  if { $pImage != {} } {
    $this.text image create $iRow.end -image $pImage
    $this.text tag add _tImg $iRow.end-1ch
  }

  # then insert placeholders and add tag for selection bindings.
  $this.text insert $iRow.end "  "
  $this.text tag add $sNode $iRow.end-2ch $iRow.end
  $this.text tag add _tNode $iRow.end-2ch $iRow.end

  # if any user tags are specified, add them.
  foreach sTag [lindex [my aNodes($sNode)] 7] {
    $this.text tag add $sTag $iRow.end-2ch $iRow.end
  }

  # insert the node's caption and make it the first in the tag list.
  $this.text insert $iRow.end-1ch "$sText"
  $this.text tag lower $sNode
}

# name: _drawChildren
# args: sNode: node name
# draws the children for the given node. if a child is in expanded state,
# will draw its children too and so on.
metawidget proc Treecontrol _drawChildren { sNode } {
  # only do it when it says it is expanded
  if { [lindex [my aNodes($sNode)] 5] != 1 } return

  set lChildren [lindex [my aNodes($sNode)] 3]
  set iRow [expr [_getRow $sNode]+1]

  # first draw the direct children...
  foreach sChild $lChildren {
    _drawNode $sChild $iRow
    incr iRow
  }

  # ...then draw their children
  foreach sChild $lChildren {
    _drawChildren $sChild
  }
}

# name: _undrawNode
# args: sNode: node name
# undraws a node. does not touch its children.
metawidget proc Treecontrol _undrawNode { sNode } {
  set iR1 [_getRow $sNode]
  set iR2 [expr $iR1+1]
  $this.text delete $iR1.0 $iR2.0
}

# name: _undrawChildren
# args: sNode: node name
# undraws the children of a node. node must me visible when this is called!
metawidget proc Treecontrol _undrawChildren { sNode } {
  set iR1 [expr [_getRow $sNode]+1]
  set iR2 [expr $iR1+[_calcChildren $sNode]]
  $this.text delete $iR1.0 $iR2.0
}

# name: _calcChildren
# args: sNode: node name
# calculates the number of *visible* children, i.e. where their parent is
# expanded. uses plain recursion and looks at node properties. (yes, marks
# would have done that, too. for me this worked better.)
metawidget proc Treecontrol _calcChildren { sNode } {
  if { [lindex [my aNodes($sNode)] 5] != 1 } {
    return 0
  }

  set lChildren [lindex [my aNodes($sNode)] 3]
  set iNum [llength $lChildren]

  foreach sChild $lChildren {
    incr iNum [_calcChildren $sChild]
  }

  return $iNum
}

# name: _calcRow
# args: sNode: node name
# calculates the row where a node should be drawn. used when the node is
# not yet visible (otherwise _getRow would apply). the node's parent *must*
# be expanded or an empty string is returned.
metawidget proc Treecontrol _calcRow { sNode } {
  set sParent [lindex [my aNodes($sNode)] 2]
  if { [lindex [my aNodes($sParent)] 5] != 1 } return

  # get siblings and the node's predecessor
  set lSiblings [lindex [my aNodes($sParent)] 3]
  set iPos [lsearch $lSiblings $sNode]
  set sPrev [lindex $lSiblings [expr $iPos-1]]

  # no predecessor means that the node is the first and its row is one below
  # its parent. otherwise the row is below the predecessor plus any expanded
  # children.
  if { $sPrev == {} } {
    set iParRow [_getRow $sParent]
    if { $iParRow == {} } { return Damn! }
    set iRow [expr $iParRow+1]
  } else {
    set iRow [expr [_getRow $sPrev]+[_calcChildren $sPrev]+1]
  }

  return $iRow
}

# name: _getRow
# args: sNode: node name
# returns the row of a node. node must be visible. looks for a tag with the
# node's name and returns its position's row portion.
metawidget proc Treecontrol _getRow { sNode } {
  set lRange [$this.text tag ranges $sNode]
  return [lindex [split $lRange .] 0]
}

# name: _recalcDepth
# args: sNode: node name
# updates the depth field in the node's properties and all of its children.
# called when a node has been reparented.
metawidget proc Treecontrol _recalcDepth { sNode } {
  set sParent [lindex [my aNodes($sNode)] 2]
  my aNodes($sNode) [lreplace [my aNodes($sNode)] 4 4 [expr [lindex [my aNodes($sParent)] 4]+1]]

  foreach sChild [lindex [my aNodes($sNode)] 3] {
    _recalcDepth $sChild
  }
}

# name: _setSymbol
# args: sNode: node name
# sets the +/-/. symbol depending on the node's state. -1 is collapsed,
# 1 is expanded, 0 is a leaf (no children).
metawidget proc Treecontrol _setSymbol { sNode } {
  # node must be visible. exits if not.
  set iRow [_getRow $sNode]

  if { $iRow == {} } return

  set sIndex $iRow.[lindex [my aNodes($sNode)] 4]
  set sImage [mkw.decode [lindex [my aNodes($sNode)] 5] "-1 [our pCol] 1 [our pExp] 0 [our pLeaf]"]

  $this.text image config $sIndex -image $sImage
}

# name: _delete
# args: sNode: node name
# deletes a node and all children logically. calls to _undrawChildren
# and _undrawNode for the visual representation must have been made prior.
# does not update the parent's children list (would slow down recursion).
metawidget proc Treecontrol _delete { sNode } {
  foreach sChild [lindex [my aNodes($sNode)] 3] {
    _delete $sChild
  }

  unmy aNodes($sNode)
  my lNodes [mkw.lshrink [my lNodes] $sNode]
}

# name: -text
# args: sText: text for root node
# sets the text in the first line's node.
metawidget proc Treecontrol -text { sText } {
  if { [my -image] == {} } {
    set sInd 1.0
  } else {
    set sInd 1.1
  }

  $this.text delete $sInd 2.0
  $this.text insert $sInd " $sText\n"
  $this.text tag add {} $sInd 1.end

  my -text $sText
}

# name: -image
# args: sImage: image for root node
# sets or removes the image in the first line's node.
metawidget proc Treecontrol -image { sImage } {
  catch {
    $this.text image cget 1.0 -image
    $this.text delete 1.0
  }

  if { $sImage != {} } {
    $this.text image create 1.0 -image $sImage
  }

  my -image $sImage
}

# name: -fullexpand
# args: sFull: boolean
# controls whether recursive expanding of nodes shall be allowed
metawidget proc Treecontrol -fullexpand { sFull } {
  set sFull [mkw.complete $sFull {yes true on no false off 0 1}]
  my -fullexpand [mkw.decode $sFull {yes 1 true 1 on 1 1 1} 0]
}

# name: -selectmode
# args: sMode: single, multiple
# if single, only one node can be selected.
metawidget proc Treecontrol -selectmode { sMode } {
  my -selectmode [mkw.complete $sMode {single multiple}]
}

# name: expand
# args: sNode: node name
#       bAll: true if recursive
# expands a node and eventually all children. prior to that it evaluates
# the -onexpand callback command, if specified and if the node is not in
# "final" state. final state means that all children are defined, therefore
# no callback for new children is required. otherwise the -onexpand command
# gets sNode as an argument and should create the node's children with
# insert. This feature is necessary for huge trees (e.g. file
# system trees) with too many nodes to be created a priori.
metawidget proc Treecontrol expand { sNode {bAll 0} } {
  # only do it if collapsed
  if { [lindex [my aNodes($sNode)] 5] == -1 } {
    # callback for children, if specified and node is not marked as final.
    # delete existing children from last callback first, then do new callback.
    if { [my -onexpand] != {} && ! [lindex [my aNodes($sNode)] 6] } {
      foreach sChild [lindex [my aNodes($sNode)] 3] {
        _delete $sChild
      }

      my aNodes($sNode) [lreplace [my aNodes($sNode)] 3 3 {}]
      eval [my -onexpand] {$sNode}
    }

    # update expanded flag (expr maps 0 to 0 and !=0 to 1)
    my aNodes($sNode) [lreplace [my aNodes($sNode)] 5 5 [expr !![llength [lindex [my aNodes($sNode)] 3]]]]

    # if the node is currently visible, draw symbol and children
    if { [_visible $sNode] } {
      _setSymbol $sNode
      _drawChildren $sNode
    }
  }

  # eventually expand all children, too
  if { $bAll } {
    foreach sChild [lindex [my aNodes($sNode)] 3] {
      expand $sChild 1
    }
  }
}

# name: collapse
# args: sNode: node name
#       bAll: true if recursive
# collapses a node and eventually all children.
metawidget proc Treecontrol collapse { sNode {bAll 0} } {
  # do it only if expanded
  if { [lindex [my aNodes($sNode)] 5] == 1 } {
    # undraw the kids if node is currently visible
    if { [_visible $sNode] } {
      _undrawChildren $sNode
    }

    # update node properties (expr maps 0 to 0 and !=0 to -1)
    my aNodes($sNode) [lreplace [my aNodes($sNode)] 5 5 [expr -1*(!![llength [lindex [my aNodes($sNode)] 3]])]]
    _setSymbol $sNode
  }

  # eventually collapse all children, too
  if { $bAll } {
    foreach sChild [lindex [my aNodes($sNode)] 3] {
      collapse $sChild 1
    }
  }
}

# name: insert
# args: sNode: node name
#       args: option-value pairs
# inserts a new node. creates a record for the node properties that
# would make the node a child of the root node. then processes the options.
metawidget proc Treecontrol insert { sNode args } {
  if { [myinfo exists aNodes($sNode)] } {
    error "Treecontrol $this: Node $sNode already exists."
  }

  # if no callback specified, then the new node is a leaf by default.
  # otherwise it *might* have children, therefore it's a collapsed node.
  if { [my -onexpand] == {} } {
    set iExpand 0
  } else {
    set iExpand -1
  }

  # create a property record for this node. fields are:
  # text, image, parent, children, depth, expanded, final, tags, userdata
  my aNodes($sNode) [list {} {} {} {} 0 $iExpand 0 {} {}]
  my lNodes [linsert [my lNodes] end $sNode]

  eval nodeconfigure {$sNode} -text {$sNode} -parent {{}} $args
}

# name: delete
# args: sNode: node name
# deletes a node logically and visually. if sNode is not specified, the
# entire tree is deleted.
metawidget proc Treecontrol delete { {sNode {}} } {
  if { $sNode == {} } {
    # shortcut for root node: delete direct children, clear text widget
    foreach sChild [lindex [my aNodes()] 3] {
      _delete $sChild
    }

    my aNodes() [lreplace [my aNodes()] 3 3 {}]
    $this.text delete 2.0 end
  } else {
    _checkNode $sNode

    # erase visual representation
    if { [_visible $sNode] } {
      _undrawChildren $sNode
      _undrawNode $sNode
    }

    # update children list of parent
    set sParent [lindex [my aNodes($sNode)] 2]
    my aNodes($sParent) [lreplace [my aNodes($sParent)] 3 3 [mkw.lshrink [lindex [my aNodes($sParent)] 3] $sNode]]

    # erase logical representation
    _delete $sNode
  }
}

# name: nodeconfigure
# args: sNode: node name
#       args: option-value pairs
# configures a node. only allows the four special options. to modify color,
# font etc., use tags.
metawidget proc Treecontrol nodeconfigure { sNode args } {
  _checkNode $sNode

  mkw.options $args {-parent *} {-text *} {-image *} {-final *} {-user *}

  # new node caption
  if { [info exists -text] } {
    my aNodes($sNode) [lreplace [my aNodes($sNode)] 0 0 ${-text}]

    if { [_visible $sNode] } {
      mkw.lassign [$this.text tag ranges $sNode] sI1 sI2
      $this.text delete $sI1+1ch $sI2-1ch
      $this.text insert $sI1+1ch ${-text}
    }
  }

  # new image, or no image. detects existing image with _tImg tag.
  if { [info exists -image] } {
    my aNodes($sNode) [lreplace [my aNodes($sNode)] 1 1 ${-image}]

    if { [_visible $sNode] } {
      mkw.lassign [$this.text tag ranges $sNode] sI1 sI2
      if { [lindex [$this.text tag names $sI1-1ch] 0] == "_tImg" } {
        $this.text delete $sI1-1ch
      }
      if { ${-image} != {} } {
        $this.text image create $sI1-1ch -image ${-image}
        $this.text tag add _tImg $sI1-1ch
      }
    }
  }

  # reparent the node. erases visual stuff, then reorganizes logical
  # structures, then repaints the visual again.
  if { [info exists -parent] } {
    _checkNode ${-parent}

    # delete any visual representation
    if { [_visible $sNode] } {
      _undrawChildren $sNode
      _undrawNode $sNode
    }

    # update children list of old and new parent
    set sOldParent [lindex [my aNodes($sNode)] 2]
    my aNodes($sOldParent) [lreplace [my aNodes($sOldParent)] 3 3 [mkw.lshrink [lindex [my aNodes($sOldParent)] 3] $sNode]]
    my aNodes(${-parent})  [lreplace [my aNodes(${-parent})] 3 3 [mkw.lextend [lindex [my aNodes(${-parent})] 3] $sNode]]

    # store new parent and recalculate indention
    my aNodes($sNode) [lreplace [my aNodes($sNode)] 2 2 ${-parent}]
    _recalcDepth $sNode

    # if new parent was a leaf, it now has a child and is a collapsed node
    if { [lindex [my aNodes(${-parent})] 5] == 0 } {
      my aNodes(${-parent}) [lreplace [my aNodes(${-parent})] 5 5 -1]
      _setSymbol ${-parent}
    }

    # if new parent is visible and expanded, draw its new child now
    if { [_visible ${-parent}] && [lindex [my aNodes(${-parent})] 5] == 1 } {
      _drawNode $sNode
      _drawChildren $sNode
    }

    # if that was the old parent's last child, turn it into a leaf
    if { ! [llength [lindex [my aNodes($sOldParent)] 3]] } {
      my aNodes($sOldParent) [lreplace [my aNodes($sOldParent)] 5 5 0]
      _setSymbol $sOldParent
    }
  }

  # change final state.
  if { [info exists -final] } {
    set sFinal [mkw.complete ${-final} {yes true on 1 no false off 0}]
    set bFinal [mkw.decode $sFinal {true 1 yes 1 on 1 1 1} 0]
    my aNodes($sNode) [lreplace [my aNodes($sNode)] 6 6 $bFinal]

    if { $bFinal && ! [llength [lindex [my aNodes($sNode)] 3]] } {
      my aNodes($sNode) [lreplace [my aNodes($sNode)] 5 5 0]
    } elseif { ! $bFinal && [lindex [my aNodes($sNode)] 5] == 0 } {
      my aNodes($sNode) [lreplace [my aNodes($sNode)] 5 5 -1]
    }

    _setSymbol $sNode
  }

  # arbitrary user data. simply replace element in property list.
  if { [info exists -user] } {
    my aNodes($sNode) [lreplace [my aNodes($sNode)] 8 8 ${-user}]
  }
}

# name: nodecget
# args: sNode: node name
#       args: option name
# returns the value of an option, including some read-only options. most
# of the options are taken from the node's property record. (note: a node
# can be expanded but not visible, e.g. if the parent is collapsed.)
metawidget proc Treecontrol nodecget { sNode args } {
  _checkNode $sNode

  set lOptions {-text -image -parent -children -indention -expanded -final -tags -user -visible}
  set sOption [mkw.complete $args $lOptions]

  if { $sOption == "-visible" } {
    return [_visible $sNode]
  } else {
    return [lindex [my aNodes($sNode)] [lsearch $lOptions $sOption]]
  }
}

# name: move
# args: sNode: node name
#       args: options and a position specifier
# changes the siblings order and also reparents a node. moves sNode to a
# position specified in args. args is normally an integer or "end",
# specifying the position amongst sNode's siblings. if -byname is found,
# args must contain a node name instead of an index. sNode will be moved
# to that new location. with -after, it will be placed after the destination
# node. if the latter belongs to a different parent, sNode is reparented.
metawidget proc Treecontrol move { sNode args } {
  _checkNode $sNode

  # extract options. what's left is either an index or a node name
  set args [mkw.options $args * {-byname} {-after}]
  set iPos [lindex $args 0]

  # if it's a node name, then reparent sNode first, if necessary
  if { [info exists -byname] } {
    set sDest $iPos
    _checkNode $sDest

    if { [lindex [my aNodes($sDest)] 2] != [lindex [my aNodes($sNode)] 2] } {
      nodeconfigure $sNode -parent [lindex [my aNodes($sDest)] 2]
    }
  }

  # get siblings without sNode itself
  set sParent [lindex [my aNodes($sNode)] 2]
  set lSiblings [mkw.lshrink [lindex [my aNodes($sParent)] 3] $sNode]

  # transform destination node or "end" string into index.
  if { [info exists -byname] } {
 	  set iPos [lsearch $lSiblings $iPos]

    if { [info exists -after] } {
      incr iPos
    }
  } elseif { $iPos == "end" } {
    set iPos [llength $lSiblings]
  }

  # erase visual stuff
  if { [_visible $sNode] } {
    _undrawChildren $sNode
    _undrawNode $sNode
  }

  # update data structures
  my aNodes($sParent) [lreplace [my aNodes($sParent)] 3 3 [linsert $lSiblings $iPos $sNode]]

  # redraw node, if new parent is visible and expanded
  if { [_visible $sParent] && [lindex [my aNodes($sParent)] 5] == 1 } {
    _drawNode $sNode
    _drawChildren $sNode
  }
}

# name: tag
# args: sAction: almost all commands allowed for tags
#       sTag: tag name. must not be a column name
#       args: depends on sAction
# similar to the "text tag" command. some tag commands require special
# treatment so the don't mix up the internal tags. tags cannot simply be
# applied to the text widget because a node is not always visible. therefore
# each node maintains a list with its tags. when the node is drawn later,
# the tags are applied.
metawidget proc Treecontrol tag { sAction sTag args } {
  # node names and sel are reserved, can't be used as user tags.
  if { [lsearch [linsert [my lNodes] end "sel"] $sTag] != -1 } {
    error "Gridcontrol $this: Tag name $sTag not allowed"
  }

  set sAction [mkw.complete $sAction {add bind cget configure delete lower raise names nodes remove}]

  switch $sAction {
    add - remove {
      # args is a list of nodes. adds or removes the tags from each node's
      # tag list. then, if the node is visible, updates the visual portion.
      set sCmd [mkw.decode $sAction {add mkw.lextend remove mkw.lshrink}]

      foreach sNode $args {
        _checkNode $sNode
        set lTags [lindex [my aNodes($sNode)] 7]
        my aNodes($sNode) [lreplace [my aNodes($sNode)] 7 7 [$sCmd $lTags $sTag]]

        if { [_visible $sNode] } {
          mkw.lassign [$this.text tag ranges $sNode] sI1 sI2
          $this.text tag $sAction $sTag $sI1 $sI2
        }
      }
    }

    names {
      # args is a node name. simply returns the node's tag list.
      return [lindex [my aNodes([lindex $args 0])] 7]
    }

    nodes {
      # args is irrelevant. checks for all nodes, if the given tag is part or
      # their tag list. if so, appends the node to a list. returns the list.
      set lNodes {}
      foreach sNode [my lNodes] {
        if { [lsearch [lindex [my aNodes($sNode)] 7] $sTag] == -1 } continue
        lappend lNodes $sNode
      }

      return $lNodes
    }

    lower {
      # the internal tags (those with the node names) *must* be the first
      # one returned by [text tag names]. the loop takes care of that.
      eval $this.text tag lower {$sTag} $args

      foreach sNode [tag nodes $sTag] {
        $this.text tag lower $sNode
      }
    }

    default {
      eval $this.text tag $sAction {$sTag} $args
    }
  }
}

# name: Listcontrol:selection
# args: sWhat: what to do with selection
#       args: depends on sWhat
# clears, sets or gets selection. get returns a list of nodes. set accepts
# a list (in the first element of args) of node names.
metawidget proc Treecontrol selection { sWhat args } {
  switch [mkw.complete $sWhat {clear get set}] {
    clear {
      $this.text tag remove sel 1.0 end
    }
    get {
      set lNodes {}

      foreach { sI1 sI2 } [$this.text tag ranges sel] {
        lappend lNodes [lindex [$this.text tag names $sI1] 0]
      }

      return $lNodes
    }
    set {
      $this.text tag remove sel 1.0 end

      foreach sNode [lindex $args 0] {
        _checkNode $sNode
        mkw.lassign [$this.text tag ranges $sNode] sI1 sI2
        $this.text tag add sel $sI1 $sI2
      }
    }
  }
}

# name: get
# args: iFirst, iLast: row number, or "current", or "cursor"
# returns the node or a range of nodes between the rows specified by
# iFirst (and iLast). "current" denotes the mouse position,
# "cursor" the position of the node that has the cursor.
metawidget proc Treecontrol get { iFirst {iLast {}} } {
  if { $iFirst == "current" } {
    set iFirst [lindex [split [$this.text index current] .] 0]
  } elseif { $iFirst == "cursor" } {
    set iFirst [my iCursor]
  }

  if { $iLast == {} } {
    set iLast $iFirst
  } elseif { $iLast == "current" } {
    set iLast [lindex [split [$this.text index current] .] 0]
  } elseif { $iLast == "cursor" } {
    set iLast [my iCursor]
  }

  set lNodes {}
  set iN1 [expr ($iFirst<$iLast)?$iFirst:$iLast]
  set iN2 [expr ($iFirst>$iLast)?$iFirst:$iLast]

  for { set i $iN1 } { $i <= $iN2 } { incr i } {
    lappend lNodes [lindex [$this.text tag names $i.end-1ch] 0]
  }

  return $lNodes
}

# name: see
# args: sNode: node name
# makes a node visible in the viewport of the text widget. expands all
# parents first up to the root node, then makes the text widget
# put the node's tag into the viewport.
metawidget proc Treecontrol see { sNode } {
  _checkNode $sNode

  set sParent [lindex [my aNodes($sNode)] 2]
  while { $sParent != {} } {
    expand $sParent
    set sParent [lindex [my aNodes($sParent)] 2]
  }

  $this.text see [lindex [$this.text tag ranges $sNode] 0]
}

# name: nodebind
# args: sNode: node name
#       args: as accepted by bind
# allows for bindings to individual nodes.
metawidget proc Treecontrol nodebind { sNode args } {
  _checkNode $sNode
  eval $this.text tag bind {$sNode} $args
}

# name: Gridcontrol:bind
# args: args: as accepted by the bind command.
# applies bindings to the main widget.
# Arthur :: Fixed the following by adding {$sWhat}
metawidget proc Treecontrol bind_ { sWhat args } {
  eval bind $this.text {$sWhat} $args
}

# name: Gridcontrol:nodes
# args: -
# returns a list with all nodes, unordered.
metawidget proc Treecontrol nodes {} {
  return [my lNodes]
}

# name: tree
# args: sNode: node name
# returns the structure of the subtree specified by sNode as a recursive
# list. each list element consists itself of two elements: the node and
# its child nodes, and their child nodes, and their child nodes ...
metawidget proc Treecontrol tree { {sNode {}} } {
  _checkNode $sNode

  set lChildren {}
  foreach sChild [lindex [my aNodes($sNode)] 3] {
    lappend lChildren [tree $sChild]
  }

  return [list $sNode $lChildren]
}

metawidget command Treecontrol _dump         _dump
metawidget command Treecontrol _toggle       _toggle
metawidget command Treecontrol _leftright    _leftright
metawidget command Treecontrol _select       _select

metawidget command Treecontrol expand        expand
metawidget command Treecontrol collapse      collapse
metawidget command Treecontrol insert        insert
metawidget command Treecontrol delete        delete
metawidget command Treecontrol nodeconfigure nodeconfigure
metawidget command Treecontrol nodecget      nodecget
metawidget command Treecontrol move          move
metawidget command Treecontrol tag           tag
metawidget command Treecontrol selection     selection
metawidget command Treecontrol get           get
metawidget command Treecontrol see           see
metawidget command Treecontrol nodebind      nodebind
metawidget command Treecontrol bind          bind_
metawidget command Treecontrol nodes         nodes
metawidget command Treecontrol tree          tree

metawidget option  Treecontrol -text         -text
metawidget option  Treecontrol -image        -image
metawidget option  Treecontrol -fullexpand   -fullexpand
metawidget option  Treecontrol -selectmode   -selectmode
metawidget option  Treecontrol -onexpand
metawidget option  Treecontrol -onselect

proc testGetNodes { sFolder } {
  puts "Require children for node $sFolder"

  foreach sFile [glob -nocomplain $sFolder/*] {
    set sTail [file tail $sFile]

    if { [file isfile $sFile] } {
      .tree insert $sFile -text $sTail -image pFi -parent $sFolder -final 1
    } else {
      .tree insert $sFile -text $sTail -image pFo -parent $sFolder
    }
  }

  .tree nodeconf $sFolder -final 1
}

proc testGetFirst { sPath } {
  .tree delete {}
  .tree config -text $sPath

  foreach sFile [glob -nocomplain $sPath/*] {
    set sTail [file tail $sFile]

    if { [file isfile $sFile] } {
      .tree insert $sFile -text $sTail -image pFi -final 1
    } else {
      .tree insert $sFile -text $sTail -image pFo
    }
  }
}

proc test {} {
  image create photo pNo -file ./demos/images/Nodes.gif
  image create photo pFo -file ./demos/images/Folder.gif
  image create photo pFi -file ./demos/images/File.gif

  pack [frame .ctrl] -side top -fill x
  pack [button .ctrl.brws -text "Browse" -command {testGetFirst $sPath}] -side right
  pack [entry  .ctrl.path -textvariable sPath] -side left -fill x -expand 1
  pack [treecontrol .tree -text "(Enter a path and press Browse)" -image pNo -onexpand testGetNodes -fullexpand 1] -side bottom -fill both -expand 1
}

#test

