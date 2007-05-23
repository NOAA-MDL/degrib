// Assume Gnu Conventions except where we over-ride them

// Extra Rules:
-fca     // Format all comments (berkeley)
-sc      // put a '*' at left of comments. (berkeley)
-di3     // put variables in column 3 or first available.
-ip0     // 0 Extra spaces per paren (KR)

// following were common to KR & Berkeley
-c33     // Put comments to the right of code in column 33
-cd33    // Put comments to the right of declarations in column 33
-cp33    // put comments after #if in column 1.

// MDL rules:
-nut     // No tabs (Rule 2.1) (Rule 3.2)
-l80     // line length max 80 (Rule 2.1)
-lc80    // line length for comment lines max 80 (Rule 2.1)
-ss      // Best I can do for (Rule 2.2)
-npsl    // type of procedure is on same line as procedure. (Ex 2.2)
-npcs    // Put space after function in function calls (Ex 2.2)
-i3      // indent 3 columns (Rule 2.3)
-bli0    // indent braces 0. (Rule 2.4)
-bls     // (Gnu) braces on line after struct. (Rule 2.4)
-bl      // (Gnu) braces on line after if (Rule 2.5)
-nce     // (Gnu) Do not cuddle { and else (Rule 2.5)
-cli3    // Case level indentation 3 spaces (Ex 2.5)
-cdw     // Cuddle do while (Rule 2.7)
-nbap    // Don't force blank lines after proc bodies (need for -cdw) 
-ci6     // Continuation line indent of 6 spaces (override -ci3)
         // Still not satisfactory for (Rule 3.4 with '+')
-ncs     // Don't put space after cast (Rule 3.5)
-cd31    // Put comments to the right of declaration (Based on 5.2)
-c31     // Put comment after code in col 33 (pref 25) (Base on 5.2)
         //   Ignored Ex. 5.6 to be consistent at column 31.
-cp31    // Put comments after #if in column 31.
-ts2     // Set tab size to 2 if exceed cd31 (Based on Ex 5.2)

// own rules (conflicting) (keep)
-c25     // Put comments to the right of code in column 25
-cd25    // Put comments to the right of declarations in column 25
-ts1     // Set tab size to 1 so if cd25 is over filled it only adds 1.

// own rules (conflicting)
//-cs      // Put space after cast.
//-pcs     // Put space after function in function calls(over ride -npcs)
//-bs      // Put a space between sizeof and its argument.
-l78     // line len max 80 (Rule 2.1) diff doesn't like '*/'
-lc78    // line len for comment lines max 80 (Rule 2.1) diff doesn't like '*/'

// own rules (conflicting) (keep)
-br      // (KR) braces on same line as if (override -bl)
-brs     // (KR) braces on struct line (override -bls)
-ce      // (KR) Cuddle { and else (override -nce)

// Recognized types.
-T sChar
-T sShort2
-T sInt4
-T uChar
-T uShort2
-T uInt4
-T size_t
-T FILE
-T LatLon

// Recognized types... Application Specific.

