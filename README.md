# Parrot

A simple serial port echo test. Inspired by a UART seemingly refusing
to drain its transmit queue.

## Overview

Parrot consists of an executable that can be run in a client or server
mode. The server side generates simple packets and the client side just
echoes each packet back to the server.

## Building

Parrot is a snapcraft project and as such is typically built by running
`snapcraft` in the project's root directory.

### Building on macOS

As of the writing of this document `snapcraft clean` does not work on
macOS with `multipass`. There is a bash function to work around that
that handles the `snapcraft clean` and passes all other arguments on
to the real `snapcraft`.

    $ source setup.rc
    $ snapcraft clean snap  

### Installing

This snap must be installed in `-devmode`:

    $ sudo snap install --devmode parrot_1.3_amd64.snap

### Configuration

Parrot can use a configuration file or command line arguments.
Command line arguments take precedence over settings in the
configuration file.

#### Command Line Arguments

    $ parrot --help
    Usage: parrot [OPTION...]
    parrot -- Used to test / "burn-in" serial ports.
    
      -b, --baudrate=BAUDRATE, --bps=BAUDRATE
                                 Set baudrate (default 38400)
      -c, --config=FILE          Read configuration from FILE
      -d, --debug[=FILE]         Enable debugging messages to FILE (or stderr if
                                 FILE not specified.)
      -D, --debug-file=FILE      Enable debugging messages to FILE (or stderr if
                                 FILE == "".)
      -i, --interface=INTERFACE  Serial INTERFACE to use (REQUIRED)
      -r, --role=ROLE            Set ROLE to client or server (REQUIRED)
      -?, --help                 Give this help list
          --usage                Give a short usage message
      -V, --version              Print program version
    
    Mandatory or optional arguments to long options are also mandatory or optional
    for any corresponding short options.
    
    Report bugs to <support@riptideio.com>.

#### Configuration File

Parrot uses on optional INI style configuration file. The keys
match the command line long argument names. Given:

    $ cat example.ini
    interface: /dev/ttyS5
    debug:
    role: server
    
then:

    $ parrot -c example.ini

and:

    $ parrot --interface=/dev/ttyS5 --debug --role=server

and:

    $ parrot -i /dev/ttyS5 -d -r server

are all equivalent.

## Usage

Connect two serial ports. In one terminal run parrot as a client on one
serial port and as a server on the other serial port. Parrot will run
until it's interrupted (via ^C, for instance) or until it detects the
UART transmit queue has stopped draining.

Without debugging enabled, there is not any output unless kernel/UART
stops sending data.

### Client Example

In this example the client reads start timing out because the
server side stops transmitting.

    $ sudo parrot -d -r client -i /dev/ttyS5
    client_loop
    Read valid 13 byte packet.
    send_packet: sending 13 byte packet.
    Sent 13 byte packet.
    Read valid 42 byte packet.
    send_packet: sending 42 byte packet.
    Sent 42 byte packet.
    Read valid 68 byte packet.
    send_packet: sending 68 byte packet.
    Sent 68 byte packet.
    Read valid 124 byte packet.
    send_packet: sending 124 byte packet.
    Sent 124 byte packet.
    read timed out
    read timed out
    read timed out
    read timed out
    read timed out
    read timed out
    read timed out
    read timed out
    read timed out
    ^C

### Server Example

In this example the server side UART stops draining the transmit
queue. Upon detecting this, it prints an error and exits.

     $ sudo parrot -d -r server -i /dev/ttyS2
     server_loop
     send_packet: sending 13 byte packet.
     Sent 13 byte packet.
     Read valid 13 byte packet.
     valid echo.
     send_packet: sending 42 byte packet.
     Sent 42 byte packet.
     Read valid 42 byte packet.
     valid echo.
     send_packet: sending 68 byte packet.
     Sent 68 byte packet.
     Read valid 68 byte packet.
     valid echo.
     send_packet: sending 124 byte packet.
     Sent 124 byte packet.
     Read valid 124 byte packet.
     valid echo.
     send_packet: sending 66 byte packet.
     ERROR: tcdrain failed, Interrupted system call.
     Closing down /dev/ttyS2 serial device...done.

## Acknowledgements

1. This project uses the [inih (INI Not Invented Here)](https://github.com/benhoyt/inih)
open source project for configuration file parsing.
2. The crc implmentation was derived from
[Mark Adler](https://stackoverflow.com/users/1180620/mark-adler)'s
[stackoverflow answer](https://stackoverflow.com/a/15171925/228670) to
[definitive CRC for C](https://stackoverflow.com/q/15169387/228670). 

## License

This project is licensed under the Modified BSD License - see
[LICENSE.md](LICENSE.md) file for details.
