### Compile ###

Unpack the distribution into a directory, open up a terminal, change to the directory you just created, and enter in the following commands:

```
./configure
make
sudo make install
sudo ldconfig
```


### Troubleshooting ###

#### "No package 'codeslayer' found" error ####

If you get the error "No package 'codeslayer' found" you need to add to add the path to the CodeSlayer pkgconfig.

```
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```

> _Note: if you use the --prefix command the path needs to be adjusted accordingly._

#### "cp: cannot create regular file `/root/.codeslayer/plugins': No such file or directory" error ####

If you get an error like the following while performing make install.

```
cp ctags.codeslayer-plugin /root/.codeslayer/plugins
cp: cannot create regular file `/root/.codeslayer/plugins': No such file or directory
```

The problem is that the sudo command is using the root HOME variable. See, in your top level Makefile.am file you have a line that copies the plugin file to your home directory .codeslayer file.

```
install-data-hook:
   cp ctags.codeslayer-plugin $(HOME)/$(CODESLAYER_HOME)/plugins
```

The quick and dirty fix is to swap out the $(HOME) variable to your home directory. The real fix is to update your sudoers file to use the HOME environmental variable of the user that kicked off the command.

From these docs:

https://wiki.archlinux.org/index.php/Sudo#sudoers_default_file_permissions

Using visudo I added the following line:

```
Defaults env_keep += "HOME"
```

