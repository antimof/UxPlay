# UxPlay 1.48:  AirPlay/AirPlay-Mirror server for Linux, macOS, and Unix.

### Now developed at GitHub site [https://github.com/FDH2/UxPlay](https://github.com/FDH2/UxPlay) (where user issues should be posted).


Highlights:

   * GPLv3, open source.
   * Support for both AirPlay Mirror and AirPlay Audio-only (Apple Lossless ALAC) protocols f
   from current iOS/iPadOS 15.2 client devices.
   * macOS computers (2011 or later) can act either as AirPlay clients, or as the server running UxPlay (tested
     on macOS 10.15 Catalina).  Using AirPlay, UxPlay can emulate a second display for macOS clients.
   * Support for older 32-bit iOS clients (such as iPad 2nd gen, iPhone 4S, when upgraded to iOS 9.3.5 or later),
   and a Windows AirPlay-client emulator, AirMyPC.
   * Uses GStreamer, with options to select different output "videosinks" and  "audiosinks".
   * Support for server behind a firewall.
   * **New**: Support for Raspberry Pi, with hardware video acceleration by Video4Linux2 (replacement for OpenMAX)

This project is a GPLv3 open source unix AirPlay2 Mirror server for Linux, macOS, and \*BSD.
It was initially developed by
[antimof](http://github.com/antimof/Uxplay) using code 
from [RPiPlay](https://github.com/FD-/RPiPlay), which in turn derives from
[AirplayServer](https://github.com/KqsMea8/AirplayServer),
[shairplay](https://github.com/juhovh/shairplay), and [playfair](https://github.com/EstebanKubata/playfair).   (The antimof site is
mainly inactive, but periodically posts updates pulled from the [main UxPlay site](https://github.com/FDH2/UxPlay)). 

UxPlay is tested on a number of systems, including (among others) Debian 10.11 "Buster" and  11.2 "Bullseye", Ubuntu 20.04 and 21.10,
Linux Mint 20.2, Pop!\_OS 21.10 (NVIDIA edition),
Rocky Linux 8.5 (a CentOS successor), OpenSUSE 15.3, Arch Linux 5.16.8, macOS 10.15.7, FreeBSD 13.0.

Its main use is to act like an AppleTV for screen-mirroring (with audio) of iOS/iPadOS/macOS clients
(iPhones, iPads, MacBooks) in a window
on the server display (with the possibility of
sharing that window on screen-sharing applications such as Zoom)
on a host running Linux, macOS, or other unix.  UxPlay supports a "legacy" form of Apple's AirPlay Mirror protocol introduced
in iOS 12; client devices running iOS/iPadOS 9.3.5 or later are supported, as is a (non-free) Windows-based
AirPlay-client software emulator, AirMyPC.
(Details of what is publically known about Apple's AirPlay2 protocol can be found
[here](https://github.com/SteeBono/airplayreceiver/wiki/AirPlay2-Protocol) and
[here](https://emanuelecozzi.net/docs/airplay2)).

The UxPlay server and its client must be on the same local area network,
on which a **Bonjour/Zeroconf mDNS/DNS-SD server**  is also running
(only DNS-SD "Service Discovery" service is strictly necessary, it is not necessary that the local network also be of the ".local" mDNS-based type). 
On Linux and BSD Unix servers, this is usually provided by [Avahi](https://www.avahi.org),
through the avahi-daemon service, and is included in  most Linux distributions (this
service can also be provided by macOS, iOS or Windows servers).

Connections to the UxPlay server by
iOS/MacOS  clients can be initiated both in AirPlay Mirror mode (which streams
lossily-compressed AAC audio while mirroring the client screen,
or in the alternative AirPlay Audio mode which streams
Apple Lossless (ALAC) audio without screen mirroring (the accompanying metadata and cover art in
this mode is not displayed).  _Switching between these two modes during an active  connection is
possible: in Mirror mode, close the mirror window and start an Audio mode connection,
switch back by initiating a Mirror mode connection._  **Note that Apple DRM
(as in Apple TV app content on the client) cannot be decrypted by UxPlay,
and (unlike with a true AppleTV), the client cannot run a http connection on the server
instead of streaming content from one on the client.**

### Possibility for using hardware-accelerated h264 video-decoding, if available.

UxPlay uses [GStreamer](https://gstreamer.freedesktop.org) Plugins for rendering audio and video,
This means that video and audio are supported "out of the box", using a choice of plugins.
AirPlay streams video in h264 format: gstreamer decoding is plugin agnostic, and uses accelerated GPU hardware
h264 decoders if available; if not, software decoding is used. 

For systems with Intel integrated graphics, hardware GPU decoding with the gstreamer VAAPI plugin is preferable. VAAPI is
open-source, and in addition to Intel, can support some AMD GPU's (the open-source "Nouveau" drivers for NVIDIA
graphics are also in principle supported when VAAPI is supplemented with firmware extracted from the proprietary NVIDIA drivers).

For NVIDIA graphics with the proprietary drivers, the nvdec plugin (recently renamed nvh264dec) can be used for
accelerated video decoding on the NVIDIA GPU with CUDA. The nvdec  plugin is part of gstreamer1.0-plugins-bad, but is generally not
included in binary packages, as NVIDIA's
proprietary [Video Codec SDK](https://docs.nvidia.com/video-technologies/video-codec-sdk/nvdec-video-decoder-api-prog-guide/)
must be downloaded, and three header files from it must be added to the gstreamer source before the plugin can be compiled.
Users must do this themselves: see [these instructions](https://gist.github.com/corenel/a615b6f7eb5b5425aa49343a7b409200), and
adapt them as necessary for your GStreamer installation.
This plugin should be used with the `-vd nvdec` (or nvh264dec) and `-vs glimagesink`  uxplay options.

*  **GPU Support for Raspberry Pi**

    Raspberry Pi (RPi) computers can run UxPlay with software decoding of h264 video (options `uxplay -rpi -avdec`) but this usually
    has unacceptable latency, and hardware-accelerated decoding by  the Pi's built-in Broadcom GPU should be used.
    RPi OS (Bulseye) has abandoned the omx (OpenMAX) driver used till now for this by [RPiPlay](http://github.com/FD-/RPiPlay), in
    favor of v4l2 (Video4Linux2).  The GStreamer Video4Linux2 plugin only works with UxPlay since GStreamer-1.21.0.0 on the development branch,
    but a backport to 1.18.4 for RPi OS (Bullseye) has been created, and will appear in some future update.   If you cannot wait for the update,
    you can find [patching instructions](https://github.com/FDH2/UxPlay/wiki/Gstreamer-Video4Linux2-plugin-patches) in
    the [UxPlay Wiki](https://github.com/FDH2/UxPlay/wiki).  Use the options `uxplay -rpi` (
    or `uxplay -rpi -vs kmssink` on RPi OS Lite  with no X11) with the patched GStreamer.  Patches for
    GStreamer-1.18.5 (used in Ubuntu 21.10 for RPi) and GStreamer-1.20.0 (used in Manjaro for RPi) are also available there.


### Note to packagers: OpenSSL-3.0.0 solves GPL v3 license issues.
Some Linux distributions such as Debian do not allow distribution of compiled
GPL code linked to OpenSSL-1.1.1 because its "dual OpenSSL/SSLeay" license
has some incompatibilities with GPL, unless all code authors have explicitly given an "exception" to allow
such linking (the historical origins of UxPlay make this impossible to obtain).    Other distributions
treat OpenSSL as a "System Library" which the GPL allows linking to.

For "GPL-strict" distributions, UxPlay can  be built using OpenSSL- 3.0.0, which has a
new [GPLv3-compatible license](https://www.openssl.org/blog/blog/2021/09/07/OpenSSL3.Final/).

# Getting UxPlay:

Either download and unzip [UxPlay-master.zip](https://github.com/FDH2/UxPlay/archive/refs/heads/master.zip), 
or (if git is installed): "git clone https://github.com/FDH2/UxPlay".   You can also download a recent or earlier
version listed in [Releases](https://github.com/FDH2/UxPlay/releases).

*Current UxPlay  is also a pull request on the
original site https://github.com/antimof/UxPlay ; that original 
project is  inactive, but the pull requests are
now being periodically merged with the antimof tree (thank you antimof!).

## Building UxPlay on  Linux (or \*BSD):

(Instructions for Debian/Ubuntu; adapt these for other Linuxes; for macOS, see below).
See [Troubleshooting](#troubleshooting) below for help with any difficulties.

Make sure that your distribution provides OpenSSL 1.1.1 or later, and libplist 2.0 or later.
(This means Debian 10 "Buster", Ubuntu 18.04 or later.) If it does not, you may
need to build and install these from source (see below).

You need a C/C++ compiler (e.g. g++) with the standard development libraries installed.  Debian-based
systems provide a package "build-essential" for use in compiling software.  Make sure that cmake>=3.4.1
and pkg-config are also installed: "sudo apt-get install cmake pkg-config".
In a terminal window, change directories to the source directory of the
downloaded source code ("UxPlay-\*", "\*" = "master" or the release tag for zipfile downloads, "UxPlay" for "git clone" downloads), then do

**Note:** By default UxPlay will be built with optimization for the computer it is built on; when this is
not the case, as when you are packaging for a distribution, use the cmake option `-DNO_MARCH_NATIVE=ON`.

1. `sudo apt-get install libssl-dev libplist-dev`  (unless you need to build OpenSSL and libplist from source).
2. `sudo apt-get install libavahi-compat-libdnssd-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev`. 
3. `sudo apt-get install libx11-dev`  (only needed if you invoke the "ZOOMFIX" X11 display-name fix in the next step)
4. `cmake .` (or "`cmake -DZOOMFIX=ON .`" to get a screen-sharing fix to
make X11 mirror display windows visible to screen-sharing applications such as
Zoom, see [Improvements](#improvements) \#3 below).
5. `make`
6. `sudo make install`    (you can afterwards uninstall with `sudo make uninstall` in the same directory in which this was run)
7.  Install GStreamer plugins that you need: `sudo apt-get install gstreamer1.0-<plugin>`; values of
    `<plugin>` needed are: "plugins-base", "libav" (for sound), "plugins-good" (for v4l2 hardware h264 decoding)
    and  "plugins-bad" (for software h264 decoding).   Also needed may be "gl" for OpenGL support, and "x" for X11 support, although
    these may  already be installed; "vaapi" is needed  for hardware-accelerated h264 video decoding by Intel graphics (not for NVIDIA).
    Also install "tools" to get the utility gst-inspect-1.0 for examining the GStreamer installation.


_If you intend to modify the code, use a separate "build" directory: replace_  "```cmake  [ ] . ```" _by_  "```mkdir build ; cd build ; cmake [ ] ..```"; _you can then clean
the build directory with_ "```rm -rf build/* ```" _(run from within the UxPlay source directory) without affecting the source directories which contain your modifications_.

The above script installs the executable file "`uxplay`" to `/usr/local/bin`, (and installs a manpage to somewhere like `/usr/local/share/man/man1` and README
files to somewhere like `/usr/local/share/doc/uxplay`).
It can also be found in the build directory after the build
processs.

**Finally, run uxplay in a terminal window**.   If it is not seen by the iOS client's drop-down "Screen Mirroring" panel,
check that your DNS-SD server (usually avahi-daemon) is running: do this in a terminal window with ```systemctl status avahi-daemon```.
If this shows the avahi-daemon is not running, control it with ```sudo systemctl [start,stop,enable,disable] avahi-daemon``` (or avahi-daemon.service). 
If UxPlay is seen,  but the client fails to connect when it is selected, there may be a firewall on the server that  prevents
UxPlay from receiving client connection requests unless some network ports are opened. See [Troubleshooting](#troubleshooting) below for
help with this or other problems.   See [Usage](#usage) for run-time options.   For OpenGL support (option -vs glimagesink), needed for
Raspberry Pi and NVIDIA  GPU-based video decoding, make sure gstreamer1.0-gl is installed.

 * **Red Hat, Fedora, CentOS (now continued as Rocky Linux or Alma Linux):** 
(sudo yum install) openssl-devel libplist-devel avahi-compat-libdns_sd-devel (some from the "PowerTools" add-on repository)
(+libX11-devel for ZOOMFIX). The required GStreamer packages (some from [rpmfusion.org](https://rpmfusion.org)) are:
gstreamer1-devel gstreamer1-plugins-base-devel gstreamer1-libav gstreamer1-plugins-bad-free (+ gstreamer1-vaapi for intel graphics). 

 * **OpenSUSE:**
(sudo zypper install) libopenssl-devel libplist-devel
avahi-compat-mDNSResponder-devel (+ libX11-devel for ZOOMFIX).  The required
GStreamer packages  (you may need to use versions from [Packman](https://ftp.gwdg.de/pub/linux/misc/packman/suse/)) are:
gstreamer-devel gstreamer-plugins-base-devel gstreamer-plugins-libav gstreamer-plugins-bad (+ gstreamer-plugins-vaapi for Intel graphics).

* **Arch Linux**
(sudo pacman -Syu) openssl libplist avahi  gst-plugins-base gst-plugins-good gst-plugins-bad gst-libav (+ gstreamer-vaapi for Intel graphics). (**Also available as a package in AUR**).

 * **FreeBSD:** (sudo pkg install) libplist gstreamer1, gstreamer1-libav, gstreamer1-plugins, gstreamer1-plugins-*
(\* = core, good,  bad, x, gtk, gl, vulkan, pulse ...), (+ gstreamer1-vaapi for Intel graphics).
Either avahi-libdns or mDNSResponder must also be installed to provide the dns_sd library.
OpenSSL is already installed as a System Library.   "ZOOMFIX" is untested; don't try to use it on FreeBSD unless you need it.

### Building OpenSSL >= 1.1.1 from source.

If you need to do this, note that you may be able to use a newer version (OpenSSL-3.0.1 is known to work).
You will need the standard development toolset (autoconf, automake, libtool, etc.).
Download the source code from
[https://www.openssl.org/source/](https://www.openssl.org/source/).
Install the downloaded
openssl by opening a terminal in your Downloads directory, and unpacking the source distribution:
("tar -xvzf openssl-3.0.1.tar.gz ; cd openssl-3.0.1"). Then build/install with
"./config ; make ; sudo make install_dev".  This will typically install the needed library ```libcrypto.*```,
either in /usr/local/lib or /usr/local/lib64.
_(Ignore the following for builds on MacOS:)_
Assuming the library was placed in /usr/local/lib64, you must
"export OPENSSL_ROOT_DIR=/usr/local/lib64" before running cmake.
On some systems like
Debian or Ubuntu, you may also need to add a missing  entry ```/usr/local/lib64```
in /etc/ld.so.conf (or place a file containing "/usr/local/lib64/libcrypto.so" in /etc/ld.so.conf.d)
and then run "sudo ldconfig". 

### Bulding libplist >= 2.0.0 from source.

_(Note: on Debian 9 "Stretch" or Ubuntu 16.04 LTS editions, you can avoid this step by installing libplist-dev
and libplist3 from Debian 10 or Ubuntu 18.04.)_
As well as the usual build tools, you  may need to also install some libpython\*-dev package.  Download the latest source
from [https://github.com/libimobiledevice/libplist](https://github.com/libimobiledevice/libplist): get
[libplist-master.zip](https://github.com/libimobiledevice/libplist/archive/refs/heads/master.zip), then
("unzip libplist-master.zip ; cd libplist-master"), build/install
("./autogen.sh ; make ; sudo make install").   This will probably install libplist-2.0.* in /usr/local/lib.
_(Ignore the following for builds on MacOS:)_  On some systems like
Debian or Ubuntu, you may also need to add a missing  entry ```/usr/local/lib```
in /etc/ld.so.conf (or place a file containing "/usr/local/lib/libplist-2.0.so" in /etc/ld.so.conf.d)
and then run "sudo ldconfig". 

## Building UxPlay on macOS:  **(Only tested on Intel X86_64 Macs)**

_Note: A native AirPlay Server feature is included in  macOS 12 Monterey, but is restricted to recent hardware.
UxPlay can run  on older macOS systems that will not be able to run Monterey, or can run Monterey  but not AirPlay._

These instructions for macOS asssume that the Xcode command-line developer tools are installed (if Xcode is installed, open the Terminal, type "sudo xcode-select --install" and accept the conditions).

It is also assumed that CMake >= 3.13 is installed:
this can be done with package managers [MacPorts](http://www.macports.org),
[Fink](http://finkproject.org) or [Homebrew](http://brew.sh), or by a download from
[https://cmake.org/download/](https://cmake.org/download/).


First get the latest macOS release of GStreamer-1.0
from [https://gstreamer.freedesktop.org/download/](https://gstreamer.freedesktop.org/download/).
Install both the macOS runtime and development installer packages. Assuming that the latest release is 1.18.6
they are ```gstreamer-1.0-1.18.6-x86_64.pkg``` and ```gstreamer-1.0-devel-1.18.6-x86_64.pkg```.   (**Note: v1.20.0 is also
available now, but if it does not work for you, use 1.18.6**.) Click on them to install (they install to
/Library/FrameWorks/GStreamer.framework).  It is recommended you use GStreamer.framework rather than install
Gstreamer with Homebrew or MacPorts (see later).

Next install OpenSSL and libplist:  these can be built from source (see above); only the
static forms of the two libraries will used for the macOS build, so you can uninstall them ("sudo make uninstall")
after you have built UxPlay. It may be  easier to get them using
MacPorts "sudo port install openssl libplist-devel" or Homebrew "brew install openssl libplist" (but not Fink).
if you don't have MacPorts or Homebrew installed, you can just install
one of them before building  uxplay, and uninstall afterwards if it is not wanted.

Finally, build and install uxplay (without ZOOMFIX): open a terminal and change into the UxPlay source directory
("UxPlay-master" for zipfile downloads, "UxPlay" for "git clone" downloads) and build/install with
"cmake . ; make ; sudo make install " (same as for Linux).  

   * On macOS with this installation of GStreamer, the only videosinks available seem to be glimagesink (default choice made by
     autovideosink) and osxvideosink.    (It seems that vaapisink is not supported
     on macOS).  The window title does not show the Airplay server name, but the window is visible to
     screen-sharing apps (e.g., Zoom). The only available audiosink seems to be osxaudiosink.

   * On macOS, the option -t _timeout_ is currently suppressed, and the option -nc is always used, whether or not it is selected.
     This is a workaround until a problem with GStreamer videosinks on macOS is fixed:
     if the GStreamer pipeline is destroyed while the mirror window is still open,  a segfault occurs.
   
   * In the case of glimagesink, the resolution settings "-s wxh" do not affect
     the (small) initial OpenGL mirror window size, but the window can be expanded using the mouse or trackpad.
     In contrast, a window created with "-vs osxvideosink" is initially big, but has the wrong aspect ratio (stretched image);
     in this case the aspect ratio changes when the window width is changed by dragging its side.


***Other ways (Homebrew, MacPorts) to install GStreamer on macOS (not recommended):***

First make sure that pkgconfig is installed  (Homebrew: "brew install pkgconfig" ; MacPorts: "sudo port install pkgconfig" ).  

(a) with Homebrew: "brew install gst-plugins-base gst-plugins-good gst-plugins-bad gst-libav".   This appears to be functionally equivalent
to using GStreamer.framework, but causes a large number of extra packages to be installed by Homebrew as dependencies.  (However, as of November 2021,
Homebrew offers a build of GStreamer for Apple Silicon, which then was not yet available on the offical GStreamer site.)

(b) with MacPorts: "sudo port install gstreamer1-gst-plugins-base gstreamer1-gst-plugins-good gstreamer1-gst-plugins-bad gstreamer1-gst-libav".
**The MacPorts GStreamer is built to use X11**, so uxplay must be run from an XQuartz terminal, can use ZOOMFIX, and needs
option "-vs ximagesink".  On an older unibody MacBook Pro, the default resolution  wxh = 1920x1080 was too large  for
the non-retina display, but using option "-s 800x600" worked; However, the GStreamer pipeline is fragile against attempts to change
the X11 window size, or to rotations that switch a connected client between portrait and landscape mode while uxplay is running. 
Using the MacPorts X11 GStreamer is only viable if the image size is left unchanged from the initial "-s wxh" setting 
(also use the iPad/iPhone setting that locks the screen orientation against switching  between portrait and landscape mode
as the device is rotated).

# Usage

Options:

**-n server_name** (Default: UxPlay);  server_name@_hostname_ will be the name that appears offering
   AirPlay services to your iPad, iPhone etc, where _hostname_ is the name of the server running uxplay. 
   This will also now be the name shown above the mirror display (X11)  window.

**-nh** Do not append "@_hostname_" at the end of the AirPlay server name.

**-s wxh** (e.g. -s 1920x1080 , which is the default ) sets the display resolution (width and height,
   in pixels).   (This may be a
   request made to the AirPlay client, and perhaps will not
   be the final resolution you get.)   w and h are whole numbers with four
   digits or less.   Note that the **height** pixel size is the controlling
   one used by the client for determining the streaming format; the width is
   dynamically adjusted to the shape of the image (portrait or landscape
   format, depending on how an iPad is held, for example). 

**-s wxh@r**  As above, but also informs the AirPlay  client about the screen
   refresh rate of the display. Default is r=60 (60 Hz); r must be a whole number
   less than 256.
   
**-fps n** sets a maximum frame rate (in frames per second) for the AirPlay
   client to stream video; n must be a whole number less than 256.
   (The client may choose to serve video at any frame rate lower
   than this;  default is 30 fps.)  A setting below 30 fps might be useful to
   reduce latency if you are running more than one instance of uxplay at the same time.
   _This setting is only an advisory to
   the client device, so setting a high value will not force a high framerate._
   (You can test using "-vs fpsdisplaysink" to see what framerate is being
   received, or use the option -FPSdata which displays video-stream performance data
   continuously sent by the client during video-streaming.)

**-FPSdata** Turns on monitoring of regular reports about video streaming performance
   that are sent by the client.  These will be displayed in the terminal window if this
   option is used.   The data is updated by the client at 1 second intervals.

**-o** turns on an "overscanned" option for the display window.    This
   reduces the image resolution by using some of the pixels requested
   by  option -s wxh (or their default values 1920x1080) by adding an empty
   boundary frame of unused pixels (which would be lost in a full-screen
   display that overscans, and is not displayed by gstreamer).
   Recommendation: **don't use this option** unless there is some special
   reason to use it.

**-p**  allows you to select the network ports used by UxPlay (these need
   to be opened if the server is behind a firewall).   By itself, -p sets
   "legacy" ports TCP 7100, 7000, 7001, UDP 6000, 6001, 7011.   -p n (e.g. -p
   35000)  sets TCP and UDP ports n, n+1, n+2.  -p n1,n2,n3 (comma-separated
   values) sets each port separately; -p n1,n2 sets ports n1,n2,n2+1.  -p tcp n
   or -p udp n sets just the TCP or UDP ports.  Ports must be in the range
   [1024-65535].

If the -p option is not used, the ports are chosen dynamically (randomly),
which will not work if a firewall is running.

**-m**  generates a random MAC address to use instead of the true hardware MAC
   number of the computer's network card.   (Different server_name,  MAC
   addresses,  and network ports are needed for each running uxplay  if you
   attempt to  run two instances of uxplay on the same computer.)
   If UxPlay fails to find the true MAC address of a  network card, (more
   specifically, the MAC address used by the first active network interface detected)
   a random MAC address will be used even if option **-m** was not specifed.
   (Note that a random MAC address will be different each time UxPlay is started).


Also: image transforms that had been added to RPiPlay have been ported to UxPlay:

**-f {H|V|I}**  implements "videoflip" image transforms: H = horizontal flip
(right-left flip, or mirror image); V = vertical flip ;  I =
180 degree rotation or inversion (which is the combination of H with V).

**-r {R|L}**  90 degree Right (clockwise) or Left (counter-clockwise)
   rotations; these are carried out after any **-f** transforms.

**-vp _parser_** choses the GStreamer pipeline's h264 parser element, default is h264parse. Using
   quotes "..." allows options to be added.
   
**-vd _decoder_** chooses the GStreamer pipeline's h264 decoder element, instead of letting
   decodebin pick it for you.  Software decoding is done by avdec_h264; various hardware decoders
   include: vaapi264dec, nvdec, nvh264dec, v4l2h264dec (these require that the appropriate hardware is
   available).  Using quotes "..." allows some parameters to be included with the decoder name.

**-vc _converter_** chooses the GStreamer pipeline's videoconverter element, instead of the default
   value "videoconvert".  When using video4linux hardware decoding by a GPU,`-vc  v4l2convert` will also use
   the GPU for video conversion.  Using quotes "..." allows some parameters to be included with the converter name.
   
**-vs _videosink_** chooses the GStreamer videosink, instead of letting
   autovideosink pick it for you.  Some videosink choices are:  ximagesink, xvimagesink,
   vaapisink (for intel graphics), gtksink, glimagesink, waylandsink, osximagesink (for macOS),  kmssink (for systems without X11, like Raspberry Pi OS lite) or
   fpsdisplaysink (which shows the streaming framerate in fps).   Using quotes
   "..." allows some parameters to be included with the videosink name. 
   For example, **fullscreen** mode is supported by the vaapisink plugin, and is
   obtained using ``-vs "vaapisink fullscreen=true"``; this also works with ``waylandsink``.
   The syntax of such options is specific to a given plugin, and some choices of videosink
   might not work on your system.

**-vs 0** suppresses display of streamed video, but plays  streamed audio.   (The client's screen
   is still mirrored at a reduced rate of 1 frame per second,  but is not rendered or displayed.)  This
   feature (which streams audio in AAC audio format) is now probably unneeded, as UxPlay can now 
   stream superior-quality Apple Lossless audio without video in Airplay non-mirror mode.

**-rpi** Video settings for Raspberry Pi, for hardware h264 video decoding in the GPU (requires the
   video4linux2 plugin from GStreamer-1.21.0 or later, or a backported patched version of an earlier release.
   (If this is unavailable, use `uxplay -rpi -avdec`).  Uses the glimagesink videosink.

**-avdec** forces use of software h264 decoding using Gstreamer element avdec_h264 (libav h264 decoder). This
   option should prevent autovideosink choosing a hardware-accelerated videosink plugin such as vaapisink.
   
**-as _audiosink_** chooses the GStreamer audiosink, instead of letting
   autoaudiosink pick it for you.  Some audiosink choices are:  pulsesink, alsasink, 
   osssink, oss4sink, and osxaudiosink (for macOS).  Using quotes
   "..." might allow some parameters to be included with the audiosink name. 
   (Some choices of audiosink might not work on your system.)   

**-as 0**  (or just **-a**) suppresses playing of streamed audio, but displays streamed video.

**-reset n** sets a limit of n consective timeout failures of the client to respond to ntp requests
   from the server (these are sent every 3 seconds to check if the client is still present).   After
   n failures, the client will be presumed to be offline, and the connection will be reset to allow a new
   connection.   The default value of n is 10; the value n = 0 means "no limit" on timeouts.

**-nc** maintains previous UxPlay < 1.45 behavior that does **not close** the video window when the the client
   sends the "Stop Mirroring" signal. _This option is currently used by default in macOS,
   as the  window created in macOS by GStreamer does not terminate correctly (it causes a segfault)
   if it is still open when the GStreamer pipeline is closed._

**-t _timeout_**  will cause the server to relaunch (without stopping uxplay) if no connections
   have been present during the previous _timeout_ seconds.  You may wish to use this if the Server
   is not visible to new Clients that were inactive when the Server was launched, and an idle Bonjour
   registration  eventually becomes unavailable for new connections (this is a workaround for what
   may be due to a problem with your DNS-SD or Avahi setup).   _This option is currently disabled in
   macOS, for the same reason that requires the -nc option._

# Troubleshooting

Note: ```uxplay```  is run from a terminal command line, and informational messages are written to the terminal.

### 0.  Problems in compiling UxPlay.
    
One user (on Ubuntu) found compilation failed with messages about linking to "usr/local/lib/libcrypto.a"  and "zlib".
This was because (in addition to the standard ubuntu installation of libssl-dev), the user was unaware that a second installation
with libcrypto in /usr/local was present.
Solution: when more than one installation of OpenSSL is present, set the environment variable OPEN_SSL_ROOT_DIR to point to the correct one;
on 64-bit Ubuntu, this is done by
running `export OPENSSL_ROOT_DIR=/usr/lib/X86_64-linux-gnu/` before running cmake.

### 1. uxplay starts, but stalls after "Initialized server socket(s)" appears, *without any server name showing on the client*.

Stalling this way, with _no_  server name showing  _on  the client_ as available,
probably means that your network **does not have a running Bonjour/zeroconf DNS-SD server.**
On Linux, make sure Avahi is installed,
and start the avahi-daemon service on the system running uxplay (your distribution will document how to do  this).
Some  systems  may instead use the mdnsd daemon as an alternative to provide DNS-SD service.
_(FreeBSD offers both alternatives, but only Avahi was tested: one of the steps needed for
getting Avahi running on a FreeBSD system is to edit ```/usr/local/etc/avahi/avahi-daemon.conf```  to
uncomment a line for airplay support._)

After starting uxplay, use the utility ```avahi-browse -a -t``` in a different terminal window on the server to
verify that the UxPlay AirTunes and AirPlay services are correctly registered (only the AirTunes service is
used in the "Legacy" AirPlay Mirror mode  used by UxPlay).    If the UxPlay service is listed by avahi-browse, but is not seen by the client,
the problem is likely to be a problem with the local network.

### 2. uxplay starts, but stalls after "Initialized server socket(s)" appears, *with the server name showing on the client* (but the client fails to connect when the UxPlay server is selected).

This shows that a *DNS-SD* service  is working, but a firewall on the server is probably blocking the connection request from the client.
(One user who insisted that the firewall had been turned off turned out to have had _two_ active firewalls (*firewalld* and *ufw*)
_both_ running on the server!)  If possible, either turn off the firewall
to see if that is the problem, or get three consecutive network ports,
starting at port n, all three in the range 1024-65535, opened  for both tcp and udp, and use "uxplay -p n"
(or open UDP 6000, 6001, 6011 TCP 7000,7001,7100 and use "uxplay -p").

### 3. Problems _after_ the client-server connection has been made:

If you do _not_ see the message ``raop_rtp_mirror starting mirroring``, something went wrong before the client-server negotiations were finished.
For such  problems, use "uxplay -d " (debug log option)  to see what is happening: it will show how far the connection process gets before
the failure occurs.    You can compare your debug  output to
that from a successful start of UxPlay in the [UxPlay Wiki](https://github.com/FDH2/UxPlay/wiki).

**If UxPlay reports that mirroring started, but you get no video or audio, the problem is probably  from  a
GStreamer plugin that doesn't work on your system** (by default,
GStreamer uses the "autovideosink"  and  "autoaudiosink" algorithms 
to guess what are  the "best" plugins to use on your system).

Sometimes "autovideosink" may select the OpenGL renderer "glimagesink" which
may not work correctly on your system.   Try the options "-vs ximagesink" or
"-vs xvimagesink" to see if using one of these fixes the problem.

Other reported problems are connected to the GStreamer VAAPI plugin
(for hardware-accelerated Intel graphics, but not NVIDIA graphics).
Use the option "-avdec"
to force software h264 video decoding: this should prevent autovideosink from selecting the vaapisink videosink.
Alternatively, find out if the
gstreamer1.0-vaapi plugin is installed, and if  so, uninstall it.
(If this does not fix the problem, you can reinstall it.) 

There are some reports of other GStreamer problems with hardware-accelerated Intel HD graphics.  One user
(on Debian) solved this with "sudo apt install intel-media-va-driver-non-free".  This is a driver for 8'th (or later) generation
"*-lake" Intel chips, that seems to be related to VAAPI accelerated graphics.

If you _do_ have Intel HD graphics, and have installed the vaapi plugin, but ``-vs vaapisink`` does not work, check that vaapi is not "blacklisted" in your GStreamer installation: run ``gst-inspect-1.0 vaapi``, if this reports ``0 features``, you need to ``export GST_VAAPI_ALL_DRIVERS=1`` before running uxplay, or set this in the default environment. 

You can try to fix audio problems by using the "-as _audiosink_"  option to choose the GStreamer audiosink , rather than
have autoaudiosink pick one for you.    The command "gst-inspect-1.0 | grep Sink | grep Audio" " will show you which audiosinks are 
available on your system.  (Replace  "Audio" by "Video" to see videosinks).   Some possible audiosinks are pulsesink, alsasink, osssink, oss4sink,
and osxaudiosink (macOS).  
 
If you ran cmake with "-DZOOMFIX=ON", check if the problem is still there without ZOOMFIX.
ZOOMFIX is only applied to the default videosink choice ("autovideosink") and the two X11 videosinks
"ximagesink" and "xvimagesink".   ZOOMFIX is only designed for these last two; if
autovideosink chooses a different videosink, ZOOMFIX is now ignored. 
If you are using the X11 windowing system (standard on Linux), and have trouble with screen-sharing on Zoom, use
ZOOMFIX and "-vs xvimagesink" (or "-vs ximagesink" if the previous choice doesn't work). 

As other  videosink choices are not affected by ZOOMFIX, they  may or may not be visible to screen-sharing apps.
Cairo-based windows created on Linux with "-vs gtksink" are visible to screen-sharing aps without ZOOMFIX; windows on macOS created by
"-vs glimagesink" (default choice) and "-vs osximagesink" are also visible.

The "OpenGL renderer" window created on Linux by "-vs glimagesink" sometimes does not close properly when its "close" button is clicked.
(this is a GStreamer issue).  You may need to terminate uxplay with Ctrl-C to close a "zombie" OpenGl window.   If similar problems happen when 
the client sends the "Stop Mirroring" signal, try the no-close option "-nc" that leaves the video window open.

### 4. GStreamer issues (missing plugins, etc.): 

To troubleshoot GStreamer execute  "export GST_DEBUG=2"
to set the GStreamer debug-level environment-variable in the terminal
where you will run uxplay, so that you see warning and error messages;
(replace "2" by "4" to see much (much) more of what is happening inside
GStreamer).   Run "gst-inspect-1.0" to see which GStreamer plugins are
installed on your system.

Some extra GStreamer packages for special plugins may need to be installed (or reinstalled: a user using a Wayland display system as an alternative to X11
reported that after reinstalling Lubuntu 18.4, UxPlay would not  work until gstreamer1.0-x was installed, presumably for Wayland's X11-compatibility mode).
Different distributions may break up GStreamer 1.x into packages in different ways; the packages listed above in the build instructions should bring in 
other required GStreamer packages as dependencies, but will not install all possible plugins.

### 5.  Mirror screen freezes:

This can happen if the  TCP video stream from the client stops arriving at the server, probably because of network problems  (the UDP audio stream may continue to arrive).   At 3-second 
intervals, UxPlay checks that the client is still connected by sending it a request for a NTP time signal.   If a reply is not received from the client within a 0.3 sec 
time-window, an "ntp timeout" is registered.    If a certain number (currently 5) of consecutive ntp timeouts occur, UxPlay assumes that the client is "dead", and resets the connection, 
becoming available for connection to a new client, or reconnection to the previous one.   Sometimes the connection may recover before the timeout limit is reached, and if the
default limit is not right  for your network,  it can be modified using the option "-reset _n_", where _n_ is the desired timeout-limit value (_n_ = 0 means "no limit").    If the connection 
starts to recover after ntp timeouts, a corrupt video packet from before the timeout may trigger a "connection reset by peer"  error, which also causes UxPlay to reset the
connection. When the connection is reset, the "frozen" mirror screen of the previous connection is left in place, and will be taken over by a new client connection when it is made.

### 6.  Failure to decrypt ALL video and audio streams from old or non-Apple clients:

This triggers an unending stream of error messages, and means that the
audio decryption key (also used in video decryption) 
was not correctly extracted from data sent by the  client.
This should not happen for iOS 9.3 or later clients.  However, if a client 
uses the same  older version of the protocol that is used by the Windows-based
AirPlay client emulator _AirMyPC_, the protocol can be switched to the older version
by the setting ```OLD_PROTOCOL_CLIENT_USER_AGENT_LIST``` in lib/global.h.
UxPlay reports the  client's "User Agent" string when it connects. If 
some other client also fails to decrypt all audio and video, try adding 
its "User Agent" string in place of "xxx" in the  entry "AirMyPC/2.0;xxx" 
in global.h and rebuild uxplay.

Note that Uxplay declares itself to be an AppleTV3,2 with a 
sourceVersion 220.68; this can also be changed in global.h. It had been thought
that it was necessary for UxPlay to claim to be an older 32 bit
AppleTV model that cannot run modern 64bit tvOS, in order for the client
to use a "legacy" protocol for pairing with the server (see 
the _"Notes on AirPlay protocol versions"_ at the end of this README).
However, UxPlay still works if it declares itself as an AppleTV6,2 with
sourceVersion 380.20.1 (an AppleTV 4K 1st gen, introduced 2017, running
tvOS 12.2.1), so it is unclear what setting prompts the client
to use the "legacy" protocol needed by UxPlay.

# ChangeLog
1.48 2022-03-11   Made the GStreamer video pipeline fully configurable, for use with hardware h264 decoding.  Support for Raspberry Pi.

1.47 2022-02-05   Added -FPSdata option to display (in the terminal) regular reports sent by the client about video streaming 
                  performance.  Internal cleanups of processing of video packets received from the client.   Added -reset n option 
                  to reset the connection after n ntp timeouts (also reset after "connection reset by peer" error in video stream).

1.46 2022-01-20   Restore pre-1.44 behavior (1.44 may have broken hardware acceleration): once again use decodebin in the video pipeline; 
                  introduce new option "-avdec" to force software h264 decoding by libav h264, if needed (to prevent selection of 
                  vaapisink by autovideosink).  Update llhttp to v6.0.6.  UxPlay now reports itself as AppleTV3,2.  Restrict connections 
                  to one client at a time (second client must now wait for first client to disconnect). 

1.45 2022-01-10   New behavior: close video window when client requests "stop mirroring".  (A new "no close" option "-nc" is added
                  for users who wish to retain previous behavior that does not close the video window).

1.44 2021-12-13   Omit hash of aeskey with ecdh_secret for an AirMyPC client; make an internal rearrangement of where this hash is 
                  done. Fully report all initial communications between client and server in -d debug mode. Replace decodebin in GStreamer
                  video pipeline by h264-specific elements.

1.43 2021-12-07   Various internal changes, such as tests for successful decryption, uniform treatment 
                  of informational/debug messages, etc., updated README.

1.42 2021-11-20   Fix MAC detection to work with modern Linux interface naming practices, MacOS and *BSD.

1.41 2021-11-11    Further cleanups of multiple audio format support (internal changes, 
                   separated RAOP and GStreamer audio/video startup)

1.40 2021-11-09    Cleanup segfault in ALAC support, manpage location fix, show request Plists in debug mode.

1.39 2021-11-06    Added support for Apple Lossless (ALAC) audio streams.

1.38 2021-10-8     Add -as _audiosink_ option to allow user to choose the  GStreamer audiosink.

1.37 2021-09-29    Append "@hostname" to AirPlay Server name, where "hostname" is the name of the
                   server running uxplay (reworked change in 1.36).

1.36 2021-09-29    Implemented suggestion (by @mrbesen and @PetrusZ) to use hostname of machine
                   runing uxplay as the default server name 

1.35.1 2021-09-28  Added the -vs 0 option for streaming audio, but not displaying video.

1.35  2021-09-10   now uses a GLib MainLoop, and builds on macOS (tested on Intel Mac, 10.15 ).
                   New option  -t _timeout_ for relaunching server if no connections were active in
                   previous _timeout_ seconds (to renew Bonjour registration).
                   
1.341 2021-09-04   fixed: render logger was not being destroyed by stop_server()

1.34  2021-08-27   Fixed "ZOOMFIX": the X11 window name fix was only being made the
                   first time the GStreamer window was created by uxplay, and
		   not if the server was relaunched after the GStreamer window
		   was closed, with uxplay still running.   Corrected in v. 1.34

# Improvements

1. Updates of the RAOP (AirPlay protocol)  collection of codes  maintained
at  https://github.com/FD-/RPiPlay.git so it is current as of 2021-08-01,
adding all changes since the original release of UxPlay by antimof.
This involved crypto updates, replacement
of the included plist library by the system-installed version, and  a change
over to a library llhttp for http parsing. 

2. Added  -s, -o -p, -m, -r,  -f, -fps, -vs, -as  and -t  options.

3. If "`cmake -DZOOMFIX=ON .`"  is run before compiling,
the mirrored window is now visible to screen-sharing applications such as
Zoom. (This applies only to X11 windows produced by videosinks ``ximagesink`` and ``xvimagesink``, which are often 
selected by default.) To compile with ZOOMFIX=ON, the X11 development libraries must be installed.
_(ZOOMFIX will not be needed once the upcoming  gstreamer-1.20 is available,
since starting with that release, the GStreamer X11 mirror window will be natively
visible for screen-sharing, but it make take some time for distributions to supply this version.)_ Thanks to David Ventura
https://github.com/DavidVentura/UxPlay for the fix
and also for getting it into  gstreamer-1.20.
[If uxplay was compiled after
cmake was run without -DZOOMFIX=ON, and your gstreamer version is older than
1.20, you can still manually make the X11 window visible to screen-sharing apps with the X11 utility
xdotool, if it is installed, with: ``` xdotool selectwindow set_window --name <name>```
(where ```<name>``` is your choice of name), and then select the uxplay window
by clicking on it with the mouse.]

4. The AirPlay server now terminates correctly when the gstreamer display window is
closed, and is relaunched with the same settings to wait for a new connection.
The program uxplay terminates when Ctrl-C is typed in the terminal window. The **-t _timeout_**
option relaunches the server after _timeout_ seconds  of inactivity to allow new connections to be made.

5.   In principle, multiple instances of uxplay can be run simultaneously
using the **-m** (generate random MAC address) option to give each a
different ("local" as opposed to "universal")  MAC address.
If the **-p [n]** option is used, they also need separate network port choices.
(However, there may be a large latency, and running two instances of uxplay
simultaneously on the same computer may not be very useful; using the **-fps** option
to force streaming framerates below 30fps could be helpful.)

6.  Without the **-p** [n] option,  uxplay makes a random dynamic assignment of
network ports. This will not work if most ports are closed by a firewall.
With e.g., **-p 45000**   you should open both TCP and UDP on
ports 45000, 45001, 45002.   Minimum allowed port is 1024, maximum is 65535.
The option "**-p**" with no argument uses a "legacy" set of ports TCP 7100,
7000, 7001, and UDP  7011, 6000, 6001.  Finer control is also
possible: "**-p udp n1,n2,n3 -p tcp n4,n5,n6**" sets all six ports individually.

7.  The default resolution setting is 1920x1080 width x height pixels.
To change this, use "**-s wxh**"  where w and h are positive  decimals
with 4 or less digits.   It seems that the width and height may be negotiated
with the AirPlay client, so this may not be the actual screen geometry that
displays.

8. The title on the GStreamer display window is now is the AirPlay server name.
(This works for X11 windows created
by gstreamer videosinks ximagesink, xvimagesink, but not OpenGL windows created by glimagesink.)

9. The avahi_compat "nag" warning on startup is suppressed, by placing
"AVAHI_COMPAT_NOWARN=1" into the runtime environment when uxplay starts.
(This uses a call to putenv() in a form that is believed to be safe against
memory leaks, at least in modern Linux; if for any reason you don't want
this fix, comment out the line in CMakeLists.txt that activates it when uxplay
is compiled.) On macOS, Avahi is not used.

10. UxPlay now builds on macOS.

11. The hostname of the server running uxplay is now appended to the AirPlay server name,
    which is now displayed as _name_@hostname, where _name_ is "UxPlay", (or whatever is set with the **-n** option).

12. Added support for audio-only streaming with original (non-Mirror) AirPlay protocol, with Apple Lossless (ALAC) audio.

13. Added suppport for the older AirPlay protocol used by third-party Windows-based AirPlay mirror emulators such as AirMyPC.

14. Made the video pipeline fully configurable with options -vp, -vd, -vc, for accelerated hardware support (e.g. NVIDIA).

15. Added Raspberry Pi support (accelerated hardware decoding) with -rpi option.

# Disclaimer

All the resources in this repository are written using only freely available information from the internet. The code and related resources are meant for educational purposes only. It is the responsibility of the user to make sure all local laws are adhered to.

This project makes use of a third-party GPL library for handling FairPlay. The legal status of that library is unclear. Should you be a representative of Apple and have any objections against the legality of the library and its use in this project, please contact me and I'll take the appropriate steps.

Given the large number of third-party AirPlay receivers (mostly closed-source) available for purchase, it is my understanding that an open source implementation of the same functionality wouldn't violate any of Apple's rights either.



# UxPlay authors

_[adapted from fdraschbacher's notes on  RPiPlay antecedents]_

The code in this repository accumulated from various sources over time. Here
is an attempt at listing the various authors and the components they created:

UxPlay was initially created by **antimof** from RPiPlay, by replacing its Raspberry-Pi-specific  video and audio rendering system with GStreamer rendering for
Desktop Linux (antimof's work on code in `renderers/` was later backported to RPiPlay).   The previous authors of code included in UxPlay by inheritance from RPiPlay include:

* **EstebanKubata**: Created a FairPlay library called [PlayFair](https://github.com/EstebanKubata/playfair). Located in the `lib/playfair` folder. License: GNU GPL
* **Juho Vähä-Herttua** and contributors: Created an AirPlay audio server called [ShairPlay](https://github.com/juhovh/shairplay), including support for Fairplay based on PlayFair. Most of the code   in `lib/` originally stems from this project. License: GNU LGPLv2.1+
* **dsafa22**: Created an AirPlay 2 mirroring server [AirplayServer](https://github.com/dsafa22/AirplayServer) (seems gone now), for Android based on ShairPlay.   Code is 
  preserved [here](https://github.com/jiangban/AirplayServer), and [see here](https://github.com/FDH2/UxPlay/wiki/AirPlay2) for  the description 
  of the analysis of the AirPlay 2 mirror protocol that made RPiPlay possible, by the AirplayServer author. All 
  code in `lib/` concerning mirroring is dsafa22's work. License: GNU LGPLv2.1+
* **Florian Draschbacher** and contributors: adapted dsafa22's Android project for the Raspberry Pi, with extensive cleanups, debugging and improvements.  The
   project [RPiPlay](https://github.com/FD-/RPiPlay) is basically a port of dsafa22's code to the Raspberry Pi, utilizing OpenMAX and OpenSSL for better performance on the Pi. License GPL v3.

Independent of UxPlay, but used by it and bundled with it:

* **Fedor Indutny** (of Node.js, and formerly Joyent, Inc) and contributors: Created an http parsing library called [llhttp](https://github.com/nodejs/llhttp). Located at `lib/llhttp/`. License: MIT

## Notes on AirPlay protocol versions  by Florian Draschbacher, RPiPlay creator
(From the  https://github.com/FD-/RPiPlay.git repository.)

For multiple reasons, it's very difficult to clearly define the protocol names and versions of the components that make up the AirPlay streaming system. In fact, it seems like the AirPlay version number used for marketing differs from that used in the actual implementation. In order to tidy up this whole mess a bit, I did a little research that I'd like to summarize here:


The very origin of the AirPlay protocol suite was launched as AirTunes sometime around 2004. It allowed to stream audio from iTunes to an AirPort Express station. Internally, the name of the protocol that was used was RAOP, or Remote Audio Output Protocol. It seems already back then, the protocol involved AES encryption. A public key was needed for encrypting the audio sent to an AirPort Express, and the private key was needed for receiving the protocol (ie used in the AirPort Express to decrypt the stream). Already in 2004, the public key was reverse-engineered, so that [third-party sender applications](http://nanocr.eu/2004/08/11/reversing-airtunes/) were developed.


Some time [around 2008](https://weblog.rogueamoeba.com/2008/01/10/a-tour-of-airfoil-3/), the protocol was revised and named AirTunes 2. It seems the changes primarily concerned timing. By 2009, the new protocol was [reverse-engineered and documented](https://git.zx2c4.com/Airtunes2/about/).


When the Apple TV 2nd generation was introduced in 2010, it received support for the AirTunes protocol. However, because this device allowed playback of visual content, the protocol was extended and renamed AirPlay. It was now possible to stream photo slideshows and videos. Shortly after the release of the Apple TV 2nd generation, AirPlay support for iOS was included in the iOS 4.2 update. It seems like at that point, the audio stream was still actually using the same AirTunes 2 protocol as described above. The video and photo streams were added as a whole new protocol based on HTTP, pretty much independent from the audio stream. Soon, the first curious developers began to [investigate how it worked](https://web.archive.org/web/20101211213705/http://www.tuaw.com/2010/12/08/dear-aunt-tuaw-can-i-airplay-to-my-mac/). Their conclusion was that visual content is streamed unencrypted.


In April 2011, a talented hacker [extracted the AirPlay private key](http://www.macrumors.com/2011/04/11/apple-airplay-private-key-exposed-opening-door-to-airport-express-emulators/) from an AirPort Express. This meant that finally, third-party developers were able to also build AirPlay receiver (server) programs.


For iOS 5, released in 2011, Apple added a new protocol to the AirPlay suite: AirPlay mirroring. [Initial investigators](https://www.aorensoftware.com/blog/2011/08/20/exploring-airplay-mirroring-internals/) found this new protocol used encryption in order to protect the transferred video data.


By 2012, most of AirPlay's protocols had been reverse-engineered and [documented](https://nto.github.io/AirPlay.html) (see also [updated version](https://openairplay.github.io/airplay-spec)). At this point, audio still used the AirTunes 2 protocol from around 2008, video, photos and mirroring still used their respective protocols in an unmodified form, so you could still speak of AirPlay 1 (building upon AirTunes 2). The Airplay server running on the Apple TV reported as version 130. The setup of AirPlay mirroring used the xml format, in particular a stream.xml file.
Additionally, it seems like the actual audio data is using the ALAC codec for audio-only (AirTunes 2) streaming and AAC for mirror audio. At least these different formats were used in [later iOS versions](https://github.com/espes/Slave-in-the-Magic-Mirror/issues/12#issuecomment-372380451).


Sometime before iOS 9, the protocol for mirroring was slightly modified: Instead of the "stream.xml" API endpoint, the same information could also be querried in binary plist form, just by changing the API endpoint to "stream", without any extension. I wasn't able to figure out which of these was actually used by what specific client / server versions.


For iOS 9, Apple made [considerable changes](https://9to5mac.com/2015/09/11/apple-ios-9-airplay-improvements-screen-mirroring/) to the AirPlay protocol in 2015, including audio and mirroring. Apparently, the audio protocol was only slightly modified, and a [minor change](https://github.com/juhovh/shairplay/issues/43) restored compatibility. For mirroring, an [additional pairing phase](https://github.com/juhovh/shairplay/issues/43#issuecomment-142115959) was added to the connection establishment procedure, consisting of pair-setup and pair-verify calls. Seemingly, these were added in order to simplify usage with devices that are connected frequently. Pair-setup is used only the first time an iOS device connects to an AirPlay receiver. The generated cryptographic binding can be used for pair-verify in later sessions. Additionally, the stream / stream.xml endpoint was replaced with the info endpoint (only available as binary plist AFAICT).
As of iOS 12, the protocol introduced with iOS 9 was still supported with only slight modifications, albeit as a legacy mode. While iOS 9 used two SETUP calls (one for general connection and mirroring video, and one for audio), iOS 12 legacy mode uses 3 SETUP calls (one for general connection (timing and events), one for mirroring video, one for audio).


The release of tvOS 10.2 broke many third-party AirPlay sender (client) programs in 2017. The reason was that it was now mandatory to perform device verification via a pin in order to stream content to an Apple TV. The functionality had been in the protocol before, but was not mandatory. Some discussion about the new scheme can be found [here](https://github.com/postlund/pyatv/issues/79). A full specification of the pairing and authentication protocol was made available on [GitHub](https://htmlpreview.github.io/?https://github.com/philippe44/RAOP-Player/blob/master/doc/auth_protocol.html). At that point, tvOS 10.2 reported as AirTunes/320.20.


In tvOS 11, the reported server version was [increased to 350.92.4](https://github.com/ejurgensen/forked-daapd/issues/377#issuecomment-309213273).


iOS 11.4 added AirPlay 2 in 2018. Although extensively covered by the media, it's not entirely clear what changes specifically Apple has made protocol-wise.


From captures of the traffic between an iOS device running iOS 12.2 and an AppleTV running tvOS 12.2.1, one can see that the communication on the main mirroring HTTP connection is encrypted after the initial handshake.
This could theoretically be part of the new AirPlay 2 protocol. The AppleTV running tvOS 12.2.1 identifies as AirTunes/380.20.1.
When connecting from the same iOS device to an AppleTV 3rd generation (reporting as AirTunes/220.68), the communication is still visible in plain. From the log messages that the iOS device produces when connected to an AppleTV 3rd generation, it becomes apparent that the iOS device is treating this plain protocol as the legacy protocol (as originally introduced with iOS 9). Further research showed that at the moment, all available third-party AirPlay mirroring receivers (servers) are using this legacy protocol, including the open source implementation of dsafa22, which is the base for RPiPlay. Given Apple considers this a legacy protocol, it can be expected to be removed entirely in the future. This means that all third-party AirPlay receivers will have to be updated to the new (fully encrypted) protocol at some point.

More specifically, the encryption starts after the pair-verify handshake completed, so the fp-setup handshake is already happening encrypted. Judging from the encryption scheme for AirPlay video (aka HLS Relay), likely two AES GCM 128 ciphers are used on the socket communication (one for sending, one for receiving). However, I have no idea how the keys are derived from the handshake data.
