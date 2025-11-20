Sample program

Overview
 This program includes the following functions in a single program:
  -Debug string display, using the TTY library
  -PC file access, using the FIO library
  -Bitmap viewer using multi-channel communications

Operation
 To run the sample program, use the following procedure.

  -Start the sample program on the target.
  -Start the host program.
  -On the left side of the window, click "Enable Server".
  -Click the "Root directory" button.
  -Set the root directory for FIO. (Choose any directory.)
  -From the list box, double-click the name of the file you want to transfer to
   the target.

Contents of communications data
 The channels and the blocks used by this sample program are as follows.

  Channel 1 TTY 1 block
  Channel 2 FIO 4 blocks
  Channel 3 Viewer 1 block

 The following is a description of the channel 3 viewer protocol.

 * Communications messages
   This describes the messages notified by the MCCNotify API call.
   When a message is received, the values and significance of the arguments to
   the callback function are as follows.

  1st Argument
     MCC_CHANNEL_3

  2nd Argument
     MCC_EVENT_USER

  3rd Argument
     1  .. File name notification (variable length)
           The file name is placed from the beginning of the block. The file
           name is terminated by '\0'.
     2  .. File name notification result code (4 bytes)
           After the file is processed the following result code is placed at
           the beginning of the block.
           If no error, 0; if there was an error, nonzero.

(EOF)
