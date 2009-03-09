/*****************************************************************************
 * tclstart.c
 *
 * DESCRIPTION
 *    This contains the main() procedure and is where execution starts.  Most
 * of this is generic, with the intersting stuff in tcldegrib.c
 *
 * Basically, main() calls Tk_Main() which in turn calls Tcl_AppInit().
 *
 * Tcl_AppInit() is where one does application specific initialization.
 * In this case that means to call Grib2_Init(), and then attempt to source
 * the main .tcl script (ndfd.tcl).
 * If Tcl_AppInit() can't find (ndfd.tcl), it returns to Tk_Main().
 *
 * Tk_Main() then sources any scripts that are on the command line.
 *
 * HISTORY
 *    9/2002 Arthur Taylor (MDL / RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
#if defined(_WINDOWS_)
#define MS_WINDOWS
#endif

#include <tk.h>
#ifdef MS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <locale.h>
#include <ctype.h>
#endif

#include "tcldegrib.h"

#ifdef MS_WINDOWS
#ifdef TRIPMATE_GPS
static int SerialWriteCmd (ClientData clientData, Tcl_Interp * interp,
                           int argc, char **argv);
static int SerialInitCmd (ClientData clientData, Tcl_Interp * interp,
                          int argc, char **argv);
#endif
#endif

#ifdef _BAD_TCL_SSH_EXIT
#include <stdlib.h>
static void myExit (ClientData clientData)
{
   exit (EXIT_SUCCESS);
}
#endif

/*****************************************************************************
 * Tcl_AppInit() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Main application specific initialization procedure.
 *
 * ARGUMENTS
 * interp = Tcl interpreter (holds the commands / data that Tcl knows)(Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *     TCL_OK = OK
 *  TCL_ERROR = ERROR
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int Tcl_AppInit (Tcl_Interp * interp)
{
#ifndef NO_TK
   int f_ndfd = 0;      /* Flag for where we found main .tcl script */
#endif

   /* Teach the interp about Tcl. */
   if (Tcl_Init (interp) == TCL_ERROR)
      return TCL_ERROR;

   /* Teach the interp about Tk. */
#ifndef NO_TK
   if (Tk_Init (interp) == TCL_ERROR)
      return TCL_ERROR;
#endif

   /* Teach the interp about the grib2 package. */
   if (Grib2_Init (interp) == TCL_ERROR)
      return TCL_ERROR;

   /* 
    * Link Tcl variable: Found_ndfd to f_ndfd, and then use Tcl to
    * search a primary location then a secondary location for ndfd.tcl.
    */
#ifndef NO_TK
   if (Tcl_LinkVar (interp, "Found_ndfdTcl", (char *) (&f_ndfd),
                    TCL_LINK_INT) == TCL_ERROR)
      return TCL_ERROR;
   /* order is secondary location then primary location. */
   Tcl_GlobalEval (interp, "if {[file exists ./tclsrc/ndfd.tcl]} "
                   "{set Found_ndfdTcl 2}");
   Tcl_GlobalEval (interp, "if {[file exists ndfd.tcl]} "
                   "{set Found_ndfdTcl 1}");
   Tcl_UnlinkVar (interp, "Found_ndfdTcl");
   Tcl_GlobalEval (interp, "unset Found_ndfdTcl");

   switch (f_ndfd) {
      case 1:
         Tcl_EvalFile (interp, "ndfd.tcl");
         break;
      case 2:
         Tcl_EvalFile (interp, "./tclsrc/ndfd.tcl");
         break;
      default:
         /* Search through the $env(PATH) for ndfd.tcl? */
         break;
   }
#endif

#ifdef MS_WINDOWS
#ifdef TRIPMATE_GPS
   Tcl_CreateCommand (interp, "halo_SerialInit", SerialInitCmd,
                      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
   Tcl_CreateCommand (interp, "halo_SerialWrite", SerialWriteCmd,
                      (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);
#endif
#endif

#ifdef _BAD_TCL_SSH_EXIT
   Tcl_CreateExitHandler (myExit, NULL);
   Tcl_CreateThreadExitHandler (myExit, NULL);
#endif

   return TCL_OK;
}

/*****************************************************************************
 * main() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Main start of program if in Linux or UNIX.
 *
 * ARGUMENTS
 * argc = Number of options on command line. (Input)
 * argv = Options on command line. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = No Errors.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created.
 *
 * NOTES
 *****************************************************************************
 */
int main (int argc, char *argv[])
{
#ifdef NO_TK
   Tcl_Main (argc, argv, Tcl_AppInit);
#else
   Tk_Main (argc, argv, Tcl_AppInit);
#endif
   return 0;
}

/*****************************************************************************
 * WinMain() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Main start of program if in MS-Windows.  It has to parse the lpszCmdLine
 * to create an argc, argv pair to pass to Tk_Main().
 *
 * ARGUMENTS
 *     hInstance = The unique number (or handle) used by MS-Windows to
 *                 identify this instance of this program. (Input)
 * hPrevInstance = A handle to the most recent previous instance that is
 *                 still active (0 for NULL). (Input)
 *   lpszCmdLine = A 0-terminatied string that contains any command line
 *                 parameters passed to the program (Input)
 *      nCmdShow = A Flag to SW_SHOWNORMAL or SW_SHOWMINNOACTIVE
 *                 (normal, or minimized.) (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *  0 = No Errors.
 *
 * HISTORY
 *  9/2002 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *****************************************************************************
 */
#ifdef MS_WINDOWS
int APIENTRY WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPSTR lpszCmdLine, int nCmdShow)
{
   int size;            /* Estimate of argc. */
   char *p;             /* Used to find non-white space spans. */
   char **argv;         /* The list of arguments. */
   int i;               /* which argument we are currently parsing. */
   int argc;            /* Number of valid arguments in argv ( < size ) */
   char buffer[MAX_PATH]; /* Used to help get the name of the program */
   /* 
    * Set up the default locale to be standard "C" locale so parsing
    * is performed correctly.
    */
   setlocale (LC_ALL, "C");
   /* 
    * Increase the application queue size from default value of 8.
    * At the default value, cross application SendMessage of WM_KILLFOCUS
    * will fail because the handler will not be able to do a PostMessage!
    * This is only needed for Windows 3.x, since NT dynamically expands
    * the queue.
    */
   SetMessageQueue (64);
   /* 
    * Precompute an overly pessimistic guess at the number of arguments
    * in the command line by counting non-space spans.  Note that we
    * have to allow room for the executable name and the trailing NULL
    * argument.
    */
   for (size = 3, p = lpszCmdLine; *p != '\0'; p++) {
      if (isspace (*p)) {
         size++;
         while (isspace (*p))
            p++;
         if (*p == '\0')
            break;
      }
   }
   argv = (char **) ckalloc ((unsigned) (size * sizeof (char *)));
   /* 
    * Parse the Windows command line string.  If an argument begins with a
    * double quote, then spaces are considered part of the argument until the
    * next double quote.  The argument terminates at the second quote.  Note
    * that this is different from the usual Unix semantics.
    */
   for (i = 1, p = lpszCmdLine; *p != '\0'; i++) {
      while (isspace (*p))
         p++;
      if (*p == '\0')
         break;
      if (*p == '"') {
         p++;
         argv[i] = p;
         while ((*p != '\0') && (*p != '"'))
            p++;
      } else {
         argv[i] = p;
         while (*p != '\0' && !isspace (*p))
            p++;
      }
      if (*p != '\0') {
         *p = '\0';
         p++;
      }
   }
   argv[i] = NULL;
   argc = i;
   /* 
    * Since Windows programs don't get passed the command name as the
    * first argument, we need to fetch it explicitly.
    */
   GetModuleFileName (NULL, buffer, sizeof (buffer));
   argv[0] = buffer;
   Tk_Main (argc, argv, Tcl_AppInit);
   return 0;
}
#endif

/*****************************************************************************
 * GPS Section...
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    This section came about as an experiment to see if I could initialize a
 * TripMate GPS Unit from Tcl/Tk.  The idea being to demonstrate the potential
 * use of GPS and NDFD.  It is still somewhat experimental.
 *
 * HISTORY
 *  9/2003 Arthur Taylor (MDL/RSIS): Created
 *
 * NOTES
 *****************************************************************************
 */
#ifdef MS_WINDOWS
#ifdef TRIPMATE_GPS

/* The following contains per-instance data for a Tcl file based channel. */
typedef struct FileInfo {
   Tcl_Channel channel; /* Pointer to channel structure. */
   int validMask;       /* OR'ed combination of TCL_READABLE, TCL_WRITABLE,
                         * or TCL_EXCEPTION: indicates which operations are
                         * valid on the file. */
   int watchMask;       /* OR'ed combination of TCL_READABLE, TCL_WRITABLE,
                         * or TCL_EXCEPTION: indicates which events should
                         * be_ reported. */
   int flags;           /* State flags, see above for a list. */
   HANDLE handle;       /* Input/output file. */
   struct FileInfo *nextPtr; /* Pointer to next registered file. */
} FileInfo;

/*****************************************************************************
 * SerialWriteCmd() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    Originally thought that I needed to call API's directly for the flush
 * after writing to the channel.  I don't think I need this procedure anymore
 * since it ignores the <data> argument.
 *
 * ARGUMENTS
 * clientData = NULL (used to interface with Tcl_API's) ()
 *     interp = Tcl interpreter (holds the commands / data that Tcl knows)(In)
 *       argc = Number of parameters passed in. (Input)
 *       argv = Parameters as char ptrs. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *    TCL_OK = OK
 * TCL_ERROR = ERROR
 *
 * HISTORY
 *  9/2003 Arthur Taylor (MDL/RSIS): Updated
 *
 * NOTES
 * 1) Should update this to using objc/objv if we keep it.
 *****************************************************************************
 */
static int SerialWriteCmd (ClientData clientData, Tcl_Interp * interp,
                           int argc, char **argv)
{
   Tcl_Channel chan;    /* The channel to write to. */
   FileInfo *infoPtr;   /* The status of the channel. */
   DWORD bytesWritten;  /* How many bytes were written to the channel. */

   if (argc != 3) {
      Tcl_AppendResult (interp, "usage: ", argv[0], " <File ID> <data>",
                        NULL);
      return TCL_ERROR;
   }

   chan = Tcl_GetChannel (interp, argv[1], NULL);
   if (chan == (Tcl_Channel) NULL) {
      Tcl_AppendResult (interp, "Couldn't Get the channel ", argv[1], NULL);
      return TCL_ERROR;
   }
   infoPtr = (FileInfo *) Tcl_GetChannelInstanceData (chan);
   if (WriteFile (infoPtr->handle, (LPVOID) "ASTRAL", (DWORD) 6,
                  &bytesWritten, (LPOVERLAPPED) NULL) == FALSE) {
      Tcl_AppendResult (interp, "Problems writing ASTRAL", NULL);
      return TCL_ERROR;
   }
   FlushFileBuffers (infoPtr->handle);
   return TCL_OK;
}

/*****************************************************************************
 * SerialInitCmd() --
 *
 * Arthur Taylor / MDL
 *
 * PURPOSE
 *    This came about because I needed to set the Baud rate when talking to
 * a serial connection.  Tcl/Tk can open the connection, but it doesn't set
 * the baud rate correctly for the Tripmate GPS unit.
 *
 * ARGUMENTS
 * clientData = NULL (used to interface with Tcl_API's) ()
 *     interp = Tcl interpreter (holds the commands / data that Tcl knows)(In)
 *       argc = Number of parameters passed in. (Input)
 *       argv = Parameters as char ptrs. (Input)
 *
 * FILES/DATABASES: None
 *
 * RETURNS: int
 *    TCL_OK = OK
 * TCL_ERROR = ERROR
 *
 * HISTORY
 *  9/2003 Arthur Taylor (MDL/RSIS): Updated
 *
 * NOTES
 * 1) Should update this to using objc/objv.
 * 2) Windows 2000 got stuck with "GetCommState"... so I've commented it out,
 *    but haven't had a chance to test it.
 *****************************************************************************
 */
static int SerialInitCmd (ClientData clientData, Tcl_Interp * interp,
                          int argc, char **argv)
{
   Tcl_Channel chan;    /* The channel to set the mode on. */
   FileInfo *infoPtr;   /* The status of the channel. */
   DCB dcb;

   if (argc != 2) {
      Tcl_AppendResult (interp, "usage: ", argv[0], " <File ID>", NULL);
      return TCL_ERROR;
   }

   chan = Tcl_GetChannel (interp, argv[1], NULL);
   if (chan == (Tcl_Channel) NULL) {
      Tcl_AppendResult (interp, "Couldn't Get the channel ", argv[1], NULL);
      return TCL_ERROR;
   }
   infoPtr = (FileInfo *) Tcl_GetChannelInstanceData (chan);

   memset ((char *) &dcb, 0, sizeof (dcb));
   dcb.DCBlength = sizeof (dcb);
/*
   if (! GetCommState (infoPtr->handle, &dcb)) {
      Tcl_AppendResult (interp, "can't get comm state", NULL);
      return TCL_ERROR;
   }
*/
   dcb.BaudRate = 4800;
   dcb.fBinary = 1;
   dcb.fDtrControl = 1;
   dcb.fRtsControl = 3;
   dcb.XonLim = 65535;
   dcb.XoffLim = 65535;
   dcb.ByteSize = 8;
   dcb.Parity = 0;
   dcb.StopBits = 0;
   dcb.XonChar = 17;
   dcb.XoffChar = 19;
   if (SetCommState (infoPtr->handle, &dcb) == FALSE) {
      Tcl_AppendResult (interp, "Couldn't Set the DCB", NULL);
      return TCL_ERROR;
   }
   return TCL_OK;
}
#endif
#endif
