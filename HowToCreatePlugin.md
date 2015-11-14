From the start of CodeSlayer I wanted to make sure that developers interested in the project had a way to create plugins that were simple to create but yet had access to all the core functionality. At the same time I wanted the API to be very focused and approached it from the standpoint that CodeSlayer  was meant to work with code.

What you will find is an API with the following goals:
  * One simple object that is passed to your plugin. This object, called [CodeSlayer](http://code.google.com/p/codeslayer/source/browse/codeslayer/codeslayer.h), has all the signals and functions that you will interact with
  * Bind directly to the editor (which extends directly from [GtkSourceView](http://projects.gnome.org/gtksourceview/)) when the editor is added
  * Can be informed (bind to) when the editor is saved, removed and switched
  * Easily add your own tabs to the side and bottom pane
  * Easily add your own menu items to the menubar tools menu as well as the projects tree popup menu
  * Open up an editor and scroll to the proper line

This API gives you an amazing amount of control in a way that is both powerful and easy.

### Getting Started ###

The easiest way to create plugins is to bring up a development instance of CodeSlayer. By a development instance I mean a version of CodeSlayer that is built into a local directory and has its own configuration. Luckily, doing this is very easy because I built this into the CodeSlayer autotools setup.

So, start by grabbing a copy of the CodeSlayer distribution and unpacking it into a directory of your choosing (for this example we will use a projects directory in your home path). Next cd into that directory and create a new build folder. Strictly speaking you do not need a build directory, but doing so will keep your compiled code out of the source folder. _This was a tip I picked up in the [Autotools](http://www.amazon.com/Autotools-Practioners-Autoconf-Automake-Libtool/dp/1593272065/ref=sr_1_1?s=books&ie=UTF8&qid=1303584383&sr=1-1) book, and I would highly recommend reading it if you want to learn more about how the autotools work._

Now run the follow commands.
```
cd $HOME/projects/codeslayer-1.1.0
mkdir build
cd build
../configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --prefix=$HOME/install --enable-codeslayer-dev=true
make
make install
```

The first part of the command is your typical autotools configure statement, with debugging and warnings enabled. The prefix is saying that you will compile this into an install directory in your home path. This will be the same path that you will install your plugins into. The enable-codeslayer-dev is how we tell the build that we want the program to be run in development mode.

Really all the enable-codeslayer-dev does is save and use the configuration in a $HOME/.codeslayer-dev directory. If your plugin has custom configuration you will get used to seeing this directory because this is where you will be placing your own configuration files.

To run your development version of CodeSlayer change to the install/bin directory and execute the codeslayer file.

```
cd $HOME/install/bin
./codeslayer
```

### Creating Plugin ###

Now onto the actual plugin creation! What I want to do here is explain how to create a plugin using the example [spike plugin](http://code.google.com/p/codeslayer/downloads/list). We will then tweak it out and convert it into a working plugin called buffy.

_For those that have not guessed it I am a big fan of Buffy The Vampire Slayer. Both Spike and Buffy were characters from the show._

#### Running The Example ####

Start by unpacking the spike plugin into a directory of your choosing (for this example we will use a projects directory in your home path).

Now run the follow commands.
```
cd $HOME/projects/spikeplugin-1.1.0
mkdir build
cd build
../configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --prefix=$HOME/install
make
make install
```

The first part of the command is your typical autotools configure statement, with debugging and warnings enabled. The prefix is saying that you will compile this into an install directory in your home path. This will be the same path that you installed your development build of CodeSlayer into.

With your new plugin installed go ahead and run your development version of CodeSlayer.

```
cd $HOME/install/bin
./codeslayer
```

#### Modifying The Example ####

As noted we are going to be modifying the Spike plugin and creating a new Buffy plugin out of it. Along the way you will learn everything that you need to know to create your own plugin.

First lets learn the basics of how the plugins work.

Your plugin needs to implement three methods, called activate, deactivate, and configure. The meaning should be self explanatory. The activate is called when your plugin is loaded and is where you will attach and bind to everything that you need. The deactivate is called when your plugin is unloaded and is used to detach and unbind everything you did when activate was called. The configure method is not mandatory, but if you want to be called to save any configuration then you need to implement the method.

```
#include <codeslayer/codeslayer.h>
#include <gmodule.h>

G_MODULE_EXPORT void activate   (CodeSlayer *codeslayer);
G_MODULE_EXPORT void deactivate (CodeSlayer *codeslayer);
G_MODULE_EXPORT void configure  (CodeSlayer *codeslayer);
```

With that in mind lets start by copying the spikeplugin in the projects directory and call it buffyplugin.

```
cp -R $HOME/projects/spikeplugin-1.1.0 $HOME/projects/buffyplugin-1.1.0
```

_The rest of the tutorial now assumes that you are in the $HOME/projects/buffyplugin-1.1.0 directory._

Now rename the spike.codeslayer-plugin.in to buffy.codeslayer-plugin.in, and adjust the settings to say buffy instead of spike.

```
[plugin]
lib=libbuffyplugin
name=Buffy Plugin
description=An example plugin for CodeSlayer...contains funny Buffy quotes.
authors=Jeff Johnston <jeff.johnston.mn@gmail.com>
copyright=Copyright © 2011 Jeff Johnston
website=http://codeslayer.org
```

Go into the configure.ac file and modify the following lines.

```
AC_INIT([BuffyPlugin], [1.1.0], [jeff.johnston.mn@gmail.com])

PKG_CHECK_MODULES(BUFFYPLUGIN, [
    glib-2.0 >= 2.24.0
    gtk+-2.0 >= $GTK_REQUIRED_VERSION
    gtksourceview-2.0 >= 2.10.0
    codeslayer >= 1.0.0
])

AC_CONFIG_FILES([
    buffy.codeslayer-plugin
    Makefile
    src/Makefile
])
```

Go into the Makefile.am file and modify the following lines.

```
EXTRA_DIST = buffy.codeslayer-plugin
```

With the configure.ac and Makefile.am modified run the autoreconf command.

```
autoreconf
```

Go into the src directory and rename the spike-plugin.c file to buffy-plugin.c. Then go into src Makefile.am file and rename everything from spike to buffy like the following.

```
INCLUDES = \
    $(BUFFYPLUGIN_CFLAGS) \
    -I$(top_srcdir) \
    -I$(srcdir)

lib_LTLIBRARIES = libbuffyplugin.la

libbuffyplugin_la_SOURCES = \
    buffy-plugin.c

libbuffyplugin_la_CPPFLAGS = -I$(top_srcdir)
```

Lastly go into the src/buffy-plugin.c and modify any string that says spike and rename it to buffy so we can see that this plugin is different.

Thats it! To finish go to your build directory and run configure, make, and make install.

```
cd $HOME/projects/buffyplugin-1.1.0/build
../configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --prefix=$HOME/install
make
make install
```

You can now bring up your development build of CodeSlayer, select the Buffy Plugin (from the tools menu) and enjoy your new plugin.

To run your development version of CodeSlayer change to the install/bin directory and execute the codeslayer file.

```
cd $HOME/install/bin
./codeslayer
```

### Notes ###

Note: you will need to run the ldconfig command when you install the buffy plugin to your non-development editor for the first time.

```
sudo /sbin/ldconfig
```