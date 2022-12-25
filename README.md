# Scaler biAkima

A [GIMP](https://www.gimp.org/) plugin that scales an image using [Akima spline](https://en.wikipedia.org/wiki/Akima_spline) instead of cubic interpolation method.

This plugin is based on [GIMP](https://www.gimp.org/) plugin [Ogniewski Scaler](https://github.com/pannacotta98/ogniewski-scaler)


## Examples and Compare

[Examples and comparison with other scaling methods](https://github.com/ImageProcessing-ElectronicPublications/gimp-plugin-scaler-biakima-demo).

## Building the Plugin

### Ubuntu

On Ubuntu 20.04.4 LTS on ARM64 in a VM on an M1 MacBook Air, the following installs were needed
```sh
sudo apt install gimp
sudo apt install build-essential
sudo apt install libgimp2.0-dev
sudo apt install autoconf automake autotools-dev m4
```

Installing GIMP via snap can have some problems with accessing the plugins folder.

In the Scaler biAkima root directory, run
```sh
./autogen.sh
make
sudo make install
```

### Windows

The plugin can be built for Windows using mingw, here given for 64bit x86. I'm not quite sure if all these steps are needed, since I tried stuff untill it worked.

Install [msys2](https://msys2.github.io/) and run "MSYS2 MinGW x64". Update the system using
```sh
pacman -Syyuu
```
Restart the terminal.

Install the following dependencies (**TODO** I suspect many of these are not needed, i took them from [here](https://wiki.gimp.org/wiki/Hacking:Building/Windows#Cross-Compiling_GIMP_under_UNIX_using_crossroad) which is for building the GIMP)
```sh
pacman -S --needed \
    base-devel \
    git \
    mingw-w64-x86_64-toolchain \
    mingw-w64-x86_64-asciidoc \
    mingw-w64-x86_64-drmingw \
    mingw-w64-x86_64-gexiv2 \
    mingw-w64-x86_64-ghostscript \
    mingw-w64-x86_64-glib-networking \
    mingw-w64-x86_64-graphviz \
    mingw-w64-x86_64-gtk2 \
    mingw-w64-x86_64-gobject-introspection \
    mingw-w64-x86_64-iso-codes \
    mingw-w64-x86_64-json-c \
    mingw-w64-x86_64-json-glib \
    mingw-w64-x86_64-lcms2 \
    mingw-w64-x86_64-lensfun \
    mingw-w64-x86_64-libheif \
    mingw-w64-x86_64-libraw \
    mingw-w64-x86_64-libspiro \
    mingw-w64-x86_64-libwebp \
    mingw-w64-x86_64-libwmf \
    mingw-w64-x86_64-meson \
    mingw-w64-x86_64-mypaint-brushes \
    mingw-w64-x86_64-openexr \
    mingw-w64-x86_64-poppler \
    mingw-w64-x86_64-python2-pygtk \
    mingw-w64-x86_64-SDL2 \
    mingw-w64-x86_64-suitesparse \
    mingw-w64-x86_64-xpm-nox
```

Set variables
```sh
export GIMP_PREFIX=`realpath ~/gimp_prefix`
export PATH="$GIMP_PREFIX/bin:$PATH"
export PKG_CONFIG_PATH="$GIMP_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"
export PKG_CONFIG_PATH="$GIMP_PREFIX/share/pkgconfig:$PKG_CONFIG_PATH"
export LD_LIBRARY_PATH="$GIMP_PREFIX/lib:$LD_LIBRARY_PATH"

#environment variable for Autotools
export ACLOCAL_FLAGS="-I/c/msys64/mingw64/share/aclocal"
#environment variable for introspection
export XDG_DATA_DIRS="$XDG_DATA_DIRS:$GIMP_PREFIX/share:/usr/local/share/:/usr/share/:/mingw64/share/"
```

Install GIMP package
```sh
pacman -S --needed mingw-w64-x86_64-gimp
```

The plugin template requires the perl module XML::Parser for the internationalization, this is what I installed. I supect not all of them are needed
```sh
pacman -S --needed mingw-w64-x86_64-perl # Don't think this was required

# A mess where I try to get XML::Parser to find the expat dependency
pacman -S mingw-w64-x86_64-expat
pacman -S libexpat-devel
pacman -S libexpat
```

Nothing seemed to work, but then I restarted the terminal, and then it worked
```sh
cpan App::cpanminus # For cpanm
cpanm XML::Parser
```

Finally, build the plugin using
```sh
# The -mwindows flag to the linker makes sure no command window is opened
./configure LDFLAGS=-mwindows
make
make install
```

in the root directory of this repo. To use the plugin, copy `src/scaler-biakima.exe` to the GIMP plugin folder, available in the preferences.


## Based on: GIMP Plug-In Template

---

Copyright (C) 2000-2003  Michael Natterer <mitch@gimp.org>


This package is an empty plug-in for GIMP 2.0. It features all the
nice things a good GIMP plug-in should have:

- The source is separated in main/interface/render files (core/ui separation).
- It's prepared for I18N.
- It installs it's own help files.
- Full autoconf/automake support.
- A dialog with examples of all libgimp widgets.

Basically everything a Plug-In author should ever need except
her own image manupulation routines.


To build and install it, just ...
```sh
./configure
make
make install
```
 ... and it's there.

And don't forget to remove (or update if really necessary) comments
for translators in po/README.translators.

The default license for the gimp-plugin-template is an ultra-permissive
XFree-style license to encourage the use of the template for GPL /or/
non-GPL plugins.  If you wish to use a different license for your plugin
(e.g. GPL) then you may change the license block in the 'COPYING' file
and the source to your favourite license.


Happy GIMPing,
--Mitch

---

