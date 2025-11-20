About the hio sample program "color"


[]Overview
  The program "color" is a sample program showing the creation of a program
  for communications between host and target using hio, being a system for 
  controlling the colors (RGB values) displayed on a TV screen from the host.


[]System configuration

  For host-target communications the HIO library is used, and transfers are 
  made through the dedicated USB interface (abbreviated "USB I/F").

  - Hardware configuration
      [TV]-[game development machine]---[USB I/F]---[PC]

  - Software configuration
      [target application]
      [hio library]
            ||
      [hio library]
      [host application]


[]Method of using application

 <Starting>
  Start the host/target color applications.
      - target: color.elf
      - host:   color.exe
                * Execution requires the directory containing hio.dll to be set
                  in the environment variable PATH.

 <Host application operation>
  The host application provides the following items:
      - Color control sliders (Red, Green, and Blue)
      - Message reception check box

  - Color control sliders:
      Moving these sliders varies the value for the corresponding color, 
      allowing the color balance on the TV screen to be changed.

  - Message reception check box:
      When this is checked, the target sends messages to the host. The host 
      receives the message sent from the target, and displays the received 
      message alongside the check box.
      The message is in the format "R:xxx G:xxx B:xxx" showing a color data 
      combination.
      When there is no check mark, the target does not notify the host, and
      moreover the host does not receive messages.
      This box is unchecked by default.

[]Communications protocol

  The information passed in host-target communications is as follows.
      - Color information (RGB)
          Routing:      USB I/F memory (32[bytes] from 0x1000)
          Direction:    host -> target
          Description:  color information (RGB values) for display on the 
                        target.
                        The R, G, and B values are each in the range 0-255.
                            ADDRESS: 0x1000  +0 +1 +2
                            FORMAT:          [R][G][B]

                        As the operating panel sliders are operated, the host 
                        writes changes in the settings to the USB I/F memory.
                        The target constantly (1V) monitors the color 
                        information values in USB I/F memory, and reflects 
                        these in the TV screen display.
                        * Example of using USB I/F memory.

      - Messages
          Routing:      USB I/F memory (32[bytes] from 0x1020)
          Direction:    host <- target

          Description:  Message information for display on the host. The 
                        messages are character strings in the following format.
                            ADDRESS: 0x1020  +0
                            FORMAT:          [R:xxx G:xxx B:xxx[\0]]

                        The target writes messages only when switched to the 
                        "Message Sending Mode" by the mode switching described
                        below.
                        When the host has set "Message Sending Mode" only, the
                        message written by a timer event is read in, and 
                        displayed on the operating panel.
                        * Example of using USB I/F memory.

      - Mode switching
          Routing:      USB I/F mailbox
          Direction:    host -> target
          Description:  Switches the Message Sending Mode.
                        When Message Sending Mode is set, messages are written
                        by the target to USB I/F memory, then the written 
                        messages are read in by the host, and displayed.
                        * Example of using mailbox


[]Processing sequence

 <Target processing>
      [Initialization]
         | *Repeat
      [Read RGB values]: (message mode), write message
      [TV screen color display]
      [*Repeat]

      [EXI interrupt]
         |
      [Read mailbox]
      [Change Message Sending Mode]

 <Host processing>
      [Slider operation]
         |
      [Write RGB values]

      [Check box operation]
         |
      ON: [Notify Message Sending Mode setting], [Start monitoring timer]
      OFF: [Notify Message Sending Mode setting], [Stop monitoring timer]

      [Monitoring timer]
         |
      [Read message, display]


[]Notes on mailbox use

  In the hio library, if the agent being communicated with writes to the 
  mailbox, the callback function registered at library initialization is 
  called. However, because of limitations of the USB I/F, until the received 
  content of the mailbox is read, even if the other end writes in the mailbox
  again, the callback function is not called again. Therefore, when the 
  callback function for mailbox writing is called, the mailbox must be read,
  in order that the next callback function can be called correctly.

  In this sample program, when initializing the target application which uses
  the callback function for mailbox writing, a dummy read of the mailbox is 
  carried out.
  * In the sample program, the dummy read is carried out unconditionally, but
    it would also be possible to get the mailbox status, and read the mailbox
    if the status indicates that it has been written to by the other end.


