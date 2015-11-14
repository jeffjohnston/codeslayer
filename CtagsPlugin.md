The Ctags plugin allows you to navigate your code using the [Exuberant Ctags](http://ctags.sourceforge.net/).

### Installation ###

First follow the [plugin installation](HowToInstallPlugin.md) guide.

Then make sure that you have the ctags library installed on your machine.

```
exuberant-ctags
```

### Features ###

The Ctags plugin was created to be simple to use, and work within the multiple project environment. It is optimized for C but will be useful for any language. To find a given tag what you do is highlight the text in the editor and then press the key F4.

The tag search rules (for C) are as follows. First it will look in the current editor for a matching tag. If it does not find a matching tag there then it will look for the first occurrence of the tag in any non-header file (.h extension). If it still does not find a matching tag then it will search the header files. I think this gives the best chance to find the tag you are most interested in.

### Configuration ###

To start with you need to use the project properties to setup each project that you want to use the Ctags with.

<img src='http://wiki-images.codeslayer.googlecode.com/git/project-properties.png' />

In the Ctags tab you have one field that you need to fill in. It is the location of the source directory. As a convenience you can also select the icon and browse to the path.

<img src='http://wiki-images.codeslayer.googlecode.com/git/ctags/ctags-configuration.png' />

### Tools Menu ###

The tools menu allows to use the global shortcut key, F4.

<img src='http://wiki-images.codeslayer.googlecode.com/git/ctags/ctags-menu.png' />

### Technical Notes ###

If you are interested in seeing the tag file that is generated look in your groups folder in the .codeslayer/(profile) folder of your home directory. The tag file is re-generated two seconds after you save your editors. The reason for the two second delay is to cut down on excessive re-generation if you are saving multiple editors.