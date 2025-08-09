# UxPlay 1.72: AirPlay-Mirror and AirPlay-Audio server for Linux, macOS, and Unix (also runs on Windows).

### **Now developed at the GitHub site <https://github.com/FDH2/UxPlay> (where ALL user issues should be posted, and latest versions can be found).**

-   **NEW on github**: option -ca (with no filename given) will now
    render Apple Music cover art (in audio-only mode) inside UxPlay.
    (-ca `<filename>` will continue to export cover art for display by
    an external viewer).

-   **NEW in v1.72**: Improved Support for (YouTube) HLS (HTTP Live
    Streaming) video with the new "-hls" option (introduced in 1.71).\*
    **Only streaming from the YouTube iOS app (in \"m3u8\" protocol) is
    currently supported**: (streaming using the AirPlay icon in a
    browser window is **not** yet supported).Click on the airplay icon
    in the YouTube app to stream video. **Please report any issues with
    this new feature of UxPlay**.

    *The default video player for HLS is GStreamer playbin v3: use "-hls
    2" to revert to playbin v2 if some videos fail to play*.

    -   user-requested features: added support for setting a password
        (as an alternative to on-screen pin codes) to control client
        access (-pw option, see "man pw" or this README for details);
        added support for setting initial client audio-streaming volume
        (-vol option), and output of audio-mode metadata to file (for
        display by some external process, -md option).

    **ISSUES** ***(Please help to solve if you have expertise)***

    -   in HLS video streaming from the YouTube app (-hls option),
        rendered using GStreamer's media player "playbin3" (or playbin2,
        with option -hls 2), we don't understand how to correctly deal
        with "interstitials" (= 15 sec commercials) when "skip" is
        pressed on the client. (HLS is handled by handlers in
        lib/http_handlers.h). (Should response to HTTP requests POST
        /action (playlistRemove) and POST /Stop be modified? *Wireshark
        data from HLS on an AppleTV model 3 with UN-upgraded original OS
        (unencrypted communications) could be useful!*

## Highlights:

-   GPLv3, open source.
-   Originally supported only AirPlay Mirror protocol, now has added
    support for AirPlay Audio-only (Apple Lossless ALAC) streaming from
    current iOS/iPadOS clients. **Now with support for Airplay HLS
    video-streaming (currently only YouTube video).**
-   macOS computers (2011 or later, both Intel and "Apple Silicon" M1/M2
    systems) can act either as AirPlay clients, or as the server running
    UxPlay. Using AirPlay, UxPlay can emulate a second display for macOS
    clients.
-   Support for older iOS clients (such as 32-bit iPad 2nd gen., iPod
    Touch 5th gen. and iPhone 4S, when upgraded to iOS 9.3.5, or later
    64-bit devices), plus a Windows AirPlay-client emulator, AirMyPC.
-   Uses GStreamer plugins for audio and video rendering (with options
    to select different hardware-appropriate output "videosinks" and
    "audiosinks", and a fully-user-configurable video streaming
    pipeline).
-   Support for server behind a firewall.
-   Raspberry Pi support **both with and without hardware video
    decoding** by the Broadcom GPU. *Tested on Raspberry Pi Zero 2 W, 3
    Model B+, 4 Model B, and 5.*
-   Support for running on Microsoft Windows (builds with the MinGW-64
    compiler in the unix-like MSYS2 environment).

Note: AirPlay2 multi-room audio streaming is not supported: use
[shairport-sync](https://github.com/mikebrady/shairport-sync) for that.

## Packaging status (Linux and \*BSD distributions)

[![Current Packaging
status](https://repology.org/badge/vertical-allrepos/uxplay.svg)](https://repology.org/project/uxplay/versions).

-   Install uxplay on Debian-based Linux systems with
    "`sudo apt install uxplay`"; on FreeBSD with
    "`sudo pkg install uxplay`"; on OpenBSD with
    "`doas pkg_add uxplay`". Also available on Arch-based systems
    through AUR. Since v. 1.66, uxplay is now also packaged in RPM
    format by Fedora 38 ("`sudo dnf install uxplay`").

-   For other RPM-based distributions which have not yet packaged
    UxPlay, a RPM "specfile" **uxplay.spec** is now provided with recent
    [releases](https://github.com/FDH2/UxPlay/releases) (see their
    "Assets"), and can also be found in the UxPlay source top directory.
    See the section on using this specfile for [building an installable
    RPM package](#building-an-installable-rpm-package).

-   If your distribution does not supply UxPlay, or you want the latest
    version, it is very easy to build it yourself: see the very
    [detailed instructions for building UxPlay from
    source](#building-uxplay-from-source). later in this document.

## After installation:

-   (On Linux and \*BSD): if a firewall is active on the server hosting
    UxPlay, make sure the default network port (UDP 5353) for
    mDNS/DNS-SD queries is open (see [Troubleshooting](#troubleshooting)
    below for more details); also open three UDP and three TCP ports for
    Uxplay, and use the "uxplay -p `<n>`{=html}" option (see
    "`man uxplay`" or "`uxplay -h`").

-   Even if you install your distribution's pre-compiled uxplay binary
    package, you may need to read the instructions below for [running
    UxPlay](#running-uxplay) to see which of your distribution's
    **GStreamer plugin packages** you should also install.

-   For Audio-only mode (Apple Music, etc.) best quality is obtained
    with the option "uxplay -async", but there is then a 2 second
    latency imposed by iOS. Use option "uxplay -ca" to display any
    "Cover Art" that accompanies the audio.

-   If you are using UxPlay just to mirror the client's screen (without
    showing videos that need audio synchronized with video), it is best
    to use the option "uxplay -vsync no".

-   Add any UxPlay options you want to use as defaults to a startup file
    `~/.uxplayrc` (see "`man uxplay`" or "`uxplay -h`" for format and
    other possible locations; the location can also be set with "uxplay
    -rc *location*"). In particular, if your system uses PipeWire audio
    or Wayland video systems, you may wish to add "as pipewiresink" or
    "vs waylandsink" as defaults to the file. *(Output from terminal
    commands "ps waux \| grep pulse" or "pactl info" will contain
    "pipewire" if your Linux/BSD system uses it).*

-   For Linux systems using systemd, there is a **systemd** service file
    **uxplay.service** found in the UxPlay top directory of the
    distribution, and also installed in `<DOCDIR>/uxplay/systemd/`
    (where DOCDIR is usually `/usr/local/share/doc`), that allows users
    to start their own instance of UxPlay as a rootless daemon: it
    should either be added to the directory /etc/systemd/user, or the
    user can just create their own systemd directory
    `~/.config/systemd/user/` and then copy uxplay.service into it. To
    save uxplay terminal output to a file \~/uxplay.log, uncomment the
    StandardOutput entry in uxplay.service. Then

    `systemctl --user [start/stop/enable/disable/status] uxplay`

    can be used to control the daemon. If it is enabled, the daemon will
    start at the user's first login and stop when they no longer have
    any open sessions. See
    https://www.baeldung.com/linux/systemd-create-user-services for more
    about systemd user services. If more than one user might
    simultaneously run uxplay this way, they should specify distinct -p
    and -m options (ports and deviceID) in their startup files. **Note:
    it is NOT recommended to run UxPlay as a root service.**

-   On Raspberry Pi: models using hardware h264 video decoding by the
    Broadcom GPU (models 4B and earlier) may require the uxplay option
    -bt709. If you use Ubuntu 22.10 or earlier, GStreamer must be
    [patched](https://github.com/FDH2/UxPlay/wiki/Gstreamer-Video4Linux2-plugin-patches)
    to use hardware video decoding by the Broadcom GPU (also recommended
    but optional for Raspberry Pi OS (Bullseye): the patched GStreamer
    does not need option " -bt709\`". The need for -bt709 when hardware
    video decoding is used seems to have reappeared starting with
    GStreamer-1.22.

-   If UxPlay is used in a public space, there are security options for
    requiring an AppleTV-style one-time pin (displayed on the terminal)
    to be entered, or a password, and for barring/permitting client
    access by their device ID. See options -pin, -reg, -pw, -restrict,
    -allow, -block.

# Detailed description of UxPlay

This project is a GPLv3 open source unix AirPlay2 Mirror server for
Linux, macOS, and \*BSD. It was initially developed by
[antimof](http://github.com/antimof/Uxplay) using code from
OpenMAX-based [RPiPlay](https://github.com/FD-/RPiPlay), which in turn
derives from [AirplayServer](https://github.com/KqsMea8/AirplayServer),
[shairplay](https://github.com/juhovh/shairplay), and
[playfair](https://github.com/EstebanKubata/playfair). (The antimof site
is no longer involved in development, but periodically posts updates
pulled from the new main [UxPlay site](https://github.com/FDH2/UxPlay)).

UxPlay is tested on a number of systems, including (among others) Debian
(10 "Buster", 11 "Bullseye", 12 "Bookworm"), Ubuntu (20.04 LTS, 22.04
LTS, 23.04 (also Ubuntu derivatives Linux Mint, Pop!\_OS), Red Hat and
clones (Fedora 38, Rocky Linux 9.2), openSUSE Leap 15.5, Mageia 9,
OpenMandriva "ROME", PCLinuxOS, Arch Linux, Manjaro, and should run on
any Linux system. Also tested on macOS Catalina and Ventura (Intel) and
Sonoma (M2), FreeBSD 14.0, Windows 10 and 11 (64 bit).

On Raspberry Pi 4 model B, it is tested on Raspberry Pi OS (Bullseye and
Bookworm) (32- and 64-bit), Ubuntu 22.04 LTS and 23.04, Manjaro RPi4
23.02, and (without hardware video decoding) on openSUSE 15.5. Also
tested on Raspberry Pi Zero 2 W, 3 model B+, and now 5.

Its main use is to act like an AppleTV for screen-mirroring (with audio)
of iOS/iPadOS/macOS clients (iPhone, iPod Touch, iPad, Mac computers) on
the server display of a host running Linux, macOS, or other unix (and
now also Microsoft Windows). UxPlay supports Apple's AirPlay2 protocol
using "Legacy Protocol", but some features are missing. (Details of what
is publicly known about Apple's AirPlay 2 protocol can be found
[here](https://openairplay.github.io/airplay-spec/),
[here](https://github.com/SteeBono/airplayreceiver/wiki/AirPlay2-Protocol)
and [here](https://emanuelecozzi.net/docs/airplay2); see also
[pyatv](https://pyatv.dev/documentation/protocols) which could be a
resource for adding modern protocols.) While there is no guarantee that
future iOS releases will keep supporting "Legacy Protocol", iOS 17
continues support.

The UxPlay server and its client must be on the same local area network,
on which a **Bonjour/Zeroconf mDNS/DNS-SD server** is also running (only
DNS-SD "Service Discovery" service is strictly necessary, it is not
necessary that the local network also be of the ".local" mDNS-based
type). On Linux and BSD Unix servers, this is usually provided by
[Avahi](https://www.avahi.org), through the avahi-daemon service, and is
included in most Linux distributions (this service can also be provided
by macOS, iOS or Windows servers).

Connections to the UxPlay server by iOS/MacOS clients can be initiated
both in **AirPlay Mirror** mode (which streams lossily-compressed AAC
audio while mirroring the client screen, or in the alternative **AirPlay
Audio** mode which streams Apple Lossless (ALAC) audio without screen
mirroring. In **Audio** mode, metadata is displayed in the uxplay
terminal; if UxPlay option `-ca <name>` is used, the accompanying cover
art is also output to a periodically-updated file `<name>`, and can be
viewed with a (reloading) graphics viewer of your choice. *Switching
between* **Mirror** *and* **Audio** *modes during an active connection
is possible: in* **Mirror** *mode, stop mirroring (or close the mirror
window) and start an* **Audio** *mode connection, switch back by
initiating a* **Mirror** *mode connection; cover-art display
stops/restarts as you leave/re-enter* **Audio** *mode.*

-   **Note that Apple video-DRM (as found in "Apple TV app" content on
    the client) cannot be decrypted by UxPlay, and the Apple TV app
    cannot be watched using UxPlay's AirPlay Mirror mode (only the
    unprotected audio will be streamed, in AAC format).**

-   **With the new "-hls" option, UxPlay now also supports non-Mirror
    AirPlay video streaming (where the client controls a web server on
    the AirPlay server that directly receives HLS content to avoid it
    being decoded and re-encoded by the client). This currently only
    supports streaming of YouTube videos. Without the -hls option, using
    the icon for AirPlay video in apps such as the YouTube app will only
    send audio (in lossless ALAC format) without the accompanying
    video.**

### Possibility for using hardware-accelerated h264/h265 video-decoding, if available.

UxPlay uses [GStreamer](https://gstreamer.freedesktop.org) "plugins" for
rendering audio and video. This means that video and audio are supported
"out of the box", using a choice of plugins. AirPlay streams video in
h264 format: gstreamer decoding is plugin agnostic, and uses accelerated
GPU hardware h264 decoders if available; if not, software decoding is
used.

-   **VAAPI for Intel and AMD integrated graphics, NVIDIA with "Nouveau"
    open-source driver**

    With an Intel or AMD GPU, hardware decoding with the open-source
    VAAPI gstreamer plugin is preferable. The open-source "Nouveau"
    drivers for NVIDIA graphics are also in principle supported: see
    [here](https://nouveau.freedesktop.org/VideoAcceleration.html), but
    this requires VAAPI to be supplemented with firmware extracted from
    the proprietary NVIDIA drivers.

-   **NVIDIA with proprietary drivers**

    The `nvh264dec` plugin (included in gstreamer1.0-plugins-bad since
    GStreamer-1.18.0) can be used for accelerated video decoding on the
    NVIDIA GPU after NVIDIA's CUDA driver `libcuda.so` is installed. For
    GStreamer-1.16.3 or earlier, the plugin is called `nvdec`, and must
    be [built by the
    user](https://github.com/FDH2/UxPlay/wiki/NVIDIA-nvdec-and-nvenc-plugins).

-   **Video4Linux2 support for h264 hardware decoding on Raspberry Pi
    (Pi 4B and older)**

    Raspberry Pi (RPi) computers (tested on Pi 4 Model B) can now run
    UxPlay using software video decoding, but hardware-accelerated
    h264/h265 decoding by firmware in the Pi's Broadcom 2835 GPU is
    prefered. UxPlay accesses this using the GStreamer-1.22 Video4Linux2
    (v4l2) plugin; Uses the out-of-mainline Linux kernel module
    bcm2835-codec maintained by Raspberry Pi, so far only included in
    Raspberry Pi OS, and two other distributions (Ubuntu, Manjaro)
    available with Raspberry Pi Imager. *(For GStreamer \< 1.22, see the
    [UxPlay
    Wiki](https://github.com/FDH2/UxPlay/wiki/Gstreamer-Video4Linux2-plugin-patches))*.
    Pi model 5 has no support for hardware H264 decoding, as its CPU is
    powerful enough for satisfactory software H264 decoding

-   **Support for h265 (HEVC) hardware decoding on Raspberry Pi (Pi 4
    model B and Pi 5)**

    These Raspberry Pi models have a dedicated HEVC decoding block (not
    the GPU), with a driver "rpivid" which is not yet in the mainline
    Linux kernel (but is planned to be there in future). Unfortunately
    it produces decoded video in a non-standard pixel format (NC30 or
    "SAND") which will not be supported by GStreamer until the driver is
    in the mainline kernel; without this support, UxPlay support for
    HEVC hardware decoding on Raspberry Pi will not work.

### Note to packagers:

UxPlay's GPLv3 license does not have an added "GPL exception" explicitly
allowing it to be distributed in compiled form when linked to OpenSSL
versions **prior to v. 3.0.0** (older versions of OpenSSL have a license
clause incompatible with the GPL unless OpenSSL can be regarded as a
"System Library", which it is in \*BSD). Many Linux distributions treat
OpenSSL as a "System Library", but some (e.g. Debian) do not: in this
case, the issue is solved by linking with OpenSSL-3.0.0 or later.

# Building UxPlay from source

Either download and unzip
[UxPlay-master.zip](https://github.com/FDH2/UxPlay/archive/refs/heads/master.zip),
or (if git is installed): "git clone https://github.com/FDH2/UxPlay".
You can also download a recent or earlier version listed in
[Releases](https://github.com/FDH2/UxPlay/releases).

-   A recent UxPlay can also be found on the original [antimof
    site](https://github.com/antimof/UxPlay); that original project is
    inactive, but is usually kept current or almost-current with the
    [active UxPlay github site](https://github.com/FDH2/UxPlay) (thank
    you antimof!).

## Building UxPlay on Linux (or \*BSD):

### Debian-based systems:

(Adapt these instructions for non-Debian-based Linuxes or \*BSD; for
macOS, see specific instruction below). See
[Troubleshooting](#troubleshooting) below for help with any
difficulties.

You need a C/C++ compiler (e.g. g++) with the standard development
libraries installed. Debian-based systems provide a package
"build-essential" for use in compiling software. You also need
pkg-config: if it is not found by "`which pkg-config`", install
pkg-config or its work-alike replacement pkgconf. Also make sure that
cmake\>=3.10 is installed: "`sudo apt install cmake`" (add
`build-essential` and `pkg-config` (or `pkgconf`) to this if needed).

Make sure that your distribution provides OpenSSL 1.1.1 or later, and
libplist 2.0 or later. (This means Debian 10 "Buster" based systems
(e.g, Ubuntu 18.04) or newer; on Debian 10 systems "libplist" is an
older version, you need "libplist3".) If it does not, you may need to
build and install these from source (see instructions at the end of this
README).

If you have a non-standard OpenSSL installation, you may need to set the
environment variable OPENSSL_ROOT_DIR (*e.g.* ,
"`export OPENSSL_ROOT_DIR=/usr/local/lib64`" if that is where it is
installed). Similarly, for non-standard (or multiple) GStreamer
installations, set the environment variable GSTREAMER_ROOT_DIR to the
directory that contains the ".../gstreamer-1.0/" directory of the
gstreamer installation that UxPlay should use (if this is *e.g.*
"\~/my_gstreamer/lib/gstreamer-1.0/", set this location with
"`export GSTREAMER_ROOT_DIR=$HOME/my_gstreamer/lib`").

-   Most users will use the GStreamer supplied by their distribution,
    but a few (in particular users of Raspberry Pi OS Lite Legacy
    (Buster) on a Raspberry Pi model 4B who wish to stay on that
    unsupported Legacy OS for compatibility with other apps) should
    instead build a newer Gstreamer from source following [these
    instructions](https://github.com/FDH2/UxPlay/wiki/Building-latest-GStreamer-from-source-on-distributions-with-older-GStreamer-(e.g.-Raspberry-Pi-OS-).)
    . **Do this *before* building UxPlay**.

In a terminal window, change directories to the source directory of the
downloaded source code ("UxPlay-\*", "\*" = "master" or the release tag
for zipfile downloads, "UxPlay" for "git clone" downloads), then follow
the instructions below:

**Note:** By default UxPlay will be built with optimization for the
computer it is built on; when this is not the case, as when you are
packaging for a distribution, use the cmake option
`-DNO_MARCH_NATIVE=ON`.

If you use X11 Windows on Linux or \*BSD, and wish to toggle in/out of
fullscreen mode with a keypress (F11 or Alt_L+Enter) UxPlay needs to be
built with a dependence on X11. Starting with UxPlay-1.59, this will be
done by default **IF** the X11 development libraries are installed and
detected. Install these with "`sudo apt install libx11-dev`". If
GStreamer \< 1.20 is detected, a fix needed by screen-sharing apps
(*e.g.*, Zoom) will also be made.

-   If X11 development libraries are present, but you wish to build
    UxPlay *without* any X11 dependence, use the cmake option
    `-DNO_X11_DEPS=ON`.

1.  `sudo apt install libssl-dev libplist-dev`". (*unless you need to
    build OpenSSL and libplist from source*).
2.  `sudo apt install libavahi-compat-libdnssd-dev`
3.  `sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev`.
    (\**Skip if you built Gstreamer from source*)
4.  `cmake .` (*For a cleaner build, which is useful if you modify the
    source, replace this by* "`mkdir build; cd build; cmake ..`": *you
    can then delete the contents of the `build` directory if needed,
    without affecting the source.*) Also add any cmake "`-D`" options
    here as needed (e.g, `-DNO_X11_DEPS=ON` or `-DNO_MARCH_NATIVE=ON`).
5.  `make`
6.  `sudo make install` (you can afterwards uninstall with
    `sudo make uninstall` in the same directory in which this was run).

This installs the executable file "`uxplay`" to `/usr/local/bin`, (and
installs a manpage to somewhere standard like
`/usr/local/share/man/man1` and README files to somewhere like
`/usr/local/share/doc/uxplay`). (If "man uxplay" fails, check if
\$MANPATH is set: if so, the path to the manpage (usually
/usr/local/share/man/) needs to be added to \$MANPATH .) The uxplay
executable can also be found in the build directory after the build
process, if you wish to test before installing (in which case the
GStreamer plugins must first be installed).

### Building on non-Debian Linux and \*BSD

\*\*For those with RPM-based distributions, a RPM spec file uxplay.spec
is also available: see [Building an installable rpm
package](#building-an-installable-rpm-package).

-   **Red Hat, or clones like CentOS (now continued as Rocky Linux or
    Alma Linux):** (sudo dnf install, or sudo yum install) openssl-devel
    libplist-devel avahi-compat-libdns_sd-devel gstreamer1-devel
    gstreamer1-plugins-base-devel (+libX11-devel for fullscreen X11)
    *(some of these may be in the "CodeReady" add-on repository, called
    "PowerTools" by clones)*

-   **Mageia, PCLinuxOS, OpenMandriva:** Same as Red Hat, except for
    name changes: (Mageia) "gstreamer1.0-devel",
    "gstreamer-plugins-base1.0-devel"; (OpenMandriva)
    "libopenssl-devel", "gstreamer-devel",
    "libgst-plugins-base1.0-devel". PCLinuxOS: same as Mageia, but uses
    synaptic (or apt) as its package manager.

-   **openSUSE:** (sudo zypper install) libopenssl-3-devel (formerly
    libopenssl-devel) libplist-2_0-devel (formerly libplist-devel)
    avahi-compat-mDNSResponder-devel gstreamer-devel
    gstreamer-plugins-base-devel (+ libX11-devel for fullscreen X11).

-   **Arch Linux** (*Also available as a package in AUR*): (sudo pacman
    -Syu) openssl libplist avahi gst-plugins-base.

-   **FreeBSD:** (sudo pkg install) libplist gstreamer1. Either
    avahi-libdns or mDNSResponder must also be installed to provide the
    dns_sd library. OpenSSL is already installed as a System Library.

-   **OpenBSD:** (doas pkg_add) libplist gstreamer1-plugins-base.
    avahi-libs must also be installed to provide the dns_sd library;
    (avahi-main must also be installed). OpenSSL is already installed as
    a System Library.

#### Building an installable RPM package

First-time RPM builders should first install the rpm-build and
rpmdevtools packages, then create the rpmbuild tree with
"`rpmdev-setuptree`". Then download and copy uxplay.spec into
`~/rpmbuild/SPECS`. In that directory, run
"`rpmdev-spectool -g -R  uxplay.spec`" to download the corresponding
source file `uxplay-*.tar.gz` into `~/rpmbuild/SOURCES`
("rpmdev-spectool" may also be just called "spectool"); then run
"`rpmbuild -ba uxplay.spec`" (you will need to install any required
dependencies this reports). This should create the uxplay RPM package in
a subdirectory of `~/rpmbuild/RPMS`. (**uxplay.spec** is tested on
Fedora 38, Rocky Linux 9.2, openSUSE Leap 15.5, Mageia 9, OpenMandriva,
PCLinuxOS; it can be easily modified to include dependency lists for
other RPM-based distributions.)

## Running UxPlay

### Installing plugins (Debian-based Linux distributions, including Ubuntu and Raspberry Pi OS) (*skip if you built a complete GStreamer from source*)

Next install the GStreamer plugins that are needed with
`sudo apt install gstreamer1.0-<plugin>`. Values of `<plugin>` required
are:

1.  "**plugins-base**"
2.  "**libav**" (for sound),
3.  "**plugins-good**" (for v4l2 hardware h264 decoding)
4.  "**plugins-bad**" (for h264 decoding).

**Debian-based distributions split some of the plugin packages into
smaller pieces:** some that may also be needed include "**gl**" for
OpenGL support (this provides the "-vs glimagesink" videosink, which can
be very useful in many systems (including Raspberry Pi), and should
always be used when using h264/h265 decoding by a NVIDIA GPU),
"**gtk3**" (which provides the "-vs gtksink" videosink), and "**x**" for
X11 support, although these may already be installed; "**vaapi**" is
needed for hardware-accelerated h264 video decoding by Intel or AMD
graphics (but not for use with NVIDIA using proprietary drivers). If
sound is not working, "**alsa**"","**pulseaudio**", or "**pipewire**"
plugins may need to be installed, depending on how your audio is set up.

-   Also install "**gstreamer1.0-tools**" to get the utility
    gst-inspect-1.0 for examining the GStreamer installation.

### Installing plugins (Non-Debian-based Linux or \*BSD) (*skip if you built a complete GStreamer from source*)

In some cases, because of patent issues, the libav plugin feature
**avdec_aac** needed for decoding AAC audio in mirror mode is not
provided in the official distribution: get it from community
repositories for those distributions.

-   **Red Hat, or clones like CentOS (now continued as Rocky Linux or
    Alma Linux):** Install gstreamer1-libav gstreamer1-plugins-bad-free
    (+ gstreamer1-vaapi for Intel/AMD graphics). In recent Fedora,
    gstreamer1-libav is renamed gstreamer1-plugin-libav. **To get
    avdec_aac, install packages from
    [rpmfusion.org](https://rpmfusion.org)**: (get ffmpeg-libs from
    rpmfusion; on RHEL or clones, but not recent Fedora, also get
    gstreamer1-libav from there).

-   **Mageia, PCLinuxOS, OpenMandriva:** Install gstreamer1.0-libav
    gstreamer1.0-plugins-bad (+ gstreamer1.0-vaapi for Intel/AMD
    graphics). **On Mageia, to get avdec_aac, install ffmpeg from the
    "tainted" repository**, (which also provides a more complete
    gstreamer1.0-plugins-bad).

-   **openSUSE:** Install gstreamer-plugins-libav gstreamer-plugins-bad
    (+ gstreamer-plugins-vaapi for Intel/AMD graphics). **To get
    avdec_aac, install libav\* packages for openSUSE from
    [Packman](https://ftp.gwdg.de/pub/linux/misc/packman/suse/)
    "Essentials"**; recommendation: after adding the Packman repository,
    use the option in YaST Software management to switch all system
    packages for multimedia to Packman).

-   **Arch Linux** Install gst-plugins-good gst-plugins-bad gst-libav (+
    gstreamer-vaapi for Intel/AMD graphics).

-   **FreeBSD:** Install gstreamer1-libav, gstreamer1-plugins,
    gstreamer1-plugins-\* (\* = core, good, bad, x, gtk, gl, vulkan,
    pulse, v4l2, ...), (+ gstreamer1-vaapi for Intel/AMD graphics).

-   **OpenBSD:** Install gstreamer1-libav, gstreamer-plugins-\* (\* =
    core, bad, base, good).

### Starting and running UxPlay

Since UxPlay-1.64, UxPlay can be started with options read from a
configuration file, which will be the first found of (1) a file with a
path given by environment variable `$UXPLAYRC`, (2) `~/.uxplayrc` in the
user's home directory ("\~"), (3) `~/.config/uxplayrc`. The format is
one option per line, omitting the initial `"-"` of the command-line
option. Lines in the configuration file beginning with `"#"` are treated
as comments and ignored.

**Run uxplay in a terminal window**. On some systems, you can specify
fullscreen mode with the `-fs` option, or toggle into and out of
fullscreen mode with F11 or (held-down left Alt)+Enter keys. Use Ctrl-C
(or close the window) to terminate it when done. If the UxPlay server is
not seen by the iOS client's drop-down "Screen Mirroring" panel, check
that your DNS-SD server (usually avahi-daemon) is running: do this in a
terminal window with `systemctl status avahi-daemon`. If this shows the
avahi-daemon is not running, control it with
`sudo systemctl [start,stop,enable,disable] avahi-daemon` (on
non-systemd systems, such as \*BSD, use
`sudo service avahi-daemon [status, start, stop, restart, ...]`). If
UxPlay is seen, but the client fails to connect when it is selected,
there may be a firewall on the server that prevents UxPlay from
receiving client connection requests unless some network ports are
opened: **if a firewall is active, also open UDP port 5353 (for mDNS
queries) needed by Avahi**. See [Troubleshooting](#troubleshooting)
below for help with this or other problems.

-   Unlike an Apple TV, the UxPlay server does not by default require
    clients to initially "pair" with it using a pin code displayed by
    the server (after which the client "trusts" the server, and does not
    need to repeat this). Since v1.67, Uxplay offers such
    "pin-authentication" as an option: see "`-pin`" and "`-reg`" in
    [Usage](#usage) for details, if you wish to use it. *Some clients
    with MDM (Mobile Device Management, often present on employer-owned
    devices) are required to use pin-authentication: UxPlay will provide
    this even when running without the pin option.* Password
    authentication (-pw *pwd*) is also offered as an alternative
    solution to pin codes: users need to know the password *pwd* and
    enter it on their iOS/macOS device to access UxPlay, when prompted
    (if *pwd* is not set, a displayed random pin code must be entered at
    **each** new connection.)

-   By default, UxPlay is locked to its current client until that client
    drops the connection; since UxPlay-1.58, the option `-nohold`
    modifies this behavior so that when a new client requests a
    connection, it removes the current client and takes over. UxPlay
    1.66 introduces a mechanism ( `-restrict`, `-allow <id>`,
    `-block <id>`) to control which clients are allowed to connect,
    using their "deviceID" (which in Apple devices appears to be
    immutable).

-   In Mirror mode, GStreamer has a choice of **two** methods to play
    video with its accompanying audio: prior to UxPlay-1.64, the video
    and audio streams were both played as soon as possible after they
    arrived (the GStreamer "*sync=false*" method), with a GStreamer
    internal clock used to try to keep them synchronized. **Starting
    with UxPlay-1.64, the other method (GStreamer's "*sync=true*" mode),
    which uses timestamps in the audio and video streams sent by the
    client, is the new default**. On low-decoding-power UxPlay hosts
    (such as Raspberry Pi Zero W or 3 B+ models) this will drop video
    frames that cannot be decoded in time to play with the audio, making
    the video jerky, but still synchronized.

The older method which does not drop late video frames worked well on
more powerful systems, and is still available with the UxPlay option
"`-vsync no`"; this method is adapted to "live streaming", and may be
better when using UxPlay as a second monitor for a Mac computer, for
example, while the new default timestamp-based method is best for
watching a video, to keep lip movements and voices synchronized.
(Without use of timestamps, video will eventually lag behind audio if it
cannot be decoded fast enough: hardware-accelerated video-decoding
helped to prevent this previously when timestamps were not being used.)

-   In Audio-only mode the GStreamer "sync=false" mode (not using
    timestamps) is still the default, but if you want to keep the audio
    playing on the server synchronized with the video showing on the
    client, use the `-async` timestamp-based option. (An example might
    be if you want to follow the Apple Music lyrics on the client while
    listening to superior sound on the UxPlay server). This delays the
    video on the client to match audio on the server, so leads to a
    slight delay before a pause or track-change initiated on the client
    takes effect on the audio played by the server.

AirPlay volume-control attenuates volume (gain) by up to -30dB: the
decibel range -30:0 can be rescaled from *Low*:0, or *Low*:*High*, using
the option `-db` ("-db *Low*" or "-db *Low*:*High*"), *Low* must be
negative. Rescaling is linear in decibels. Note that GStreamer's audio
format will "clip" any audio gain above +20db, so keep *High* below that
level. The option `-taper` provides a "tapered" AirPlay volume-control
profile some users may prefer.

The -vsync and -async options also allow an optional positive (or
negative) audio-delay adjustment in *milliseconds* for fine-tuning :
`-vsync 20.5` delays audio relative to video by 0.0205 secs; a negative
value advances it.)

-   you may find video is improved by the setting -fps 60 that allows
    some video to be played at 60 frames per second. (You can see what
    framerate is actually streaming by using -vs fpsdisplaysink, and/or
    -FPSdata.) When using this, you should use the default
    timestamp-based synchronization option `-vsync`.

-   You can now display (inside UxPlay) the accompanying "Cover Art"
    from sources like Apple Music in Audio-Only (ALAC) mode with the
    option `uxplay -ca`. *The older method of exporting cover art to an
    external viewer remains available: run "`uxplay -ca <name> &`" in
    the background, then run a image viewer with an autoreload feature:
    an example is "feh": run "`feh -R 1 <name>`" in the foreground;
    terminate feh and then Uxplay with "`ctrl-C fg ctrl-C`"*.

By default, GStreamer uses an algorithm to search for the best
"videosink" (GStreamer's term for a graphics driver to display images)
to use. You can overide this with the uxplay option `-vs <videosink>`.
Which videosinks are available depends on your operating system and
graphics hardware: use
"`gst-inspect-1.0 | grep sink | grep -e video -e Video -e image`" to see
what is available. Some possibilites on Linux/\*BSD are:

-   **glimagesink** (OpenGL), **waylandsink**

-   **xvimagesink**, **ximagesink** (X11)

-   **kmssink**, **fbdevsink** (console graphics without X11)

-   **vaapisink** (for Intel/AMD hardware-accelerated graphics); for
    NVIDIA hardware graphics (with CUDA) use **glimagesink** combined
    with "`-vd nvh264dec`" (or "nvh264sldec", a new variant which will
    become "nvh264dec" in GStreamer-1.24).

-   If the server is "headless" (no attached monitor, renders audio
    only) use `-vs 0`.

Note that videosink options can set using quoted arguments to -vs:
*e.g.*, `-vs "xvimagesink display=:0"`: ximagesink and xvimagesink allow
an X11 display name to be specified, and waylandsink has a similar
option. Videosink options ("properties") can be found in their GStreamer
description pages,such as
https://gstreamer.freedesktop.org/documentation/xvimagesink .

GStreamer also searches for the best "audiosink"; override its choice
with `-as <audiosink>`. Choices on Linux include pulsesink, alsasink,
pipewiresink, oss4sink; see what is available with
`gst-inspect-1.0 | grep sink | grep -e audio -e Audio`.

**One common problem involves GStreamer attempting to use
incorrectly-configured or absent accelerated hardware h264 video
decoding (e.g., VAAPI). Try "`uxplay -avdec`" to force software video
decoding; if this works you can then try to fix accelerated hardware
video decoding if you need it, or just uninstall the GStreamer vaapi
plugin.**

See [Usage](#usage) for more run-time options.

### **Special instructions for Raspberry Pi (tested on Raspberry Pi Zero 2 W, 3 Model B+, 4 Model B, and 5 only)**:

-   For Framebuffer video (for Raspberry Pi OS "Lite" and other non-X11
    distributions) use the KMS videosink "-vs kmssink" (the DirectFB
    framebuffer videosink "dfbvideosink" is broken on the Pi, and
    segfaults). *In this case you should explicitly use the "-vs
    kmssink" option, as without it, autovideosink does not find the
    correct videosink.*

-   Raspberry Pi 5 does not provide hardware H264 decoding (and does not
    need it).

-   Pi Zero 2 W, 3 Model B+ and 4 Model B should use hardware H264
    decoding by the Broadcom GPU, but it requires an out-of-mainstream
    kernel module bcm2835_codec maintained in the [Raspberry Pi kernel
    tree](https://github.com/raspberrypi/linux); distributions that are
    known to supply it include Raspberry Pi OS, Ubuntu, and
    Manjaro-RPi4. Use software decoding (option -avdec) if this module
    is not available.

-   Uxplay uses the Video4Linux2 (v4l2) plugin from GStreamer-1.22 and
    later to access the GPU, if hardware H264 decoding is used. This
    should happen automatically. The option -v4l2 can be used, but it is
    usually best to just let GStreamer find the best video pipeline by
    itself.

-   On older distributions (GStreamer \< 1.22), the v4l2 plugin needs a
    patch: see the [UxPlay
    Wiki](https://github.com/FDH2/UxPlay/wiki/Gstreamer-Video4Linux2-plugin-patches).
    Legacy Raspberry Pi OS (Bullseye) has a partially-patched
    GStreamer-1.18.4 which needs the uxplay option -bt709 (and don't use
    -v4l2); it is still better to apply the full patch from the UxPlay
    Wiki in this case.

-   **It appears that when hardware h264 video decoding is used, the
    option -bt709 became needed again in GStreamer-1.22 and later.**

-   For "double-legacy" Raspberry Pi OS (Buster), there is no patch for
    GStreamer-1.14. Instead, first build a complete newer
    GStreamer-1.18.6 from source using [these
    instructions](https://github.com/FDH2/UxPlay/wiki/Building-latest-GStreamer-from-source-on-distributions-with-older-GStreamer-(e.g.-Raspberry-Pi-OS-).)
    before building UxPlay.

-   Raspberry Pi 3 Model B+ running a 32 bit OS can also access the GPU
    with the GStreamer OMX plugin (use option "`-vd omxh264dec`"), but
    this is broken by Pi 4 Model B firmware. OMX support was removed
    from Raspberry Pi OS (Bullseye), but is present in Buster.

-   **H265 (4K)** video is potentially supported by hardware decoding on
    Raspberry Pi 5 models, as well as on Raspberry Pi 4 model B, using a
    dedicated HEVC decoding block, but the "rpivid" kernel driver for
    this is not yet supported by GStreamer (this driver decodes video
    into a non-standard format that cannot be supported by GStreamer
    until the driver is in the mainline Linux kernel). Raspberry Pi
    provides a version of ffmpeg that can use that format, but at
    present UxPlay cannot use this. The best solution would be for the
    driver to be "upstreamed" to the kernel, allowing GStreamer support.
    (Software HEVC decoding works, but does not seem to give
    satisfactory results on the Pi).

Even with GPU video decoding, some frames may be dropped by the
lower-power models to keep audio and video synchronized using
timestamps. In Legacy Raspberry Pi OS (Bullseye), raspi-config
"Performance Options" allows specifying how much memory to allocate to
the GPU, but this setting appears to be absent in Bookworm (but it can
still be set to e.g. 128MB by adding a line "gpu_mem=128" in
/boot/config.txt). A Pi Zero 2 W (which has 512MB memory) worked well
when tested in 32 bit Bullseye or Bookworm Lite with 128MB allocated to
the GPU (default seems to be 64MB).

The basic uxplay options for R Pi are `uxplay [-vs <videosink>]`. The
choice `<videosink>` = `glimagesink` is sometimes useful. With the
Wayland video compositor, use `<videosink>` = `waylandsink`. With
framebuffer video, use `<videosink>` = `kmssink`.

-   Tip: to start UxPlay on a remote host (such as a Raspberry Pi) using
    ssh:

```{=html}
<!-- -->
```
    ssh user@remote_host
           export DISPLAY=:0
           nohup uxplay [options] > FILE &

Sound and video will play on the remote host; "nohup" will keep uxplay
running if the ssh session is closed. Terminal output is saved to FILE
(which can be /dev/null to discard it)

## Building UxPlay on macOS: **(Intel X86_64 and "Apple Silicon" M1/M2 Macs)**

*Note: A native AirPlay Server feature is included in macOS since macOS
12 Monterey, but is restricted to recent hardware. As well as running on
latest macOS, UxPlay can run on older macOS systems that will cannot run
Monterey, or can run Monterey but not AirPlay.*

These instructions for macOS assume that the Xcode command-line
developer tools are installed (if Xcode is installed, open the Terminal,
type "sudo xcode-select --install" and accept the conditions).

It is also assumed that CMake \>= 3.13 is installed: this can be done
with package managers [MacPorts](http://www.macports.org)
(`sudo port install cmake`), [Homebrew](http://brew.sh)
(`brew install cmake`), or by a download from
<https://cmake.org/download/>. Also install `git` if you will use it to
fetch UxPlay.

Next install libplist and openssl-3.x. Note that static versions of
these libraries will be used in the macOS builds, so they can be
uninstalled after building uxplay, if you wish.

-   If you use Homebrew: `brew install libplist openssl@3`

-   if you use MacPorts: `sudo port install libplist-devel openssl3`

Otherwise, build libplist and openssl from source: see instructions near
the end of this README; requires development tools (autoconf, automake,
libtool, *etc.*) to be installed.

Next get the latest macOS release of GStreamer-1.0.

**Using "Official" GStreamer (Recommended for both MacPorts and Homebrew
users)**: install the GStreamer release for macOS from
<https://gstreamer.freedesktop.org/download/>. (This release contains
its own pkg-config, so you don't have to install one.) Install both the
gstreamer-1.0 and gstreamer-1.0-devel packages. After downloading,
Shift-Click on them to install (they install to
/Library/FrameWorks/GStreamer.framework). Homebrew or MacPorts users
should **not** install (or should uninstall) the GStreamer supplied by
their package manager, if they use the "official" release.

-   Since GStreamer v1.22, the "Official" (gstreamer.freedesktop.org)
    macOS binaries require a wrapper "gst_macos_main" around the actual
    main program (uxplay). This should have been applied during the
    UxPlay compilation process, and the initial UxPlay terminal message
    should confirm it is being used. (UxPlay can also be built using
    "Official" GStreamer v.1.20.7 binaries, which work without the
    wrapper.)

**Using Homebrew's GStreamer**: pkg-config is needed: ("brew install
pkg-config gstreamer"). This causes a large number of extra packages to
be installed by Homebrew as dependencies. The [Homebrew gstreamer
installation](https://formulae.brew.sh/formula/gstreamer#default) has
recently been reworked into a single "formula" named `gstreamer`, which
now works without needing GST_PLUGIN_PATH to be set in the enviroment.
Homebrew installs gstreamer to `HOMEBREW_PREFIX/lib/gstreamer-1.0` where
by default `HOMEBREW_PREFIX/*` is `/opt/homebrew/*` on Apple Silicon
Macs, and `/usr/local/*` on Intel Macs; do not put any extra
non-Homebrew plugins (that you build yourself) there, and instead set
GST_PLUGIN_PATH to point to their location (Homebrew does not supply a
complete GStreamer, but seems to have everything needed for UxPlay).
**New: the UxPlay build script will now also detect Homebrew
installations in non-standard locations indicated by the environment
variable `$HOMEBREW_PREFIX`.**

**Using GStreamer installed from MacPorts**: MacPorts is now providing
recent GStreamer releases: install pkgconf ("sudo port install
pkgconf"), then "sudo port install gstreamer1
gstreamer1-gst-plugins-base gstreamer1-gst-plugins-good
gstreamer1-gst-plugins-bad gstreamer1-gst-libav". (The following may no
longer be relevant: *For X11 support on macOS, compile UxPlay using a
special cmake option `-DUSE_X11=ON`, and run it from an XQuartz terminal
with -vs ximagesink; older non-retina macs require a lower resolution
when using X11: `uxplay -s 800x600`.)*

After installing GStreamer, build and install uxplay: open a terminal
and change into the UxPlay source directory ("UxPlay-master" for zipfile
downloads, "UxPlay" for "git clone" downloads) and build/install with
"cmake . ; make ; sudo make install" (same as for Linux).

-   Running UxPlay while checking for GStreamer warnings (do this with
    "export GST_DEBUG=2" before runnng UxPlay) reveals that with the
    default (since UxPlay 1.64) use of timestamps for video
    synchonization, many video frames are being dropped (only on macOS),
    perhaps due to another error (about videometa) that shows up in the
    GStreamer warnings. **Recommendation: use the UxPlay "no timestamp"
    option "`-vsync no`"** (you can add a line "vsync no" in the
    uxplayrc configuration file).

-   On macOS with this installation of GStreamer, the only videosinks
    available are glimagesink (default choice made by autovideosink) and
    osxvideosink. The window title does not show the Airplay server
    name, but the window can be shared on Zoom. Because of issues with
    glimagesink, you may find osxvideosink works better. The only
    available audiosink is osxaudiosink.

-   The option -nc is currently used by default on macOS, This is a
    workaround for window-closing problems with GStreamer videosinks on
    macOS. This option can be canceled with "-nc no", if not needed.

-   In the case of glimagesink, the resolution settings "-s wxh" may not
    affect the (small) initial OpenGL mirror window size, but the window
    can be expanded using the mouse or trackpad.

## Building UxPlay on Microsoft Windows, using MSYS2 with the MinGW-64 compiler.

-   tested on Windows 10 and 11, 64-bit.

1.  Download and install **Bonjour SDK for Windows v3.0**. You can
    download the SDK without any registration at
    [softpedia.com](https://www.softpedia.com/get/Programming/SDK-DDK/Bonjour-SDK.shtml),
    or get it from the official Apple site
    [https://developer.apple.com/download](https://developer.apple.com/download/all/?q=Bonjour%20SDK%20for%20Windows)
    (Apple makes you register as a developer to access it from their
    site). This should install the Bonjour SDK as
    `C:\Program Files\Bonjour SDK`.

2.  (This is for 64-bit Windows; a build for 32-bit Windows should be
    possible, but is not tested.) The unix-like MSYS2 build environment
    will be used: download and install MSYS2 from the official site
    [https://www.msys2.org/](https://www.msys2.org). Accept the default
    installation location `C:\mysys64`.

3.  [MSYS2 packages](https://packages.msys2.org/package/) are installed
    with a variant of the "pacman" package manager used by Arch Linux.
    Open a "MSYS2" terminal from the MSYS2 tab in the Windows Start
    menu, and update the new MSYS2 installation with "pacman -Syu".

    -   \_NEW: MSYS2 now recommends using the newer UCRT64 terminal
        environment (which uses the newer Microsoft UCRT "Universal C
        RunTime Library", included as part of the Windows OS since
        Windows 10) rather than the MINGW64 terminal environment (which
        uses the older Microsoft MSVCRT C library, which has "legacy"
        status, but is available on all Windows systems). If you wish to
        use the legacy MSVCRT library, to support older Windows
        versions, modify the instructions below as follows:

    (1) change the MSYS2 terminal type from UCRT64 to MINGW64; (2)
        modify mingw-w64-ucrt-x86_64-\* package names to
        mingw-w64-x86_64-\*, (just omit "-ucrt");
    (2) replace `ucrt64` by `mingw64` in directory names.\_

    Open a new MSYS2 UCRT64 terminal, and install the gcc compiler and
    cmake:

    `pacman -S mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-gcc`

    We will simply build UxPlay from the command line in the MSYS2
    environment (using "`ninja`" in place of "`make`" for the build
    system).

4.  Download the latest UxPlay from github **(to use `git`, install it
    with `pacman -S git`, then
    "`git clone https://github.com/FDH2/UxPlay`")**, then install UxPlay
    dependencies (openssl is already installed with MSYS2):

        `pacman -S mingw-w64-ucrt-x86_64-libplist mingw-w64-ucrt-x86_64-gstreamer mingw-w64-ucrt-x86_64-gst-plugins-base`

    If you are trying a different Windows build system, MSVC versions of
    GStreamer for Windows are available from the [official GStreamer
    site](https://gstreamer.freedesktop.org/download/), but only the
    MinGW 64-bit build on MSYS2 has been tested.

5.  cd to the UxPlay source directory, then "`mkdir build`" and
    "`cd build`". The build process assumes that the Bonjour SDK is
    installed at `C:\Program Files\Bonjour SDK`. If it is somewhere
    else, set the enviroment variable BONJOUR_SDK_HOME to point to its
    location. Then build UxPlay with

    `cmake ..`

    `ninja`

6.  Assuming no error in either of these, you will have built the uxplay
    executable **uxplay.exe** in the current ("build") directory. The
    "sudo make install" and "sudo make uninstall" features offered in
    the other builds are not available on Windows; instead, you can
    install the uxplay.exe executable in `C:/msys64/ucrt64/bin` (plus
    manpage and documentation in `C:/msys64/ucrt64/share/...`) with

    `cmake --install . --prefix $HOME/../../ucrt64`

    You can later uninstall uxplay by returning to the build directory
    and running

    `ninja uninstall`

    (This assumes that certain files in the build directory were not
    deleted since building UxPlay).

    To be able to view the manpage, you need to install the manpage
    viewer with "`pacman -S man`".

To run **uxplay.exe** you need to install some gstreamer plugin packages
with `pacman -S mingw-w64-ucrt-x86_64-gst-<plugin>`, where the required
ones have `<plugin>` given by

1.  **libav**
2.  **plugins-good**
3.  **plugins-bad**

Other possible MSYS2 gstreamer plugin packages you might use are listed
in [MSYS2 packages](https://packages.msys2.org/package/).

You also will need to grant permission to the uxplay executable
uxplay.exe to access data through the Windows firewall. You may
automatically be offered the choice to do this when you first run
uxplay, or you may need to do it using **Windows Settings-\>Update and
Security-\>Windows Security-\>Firewall & network protection -\> allow an
app through firewall**. If your virus protection flags uxplay.exe as
"suspicious" (but without a true malware signature) you may need to give
it an exception.

Now test by running "`uxplay`" (in a MSYS2 UCRT64 terminal window. If
you need to specify the audiosink, there are two main choices on
Windows: the older DirectSound plugin "`-as directsoundsink`", and the
more modern Windows Audio Session API (wasapi) plugin
"`-as wasapisink`", which supports [additional
options](https://gstreamer.freedesktop.org/documentation/wasapi/wasapisink.html)
such as

    uxplay -as 'wasapisink device=\"<guid>\"' 

where `<guid>` specifies an available audio device by its GUID, which
can be found using "`gst-device-monitor-1.0 Audio`": `<guid>` has a form
like `\{0.0.0.00000000\}.\{98e35b2b-8eba-412e-b840-fd2c2492cf44\}`. If
"`device`" is not specified, the default audio device is used.

If you wish to specify the videosink using the `-vs <videosink>` option,
some choices for `<videosink>` are `d3d12videosink`, `d3d11videosink`,
`d3dvideosink`, `glimagesink`, `gtksink`, `autovideosink`. If you do not
specify the videosink, the d3d11videosink will be used (users have
reported segfaults of the newer d3d12 videodecoder on certain older
Nvidia cards when the image resolution changes: d3d11 will used by
default until this is fixed).

-   With Direct3D 11.0 or greater, various options can be set using
    e.g. `-vs "d3d11videosink <options>"` (see the gstreamer videosink
    documentation for these videosinks). For convenience, if no
    `<options>` are set, the option to toggle in and out of fullscreen
    mode with the Alt-Enter key combination is added.

The executable uxplay.exe can also be run without the MSYS2 environment,
in the Windows Terminal, with `C:\msys64\ucrt64\bin\uxplay`.

# Usage

Options:

-   These can also be written (one option per line, without the initial
    "`-`" character) in the UxPlay startup file (either given by
    environment variable `$UXPLAYRC`, or `~/.uxplayrc` or
    `~/.config/uxplayrc`); lines begining with "`#`" are treated as
    comments, and ignored. Command line options supersede options in the
    startup file.

**-rc *file*** can also be used to specify the startup file location:
this overrides `$UXPLAYRC`, `~/.uxplayrc`, etc.

**-n server_name** (Default: UxPlay); server_name@\_hostname\_ will be
the name that appears offering AirPlay services to your iPad, iPhone
etc, where *hostname* is the name of the server running uxplay. This
will also now be the name shown above the mirror display (X11) window.

**-nh** Do not append "@_hostname_" at the end of the AirPlay server
name.

**-h265** Activate "ScreenMultiCodec" support (AirPlay "Features" bit
42) for accepting h265 (4K/HEVC) video in addition to h264 video (1080p)
in screen-mirror mode. When this option is used, two "video pipelines"
(one for h264, one for h265) are created. If any GStreamer plugins in
the pipeline are specific for h264 or h265, the correct version will be
used in each pipeline. A wired Client-Server ethernet connection is
preferred over Wifi for 4K video, and might be required by the client.
Only recent Apple devices (M1/M2 Macs or iPads, and some iPhones) can
send h265 video if a resolution "-s wxh" with h \> 1080 is requested.
The "-h265" option changes the default resolution ("-s" option) from
1920x1080 to 3840x2160, and leaves default maximum framerate ("-fps"
option) at 30fps.

**-hls \[v\]** Activate HTTP Live Streaming support. With this option
YouTube videos can be streamed directly from YouTube servers to UxPlay
(without passing through the client) by clicking on the AirPlay icon in
the YouTube app. Optional \[v\] (allowed values 2 or 3, default: 3)
allows selection of the version of GStreamer's \"playbin\" video player
to use for playing HLS video. *(Playbin v3 is the recommended player,
but if some videos fail to play, you can try with version 2.)*

**-pin \[nnnn\]**: (since v1.67) use Apple-style (one-time) "pin"
authentication when a new client connects for the first time: a
four-digit pin code is displayed on the terminal, and the client screen
shows a login prompt for this to be entered. When "-pin" is used by
itself, a new random pin code is chosen for each authentication; if
"-pin nnnn" (e.g., "-pin 3939") is used, this will set an unchanging
fixed code. Authentication adds the server to the client's list of
"trusted servers" and the client will not need to reauthenticate
provided that the client and server public keys remain unchanged. (By
default since v1.68, the server public key is generated from the MAC
address, which can be changed with the -m option; see the -key option
for an alternative method of key generation). *(Add a line "pin" in the
UxPlay startup file if you wish the UxPlay server to use the pin
authentication protocol).*

**-reg \[*filename*\]**: (since v1.68). If "-pin" is used, this option
maintains a register of pin-authenticated "trusted clients" in
\$HOME/.uxplay.register (or optionally, in *filename*). Without this
option, returning clients that skip pin-authentication are trusted and
not checked. This option may be useful if UxPlay is used in a more
public environment, to record client details; the register is text, one
line per client, with client's public key (base-64 format), Device ID,
and Device name; commenting out (with "\#") or deleting a line
deregisters the corresponding client (see options -restrict, -block,
-allow for more ways to control client access). *(Add a line "reg" in
the startup file if you wish to use this feature.)*

**-pw** \[*pwd*\]. (since 1.72). As an alternative to -pin, client
access can be controlled with a password. If a password *pwd* (of length
at least six characters) is set when uxplay starts (usually set in the
startup file, where it is stored as cleartext), all users must know this
password to connect to UxPlay (the client prompts for it). This method
uses HTTP md5 Digest authentication, which is now regarded as providing
weak security, but it is only used to validate the uxplay password, and
no user credentials are exposed. After a successful authentication, the
client stores the password, and will use it initially for future
authentications without prompting, so long as the UxPlay deviceID has
not changed (this initial authentication will fail if the UxPlay
password has changed). If *pwd* is **not** specified with the -pw option
when UxPlay starts, a new random 4-digit pin code is generated and
displayed on the UxPlay terminal for **each** new connection, and must
be entered on the client (there are three chances to enter it, before it
is changed). *Note: -pin and -pw are alternatives: if both are specified
at startup, the earlier of these two options is discarded.*

**-vsync \[x\]** (In Mirror mode:) this option (**now the default**)
uses timestamps to synchronize audio with video on the server, with an
optional audio delay in (decimal) milliseconds (*x* = "20.5" means
0.0205 seconds delay: positive or negative delays less than a second are
allowed.) It is needed on low-power systems such as Raspberry Pi without
hardware video decoding.

**-vsync no** (In Mirror mode:) this switches off timestamp-based
audio-video synchronization, restoring the default behavior prior to
UxPlay-1.64. Standard desktop systems seem to work well without use of
timestamps: this mode is appropriate for "live streaming" such as using
UxPlay as a second monitor for a mac computer, or monitoring a webcam;
with it, no video frames are dropped.

**-async \[x\]** (In Audio-Only (ALAC) mode:) this option uses
timestamps to synchronize audio on the server with video on the client,
with an optional audio delay in (decimal) milliseconds (*x* = "20.5"
means 0.0205 seconds delay: positive or negative delays less than a
second are allowed.) Because the client adds a video delay to account
for latency, the server in -async mode adds an equivalent audio delay,
which means that audio changes such as a pause or a track-change will
not take effect immediately. *This might in principle be mitigated by
using the `-al` audio latency setting to change the latency (default
0.25 secs) that the server reports to the client, but at present
changing this does not seem to have any effect*.

**-async no**. This is the still the default behavior in Audio-only
mode, but this option may be useful as a command-line option to switch
off a `-async` option set in a "uxplayrc" configuration file.

**-db *low*\[:*high*\]** Rescales the AirPlay volume-control attenuation
(gain) from -30dB:0dB to *low*:0dB or *low*:*high*. The lower limit
*low* must be negative (attenuation); the upper limit *high* can be
either sign. (GStreamer restricts volume-augmentation by *high* so that
it cannot exceed +20dB). The rescaling is "flat", so that for -db
-50:10, a change in Airplay attenuation by -7dB is translated to a -7 x
(60/30) = -14dB attenuation, and the maximum volume (AirPlay 0dB) is a
10dB augmentation, and Airplay -30dB would become -50dB. Note that the
minimum AirPlay value (-30dB exactly) is translated to "mute".

**-taper** Provides a "tapered" Airplay volume-control profile (matching
the one called "dasl-tapering" in
[shairport-sync](https://github.com/mikebrady/shairport-sync)): each
time the length of the volume slider (or the number of steps above mute,
where 16 steps = full volume) is reduced by 50%, the perceived volume is
halved (a 10dB attenuation). (This is modified at low volumes, to use
the "untapered" volume if it is louder.)

**-vol *v*** Sets initial audio-streaming volume (on client): range is
\[0:1\], with 0.0 = mute, 1.0 = full volume (*v* is a decimal number).

**-s wxh** e.g. -s 1920x1080 (= "1080p"), the default width and height
resolutions in pixels for h264 video. (The default becomes 3840x2160 (=
"4K") when the -h265 option is used.) This is just a request made to the
AirPlay client, and perhaps will not be the final resolution you get. w
and h are whole numbers with four digits or less. Note that the
**height** pixel size is the controlling one used by the client for
determining the streaming format; the width is dynamically adjusted to
the shape of the image (portrait or landscape format, depending on how
an iPad is held, for example).

**-s wxh@r** As above, but also informs the AirPlay client about the
screen refresh rate of the display. Default is r=60 (60 Hz); r must be a
whole number less than 256.

**-o** turns on an "overscanned" option for the display window. This
reduces the image resolution by using some of the pixels requested by
option -s wxh (or their default values 1920x1080) by adding an empty
boundary frame of unused pixels (which would be lost in a full-screen
display that overscans, and is not displayed by gstreamer).
Recommendation: **don't use this option** unless there is some special
reason to use it.

**-fs** uses fullscreen mode, but currently only works with X11,
Wayland, VAAPI, kms and D3D11 (Windows).

**-p** allows you to select the network ports used by UxPlay (these need
to be opened if the server is behind a firewall). By itself, -p sets
"legacy" ports TCP 7100, 7000, 7001, UDP 6000, 6001, 7011. -p n (e.g. -p
35000) sets TCP and UDP ports n, n+1, n+2. -p n1,n2,n3 (comma-separated
values) sets each port separately; -p n1,n2 sets ports n1,n2,n2+1. -p
tcp n or -p udp n sets just the TCP or UDP ports. Ports must be in the
range \[1024-65535\].

If the -p option is not used, the ports are chosen dynamically
(randomly), which will not work if a firewall is running.

**-avdec** forces use of software h264 decoding using Gstreamer element
avdec_h264 (libav h264 decoder). This option should prevent
autovideosink choosing a hardware-accelerated videosink plugin such as
vaapisink.

**-vp *parser*** choses the GStreamer pipeline's h264 parser element,
default is h264parse. Using quotes "..." allows options to be added.

**-vd *decoder*** chooses the GStreamer pipeline's h264 decoder element,
instead of the default value "decodebin" which chooses it for you.
Software decoding is done by avdec_h264; various hardware decoders
include: vaapih264dec, nvdec, nvh264dec, v4l2h264dec (these require that
the appropriate hardware is available). Using quotes "..." allows some
parameters to be included with the decoder name.

**-vc *converter*** chooses the GStreamer pipeline's videoconverter
element, instead of the default value "videoconvert". When using
Video4Linux2 hardware-decoding by a GPU,`-vc  v4l2convert` will also use
the GPU for video conversion. Using quotes "..." allows some parameters
to be included with the converter name.

**-vs *videosink*** chooses the GStreamer videosink, instead of the
default value "autovideosink" which chooses it for you. Some videosink
choices are: ximagesink, xvimagesink, vaapisink (for intel graphics),
gtksink, glimagesink, waylandsink, osxvideosink (for macOS), kmssink
(for systems without X11, like Raspberry Pi OS lite) or fpsdisplaysink
(which shows the streaming framerate in fps). Using quotes "..." allows
some parameters to be included with the videosink name. For example,
**fullscreen** mode is supported by the vaapisink plugin, and is
obtained using `-vs "vaapisink fullscreen=true"`; this also works with
`waylandsink`. The syntax of such options is specific to a given plugin
(see GStreamer documentation), and some choices of videosink might not
work on your system.

**-vs 0** suppresses display of streamed video. In mirror mode, the
client's screen is still mirrored at a reduced rate of 1 frame per
second, but is not rendered or displayed. This option should always be
used if the server is "headless" (with no attached screen to display
video), and only used to render audio, which will be AAC
lossily-compressed audio in mirror mode with unrendered video, and
superior-quality ALAC Apple Lossless audio in Airplay audio-only mode.

**-v4l2** Video settings for hardware h264 video decoding in the GPU by
Video4Linux2. Equivalent to `-vd v4l2h264dec -vc v4l2convert`.

**-bt709** A workaround for the failure of the older Video4Linux2 plugin
to recognize Apple's use of an uncommon (but permitted) "full-range
color" variant of the bt709 color standard for digital TV. This was no
longer needed by GStreamer-1.20.4 and backports from it, but appears to
again be required in GStreamer-1.22 and later.

**-srgb** A workaround for a failure to display full-range 8-bit color
\[0-255\], and instead restrict to limited range \[16-235\] "legal
BT709" HDTV format. The workaround works on x86_64 desktop systems, but
does not yet work on Raspberry Pi. The issue may be fixed in a future
GStreamer release: it only occurs in Linux and \*BSD.

**-srbg no**. Disables the -srgb option, which is enabled by default in
Linux and \*BSD, but may be useless on Raspberry Pi, and may be
unwanted, as it adds extra processing load.

**-rpi** Equivalent to "-v4l2" (Not valid for Raspberry Pi model 5, and
removed in UxPlay 1.67)

**-rpigl** Equivalent to "-rpi -vs glimagesink". (Removed since UxPlay
1.67)

**-rpifb** Equivalent to "-rpi -vs kmssink" (Removed since UxPlay 1.67)

**-rpiwl** Equivalent to "-rpi -vs waylandsink". (Removed since UxPlay
1.67)

**-as *audiosink*** chooses the GStreamer audiosink, instead of letting
autoaudiosink pick it for you. Some audiosink choices are: pulsesink,
alsasink, pipewiresink, osssink, oss4sink, jackaudiosink, osxaudiosink
(for macOS), wasapisink, directsoundsink (for Windows). Using quotes
"..." might allow some optional parameters
(e.g. `-as "alsasink device=..."` to specify a non-default output
device). The syntax of such options is specific to a given plugin (see
GStreamer documentation), and some choices of audiosink might not work
on your system.

**-as 0** (or just **-a**) suppresses playing of streamed audio, but
displays streamed video.

**-al *x*** specifies an audio latency *x* in (decimal) seconds in
Audio-only (ALAC), that is reported to the client. Values in the range
\[0.0, 10.0\] seconds are allowed, and will be converted to a whole
number of microseconds. Default is 0.25 sec (250000 usec). *(However,
the client appears to ignore this reported latency, so this option seems
non-functional.)*

**-ca** (without specifying a filename) now displays "cover art" that
accompanies Apple Music when played in "Audio-only" (ALAC) mode.

**-ca *filename*** provides a file (where *filename* can include a full
path) used for output of "cover art" (from Apple Music, *etc.*,) in
audio-only ALAC mode. This file is overwritten with the latest cover art
as it arrives. Cover art (jpeg format) is discarded if this option is
not used. Use with a image viewer that reloads the image if it changes,
or regularly (*e.g.* once per second.). To achieve this, run
"`uxplay -ca [path/to/]filename &`" in the background, then run the the
image viewer in the foreground. Example, using `feh` as the viewer: run
"`feh -R 1 [path/to/]filename`" (in the same terminal window in which
uxplay was put into the background). To quit, use `ctrl-C fg ctrl-C` to
terminate the image viewer, bring `uxplay` into the foreground, and
terminate it too.

**-md *filename*** Like the -ca option, but exports audio metadata text
(Artist, Title, Genre, etc.) to file for possible display by a process
that watches the file for changes. Previous text is overwritten as new
metadata is received, and the file is deleted when uxplay terminates.

**-reset n** sets a limit of *n* consecutive failures of the client to
send feedback requests (these "heartbeat signals" are sent by the client
once per second to ask for a response showing that the server is still
online). After *n* missing signals, the client will be presumed to be
offline, and the connection will be reset to allow a new connection. The
default value of *n* is 15 seconds; the value *n* = 0 means "no limit".

**-nofreeze** closes the video window after a reset due to client going
offline (default is to leave window open to allow a smoother reconection
to the same client). This option may be useful in fullscreen mode.

**-nc** maintains previous UxPlay \< 1.45 behavior that does **not
close** the video window when the the client sends the "Stop Mirroring"
signal. *This option is currently used by default in macOS, as the
window created in macOS by GStreamer does not terminate correctly (it
causes a segfault) if it is still open when the GStreamer pipeline is
closed.*

**-nohold** Drops the current connection when a new client attempts to
connect. Without this option, the current client maintains exclusive
ownership of UxPlay until it disconnects.

**-restrict** Restrict clients allowed to connect to those specified by
`-allow <deviceID>`. The deviceID has the form of a MAC address which is
displayed by UxPlay when the client attempts to connect, and appears to
be immutable. It has the format `XX:XX:XX:XX:XX:XX`, X = 0-9,A-F, and is
possibly the "true" hardware MAC address of the device. Note that iOS
clients generally expose different random "private Wi_Fi addresses"
("fake" MAC addresses) to different networks (for privacy reasons, to
prevent tracking), which may change, and do not correpond to the
deviceID.

**-restrict no** Remove restrictions (default). This is useful as a
command-line argument to overide restrictions set in the Startup file.

**-allow *id*** Adds the deviceID = *id* to the list of allowed clients
when client restrictions are being enforced. Usually this will be an
entry in the uxplayrc startup file.

**-block *id*** Always block clients with deviceID = *id*, even when
client restrictions are not being enforced generally. Usually this will
be an entry in the uxplayrc startup file.

**-FPSdata** Turns on monitoring of regular reports about video
streaming performance that are sent by the client. These will be
displayed in the terminal window if this option is used. The data is
updated by the client at 1 second intervals.

**-fps n** sets a maximum frame rate (in frames per second) for the
AirPlay client to stream video; n must be a whole number less than 256.
(The client may choose to serve video at any frame rate lower than this;
default is 30 fps.) A setting of 60 fps may give you improved video but
is not recommended on Raspberry Pi. A setting below 30 fps might be
useful to reduce latency if you are running more than one instance of
uxplay at the same time. *This setting is only an advisory to the client
device, so setting a high value will not force a high framerate.* (You
can test using "-vs fpsdisplaysink" to see what framerate is being
received, or use the option -FPSdata which displays video-stream
performance data continuously sent by the client during
video-streaming.)

**-f {H\|V\|I}** implements "videoflip" image transforms: H = horizontal
flip (right-left flip, or mirror image); V = vertical flip ; I = 180
degree rotation or inversion (which is the combination of H with V).

**-r {R\|L}** 90 degree Right (clockwise) or Left (counter-clockwise)
rotations; these image transforms are carried out after any **-f**
transforms.

**-m \[mac\]** changes the MAC address (Device ID) used by UxPlay
(default is to use the true hardware MAC address reported by the host
computer's network card). (Different server_name, MAC addresses, and
network ports are needed for each running uxplay if you attempt to run
more than one instance of uxplay on the same computer.) If \[mac\] (in
form xx:xx:xx:xx:xx:xx, 6 hex octets) is not given, a random MAC address
is generated. If UxPlay fails to find the true MAC address of a network
card, (more specifically, the MAC address used by the first active
network interface detected) a random MAC address will be used even if
option **-m** was not specified. (Note that a random MAC address will be
different each time UxPlay is started).

**-key \[*filename*\]**: This (more secure) option for generating and
storing a persistant public key (needed for the -pin option) has been
replaced by default with a (less secure) method which generates a key
from the server's "device ID" (MAC address, which can be changed with
the -m option, conveniently as a startup file option). When the -key
option is used, a securely generated keypair is generated and stored in
`$HOME/.uxplay.pem`, if that file does not exist, or read from it, if it
exists. (Optionally, the key can be stored in *filename*.) This method
is more secure than the new default method, (because the Device ID is
broadcast in the DNS_SD announcement) but still leaves the private key
exposed to anyone who can access the pem file. This option should be set
in the UxPlay startup file as a line "key" or "key *filename*" (no
initial "-"), where *filename* is a full path which should be enclosed
in quotes (`"...."`) if it contains any blank spaces. **Because the
default method is simpler, and security of client access to uxplay is
unlikely to be an important issue, the -key option is no longer
recommended**.

**-dacp \[*filename*\]**: Export current client DACP-ID and
Active-Remote key to file: default is \$HOME/.uxplay.dacp. (optionally
can be changed to *filename*). Can be used by remote control
applications. File is transient: only exists while client is connected.

**-vdmp** Dumps h264 video to file videodump.h264. -vdmp n dumps not
more than n NAL units to videodump.x.h264; x= 1,2,... increases each
time a SPS/PPS NAL unit arrives. To change the name *videodump*, use
-vdmp \[n\] *filename*.

**-admp** Dumps audio to file audiodump.x.aac (AAC-ELD format audio),
audiodump.x.alac (ALAC format audio) or audiodump.x.aud (other-format
audio), where x = 1,2,3... increases each time the audio format changes.
-admp *n* restricts the number of packets dumped to a file to *n* or
less. To change the name *audiodump*, use -admp \[n\] *filename*. *Note
that (unlike dumped video) the dumped audio is currently only useful for
debugging, as it is not containerized to make it playable with standard
audio players.*

**-d \[n\]** Enable debug output; optional argument n=1 suppresses
audio/video packet data in debug output. Note: this does not show
GStreamer error or debug messages. To see GStreamer error and warning
messages, set the environment variable GST_DEBUG with "export
GST_DEBUG=2" before running uxplay. To see GStreamer information
messages, set GST_DEBUG=4; for DEBUG messages, GST_DEBUG=5; increase
this to see even more of the GStreamer inner workings.

# Troubleshooting

Note: `uxplay` is run from a terminal command line, and informational
messages are written to the terminal.

### 0. Problems in compiling UxPlay.

One user (on Ubuntu) found compilation failed with messages about
linking to "usr/local/lib/libcrypto.a" and "zlib". This was because (in
addition to the standard ubuntu installation of libssl-dev), the user
was unaware that a second installation with libcrypto in /usr/local was
present. Solution: when more than one installation of OpenSSL is
present, set the environment variable OPEN_SSL_ROOT_DIR to point to the
correct one; on 64-bit Ubuntu, this is done by running
`export OPENSSL_ROOT_DIR=/usr/lib/X86_64-linux-gnu/` before running
cmake.

### 1. **Avahi/DNS_SD Bonjour/Zeroconf issues**

The DNS_SD Service-Discovery ("Bonjour" or "Zeroconf") service is
required for UxPlay to work. On Linux, it will be usually provided by
Avahi, and to troubleshoot this, you should use the tool `avahi-browse`.
(You may need to install a separate package with a name like
`avahi-utils` to get this.)

On Linux, make sure Avahi is installed, and start the avahi-daemon
service on the system running uxplay (your distribution will document
how to do this, for example: `sudo systemctl <cmd> avahi-daemon` or
`sudo service avahi-daemon <cmd>`, with `<cmd>` one of enable, disable,
start, stop, status. You might need to edit the avahi-daemon.conf file
(it is typically in /etc/avahi/, find it with
"`sudo find /etc -name avahi-daemon.conf`"): make sure that
"disable-publishing" is **not** a selected option). Some systems may
instead use the mdnsd daemon as an alternative to provide DNS-SD
service. (FreeBSD offers both alternatives, but only Avahi was tested;
see [here](https://gist.github.com/reidransom/6033227).)

-   **uxplay starts, but either stalls or stops after "Initialized
    server socket(s)" appears (*without the server name showing on the
    client*)**.

If UxPlay stops with the "No DNS-SD Server found" message, this means
that your network **does not have a running Bonjour/zeroconf DNS-SD
server.** Before v1.60, UxPlay used to stall silently if DNS-SD service
registration failed, but now stops with an error message returned by the
DNSServiceRegister function: kDNSServiceErr_Unknown if no DNS-SD server
was found: *(A NixOS user found that in NixOS, this error can also occur
if avahi-daemon service IS running with publishing enabled, but reports
"the error disappeared on NixOS by setting services.avahi.openFirewall
to true".)* Other mDNS error codes are in the range FFFE FF00 (-65792)
to FFFE FFFF (-65537), and are listed in the dnssd.h file. An older
version of this (the one used by avahi) is found
[here](https://github.com/lathiat/avahi/blob/master/avahi-compat-libdns_sd/dns_sd.h).
A few additional error codes are defined in a later version from
[Apple](https://opensource.apple.com/source/mDNSResponder/mDNSResponder-544/mDNSShared/dns_sd.h.auto.html).

If UxPlay stalls *without an error message* and *without the server name
showing on the client*, **this is a network problem** (if your UxPlay
version is older than 1.60, it is also the behavior when no DNS-SD
server is found.)

A useful tool for examining such network problems from the client end is
the (free) Discovery DNS-SD browser [available in the Apple App
Store](https://apps.apple.com/us/developer/lily-ballard/id305441020) for
both iOS (works on iPadOS too) and macOS.

-   Some users using dual-band (2.4GHz/5GHz) routers have reported that
    clients using the 5GHz band (sometimes) "fail to see UxPlay" (i.e.,
    do not get a response to their mDNS queries), but the 2.4GHz band
    works. Other projects using Bonjour/mDNS have had similar reports;
    the issue seems to be router-specific, perhaps related to "auto"
    rather than fixed channel selection (5GHz has many more channels to
    switch between), or channel width selections; one speculation is
    that since mDNS uses UDP protocol (where "lost" messages are not
    resent), a mDNS query might get lost if channel switching occurs
    during the query.

If your router has this problem, a reported "fix" is to (at least on
5GHz) use fixed channel and/or fixed (not dynamic) channel width.

-   **Avahi works at first, but new clients do not see UxPlay, or
    clients that initially saw it stop doing so after they disconnect**.

This is usually because Avahi is only using the "loopback" network
interface, and is not receiving mDNS queries from new clients that were
not listening when UxPlay started.

To check this, after starting uxplay, use the utility
`avahi-browse -a -t` **in a different terminal window** on the server to
verify that the UxPlay AirTunes and AirPlay services are correctly
registered (only the AirTunes service is used in the "Legacy" AirPlay
Mirror mode used by UxPlay, but the AirPlay service is used for the
initial contact).

The results returned by avahi-browse should show entries for uxplay like

    +   eno1 IPv6 UxPlay                                        AirPlay Remote Video local
    +   eno1 IPv4 UxPlay                                        AirPlay Remote Video local
    +     lo IPv4 UxPlay                                        AirPlay Remote Video local
    +   eno1 IPv6 863EA27598FE@UxPlay                           AirTunes Remote Audio local
    +   eno1 IPv4 863EA27598FE@UxPlay                           AirTunes Remote Audio local
    +     lo IPv4 863EA27598FE@UxPlay                           AirTunes Remote Audio local

If only the loopback ("lo") entries are shown, a firewall on the UxPlay
host is probably blocking full DNS-SD service, and you need to open the
default UDP port 5353 for mDNS requests, as loopback-based DNS-SD
service is unreliable.

If the UxPlay services are listed by avahi-browse as above, but are not
seen by the client, the problem is likely to be a problem with the local
network.

### 2. uxplay starts, but stalls after "Initialized server socket(s)" appears, *with the server name showing on the client* (but the client fails to connect when the UxPlay server is selected).

This shows that a *DNS-SD* service is working, clients hear UxPlay is
available, but the UxPlay server is not receiving the response from the
client. This is usually because a firewall on the server is blocking the
connection request from the client. (One user who insisted that the
firewall had been turned off turned out to have had *two* active
firewalls (*firewalld* and *ufw*) *both* running on the server!) If
possible, either turn off the firewall to see if that is the problem, or
get three consecutive network ports, starting at port n, all three in
the range 1024-65535, opened for both tcp and udp, and use "uxplay -p n"
(or open UDP 7011,6001,6000 TCP 7100,7000,7001 and use "uxplay -p").

If you are *really* sure there is no firewall, you may need to
investigate your network transmissions with a tool like netstat, but
almost always this is a firewall issue.

### 3. Problems *after* the client-server connection has been made:

If you do *not* see the message `raop_rtp_mirror starting mirroring`,
something went wrong before the client-server negotiations were
finished. For such problems, use "uxplay -d" (debug log option) to see
what is happening: it will show how far the connection process gets
before the failure occurs. You can compare your debug output to that
from a successful start of UxPlay in the [UxPlay
Wiki](https://github.com/FDH2/UxPlay/wiki).

**If UxPlay reports that mirroring started, but you get no video or
audio, the problem is probably from a GStreamer plugin that doesn't work
on your system** (by default, GStreamer uses the "autovideosink" and
"autoaudiosink" algorithms to guess what are the "best" plugins to use
on your system). A different reason for no audio occurred when a user
with a firewall only opened two udp network ports: **three** are
required (the third one receives the audio data).

**Raspberry Pi** devices (*Pi 4B+ and earlier: this does not apply to
the Pi 5, which does not provide hardware h264 decoding, and does not
need it*) work best with hardware GPU h264 video decoding if the
Video4Linux2 plugin in GStreamer v1.20.x or earlier has been patched
(see the UxPlay
[Wiki](https://github.com/FDH2/UxPlay/wiki/Gstreamer-Video4Linux2-plugin-patches)
for patches). This is fixed in GStreamer-1.22, and by backport patches
from this in distributions such as Raspberry Pi OS (Bullseye): **use
option `-bt709` with the GStreamer-1.18.4 from Raspberry Pi OS**. This
also needs the bcm2835-codec kernel module that is not in the standard
Linux kernel (it is available in Raspberry Pi OS, Ubuntu and Manjaro).

-   **If this kernel module is not available in your Raspberry Pi
    operating system, or if GStreamer \< 1.22 is not patched, use option
    `-avdec` for software h264-decoding.**

Sometimes "autovideosink" may select the OpenGL renderer "glimagesink"
which may not work correctly on your system. Try the options "-vs
ximagesink" or "-vs xvimagesink" to see if using one of these fixes the
problem.

Other reported problems are connected to the GStreamer VAAPI plugin (for
hardware-accelerated Intel graphics, but not NVIDIA graphics). Use the
option "-avdec" to force software h264 video decoding: this should
prevent autovideosink from selecting the vaapisink videosink.
Alternatively, find out if the gstreamer1.0-vaapi plugin is installed,
and if so, uninstall it. (If this does not fix the problem, you can
reinstall it.)

There are some reports of other GStreamer problems with
hardware-accelerated Intel HD graphics. One user (on Debian) solved this
with "sudo apt install intel-media-va-driver-non-free". This is a driver
for 8'th (or later) generation "\*-lake" Intel chips, that seems to be
related to VAAPI accelerated graphics.

If you *do* have Intel HD graphics, and have installed the vaapi plugin,
but `-vs vaapisink` does not work, check that vaapi is not "blacklisted"
in your GStreamer installation: run `gst-inspect-1.0 vaapi`, if this
reports `0 features`, you need to `export GST_VAAPI_ALL_DRIVERS=1`
before running uxplay, or set this in the default environment.

You can try to fix audio or video problems by using the
"`-as <audiosink>`" or "`-vs <videosink>`" options to choose the
GStreamer audiosink or videosink , rather than letting GStreamer choose
one for you. (See above, in [Starting and running
UxPlay](#starting-and-running-uxplay) for choices of `<audiosink>` or
`<videosink>`.)

The "OpenGL renderer" window created on Linux by "-vs glimagesink"
sometimes does not close properly when its "close" button is clicked.
(this is a GStreamer issue). You may need to terminate uxplay with
Ctrl-C to close a "zombie" OpenGl window. If similar problems happen
when the client sends the "Stop Mirroring" signal, try the no-close
option "-nc" that leaves the video window open.

### 4. GStreamer issues (missing plugins, etc.):

-   clearing the user's GStreamer cache with
    `rm -rf ~/.cache/gstreamer-1.0/*` may be the solution to problems
    where gst-inspect-1.0 does not show a plugin that you believe is
    installed. The cache will be regenerated next time GStreamer is
    started. **This is the solution to puzzling problems that turn out
    to come from corruption of the cache, and should be tried first.**

If UxPlay fails to start, with a message that a required GStreamer
plugin (such as "libav") was not found, first check with the GStreamer
tool gst-inspect-1.0 to see what GStreamer knows is available. (You may
need to install some additional GStreamer "tools" package to get
gst-inspect-1.0). For, *e.g.* a libav problem, check with
"`gst-inspect-1.0 libav`". If it is not shown as available to GStreamer,
but your package manager shows the relevant package as installed (as one
user found), try entirely removing and reinstalling the package. That
user found that a solution to a "**Required gstreamer plugin 'libav' not
found**" message that kept recurring was to clear the user's gstreamer
cache.

If it fails to start with an error like '`no element "avdec_aac"`' this
is because even though gstreamer-libav is installed. it is incomplete
because some plugin features are missing:
"`gst-inspect-1.0 | grep avdec_aac`" will show if avdec_aac is
available. Unlike other GStreamer plugins, the libav plugin is a front
end to FFmpeg codecs which provide avdec\_\*.

-   Some distributions (RedHat, SUSE, etc) provide incomplete versions
    of FFmpeg because of patent issues with codecs used by certain
    plugins. In those cases there will be some "extra package" provider
    like [RPM fusion](https://rpmfusion.org) (RedHat),
    [packman](http://packman.links2linux.org/) (SUSE) where you can get
    complete packages (your distribution will usually provide
    instructions for this, Mageia puts them in an optional "tainted"
    repo). The packages needed may be "ffmpeg\*" or "libav\*" packages:
    the GStreamer libav plugin package does not contain any codecs
    itself, it just provides a way for GStreamer to use ffmpeg/libav
    codec libraries which must be installed separately. For similar
    reasons, distributions may ship incomplete packages of GStreamer
    "plugins-bad". Use user on Fedora thought they had installed from
    rpmfusion, but the system had not obeyed: *"Adding --allowerasing to
    the dnf command fixed it after a restart"*.

-   starting with release UxPlay-1.65.3, UxPlay will continue to
    function, but without audio in mirror mode, if avdec_aac is missing.

To troubleshoot GStreamer execute "export GST_DEBUG=2" to set the
GStreamer debug-level environment-variable in the terminal where you
will run uxplay, so that you see warning and error messages; see
[GStreamer debugging
tools](https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html)
for how to see much more of what is happening inside GStreamer. Run
"gst-inspect-1.0" to see which GStreamer plugins are installed on your
system.

Some extra GStreamer packages for special plugins may need to be
installed (or reinstalled: a user using a Wayland display system as an
alternative to X11 reported that after reinstalling Lubuntu 18.4, UxPlay
would not work until gstreamer1.0-x was installed, presumably for
Wayland's X11-compatibility mode). Different distributions may break up
GStreamer 1.x into packages in different ways; the packages listed above
in the build instructions should bring in other required GStreamer
packages as dependencies, but will not install all possible plugins.

The GStreamer video pipeline, which is shown in the initial output from
`uxplay -d`, has the default form

    appsrc name=video_source ! queue ! h264parse ! decodebin ! videoconvert ! autovideosink name=video_sink sync=false

The pipeline is fully configurable: default elements "h264parse",
"decodebin", "videoconvert", and "autovideosink" can respectively be
replaced by using uxplay options `-vp`, `-vd`, `-vc`, and `-vs`, if
there is any need to modify it (entries can be given in quotes "..." to
include options).

### 5. Mirror screen freezes (a network problem):

This can happen if the TCP video stream from the client stops arriving
at the server, probably because of network problems (the UDP audio
stream may continue to arrive). At 3-second intervals, UxPlay checks
that the client is still connected by sending it a request for a NTP
time signal. If a reply is not received from the client within a 0.3 sec
time-window, an "ntp timeout" is registered. If a certain number
(currently 5) of consecutive ntp timeouts occur, UxPlay assumes that the
client is "dead", and resets the connection, becoming available for
connection to a new client, or reconnection to the previous one.
Sometimes the connection may recover before the timeout limit is
reached, and if the default limit is not right for your network, it can
be modified using the option "-reset *n*", where *n* is the desired
timeout-limit value (*n* = 0 means "no limit"). If the connection starts
to recover after ntp timeouts, a corrupt video packet from before the
timeout may trigger a "connection reset by peer" error, which also
causes UxPlay to reset the connection.

-   When the connection is reset, the "frozen" mirror screen of the
    previous connection is left in place, but does **not** block new
    connections, and will be taken over by a new client connection when
    it is made.

### 6. Protocol issues (with decryption of the encrypted audio and video streams sent by the client).

A protocol failure may trigger an unending stream of error messages, and
means that the audio decryption key (also used in video decryption) was
not correctly extracted from data sent by the client.

The protocol was modifed in UxPlay-1.65 after it was discovered that the
client-server "pairing" step could be avoided (leading to a much quicker
connection setup, without a 5 second delay) by disabling "Supports
Legacy Pairing" (bit 27) in the "features" code UxPlay advertises on
DNS-SD Service Discovery. Most clients will then not attempt the setup
of a "shared secret key" when pairing, which is used by AppleTV for
simultaneous handling of multiple clients (UxPlay only supports one
client at a time). **This change is now well-tested, but in case it
causes any protocol failures, UxPlay can be reverted to the previous
behavior by uncommenting the previous "FEATURES_1" setting (and
commenting out the new one) in lib/dnssdint.h, and then rebuilding
UxPlay.** ("Pairing" is re-enabled when the new Apple-style one-time
"pin" authentication is activated by running UxPlay with the "-pin"
option introduced in UxPlay 1.67.)

Protocol failure should not happen for iOS 9.3 or later clients.
However, if a client uses the same older version of the protocol that is
used by the Windows-based AirPlay client emulator *AirMyPC*, the
protocol can be switched to the older version by the setting
`OLD_PROTOCOL_CLIENT_USER_AGENT_LIST` in `UxPlay/lib/global.h`. UxPlay
reports the client's "User Agent" string when it connects. If some other
client also fails to decrypt all audio and video, try adding its "User
Agent" string in place of "xxx" in the entry "AirMyPC/2.0;xxx" in
global.h and rebuild uxplay.

Note that for DNS-SD Service Discovery, Uxplay declares itself to be an
AppleTV3,2 (a 32 bit device) with a sourceVersion 220.68; this can also
be changed in global.h. UxPlay also works if it declares itself as an
AppleTV6,2 with sourceVersion 380.20.1 (an AppleTV 4K 1st gen,
introduced 2017, running tvOS 12.2.1), so it does not seem to matter
what version UxPlay claims to be.

# Changelog

xxxx 2025-07-07 Render Audio cover-art inside UxPlay with -ca option (no
file specified).

1.72.2 2025-07-07 Fix bug (typo) in DNS_SD advertisement introduced with
-pw option. Update llhttp to v 9.3.0

1.72.1 2025-06-06 minor update: fix regression in -reg option; add
option -rc `<rcfile>`{=html} to specify initialization file; add "-nc
no" to unset "-nc" option (for macOS users, where -nc is default); add
user-installable systemd script for running UxPlay as an
always-available "rootless daemon"

1.72 2025-05-07. Improved HLS Live Streaming (YouTube) support,
including "scrub". Add requested options -md \<filename\> to output
audio metadata text to a file for possible display (complements -ca
option), and -vol `<v>`{=html} option to set initial audio-streaming
volume. Add support password user access control with HTTP digest
Authentication (-pw \[pwd\]). If no pwd is set, a random pin is
displayed for entry at each new connection.

1.71 2024-12-13 Add support for HTTP Live Streaming (HLS), initially
only for YouTube movies. Fix issue with NTP timeout on Windows.

1.70 2024-10-04 Add support for 4K (h265) video (resolution 3840 x
2160). Fix issue with GStreamer \>= 1.24 when client sleeps, then wakes.

1.69 2024-08-09 Internal improvements (e.g. in -nohold option,
identifying GStreamer videosink selected by autovideosink, finding X11
display) in anticipation of future HLS video support. New -nofreeze
option to not leave frozen video in place when a network connection is
reset. Fixes for GStreamer-1.24.x changes.

1.68 2023-12-31 New simpler (default) method for generating a persistent
public key from the server MAC address (which can now be set with the -m
option). (The previous method is still available with -key option). New
option -reg to maintain a register of pin-authenticated clients.
Corrected volume-control: now interprets AirPlay volume range -30dB:0dB
as decibel gain attenuation, with new option -db low\[:high\] for "flat"
rescaling of the dB range. Add -taper option for a "tapered" AirPlay
volume-control profile.

1.67 2023-11-30 Add support for Apple-style one-time pin authentication
of clients with option "-pin": (uses SRP6a authentication protocol and
public key persistence). Detection with error message of (currently)
unsupported H265 video when requesting high resolution over wired
ethernet. Removed rpi\* options (which are not valid with new Raspberry
Pi model 5, and can be replaced by combinations of other options). Added
optional argument "mac" to "-m" option, to specify a replacement MAC
address/Device ID. Update llhttp to v. 9.1.3. Add -dacp option for
exporting current client DACP info (for remotes).

1.66 2023-09-05 Fix IPV6 support. Add option to restrict clients to
those on a list of allowed deviceIDs, or to block connections from
clients on a list of blocked deviceIDs. Fix for #207 from @thiccaxe
(screen lag in vsync mode after client wakes from sleep).

1.65.3 2023-07-23 Add RPM spec file; add warning if required gstreamer
libav feature "avdec_aac" is missing: (this occurs in RPM-based
distributions that ship an incomplete FFmpeg for Patent or License
reasons, and rely on users installing an externally-supplied complete
FFmpeg). Mirror-mode airplay will now work without audio if avdec_aac is
missing.

1.65 2023-06-03 Eliminate pair_setup part of connection protocol to
allow faster connections with clients (thanks to @shuax #176 for this
discovery); to revert, uncomment a line in lib/dnssdint.h. Disconnect
from audio device when connection closes, to not block its use by other
apps if uxplay is running but not connected. Fix for AirMyPC client
(broken since 1.60), so its older non-NTP timestamp protocol works with
-vsync. Corrected parsing of configuration file entries that were in
quotes.

1.64 2023-04-23 Timestamp-based synchronization of audio and video is
now the default in Mirror mode. (Use "-vsync no" to restore previous
behavior.) A configuration file can now be used for startup options.
Also some internal cleanups and a minor bugfix that fixes #192.

1.63 2023-02-12 Reworked audio-video synchronization, with new options
-vsync (for Mirror mode) and -async (for Audio-Only mode, to sync with
client video). Option -vsync makes software h264 decoding of streamed
videos with option -avdec viable on some recent Raspberry Pi models.
Internal change: all times are now processed in nanoseconds units.
Removed -ao option introduced in 1.62.

1.62 2023-01-18 Added Audio-only mode time offset -ao x to allow user
synchronization of ALAC audio playing on the server with video, song
lyrics, etc. playing on the client. x = 5.0 appears to be optimal in
many cases. Quality fixes: cleanup in volume changes, timestamps, some
bugfixes.

1.61 2022-12-30 Removed -t option (workaround for an Avahi issue,
correctly solved by opening network port UDP 5353 in firewall). Remove
-g debug flag from CMAKE_CFLAGS. Postpend (instead of prepend) build
environment CFLAGS to CMAKE_CFLAGS. Refactor parts of uxplay.cpp

1.60 2022-12-15 Added exit with error message if DNSServiceRegister
fails (instead of just stalling). Test for Client's attempt to using
unsupported AirPlay 2 "REMOTE CONTROL" protocol (with no timing
channel), and exit if this occurs. Reworked metadata processing to
correctly parse DMAP header (previous version worked with DMAP messages
currently received, but was not correct).

1.59 2022-12-12 remove "ZOOMFIX" compile option and make compilation
with X11-dependence the default if X11 development libraries are
detected (this now also provides fullscreen mode with a F11 or Alt+Enter
key toggle); ZOOMFIX is now automatically applied for GStreamer \< 1.20.
New cmake option -DNO_X11_DEPS compiles uxplay without X11 dependence.
Reworked internal metadata handling. Fix segfault with "-vs 0".

1.58 2022-10-29 Add option "-nohold" that will drop existing connections
when a new client connects. Update llhttp to v8.1.0.

1.57 2022-10-09 Minor fixes: (fix coredump on AUR on "stop mirroring",
occurs when compiled with AUR CFLAGS -DFORTIFY_SOURCE); graceful exit
when required plugins are missing; improved support for builds on
Windows. Include audioresample in GStreamer audio pipeline.

1.56 2022-09-01 Added support for building and running UxPlay-1.56 on
Windows (no changes to Unix (Linux, \*BSD, macOS) codebase.)

1.56 2022-07-30 Remove -bt709 from -rpi, -rpiwl, -rpifb as GStreamer is
now fixed.

1.55 2022-07-04 Remove the bt709 fix from -v4l2 and create a new -bt709
option (previous "-v4l2" is now "-v4l2 -bt709"). This allows the
currently-required -bt709 option to be used on its own on RPi without
-v4l2 (sometimes this give better results).

1.54 2022-06-25 Add support for "Cover Art" display in Audio-only (ALAC)
mode. Reverted a change that caused VAAPI to crash with AMD POLARIS
graphics cards. Minor internal changes to plist code and uxplay option
parsing.

1.53 2022-06-13 Internal changes to audio sync code, revised
documentation, Minor bugfix (fix assertion crash when resent audio
packets are empty).

1.52 2022-05-05 Cleaned up initial audio sync code, and reformatted
streaming debug output (readable aligned timestamps with decimal points
in seconds). Eliminate memory leaks (found by valgrind). Support for
display of ALAC (audio-only) metadata (soundtrack artist names, titles
etc.) in the uxplay terminal.

1.51 2022-04-24 Reworked options forVideo4Linux2 support (new option
-v4l2) and short options -rpi, -rpifb, -rpiwl as synonyms for -v4l2,
-v4l2 -vs kmssink, and -v4l2 -vs waylandsink. Reverted a change from
1.48 that broke reconnection after "Stop Mirroring" is sent by client.

1.50 2022-04-22 Added -fs fullscreen option (for Wayland or VAAPI
plugins only), Changed -rpi to be for framebuffer ("lite") RPi systems
and added -rpigl (OpenGL) and -rpiwl (Wayland) options for RPi Desktop
systems. Also modified timestamps from "DTS" to "PTS" for latency
improvement, plus internal cleanups.

1.49 2022-03-28 Addded options for dumping video and/or audio to file,
for debugging, etc. h264 PPS/SPS NALU's are shown with -d. Fixed
video-not-working for M1 Mac clients.

1.48 2022-03-11 Made the GStreamer video pipeline fully configurable,
for use with hardware h264 decoding. Support for Raspberry Pi.

1.47 2022-02-05 Added -FPSdata option to display (in the terminal)
regular reports sent by the client about video streaming performance.
Internal cleanups of processing of video packets received from the
client. Added -reset n option to reset the connection after n ntp
timeouts (also reset after "connection reset by peer" error in video
stream).

1.46 2022-01-20 Restore pre-1.44 behavior (1.44 may have broken hardware
acceleration): once again use decodebin in the video pipeline; introduce
new option "-avdec" to force software h264 decoding by libav h264, if
needed (to prevent selection of vaapisink by autovideosink). Update
llhttp to v6.0.6. UxPlay now reports itself as AppleTV3,2. Restrict
connections to one client at a time (second client must now wait for
first client to disconnect).

1.45 2022-01-10 New behavior: close video window when client requests
"stop mirroring". (A new "no close" option "-nc" is added for users who
wish to retain previous behavior that does not close the video window).

1.44 2021-12-13 Omit hash of aeskey with ecdh_secret for an AirMyPC
client; make an internal rearrangement of where this hash is done. Fully
report all initial communications between client and server in -d debug
mode. Replace decodebin in GStreamer video pipeline by h264-specific
elements.

1.43 2021-12-07 Various internal changes, such as tests for successful
decryption, uniform treatment of informational/debug messages, etc.,
updated README.

1.42 2021-11-20 Fix MAC detection to work with modern Linux interface
naming practices, MacOS and \*BSD.

1.41 2021-11-11 Further cleanups of multiple audio format support
(internal changes, separated RAOP and GStreamer audio/video startup)

1.40 2021-11-09 Cleanup segfault in ALAC support, manpage location fix,
show request Plists in debug mode.

1.39 2021-11-06 Added support for Apple Lossless (ALAC) audio streams.

1.38 2021-10-8 Add -as *audiosink* option to allow user to choose the
GStreamer audiosink.

1.37 2021-09-29 Append "@hostname" to AirPlay Server name, where
"hostname" is the name of the server running uxplay (reworked change in
1.36).

1.36 2021-09-29 Implemented suggestion (by @mrbesen and @PetrusZ) to use
hostname of machine runing uxplay as the default server name

1.35.1 2021-09-28 Added the -vs 0 option for streaming audio, but not
displaying video.

1.35 2021-09-10 now uses a GLib MainLoop, and builds on macOS (tested on
Intel Mac, 10.15 ). New option -t *timeout* for relaunching server if no
connections were active in previous *timeout* seconds (to renew Bonjour
registration).

1.341 2021-09-04 fixed: render logger was not being destroyed by
stop_server()

1.34 2021-08-27 Fixed "ZOOMFIX": the X11 window name fix was only being
made the first time the GStreamer window was created by uxplay, and not
if the server was relaunched after the GStreamer window was closed, with
uxplay still running. Corrected in v. 1.34

### Building OpenSSL \>= 1.1.1 from source.

If you need to do this, note that you may be able to use a newer version
(OpenSSL-3.0.1 is known to work). You will need the standard development
toolset (autoconf, automake, libtool). Download the source code from
<https://www.openssl.org/source/>. Install the downloaded openssl by
opening a terminal in your Downloads directory, and unpacking the source
distribution: ("tar -xvzf openssl-3.0.1.tar.gz ; cd openssl-3.0.1").
Then build/install with "./config ; make ; sudo make install_dev". This
will typically install the needed library `libcrypto.*`, either in
/usr/local/lib or /usr/local/lib64.

*(Ignore the following for builds on MacOS:)* On some systems like
Debian or Ubuntu, you may also need to add a missing entry
`/usr/local/lib64` in /etc/ld.so.conf (or place a file containing
"/usr/local/lib64/libcrypto.so" in /etc/ld.so.conf.d) and then run "sudo
ldconfig".

### Building libplist \>= 2.0.0 from source.

*(Note: on Debian 9 "Stretch" or Ubuntu 16.04 LTS editions, you can
avoid this step by installing libplist-dev and libplist3 from Debian 10
or Ubuntu 18.04.)* As well as the usual build tools (autoconf, automake,
libtool), you may need to also install some libpython\*-dev package.
Download the latest source with git from
<https://github.com/libimobiledevice/libplist>, or get the source from
the Releases section (use the \*.tar.bz2 release, **not** the \*.zip or
\*.tar.gz versions): download
[libplist-2.3.0](https://github.com/libimobiledevice/libplist/releases/download/2.3.0/libplist-2.3.0.tar.bz2),
then unpack it ("tar -xvjf libplist-2.3.0.tar.bz2 ; cd libplist-2.3.0"),
and build/install it: ("./configure ; make ; sudo make install"). This
will probably install libplist-2.0.\* in /usr/local/lib. The new
libplist-2.3.0 release should be compatible with UxPlay;
[libplist-2.2.0](https://github.com/libimobiledevice/libplist/releases/download/2.2.0/libplist-2.2.0.tar.bz2)
is also available if there are any issues.

*(Ignore the following for builds on MacOS:)* On some systems like
Debian or Ubuntu, you may also need to add a missing entry
`/usr/local/lib` in /etc/ld.so.conf (or place a file containing
"/usr/local/lib/libplist-2.0.so" in /etc/ld.so.conf.d) and then run
"sudo ldconfig".

# Disclaimer

All the resources in this repository are written using only freely
available information from the internet. The code and related resources
are meant for educational purposes only. It is the responsibility of the
user to make sure all local laws are adhered to.

This project makes use of a third-party GPL library for handling
FairPlay. The legal status of that library is unclear. Should you be a
representative of Apple and have any objections against the legality of
the library and its use in this project, please contact the developers
and the appropriate steps will be taken.

Given the large number of third-party AirPlay receivers (mostly
closed-source) available for purchase, it is our understanding that an
open source implementation of the same functionality wouldn't violate
any of Apple's rights either.

# UxPlay authors

*\[adapted from fdraschbacher's notes on RPiPlay antecedents\]*

The code in this repository accumulated from various sources over time.
Here is an attempt at listing the various authors and the components
they created:

UxPlay was initially created by **antimof** from RPiPlay, by replacing
its Raspberry-Pi-adapted OpenMAX video and audio rendering system with
GStreamer rendering for desktop Linux systems; the antimof work on code
in `renderers/` was later backported to RPiPlay, and the antimof project
became dormant, but was later revived at the [current GitHub
site](http://github.com/FDH2/UxPlay) to serve a wider community of
users.

The previous authors of code included in UxPlay by inheritance from
RPiPlay include:

-   **EstebanKubata**: Created a FairPlay library called
    [PlayFair](https://github.com/EstebanKubata/playfair). Located in
    the `lib/playfair` folder. License: GNU GPL
-   **Juho Vähä-Herttua** and contributors: Created an AirPlay audio
    server called [ShairPlay](https://github.com/juhovh/shairplay),
    including support for Fairplay based on PlayFair. Most of the code
    in `lib/` originally stems from this project. License: GNU LGPLv2.1+
-   **dsafa22**: Created an AirPlay 2 mirroring server
    [AirplayServer](https://github.com/dsafa22/AirplayServer) (seems
    gone now), for Android based on ShairPlay. Code is preserved
    [here](https://github.com/jiangban/AirplayServer), and [see
    here](https://github.com/FDH2/UxPlay/wiki/AirPlay2) for the
    description of the analysis of the AirPlay 2 mirror protocol that
    made RPiPlay possible, by the AirplayServer author. All code in
    `lib/` concerning mirroring is dsafa22's work. License: GNU
    LGPLv2.1+
-   **Florian Draschbacher** (FD-) and contributors: adapted dsafa22's
    Android project for the Raspberry Pi, with extensive cleanups,
    debugging and improvements. The project
    [RPiPlay](https://github.com/FD-/RPiPlay) is basically a port of
    dsafa22's code to the Raspberry Pi, utilizing OpenMAX and OpenSSL
    for better performance on the Pi. License GPL v3. FD- has written an
    interesting note on the history of [Airplay protocol
    versions](http://github.com/FD-/RPiPlay#airplay-protocol-versions),
    available at the RPiPlay github repository.

Independent of UxPlay, but used by it and bundled with it:

-   **Fedor Indutny** (of Node.js, and formerly Joyent, Inc) and
    contributors: Created an http parsing library called
    [llhttp](https://github.com/nodejs/llhttp). Located at
    `lib/llhttp/`. License: MIT
