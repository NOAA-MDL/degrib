package ifneeded tls 1.6.5 \
    "[list source [file join $dir tls.tcl]] ; \
     [list tls::initlib $dir tls165.dll]"
