package provide Metawidget 1.3

# all new procedures are defined in the metawidget namespace. each created
# metawidget will use its own namespace, which has the name of the widget's
# class. in such a class namespace, all procedures defined for the class
# are held, as well as arrays containing information about commands and
# options. also, class common variables are stored in the class namespace.
# each instance of a metawidget defines another namespace with its name,
# where all private variables are held.
namespace eval ::metawidget {
  # name: metawidget
  # args: -
  # the only command exported by this package for use outside of actual
  # metawidget code. all others can or should only be used within
  # procedure definitions of metawidgets. this command allows to create,
  # delete, and build a metawidget.
  proc metawidget { sCmd {sClass {}} args } {
    set sCommand [mkw.complete $sCmd {create delete command option proc names info export}]
    set iArgc [llength $args]

    switch $sCommand {
      create {
        # creates a metawidget by calling _createClass. constructor and
        # destructor scripts must be provided, plus a list of options that
        # specify a) the widget type for the enclosing widget, b) the
        # (relative) name of the widget where all calls to configure and
        # cget are applied to, and c) any number of option-value pairs
        # to configure the enclosing widget.
        if { $iArgc < 2 } {
          error "Wrong # args: Must be metawidget create class initbody destroybody ?options?"
        } elseif { ! [string match {[A-Z]*} $sClass] } {
          error "Bad metawidget class name '$sClass'. Class name must start with uppercase character"
        }

        set sConstr [lindex $args 0]
        set sDestr  [lindex $args 1]
        set lArgs   [lrange $args 2 end]

        _createClass $sClass $sConstr $sDestr $lArgs

        # store new metawidget in list of all metawidgets
        variable lMetawidgets
        set lMetawidgets [mkw.lextend $lMetawidgets $sClass]
      }

      delete {
        # deletes a metawidget by deleting its namespace
        if { $iArgc > 0 } {
          error "Wrong # args: Must be metawidget delete class"
        }

        _checkClass $sClass
        namespace delete ::$sClass

        # remove metawidget from list of all metawidgets
        variable lMetawidgets
        set lMetawidgets [mkw.lshrink $lMetawidgets $sClass]
      }

      proc {
        # defines a metawidget procedure. this is just like any other
        # procedure, i.e. it has a name, args and body, except that it
        # is created in the metawidget's namespace, plus its variable
        # context always contains a variable 'this' that holds the
        # name of the current widget instance.
        if { $iArgc != 3 } {
          error "Wrong # args: Must be metawidget proc class name args body"
        }

        _checkClass $sClass
        mkw.lassign $args sProc lArgs sBody

        if { [lsearch [list $sClass ~$sClass __dispatch __configure __cget] $sProc] != -1 } {
          error "Bad proc name '$sProc': This name is predefined or reserved."
        }

        proc ::${sClass}::$sProc $lArgs "
          upvar this this
          $sBody
        "
      }

      command {
        # defines a metawidget command. a metawidget command does not contain
        # own code but is simply mapped to a metawidget procedure. therefore,
        # two arguments are expected: command name and procedure name.
        # this information is stored in an array within the metawidget's
        # namespace.
        if { $iArgc != 2 } {
          error "Wrong # args: Must be metawidget command class name proc"
        }

        _checkClass $sClass
        mkw.lassign $args sCommand sProc

        set ::${sClass}::__aCmd($sCommand) $sProc
      }

      option {
        # defines a metawidget option. a metawidget option is an option that
        # cannot be mapped to the default widget, but is handled explicitely
        # by the metawidget itself. the first argument to this call must be
        # the option name. there are three ways how to implement an option:
        # a) no additional arguments: if the option is set, then a private
        # variable with the same name as the option is set. if the option
        # is retrieved, the value is taken from that private variable.
        # b) one argument: this must specify a procedure that actually
        # sets the option, i.e. contains necessary code to process it.
        # if the option is retrieved, the value is taken from a private
        # variable with the same name as the option. therefore the
        # specified procedure *must* eventually set this private variable!
        # c) two arguments: like above, but the second argument is used
        # to retrieve the option value. no private variable needs to be set.
        if { $iArgc < 1 || $iArgc > 3 } {
          error "Wrong # args: Must be metawidget option class name ?setProc? ?getProc?"
        }

        _checkClass $sClass
        mkw.lassign $args sOption sSetProc sGetProc

        switch $iArgc {
          1 { set ::${sClass}::__aOpt($sOption) {} }
          2 { set ::${sClass}::__aOpt($sOption) [list $sSetProc] }
          3 { set ::${sClass}::__aOpt($sOption) [list $sSetProc $sGetProc] }
        }
      }

      names {
        # returns the names of all defined metawidgets
        if { $iArgc } {
          error "Wrong # args: Must be metawidget names"
        }

        variable lMetawidgets
        return $lMetawidgets
      }

      info {
        # returns various information about metawidgets
        if { $iArgc != 1 } {
          error "Wrong # args: Must be metawidget info name option"
        }

        _checkClass $sClass
        _getInfo $sClass [lindex $args 0]
      }

      export {
        # returns the metawidget's current namespace code. this code
        # would create the metawidget without mkWidgets installed.
        if { $iArgc != 0 } {
          error "Wrong # args: Must be metawidget export name"
        }

        _checkClass $sClass

        set sCmd "namespace eval ::$sClass {\n"

        append sCmd "  array set __aOpt \[list [array get ::${sClass}::__aOpt]\]\n"
        append sCmd "  array set __aCmd \[list [array get ::${sClass}::__aCmd]\]\n\n"

        foreach sProc [lsort [info procs ::${sClass}::*]] {
          append sCmd "  proc [namespace tail $sProc] { "
          foreach sArg [info args $sProc] {
            if { [info default $sProc $sArg sDefault] } {
              append sCmd "{$sArg {$sDefault}} "
            } else {
              append sCmd "$sArg "
            }
          }
          append sCmd "} {[info body $sProc]}\n\n"
        }

        append sCmd "  namespace export $sClass\n}\n\n"

        append sCmd "namespace eval :: {
          namespace import ::${sClass}::$sClass
          rename $sClass [string tolower $sClass]
        }"
      }
    }
  }

  # name: _checkClass
  # args: sClass: metawidget class name
  # simply checks if a namespace with the name $sClass exists. not really
  # a bulletproof check.
  proc _checkClass { sClass } {
    if { [info command ::${sClass}::$sClass] == "" } {
      error "Metawidget $sClass does not exist."
    }
  }

  # name: _createClass
  # args: sClass: metawidget class to create
  #       sConstr: constructor
  #       sDestr: destructor
  #       lArgs: option-value pairs
  # creates the framework for a new metawidget: it defines a new namespace
  # and imports the my/our functions that handle private/class common
  # variables as well as the mkw.* tool functions. then creates and exports
  # the new widget command within the new namespace, which will embedd
  # the constructor script. also defines a proc holding the destructor
  # (called from a <destroy> event, and the standard widget commands
  # configure and cget.
  proc _createClass { sClass sConstr sDestr lArgs } {
    # extract special options, the rest is for the default widget
    set lArgs [mkw.options $lArgs * {-default *} {-type {frame toplevel} frame} {-command *}]

    # type is the widget type for the enclosing widget (a frame by default)
    set sType [string tolower ${-type}]

    # if no command name specified, it will be sClass in lower case
    if { [info exists -command] } {
      set sCommand ${-command}
    } else {
      set sCommand [string tolower $sClass]
    }

    # if no default widget specifies, we take the enclosing widget
    if { [info exists -default] } {
      set wDefault \$this.${-default}
    } else {
      set wDefault \${this}::\$this
    }

    # create a namespace for the new metawidget class
    namespace eval ::$sClass "
      # this creates an instance of the new metawidget class. this is the name
      # of the widget, args are option-value pairs.
      proc $sClass { this args } {
        # create enclosing widget (normally a frame), and configure it with the
        # options specified during the metawidget's definition.
        eval {$sType} {\$this} -class {$sClass} $lArgs

        # exchange the class name in the widget's bindtags
        bindtags \$this \[lreplace \[bindtags \$this\] 1 1 $sClass\]

        # move the widget from the global into its own namespace...
        rename ::\$this ::\${this}::\$this

        # ...and create a procedure instead, that takes the same args as
        # a widget would. it will pass the args to the dispatch procedure.
        namespace eval :: \"
          proc \$this { {sCmd {}} args } {
            ::${sClass}::__dispatch \$this \\\$sCmd \\\$args
          }
        \"

        # then evaluate the constructor script
        $sConstr

        # configure the default widget, if options were passed in args
        eval __configure \$args

        return \$this
      }

      # this is called when a widget is destroyed. evaluates the destructor
      # script and deletes the widget's namespace.
      proc ~$sClass { this } {
        $sDestr
        namespace delete ::\$this
      }

      # whenever a metawidget command is called (e.g. '.mycombobox config
      # -state normal'), the widget uses this procedure to call the
      # corresponding metawidget procedure. it also resolves abbreviated
      # command names. if the command called is not defined, it applies
      # it to the default widget.
      proc __dispatch { this sCmd lArgs } {
        if { \$sCmd == {} } {
          error \"Wrong # args: Should be '\$this command args ?args ...?'\"
        }

        # here are all commands and their associated procs
        variable __aCmd

        # if the command is not in the array, it might be abbreviated
        if { \[info exists __aCmd(\$sCmd)\] } {
          eval {\$__aCmd(\$sCmd)} \$lArgs
        } else {
          set sCommand \[array names __aCmd \${sCmd}*\]
          set iMatches \[llength \$sCommand\]

          if { \$iMatches == 1 } {
            eval {\$__aCmd(\$sCommand)} \$lArgs    ;# successfully resolved
          } elseif { \$iMatches == 0 } {
            eval {$wDefault} {\$sCmd} \$lArgs      ;# not found, try default
          } else {
            error \"Ambiguous command '\$sCmd': Must be \[join \[array names __aCmd\] {, }\]\"
          }
        }
      }

      # each widget understands the configure and cget commands. therefore a
      # metawidget comes with them, too. this proc reads all the option-value
      # pairs in args and does a lookup in the option array. if it does not
      # find it, it applies the option to the default widget. otherwise it
      # either sets a private variable or calls the associated proc.
      proc __configure { args } {
        upvar this this
        variable __aOpt

        set iArgs \[llength \$args\]

        if { \$iArgs == 0 } {                        ;# return all options
          return \"Function not yet implemented\"
        } elseif { \$iArgs == 1 } {                  ;# return data for option
          return \"Function not yet implemented\"
        } elseif { \[expr \$iArgs % 2\] } {          ;# odd number of args
          error \"Failed to configure \$this: Invalid number of arguments\"
        } else {                                     ;# process options
          foreach { sOpt sValue } \$args {
            if { \[info exists __aOpt(\$sOpt)\] } {  ;# found in array
              set sOption \$sOpt
            } else {                                 ;# not found, resolve it
              set sOption \[array names __aOpt \${sOpt}*\]
              set iMatches \[llength \$sOption\]

              if { \$iMatches == 0 } {               ;# not found, try default
                $wDefault configure \$sOpt \$sValue
                continue
              } elseif { \$iMatches > 1 } {
                error \"Ambiguous option '\$sOpt': Must be \[join \[array names __aOpt\] {, }\]\"
              }
            }

            if { \[llength \$__aOpt(\$sOption)\] == 0 } { ;# just set variable
              my \$sOpt \$sValue
            } else {                                      ;# call procedure
              eval \[lindex \$__aOpt(\$sOption) 0\] {\$sValue}
            }
          }
        }
      }

      # similar to __configure. looks up the option array. if it does not find
      # the option there, then it tries to get the value from the default
      # widget. otherwise either gets it from a private variable or from
      # the associated procedure.
      proc __cget { sOpt } {
        upvar this this
        variable __aOpt

        if { \[catch {llength \$__aOpt(\$sOpt)} iLen\] } {
          $wDefault cget \$sOpt
        } elseif { \$iLen != 2 } {
          my \$sOpt
        } else {
          eval \[lindex \$__aOpt(\$sOpt) 1\]
        }
      }

      # this will call the destructor on deletion of the widget
      bind $sClass <Destroy> {::${sClass}::~$sClass %W}

      # preset the commands array with the standard commands configure and cget
      set __aCmd(configure) __configure
      set __aCmd(cget)      __cget

      # finally export the new class command and import the other stuff
      namespace export $sClass
      namespace import ::metawidget::un* ::metawidget::my* ::metawidget::our* ::metawidget::mkw.*
    "

    # in the global namespace, import the new class command and immediately
    # rename it to the widget command (normally lower case).
    namespace eval :: "
      namespace import ::${sClass}::$sClass
      rename $sClass $sCommand
    "
  }

  # name: _getInfo
  # args: sClass: metawidget class
  #       sOption: procs, commands, or options
  # returns information about a metawidget. for procs, returns a list of
  # defined metawidget procedures, where each element consists of a sublist
  # with the three elements procecure name, args, body. for commands, returns
  # a list of defined metawidget commands, where each element consists of a
  # sublist containing the command name and its associated procedure.
  # for options, returns a list of defined metawidget options, where each
  # element consists of a list containing the elements option name, set-
  # procedure (if defined) and get-procedure (if defined).
  proc _getInfo { sClass sOption } {
    switch [mkw.complete $sOption {procs commands options}] {
      procs {
        set lProcs {}
        foreach sProc [info procs ::${sClass}::*] {
          lappend lProcs [list [namespace tail $sProc] [info args $sProc] [info body $sProc]]
        }

        return [lsort -index 0 $lProcs]
      }

      commands {
        set lCmds {}
        foreach {sCmd sProc} [array get ::${sClass}::__aCmd] {
          lappend lCmds [list $sCmd $sProc]
        }

        return [lsort -index 0 $lCmds]
      }

      options {
        set lOpts {}
        foreach {sOpt lProcs} [array get ::${sClass}::__aOpt] {
          lappend lOpts [list $sOpt $lProcs]
        }

        return [lsort -index 0 $lOpts]
      }
    }
  }

  # name: my
  # args: args: varname ?value?
  # sets or retrieves a private variable. very similar to the set command.
  # sets a variable in the widget's namespace.
  proc my { args } {
    upvar this this
    namespace eval ::$this set $args
  }

  # name: unmy
  # args: args: list of private variables
  # unsets private variables, just like the unset command.
  proc unmy { args } {
    upvar this this
    namespace eval ::$this "catch {unset $args}"
  }

  # name: myarray
  # args: args: anything allowed for the array command
  # performs array operations on private arrays. maps to the array command.
  proc myarray { args } {
    upvar this this
    namespace eval ::$this array $args
  }

  # name: myinfo
  # args: sOption: class, exists, vars
  #       args: depends on sOption
  # provides information regarding the widget and its state. 'class' returns
  # the widget's class, 'exists' checks if a private variable exists, and
  # 'vars' returns a list of private variables. the latter allows to
  # specify a pattern following the glob rules.
  proc myinfo { sOption args } {
    upvar this this
    set sClass [lindex [bindtags $this] 1]

    switch $sOption {
      class {
        set sClass
      }

      exists {
        namespace eval ::$this info exists $args
      }

      vars {
        set lVars {}
        if { [llength $args] } {
          set sPattern [lindex $args 0]
        } else {
          set sPattern *
        }

        foreach sVar [info vars ::${this}::$sPattern] {
          lappend lVars [namespace tail $sVar]
        }

        set lVars
      }

      default {
        error "Bad option '$sOption'. Must be class, exists, vars"
      }
    }
  }

  # name: our
  # args: args: varname ?value?
  # sets or retrieves a class-common variable. very similar to the set command.
  # sets a variable in the namespace of the object's class.
  proc our { args } {
    upvar this this

    set sClass [lindex [bindtags $this] 1]
    namespace eval ::$sClass set $args
  }

  # name: unour
  # args: args: list of class-common variables
  # unsets class-common variables, just like the unset command.
  proc unour { args } {
    upvar this this
    set sClass [lindex [bindtags $this] 1]
    namespace eval ::$sClass "catch {unset $args}"
  }

  # name: ourarray
  # args: args: anything allowed for the array command
  # performs array operations on class-common arrays. maps to the array command.
  proc ourarray { args } {
    upvar this this
    set sClass [lindex [bindtags $this] 1]
    namespace eval ::$sClass array $args
  }


  # name: ourinfo
  # args: sOption: exists, vars
  #       args: depends on sOption
  # provides information regarding class-common variables. 'exists' checks if
  # a class-common variable exists, 'vars' returns a list of those variables.
  # the latter allows to specify a pattern following the glob rules.
  proc ourinfo { sOption args } {
    upvar this this
    set sClass [lindex [bindtags $this] 1]

    switch $sOption {
      exists {
        namespace eval ::$sClass info exists $args
      }

      vars {
        set lVars {}
        if { [llength $args] } {
          set sPattern [lindex $args 0]
        } else {
          set sPattern *
        }

        foreach sVar [info vars ::${sClass}::$sPattern] {
          lappend lVars [namespace tail $sVar]
        }

        set lVars
      }

      default {
        error "Bad option '$sOption'. Must be exists, vars"
      }
    }
  }

  # name: mkw.lextend
  # args: lList: any tcl list
  #       args: elements to append to list
  # for each element in args, checks if it already exists in lList, and,
  # if does not yet exist, appends it to lList. returns the new list.
  proc mkw.lextend { lList args } {
    foreach sArg $args {
      if { [lsearch $lList $sArg] != -1 } continue
      lappend lList $sArg
    }

    return $lList
  }

  # name: mkw.lshrink
  # args: lList: any tcl list
  #       args: elements to delete from list
  # for each element in args, checks if it exists in lList, and,
  # if it does, deletes it. returns the new list.
  proc mkw.lshrink { lList args } {
    foreach sArg $args {
      set iPos [lsearch $lList $sArg]
      if { $iPos == -1 } continue
      set lList [lreplace $lList $iPos $iPos]
    }

    return $lList
  }

  # name: mkw.lchange
  # args: lList: any tcl list
  #       sOld: element to replace
  #       sNew: new value to replace sOld with
  # searches sOld in lList and replaces it with sNew. return the
  # new list.
  proc mkw.lchange { lList sOld sNew } {
    set iPos [lsearch $lList $sOld]
    if { $iPos != -1 } {
      set lList [lreplace $lList $iPos $iPos $sNew]
    }

    return $lList
  }

  # name: mkw.lassign
  # args: lList: any tcl list
  #       args: list of variable names
  # assigns the first element in lList to the variable specified with the
  # first element in args, and so on. does not change remaining variables
  # in args, if there are too few elements in lList.
  proc mkw.lassign { lList args } {
    set iMax [llength $lList]
    set iCnt 0

    foreach sElem $lList sVar $args {
      if { $sVar == {} } break
      uplevel [list set $sVar $sElem]

      if { [incr iCnt] >= $iMax } break
    }
  }

  # name: mkw.decode
  # args: sValue: value to decode
  #       lPairs: old/new value pairs
  #       args: one element, the default value
  # performs a mapping of values, similar to 'string map' (>= Tcl 8.1).
  # searches sValue in the even-numbered elements of lPairs and
  # returns the corresponding value (right next to a found match in
  # lPairs). if sValue is not found, either returns sValue or any
  # default value specified in args.
  proc mkw.decode { sValue lPairs args } {
    foreach { sMatch sRet } $lPairs {
      if { $sMatch != $sValue } continue
      return $sRet
    }

    if { [llength $args] } {
      return [lindex $args 0]
    } else {
      return $sValue
    }
  }

  # name: mkw.complete
  # args: sShort: abbreviated name
  #       lValues: list of full names
  #       sWhat: text prefix for error message
  # checks if sShort is a non-ambiguous abbreviation of any of the values
  # in lValues and returns that value.
  proc mkw.complete { sShort lValues {sWhat option}} {
    foreach sValue $lValues {
      set aTmp($sValue) {}
    }

    if { [info exists aTmp($sShort)] } {
      return $sShort
    } else {
      set sLong [array names aTmp $sShort*]

      if { [llength $sLong] == 1 } {
        return $sLong
      } elseif { [llength $sLong] == 0 } {
        error "Bad $sWhat '$sShort': Must be [join $lValues ,]"
      } else {
        error "Ambiguous $sWhat '$sShort': Must be [join $lValues ,]"
      }
    }
  }

  # name: mkw.options
  # args: lArgs: list of options or option-value pairs
  #       args: list of option specifications
  # a simple option processing routine. options can have no arguments (like
  # with lsort -dictionary) or one argument (like fconfigure ... -blocking 1).
  # lArgs is expected to consist of any combination of zero- or one-argument
  # options, e.g. '-dictionary -command mySort -index 1 -decreasing'.
  # allowed options are passed in args with option specifications. for options
  # with no argument, the option specification is simply the name of the
  # option, e.g. -dictionary. for options with a value, an option specification
  # consists of two or three elements: the first is the option name, the second
  # a list of allowed values or a * for any value. the third element is
  # optional and specifies a default value if the option is not found in lArgs.
  # for each option found, a variable with the same name as the option is set
  # in the calling procedure's context. this variable's value is either a 1
  # for options without argument, or the option's value. options can be
  # abbreviated, as well as option values that map to a list of allowed values
  # in the option specification. the command will return an error message
  # if an unknown option was found, or, if one of the option specifications is
  # a plain *, will return all elements in lArgs that could not be resolved.
  proc mkw.options { lArgs args } {
    # first store the option specifications in some arrays for fast access
    foreach lSpec $args {
      set iArgc [llength $lSpec]

      if { $iArgc < 1 || $iArgc > 3 } {
        error "Bad option spec '$lSpec': Must be name ?valuelist or *? ?defaultValue?"
      }

      set sOpt [lindex $lSpec 0]
      set aOpt($sOpt) $iArgc
      set aVal($sOpt) [lindex $lSpec 1]

      # if default value is specified, set it right away
      if { $iArgc == 3 } {
        uplevel [list set $sOpt [lindex $lSpec 2]]
      }
    }

    set lRest  {}            ;# remaining arguments not covered by option specs
    set iArgc  [llength $lArgs]
    set bAllow [info exists aOpt(*)]   ;# true if command shall not complain

    # go through the option string and resolve
    for { set i 0 } { $i < $iArgc } { incr i } {
      set sOpt [lindex $lArgs $i]

      # get defined options that match
      if { [info exists aOpt($sOpt)] } {
        set sOption $sOpt
      } else {
        set sOption [array names aOpt $sOpt*]
      }

      # resolve to exactly one option or complain or add to lRest
      if { [llength $sOption] == 1 } {
        if { $aOpt($sOption) == 1 } {
          uplevel [list set $sOption 1]
        } elseif { $aVal($sOpt) == "*" } {
          incr i
          uplevel [list set $sOption [lindex $lArgs $i]]
        } else {
          incr i
          uplevel [list set $sOption [mkw.complete [lindex $lArgs $i] $aVal($sOpt) "option value"]]
        }
      } elseif { [llength $sOption] > 1 } {
        error "Ambiguous option '$sOpt': Must be [join [array names aOpt] ,]"
      } elseif { ! $bAllow } {
        error "Bad option '$sOpt': Must be [join [array names aOpt] ,]"
      } else {
        lappend lRest $sOpt
      }
    }

    return $lRest
  }

  # maintain a list of metawidgets defined so far
  set lMetawidgets {}

  # export metawidget command, the tool functions and the other stuff
  namespace export metawidget mkw.* un* my* our*
}

# import the metawidget command
namespace import ::metawidget::metawidget

