The Autotools plugin allows you to compile your code using the autotools system.

_Disclaimer...it seems that the configure and autoreconf do not work as good as they need to be. I find myself having to run those from the command line. But the make, make install and make clean work perfectly. Also, be sure to use fully qualified paths so that the links back to the source file work._

### Installation ###

First follow the [plugin installation](HowToInstallPlugin.md) guide.

### Features ###

The Autotools plugin was created to be simple to use, and work within the multiple project environment. To accommodate having multiple projects you can either build using the shortcut key (F9) or use the projects tree popup menu. If you use the F9 key it will build the project that is part of the current (opened) editor. If you want to build a specific project then use the projects tree popup menu.

### Configuration ###

To start with you need to use the project properties to setup each project that you want to use the Autotools with.

<img src='http://wiki-images.codeslayer.googlecode.com/git/project-properties.png' />

In the Autotools tab you have three fields that you need to fill in. They are the location of the configure.ac file, the configure parameters, and the location of the build directory. The only one that is optional is the configure parameters. As a convenience you can also select the icon and browse to the file and path.

<img src='http://wiki-images.codeslayer.googlecode.com/git/autotools/autotools-configuration.png' />

Once you have the settings in place you will want to run the configure command. To do that use the projects tree popup menu (see the projects popup menu section below).

### Tools Menu ###

The tools menu allows to use the global shortcut keys, F9, Ctrl+F9 and Ctrl+Shift+F9. These allow you to run the commands make, make install, and make clean. When using the tools menu the project that is referenced is the project that is associated with the current (opened) editor.

<img src='http://wiki-images.codeslayer.googlecode.com/git/autotools/autotools-menu.png' />


### Projects Popup Menu ###

Usually running the Autotools is most convenient by using the shortcut keys. However, a lot of times you just want to specify the project directly. To do that then just right click on any part of the projects tree and use the Autotools menu. In addition to make, make install and make clean, you can also run configure and autoreconf.

<img src='http://wiki-images.codeslayer.googlecode.com/git/autotools/autotools-projects-popup.png' />