About the Sample Program


[] Overview
  This release package offers a sample program for communication between 
  target and host employing the USB Communication Library.

[] About the sample program
  The sample program is a bitmap viewer application which enables bitmap 
  files to be read and displayed on the target as the sample program 
  communicates with the server application on the host. This sample program 
  is provided with a protocol driver that makes use of multi-channel 
  communication API for communication.
  The protocol driver comprises of FIO for processing the file transfer 
  protocol, and TTY for processing the standard output protocol. The driver 
  is provided as API to the target, as server DLL to the host.
  To serve as reference when writing applications for communication between 
  the target and host, the source is provided for the host side server DLL. 
  Please reference this when creating new communication protocols employing 
  multi-channel communication API. 

  The host sample of this sample program was created using VisualC++V6.0.


[] How to use the sample program 

- How to start the sample program 
  When the target side's viewer application and the host side's viewer server
  are paired up, they function as the bitmap viewer.

  The order in which the view application and viewer server is started up is 
  of no consequence but it is recommended to start up the viewer server first.

- Startup sequence of sample program

  - Target start up 
  MCC waiting for connection                - Host start up
                                            MCC connection
    --------   MCC connection completed   --------
  fio/tty waiting for server start up 
    --------  fio/tty start-up confirmed  --------
  fio/tty connection
    -------- fio/tty connection completed --------
        (following this,fio/tty communication processed)

- How to operate the host sample 
  * The ttyserver.dll,fiosetver.dll files are required for execution of the 
    host sample.
  Check that these files are available in the directory in which the executive
  file is also located.
  When the viewer server is started up in the host sample, the viewer server
  dialog appears and the viewer server waits for connection from the target 
  program. When connection with the target program is established, the 
  dialog's [ServerSTART] button becomes operative and pressing the button 
  starts up the viewer server.

* To change the connection channels of tty/fio and the server root path
  referenced by fio, be sure to choose and set the appropriate items from 
  the dialog before starting the server.

- How to operate the target sample 
  Following "make," start up the target sample in the same manner as other 
  target applications.

  Start up the target application. When connection with viewer server 
  (as host sample program) is established, information is displayed on the
   console of the viewer server via tty. When started up, the contents of 
   the root path of the fio server are displayed.

  Using the connected controller, use the [X][Y] buttons to choose the 
  directory to be moved or the bitmap file to be displayed and confirm the 
  choice with the [A] button. When a directory is selected, you move to this
  directory, and when a bitmap is specified, the file is read to the target 
  via fio and displayd on the TV screen.
  The [B] button is used to display the current directory on the console, 
  and the [MENU] button is used to select the destination of the file as 
  [DVD] or [FIO]. 

<EOF>
