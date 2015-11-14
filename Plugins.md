CodeSlayer has a very powerful, yet simple plugin system. From the start of CodeSlayer I wanted to make sure that developers interested in the project had a way to create plugins that were simple to create but yet had access to all the core functionality. At the same time I wanted the API to be very focused and approached it from the standpoint that CodeSlayer was meant to work with code.

Plugins are associated to profiles. That means for each new profile created you will need to select the plugins that you want for that profile. This is very powerful because a lot of times profiles are set up for different languages, and so this allows you to work how you want and not be forced into one paradigm.

### Available Plugins ###

  * [AutotoolsPlugin](AutotoolsPlugin.md)
    * A plugin to perform builds using the autotools.
  * [CommentsPlugin](CommentsPlugin.md)
    * A plugin to insert comments in the editor.
  * [CtagsPlugin](CtagsPlugin.md)
    * A plugin to search the source files using the ctags.
  * [FileSearchPlugin](FileSearchPlugin.md)
    * A plugin to search the files in the Projects side pane.
  * WordCompletionPlugin
    * A plugin to complete words in the editor. Use it by pressing Ctrl+Spacebar after typing in one or more characters in the editor.


### Install Plugin ###

To use one of the many available plugins you first need to [install](HowToInstallPlugin.md) the plugin.

### Create Plugin ###

If you need some functionality not included within CodeSlayer you can [create](HowToCreatePlugin.md) your own plugin.