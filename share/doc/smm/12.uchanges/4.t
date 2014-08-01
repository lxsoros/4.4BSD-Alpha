.\" Copyright (c) 1986 The Regents of the University of California.
.\" All rights reserved.
.\"
.\" Redistribution and use in source and binary forms, with or without
.\" modification, are permitted provided that the following conditions
.\" are met:
.\" 1. Redistributions of source code must retain the above copyright
.\"    notice, this list of conditions and the following disclaimer.
.\" 2. Redistributions in binary form must reproduce the above copyright
.\"    notice, this list of conditions and the following disclaimer in the
.\"    documentation and/or other materials provided with the distribution.
.\" 3. All advertising materials mentioning features or use of this software
.\"    must display the following acknowledgement:
.\"	This product includes software developed by the University of
.\"	California, Berkeley and its contributors.
.\" 4. Neither the name of the University nor the names of its contributors
.\"    may be used to endorse or promote products derived from this software
.\"    without specific prior written permission.
.\"
.\" THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
.\" ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
.\" IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
.\" ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
.\" FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
.\" DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
.\" OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
.\" HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
.\" LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
.\" OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
.\" SUCH DAMAGE.
.\"
.\"	@(#)4.t	6.9 (Berkeley) 10/14/90
.\"
.SH
.LG
.ce
Section 4
.SM
.sp
.PP
The system now supports the 64Kbit and 256Kbit RAM memory
controllers for the VAX-11/780 and VAX-11/785, the second UNIBUS
adapter for the VAX-11/750, and the new VAX 8600 with UNIBUS and/or
MASSBUS peripherals.
The Unibus management routines for network interfaces
have been generalized in 4.3BSD;
this change requires stylized changes within most of the network drivers.
A number of changes were made to each terminal multiplexor driver as well.
See sections 9 and 11 of the ``Changes to the Kernel in 4.3BSD'' document
for details.
.PP
New manual entries in Section 4 have been created to describe
the new communications protocols and network architectures that are supported.
The most recent addition in 4.3BSD is the Xerox Network System protocols.
.BP arp
\fIIoctl\fPs have been added to enter and delete entries in
the Internet-to-Ethernet\(dg address translation tables.
.FS
\(dg Ethernet is a trademark of Xerox Corporation.
.FE
Entries may be made permanent,
and may be ``published'' to allow a host to act as an ARP server.
.BP ddn
A new DDN Standard Mode X.25 IMP interface driver.
.BP de
A new DEC DEUNA 10 Mb/s Ethernet interface driver.
.BP dhu
A new DEC DHU-11 communications multiplexor driver.
.BP dmc
The configuration flags may be used to specify how to set up the device.
Multiple outstanding DMA requests can now be handled.
A new encapsulation is used that allows multiple protocols to be supported,
but is incompatible with that used by 4.2BSD and earlier Ultrix releases.
.BP dmz
A new DEC DMZ-32 communications multiplexor driver.
.BP ec
Has a corrected backoff algorithm.        
Multiple units are supported by placing the Unibus memory address
in the device \fIflags\fP field.
.BP ex
A new Excelan 204 10 Mb/s Ethernet interface driver.
.BP hdh
A new ACC IF-11/HDH IMP interface driver.
.BP idp
A description of the new Xerox Internet Datagram Protocol.
.BP il
The driver has additional diagnostics and
now supports Xerox NS.
.BP ip
Support for IP options was added.
.BP ix
A new Interlan NP100 10 Mb/s Ethernet interface driver.
.BP np
A new device for downloading microcode into the
Interlan NP100 10 Mb/s Ethernet interface driver.
.BP ns
A description of the new Xerox Network Systems protocol family.
.BP nsip
A description of the new software network interface
encapsulating NS packets in IP packets.
.BP ps
The driver for the Picture System 2 has a small change in interrupt handling.
.BP pty
A new mode was added to allow a small set of commands to be passed
to the pty master from the slave as a rudimentary type of \fIioctl\fP,
analogous to that of PKT mode.
Using this mode or PKT mode, a \fIselect\fP for exceptional conditions
on the master side of a pty returns \fBtrue\fP when a command operation is
available to be read.
\fISelect\fP for writing on the master side has been fixed.
.BP spp
A description of the new Xerox Sequenced Packet Protocol.
.BP tcp
An option was added to disable small-packet avoidance
under certain circumstances.
.BP tty
PASS8 mode has been added to pass all 8 bits of input.
New \fIioctl\fPs were added to support the getting and setting of window
size information for the terminal.  A signal was
added to notify processes when the window size changes.
