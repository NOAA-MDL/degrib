================= myopt.c ======================

 * myUsage() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This ouputs a Gnu usage message with getopt (as opposed to argp).  For
 * more info, see:
 * http://www.cs.utah.edu/dept/old/texinfo/standards/standards.html#SEC22
 * http://www.gnu.org/software/libc/manual/html_node/Argp.html#Argp
 *
 * ARGUMENTS
 *    name = Name of the program (Input)
 * argsDoc = Documentation on how to call the program (Input)
 *     doc = Main documentation for the program.  A \v separates the text
 *           into before and after the optlist (Input)
 * optLong = The long options used with getopt_long (Input)
 * optHelp = Supplemental to optLong, describes each option.  A \v at the
 *           beginning of an optHelp[].doc means no 'long option' (Typically
 *           used as a place holder).  A 0 for optHelp[].val means no 'short
 *           option'. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 ran out of optHelp[] array elements

-----------------

 * myGetOpt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Decode options from argv.  A -- indicates a long option, or end of
 * options.  A - indicated first a long option.  If it doesn't match the long
 * options, then the - indicates a set of short options.  For example, '-foo'
 * means first look for long option "foo", then short options 'f', 'o', 'o'.
 *    If a option has an argument, a pointer to it is stored in optarg.
 *    After all options are found, gl.optind points to the next index in argv.
 *
 * For more info see:
 *   http://www.gnu.org/software/libc/manual/html_node/Getopt.html
 * Similar to getopt_long_only except it doesn't use global variables.
 *
 * Uses 2 static variables...
 *    f_first : true for first call (so it init gl.optind to 1) false after.
 *    nextChar : location in short option we are working on.
 *
 * ARGUMENTS
 *     argc = The number of command line options (Input)
 *     argv = The command line options (Input)
 * optShort = The short options.  A ':' means a required argument, a '::'
 *            means an optional argument. (Input)
 *  optLong = The long options (Input)
 *       gl = The return values: (Output)
 *            (char *) optarg => any arguments associated with option.
 *            (int) optind => index to point after options in argv
 *            (int) optopt => value of bad option if error with optShort
 *            (int) index => index value of current option in optLong or -1
 *
 * RETURNS: int
 *   -1 = found all options.
 *    0 = Set a long option flag (optLong.flag != NULL),
 *        or optLong.flag == NULL, and optLong.val = 0.
 *   '?' = unrecognized option.
 *   1..255 = val of the short option, or if optLong.flag == NULL then value
 *            of the optLong.val.

================= myassert.c ===================

 * myAssert() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This is an Assert routine from "Writing Solid Code" by Steve Maguire.
 *
 *    Advantages of this over "assert" is that assert stores the expression
 * string for printing.  Where does assert store it?  Probably in global data,
 * but that means assert is gobbling up space that the program may need for no
 * real advantage.  If you trigger assert, you're going to look in the file
 * and see the code.
 *
 * ARGUMENTS
 *    file = Filename that assert was in. (Input)
 * lineNum = Line number in file of the assert. (Input)
 *
 * RETURNS: void

================= allocSprintf.c ===================

 * allocSprintf() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Based on minprintf (see K&R C book (2nd edition) page 156.  This code
 * tries to provide some of the functionality of sprintf, while at the same
 * time handling memory allocation.  In addition, it provides a %S option,
 * which allows one to pass in an array of strings, and get back a comma
 * delimited string.
 *    The code allocates exactly the amount needed.  This could result in
 * extra calls to 'realloc'.  In addition, if Size != 0, it always starts new
 * writes at strlen(Ptr) (ie it doesn't over-write).
 *
 *    Supported formats:
 * %0.4f => float, double
 * %03d %ld %10ld => int, sInt4.
 * %c => int
 * %e => float, double
 * %g => float, double
 * %s => Null terminated char string. (no range specification)
 * %S => take a char ** and turn it into a comma delimited string.
 *
 * ARGUMENTS
 *  Ptr = An array of data that is of size LenBuff. (Input/Output)
 * Size = The allocated length of Ptr. (Input/Output)
 *  fmt = Format similar to the one used by sprintf to define how to print the
 *        message (Input)
 *   ap = argument list initialized by a call to va_start.  Contains the
 *        data needed by fmt. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 memory allocation error

-----------------

 * mallocSprintf() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    The procedure combines malloc and sprintf.  It does so by allocating the
 * memory as it does the sprintf.  It does not save any information that the
 * original pointer may have pointed to.
 *
 *    Supported formats:
 * %0.4f => float, double
 * %03d %ld %10ld => int, sInt4.
 * %c => int
 * %e => float, double
 * %g => float, double
 * %s => Null terminated char string. (no range specification)
 * %S => take a char ** and turn it into a comma delimited string.
 *
 * ARGUMENTS
 * Ptr = Place to point to new memory which contains the message (Output)
 * fmt = Format similar to the one used by sprintf to define how to print the
 *       message (Input)
 * ... = Extra arguments
 *
 * RETURNS: int
 *    0 ok
 *   -1 memory allocation error

-----------------

 * reallocSprintf() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    The procedure combines malloc, sprintf, and strcat.  It does so by
 * starting to perform an sprintf at the end of the string (kind of like
 * strcat) but then allocating more memory as it needs to.
 *
 *    Supported formats:
 * %0.4f => float, double
 * %03d %ld %10ld => int, sInt4.
 * %c => int
 * %e => float, double
 * %g => float, double
 * %s => Null terminated char string. (no range specification)
 * %S => take a char ** and turn it into a comma delimited string.
 *
 * ARGUMENTS
 * Ptr = Pointer to memory to add the message to. (Input/Output)
 * fmt = Format similar to the one used by sprintf to define how to print the
 *       message (Input)
 * ... = Extra arguments
 *
 * RETURNS: int
 *    0 ok
 *   -1 memory allocation error

================= mywarn.c ===================

 * myWarnSet() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This sets the parameters for the warning module.
 *
 * All errors to stderr (DEFAULT): myWarnSet(0, 1, 0, 0, NULL);
 * All errors to stdout: myWarnSet(1, 0, 0, 0, NULL);
 *
 * ARGUMENTS
 *    Following 4 flags are as follows: 0=don't output, 1=notes+warn+err,
 *                                      2=warn+err, 3(or more)=err.
 * f_stdout = flag for when to output to stdout (Input)
 * f_stderr = flag for when to output to stderr (Input)
 *    f_mem = flag for when to output to memory buffer (Input)
 *    f_log = flag for when to output to log (file) (Input)
 *  logFile = Opened file to write log messages to (or NULL) (Input)
 *
 * RETURNS: void

-----------------

 * myWarnClear() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This clears the warning module.  It closes the logfile (if requested),
 * returns what is in memory, and resets memory to NULL.
 *
 * ARGUMENTS
 * f_closeFile = true if we should close the log file. (Input)
 *         msg = Any memory stored in the warning module (Output)
 *
 * RETURNS: void

-----------------

 * myWarn_Note() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This prints a warning message of level "1=Note" to the devices that are
 * allowed to receive those levels of warning messages.
 *
 * ARGUMENTS
 * fmt = Format to define how to print the msg (Input)
 * ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems

-----------------

 * myWarn_Warn() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This prints a warning message of level "2=Warn" to the devices that are
 * allowed to receive those levels of warning messages.
 *
 * ARGUMENTS
 * fmt = Format to define how to print the msg (Input)
 * ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems

-----------------

 * myWarn_Err() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This prints a warning message of level "3=Err" to the devices that are
 * allowed to receive those levels of warning messages.
 *
 * ARGUMENTS
 * fmt = Format to define how to print the msg (Input)
 * ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems

-----------------

 * myWarn_Loc() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This allows us to create a set of macros which will provide the filename
 * and line number to myWarn at various warning levels.  This should allow one
 * to switch from:
 * myWarn_Err("(%s line %d) Test: Ran out of memory\n", __FILE__, __LINE__);
 * to:
 * myWarn_Err1ARG("Test: Ran out of memory\n");
 * myWarn_Err2ARG("Test: Ran out of memory %d\n", value);
 * ...
 *
 * ARGUMENTS
 *     fmt = Format to define how to print the msg (Input)
 *    file = File of initial call to myWarn module (or NULL). (Input)
 * lineNum = Line number of inital call to myWarn module. (Input)
 *     ... = The actual message arguments. (Input)
 *
 * RETURNS: int
 *    0 ok
 *   -1 vfprintf had problems
 *   -2 allocSprintf had problems

-----------------

 * myWarn_Note1Arg(f) :: Macro for a note with 1 argument
 * myWarn_Warn1Arg(f) :: Macro for a warning with 1 argument
 * myWarn_Err1Arg(f)  :: Macro for an error with 1 argument

 * myWarn_Note2Arg(f) :: Macro for a note with 2 argument
 * myWarn_Warn2Arg(f) :: Macro for a warning with 2 argument
 * myWarn_Err2Arg(f)  :: Macro for an error with 2 argument

 * myWarn_Note3Arg(f) :: Macro for a note with 3 argument
 * myWarn_Warn3Arg(f) :: Macro for a warning with 3 argument
 * myWarn_Err3Arg(f)  :: Macro for an error with 3 argument

 * myWarn_Note4Arg(f) :: Macro for a note with 4 argument
 * myWarn_Warn4Arg(f) :: Macro for a warning with 4 argument
 * myWarn_Err4Arg(f)  :: Macro for an error with 4 argument

================= myutil.c ===================

 * reallocFGets() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Read in data from file until a \n is read.  Reallocate memory as needed.
 * Similar to fgets, except we don't know ahead of time that the line is a
 * specific length.
 *    Assumes that S is either NULL, or points to Len memory.  Responsibility
 * of caller to free the memory.
 *
 * ARGUMENTS
 *    S = The string of size Size to store data in. (Input/Output)
 * Size = The allocated length of S. (Input/Output)
 *   fp = Input file stream (Input)
 *
 * RETURNS: int
 *   -1 on error (memory allocation)
 *    0 we read only EOF
 *    strlen (*S) (0 = Read only EOF, 1 = Read "\nEOF" or "<char>EOF")

-----------------

 * strncpyTrim() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Perform a strncpy, but only copy the non-white space.  It looks at the
 * first n bytes of src, and removes white space from left and right sides,
 * copying the result to dst.
 *    Unlike strncpy, it doesn't fill with '\0'.
 *    Also, it adds a '\0' to end of dst, so it assumes dst is allocated to at
 * least (n+1).
 *
 * ARGUMENTS
 * dst = The resulting string (Output)
 * src = The string to copy/trim (Input)
 *   n = The number of bytes to copy/trim (Input/Output)
 *
 * RETURNS: char *
 *    returns "dst"

-----------------

 * mySplit() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Split a character array according to a given symbol.  Responsibility of
 * caller to free the memory.
 *    Assumes that argc is either 0, or is the number of entries allocated in
 * argv.
 *
 * ARGUMENTS
 *   data = character string to look through. (Input)
 * symbol = character to split based on. (Input)
 *   argc = number of groupings found. (Input/Output)
 *   argv = characters in each grouping. (Input/Output)
 * f_trim = Should trim the white space from each element in list? (Input)
 *
 * RETURNS: int
 * -1 = Memory allocation error.
 *  0 = Ok

-----------------

 * myAtoI() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns true if all char are digits except a leading + or -, or a
 * trailing ','.  Ignores leading or trailing white space.  Value is set to
 * atoi(s).
 *
 * ARGUMENTS
 *     s = character string to look at. (Input)
 * value = the converted value of 's', if 's' is a number. (Output)
 *
 * RETURNS: int
 *   0 = Not an integer
 *   1 = Integer

-----------------

 * myAtoI_Len() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns true if "len" char are digits except a leading + or -, or a
 * trailing ','.  Ignores leading or trailing white space.  Value is set to
 * atoi(s).
 *
 * ARGUMENTS
 *     s = character string to look at. (Input)
 *   len = number of characters to pay attention to. (Input)
 * value = the converted value of 's', if 's' is a number. (Output)
 *
 * RETURNS: int
 *   0 = Not an integer
 *   1 = Integer

-----------------

 * myAtoF() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns true if all char are digits except a leading + or -, or a
 * trailing ',' and up to one '.'.  Ignores leading or trailing white space.
 * Value is set to atof(s).
 *
 * ARGUMENTS
 *     s = character string to look at. (Input)
 * value = the converted value of 's', if 's' is a number. (Output)
 *
 * RETURNS: int
 *   0 = Not a real number,
 *   1 = Real number.

-----------------

 * myAtoF_Len() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns true if "len" char are digits except a leading + or -, or a
 * trailing ',' and up to one '.'.  Ignores leading or trailing white space.
 * Value is set to atof(s).
 *
 * ARGUMENTS
 *     s = character string to look at. (Input)
 *   len = number of characters to pay attention to. (Input)
 * value = the converted value of 's', if 's' is a number. (Output)
 *
 * RETURNS: int
 *   0 = Not a real number,
 *   1 = Real number.

-----------------

 * myRound() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Round a number to a given number of decimal places.
 *
 * ARGUMENTS
 *     x = number to round (Input)
 * place = How many decimals to round to (Input)
 *
 * RETURNS: double (rounded value)

-----------------

 * strTrim() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Trim the white space from both sides of a char string.
 *
 * ARGUMENTS
 * str = The string to trim (Input/Output)
 *
 * RETURNS: void

-----------------

 * strToLower() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Convert a string to all lowercase.
 *
 * ARGUMENTS
 * s = The string to adjust (Input/Output)
 *
 * RETURNS: void

-----------------

 * strToUpper() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   Convert a string to all uppercase.
 *
 * ARGUMENTS
 * s = The string to adjust (Input/Output)
 *
 * RETURNS: void

-----------------

 * ListSearch() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Looks through a list of strings for a given string.  Returns the index
 * where it found it.
 *    Originally "GetIndexFromStr(cur, UsrOpt, &index);"
 * now becomes "index = ListSearch(UsrOpt, sizeof(UsrOpt), cur);"
 * Advantage is that UsrOpt doesn't need a NULL last element.
 *
 * ARGUMENTS
 * List = The list to look for s in. (Input)
 *    N = The length of the List. (Input)
 *    s = The string to look for. (Input)
 *
 * RETURNS: int
 *   # = Where s is in List.
 *  -1 = Couldn't find it.

-----------------

 * fileAllocNewExten() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Replace the extension of a filename with the given extension, by copying
 * the old filename, without the extension (if there is one), to newly
 * allocated memory, and then strcat the extension on.
 *
 * ARGUMENTS
 *    name = The orignal filename to work with. (Input)
 *     ext = The file extension to replace the old one with, or add (Input)
 * newName = The newly allocated and copied to memory (Output)
 *
 * RETURNS: int
 *  0 = OK
 * -1 = Memory allocation error.

-----------------

 * myCyclicBounds() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Retun a value within the bounds of [min...max] by adding or subtracting
 * the range of max - min.
 *
 * ARGUMENTS
 * value = The orignal filename to work with. (Input)
 *   min = The minimum value of the range. (Input)
 *   max = The maximum value of the range. (Input)
 *
 * RETURNS: double
 *   The value that falls in the range of [min..max]

-----------------

 * myCntNumLines() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Counts the number of new lines in an open file, then rewinds to the
 * beginning of the file, and returns the count + 1, since the last line
 * might not have a new line.
 *
 * ARGUMENTS
 * fp = An open file pointer to look at. (Input)
 *
 * RETURNS: size_t
 * The number of new lines in the file + 1.

================= mycomplex.c ===================

 * myCset() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Sets a complex number to the given real and imaginary parts.
 *
 * ARGUMENTS
 * x = The real part. (Input)
 * y = The imaginary part. (Input)
 *
 * RETURNS: myComplex
 *    The resulting complex number

-----------------

 * myCprint() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Prints a complex number.
 *
 * ARGUMENTS
 * z = The complex number to print. (Input)
 *
 * RETURNS: void

-----------------

 * myCreal (Macro for my_Creal()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns the real part of a complex number.
 *
 * ARGUMENTS
 * z = The complex number of interest. (Input)
 *
 * RETURNS: double
 *    Real(z)

-----------------

 * myCimag (Macro for my_Cimag()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Returns the imaginary part of a complex number.
 *
 * ARGUMENTS
 * z = The complex number of interest. (Input)
 *
 * RETURNS: double
 *    Imag(z)

-----------------

 * myCadd (Macro for my_Cadd()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Adds two complex numbers.
 *
 * ARGUMENTS
 * z1 = The first complex number to add. (Input)
 * z2 = The second complex number to add. (Input)
 *
 * RETURNS: myComplex
 *    z1 + z2.

-----------------

 * myCsub (Macro for my_Csub()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Subtracts two complex numbers.
 *
 * ARGUMENTS
 * z1 = The complex number to subtract from. (Input)
 * z2 = The complex number to subtract. (Input)
 *
 * RETURNS: myComplex
 *    z1 - z2.

-----------------

 * myCmul (Macro for my_Cmul()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Multiplies two complex numbers.
 *
 * ARGUMENTS
 * z1 = The first complex number to multiply. (Input)
 * z2 = The second complex number to multiply. (Input)
 *
 * RETURNS: myComplex
 *    z1 * z2.

-----------------

 * myCmul_Real (Macro for my_Cmul_Real()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Multiplies a complex times a real number.
 *
 * ARGUMENTS
 * z = The complex number to multiply. (Input)
 * a = The real number to multiply. (Input)
 *
 * RETURNS: myComplex
 *    z * a.

-----------------

 * myCinv (Macro for my_Cinv()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    The multiplicative inverse of a complex number.
 *
 * ARGUMENTS
 * z = The complex number to take the inverse of. (Input)
 *
 * RETURNS: myComplex
 *    1 / z.

-----------------

 * myCexp (Macro for my_Cexp()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Computes e raised to a complex number.
 *
 * ARGUMENTS
 * z = The complex number to raise e to. (Input)
 *
 * RETURNS: myComplex
 *    e to the z power

-----------------

 * myClog (Macro for my_Clog()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Computes the base e logarithm of a complex number.
 *
 * ARGUMENTS
 * z = The complex number to compute the base e logarithm of. (Input)
 *
 * RETURNS: myComplex
 *    log(e) (z)

-----------------

 * myCsqrt (Macro for my_Csqrt()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Computes the square root of a complex number.
 *
 * ARGUMENTS
 * z = The complex number to compute the square root of. (Input)
 *
 * RETURNS: myComplex
 *    sqrt(z)

================= tendian.c ===================

 * memswp() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To swap memory in the Data array based on the knownledge that there are
 * "num" elements, each of size "size".
 *
 * ARGUMENTS
 * Data = A pointer to the data to be swapped. (Input/Output)
 * size = The size of an individual element. (Input)
 *  num = The number of elements to swap. (Input)
 *
 * RETURNS: void

-----------------

 * MEMCPY_BIG (sometimes macro for revmemcpy()) -- Arthur Taylor / MDL
 * MEMCPY_LIT (sometimes macro for revmemcpy()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Copies and reverses memory (similar to memcpy).  In order to have the
 * same arguments as memcpy, this can not handle arrays.  For arrays use
 * revmemcpyRay().  Returns the same thing that memcpy does.
 *    This assumes that Dst is allocated to a size of "len".  If Dst is larger
 * then "len", the data will be in the first "len" bytes.
 *
 * ARGUMENTS
 * Dst = The destination for the data. (Output)
 * Src = The source of the data. (Input)
 * len = The length of Src in bytes. (Input)
 *
 * RETURNS: void *
 *    A pointer to Dst.

-----------------

 * revmemcpyRay() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    Copies and reverses memory (similar to memcpy).  This handles the case
 * when we need to reverse memcpy an array of data.
 *    This assumes that Dst is allocated to a size of "len" * "num".  If Dst
 * is larger then "len" * "num", the data will be in the first "len" * "num"
 * bytes.
 *
 * ARGUMENTS
 * Dst = The destination for the data. (Output)
 * Src = The source of the data. (Input)
 * len = The size of a single element. (Input)
 * num = The number of elements in Src. (Input)
 *
 * RETURNS: void *
 *    A pointer to Dst.

-----------------

 * memBitRead() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   To read bits from an uChar buffer array of memory.  Assumes BufLoc is
 * valid before first call.  Typically this means do a "bufLoc = 8;" before
 * the first call.
 *
 * ARGUMENTS
 *     Dst = Where to put the results. (Output)
 *  dstLen = Length in bytes of Dst. (Input)
 *     Src = The data to read the bits from. (Input)
 * numBits = How many bits to read. (Input)
 *  BufLoc = In Src, which bit to start reading from.
 *           Starts at 8 goes to 1. (Input/Output)
 * numUsed = How many bytes from Src were used while reading (Output)
 *
 * RETURNS: int
 *    1 on error, 0 if ok.

-----------------

 * memBitWrite() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To write bits from a data structure to an array of uChar.  Assumes that
 * the part of Dst we don't write to have been correctly initialized.
 * Typically this means do a "memset (dst, 0, sizeof (dst));" before the first
 * call.  Also assumes BufLoc is valid before first call.  Typically this
 * means do a "bufLoc = 8;" before the first call.
 *
 * ARGUMENTS
 *     Src = The data to read from. (Input)
 *  srcLen = Length in bytes of Src. (Input)
 *     Dst = The char buffer to write the bits to. (Output)
 * numBits = How many bits to write. (Input)
 *  BufLoc = Which bit in Dst to start writing to.
 *           Starts at 8 goes to 1. (Input/Output)
 * numUsed = How many bytes were written to Dst. (Output)
 *
 * RETURNS: int
 *    1 on error, 0 if ok.

-----------------

 * FREAD_BIG (sometimes macro for revfread()) -- Arthur Taylor / MDL
 * FREAD_LIT (sometimes macro for revfread()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fread", but in a reverse manner.  It is assumed that file is
 * already opened and in the correct place.
 *
 * ARGUMENTS
 *  Dst = The destination for the data. (Output)
 * size = The size of a single element. (Input)
 *  num = The number of elements in Src. (Input)
 *   fp = The file to read from. (Input)
 *
 * RETURNS: size_t
 *    Number of elements read, or short count (possibly 0) on EOF or error.

-----------------

 * FWRITE_BIG (sometimes macro for revfwrite()) -- Arthur Taylor / MDL
 * FWRITE_LIT (sometimes macro for revfwrite()) -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fwrite", but in a reverse manner.  It is assumed that file is
 * already opened and in the correct place.
 *
 * ARGUMENTS
 *  Src = The source of the data. (Input)
 * size = The size of a single element. (Input)
 *  num = The number of elements in Src. (Input)
 *   fp = The file to write to. (Output)
 *
 * RETURNS: size_t
 *    Number of elements written, or short count (possibly 0) on EOF or error.

-----------------

 * FREAD_ODDINT_BIG() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fread" into a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It reads big endian data from disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * dst = Where to store the data. (Output)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to read from. (Input)
 *
 * RETURNS: size_t
 *    Number of elements read, or short count (possibly 0) on EOF or error.

-----------------

 * FREAD_ODDINT_LIT() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fread" into a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It reads little endian data from disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * dst = Where to store the data. (Output)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to read from. (Input)
 *
 * RETURNS: size_t
 *    Number of elements read, or short count (possibly 0) on EOF or error.

-----------------

 * FWRITE_ODDINT_BIG() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fwrite" from a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It writes big endian data to disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * src = Where to read the data from. (Input)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to write the data to. (Input)
 *
 * RETURNS: size_t
 *    Number of elements written, or short count (possibly 0) on EOF or error.

-----------------

 * FWRITE_ODDINT_LIT() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To do an "fwrite" from a sInt4, but in a reverse manner with not
 * necessarily all 4 bytes.  It writes little endian data to disk.  It is
 * assumed that the file is already opened and in the correct place.
 *
 * ARGUMENTS
 * src = Where to read the data from. (Input)
 * len = The number of bytes to read. (<= 4) (Input)
 *  fp = The file to write the data to. (Input)
 *
 * RETURNS: size_t
 *    Number of elements written, or short count (possibly 0) on EOF or error.

-----------------

 * fileBitRead() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    To read bits from a file.  Stores the current byte, and passes the bits
 * that were requested to the user.  Leftover bits, are stored in "gbuf",
 * which should be passed in for future reads.
 *    If numBits == 0, then flush the gbuf.
 *
 * ARGUMENTS
 *     Dst = The storage place for the data read from file. (Output)
 *  dstLen = The size of dst (in bytes) (Input)
 *      fp = The open file to read from. (Input)
 * numBits = The number of bits to read from the file. (Input)
 *    gbuf = The current bit buffer (Input/Output)
 * gbufLoc = Where we are in the current bit buffer. (Input/Output)
 *
 * RETURNS: int
 *    EOF if EOF, 1 if error, 0 if ok.

-----------------

 * fileBitWrite() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *   To write bits from src out to file.  First writes out any leftover bits
 * in pbuf, then bits from src.  Any leftover bits that aren't on a full byte
 * boundary, are stored in pbuf.
 *   If numBits == 0, then flush the pbuf.
 *
 * ARGUMENTS
 *     Src = The data to put out to file. (Input)
 *  srcLen = Length in bytes of src. (Input)
 * numBits = The number of bits to write to file. (Input)
 *      fp = The opened file ptr to write to. (Input)
 *    pbuf = The extra bit buffer (Input/Output)
 * pBufLoc = The location in the bit buffer.
 *
 * RETURNS: int
 *    1 on error, 0 if ok.

================= shpfile.c ===================

 * shpCreatePnt() -- Arthur Taylor / MDL
 *
 * PURPOSE
 *    This creates a POINT .shp/.shx file.  The .shp/.shx file contains the
 * lat/lon values of a given vector as points, and allows one to pass in a
 * vector bitmask.
 *
 * ARGUMENTS
 * Filename = Name of file to save to. (Output)
 *       dp = Vector of lat/lon pairs. (Input)
 *    numDP = number of pairs in dp. (Input)
 *   f_mask = NULL, or a vector of length numDP where 0 means missing,
 *            1 means valid (Input)
 *
 * RETURNS: int
 *  0 = OK
 * -1 = Memory allocation error.
 * -2 = Opening either .shp or .shx file
 * -3 = Problems writing entire .shp or .shx file
