_This is just a page of random notes for things that I use during development._


CodeSlayer follows the GTK+ coding standard. This is the typical compiler configuration that is used during development.

```
../configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --prefix=$HOME/install --enable-codeslayer-dev=true
```

When running with [Valgrind](http://live.gnome.org/Valgrind) use the following:

```
G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --log-file=vgdump ./codeslayer
```

Or this if you do not want to see the possibly lost

```
G_SLICE=always-malloc G_DEBUG=gc-friendly valgrind --tool=memcheck --leak-check=full --leak-resolution=high --num-callers=20 --show-possibly-lost=no --log-file=vgdump ./codeslayer
```

To run the GtkDoc's

```
# run the next two commands if you have problems linking
LD_LIBRARY_PATH=/home/jeff/workspace/codeslayer/build/codeslayer/.libs/
export LD_LIBRARY_PATH

../configure CFLAGS="-Wall -Werror -Wdeclaration-after-statement -g -O0" --prefix=$HOME/install --enable-codeslayer-dev=true --enable-gtk-doc 
```

To find Gtk+ errors with gdb:

```
gdb ./codeslayer
run --g-fatal-warnings

or

G_DEBUG=fatal_warnings gdb ./codeslayer
```


Converting from OGG to AVI:

```
ffmpeg -i file.ogg file.avi

- or -

mencoder -idx codeslayer.ogv -ovc lavc -oac mp3lame -o codeslayer.avi
```

To verify that the proper folders have internationalization support run the following in your po directory:

```
intltool-update -m
```

To fix the links of the gtk-docs run the gtkdoc-rebase command:

```
gtkdoc-rebase --online --html-dir=html
```

To update your ldconfig run the command.

```
sudo /sbin/ldconfig
```

To create a patch file with subversion run the command:

```
svn diff > patchfile.patch
```

To apply the patch file run the command:

```
patch -p0 < patchfile.patch
```

If you push changes into a non-bare repository you need to run the following command the first time.

```
git push origin master:master
```

GtkSourceView Reference:

```
https://live.gnome.org/GtkSourceView/StyleSchemes
```

The production header file install path

```
/usr/local/include/codeslayer
```

#### Xlint Ant Task ####

```
<javac srcdir="${src}" destdir="${build}" includeantruntime="false" debug="true">
  <classpath>
    <fileset dir="${lib}"/>
  </classpath>
  <compilerarg value="-Xlint"/>
</javac>

http://docs.oracle.com/javase/6/docs/technotes/tools/windows/javac.html
```