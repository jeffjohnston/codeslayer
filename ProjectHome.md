CodeSlayer is a text editor that boasts a clean interface but powerful features. It is written in C using the GTK+ toolkit. At the heart it is a simple text editor that can then be extended to be as full feature a tool as desired. What makes CodeSlayer different is an (optional) projects view in the side pane and the ability to have multiple configurations through what I call profiles. It can also be used as your go-to tool for dealing with regular expressions (>= 4.1.0).

_Google Code Hosting removed support for the Downloads section. In its place they are encouraging projects to use Google Drive. So now to get the [downloads](https://drive.google.com/folderview?id=0BzXLXuBmSmiXYmUtWmhRM0s2ZlU&usp=sharing#list) you will go to my drive folder._

**09/23/2014** CodeSlayer 4.3.1 is out! I have the file search plugin integrated into the core project. This allows you to search for files easily when working with projects. You can activate it from the Search menu (Search For Document), or use the shortcut key Ctrl+K.

I also fixed two major bugs that were symptoms of the same problem. If you experienced any lockups when adding and/or removing projects multiple times within the same session it is now fixed. I just found out about it myself but I wouldn't be surprised if someone else has stumbled on it. It was actually a simple fix once I understood the problem.

**05/21/2014** CodeSlayer 4.2.1 is out! When working with regular expressions it was not very user friendly to lose your results by accidentally hitting the up or down arrow key so I just disabled them in the find and replace text fields.

**04/28/2014** CodeSlayer 4.2 adds a recent documents list in the file menu.

**03/08/2014** CodeSlayer 4.1 Is out! We now have a nice regular expression view that allows you to view and extract regular expressions out of your documents. You can find it in the View menu labeled Regular Expression. Once you select it the view will appear in the bottom pane (read more about it below).

Be sure to visit the [plugins](Plugins.md) page to explore the list of plugins. If you have problems installing a plugin visit the [installation](HowToInstallPlugin.md) page, which also contains answers on the most common install problems, or just email me at jeff.johnston.mn@gmail.com and I will help out however I can.

Especially watch out for an error that says **"cp: cannot create regular file `/root/.codeslayer/plugins': No such file or directory"** error. You need to [adjust](HowToInstallPlugin.md) your sudoers file to use the HOME environmental variable of the user that is running off the command. I do not see a way to adjust this at runtime, but if somebody knows be sure to let me know and I will adjust the script!

_You can find out more information about what CodeSlayer is all about [here](CodeSlayer.md). Be sure to email me at jeff.johnston.mn@gmail.com if you have any comments, questions, or suggestions._

## Installation ##

To install CodeSlayer first make sure that you have the proper libraries installed.

```
sudo apt-get install libglib2.0-dev libgtk-3-dev libgtksourceview-3.0-dev intltool libtool gtk-doc-tools
```

Then unpack the distribution into a directory, open up a terminal, change to the directory you just created, and enter in the following commands:

```
./configure
make
sudo make install
sudo ldconfig
```

## Projects View ##

It is very easy to use with the projects view. First create a new profile in the File->Profiles menu. There you will Add a profile. Be sure to select the Enable Projects checkbox. Next go to the Projects menu and select Add Projects. Then select the top level folders that you want to be part of your profile. Once you have that visit the Preferences in the Edit menu and tweak things to your liking. Also keep in mind that each profile will have its own preferences and plugins defined. That way you can tailor each profile to be exactly what you want.

## Key Strokes ##

The (non-discoverable) key stroke commands that are specific to CodeSlayer.

#### Editor ####

| **key strokes** | **command** |
|:----------------|:------------|
| F3              |  Find Next (editor must have focus) |

I think one thing that makes CodeSlayer special is the way that that the editor search feels as you use it. There is a lot of subtle things that it does that may not be obvious at first. For instance if you highlight some text and press Ctrl+F then it will highlight matches in the editor, but not scroll. But if you just start typing in the find field it will start scrolling to find what you are searching for as you type. To make searching faster you can also use the F3 key to 'find next' in place of Ctrl+G. You will also notice that you can keep using the F3 key even when the inline search field is closed! It will just use the last thing that you were searching for.

#### Projects ####

| **key strokes** | **command** |
|:----------------|:------------|
| Ctrl+X          | Cut         |
| Ctrl+C          | Copy        |
| Ctrl+V          | Paste       |
| Ctrl+R          | Rename      |
| Delete          | Move To Trash |
| Ctrl+F          | Find In Projects |

## Regular Expression View ##

As of CodeSlayer 4.1 there is a nice regular expression view that allows you to view and extract regular expressions out of your documents. You can find it in the View menu labeled Regular Expression. Once you select it the view will appear in the bottom pane.

For the most part when I work on regular expressions I want to validate that my regular expression matches things in the document. But then the next thing I usually want to do is rip those results out of the document and format it in such a way that I can run it with some other program. For example lets say I have have a bunch of words that I need to rip out of a document and use them as an IN clause for a SQL query. In CodeSlayer what I can do now is use my find to get all the matches and then use the replace to rip out the find results using the grouping syntax and then put ticks and commas around each word.

I use this all the time now and it makes you wonder why other regex tools do not offer it. It is a very natural use of find/replace when working with regular expressions. So, some things to remember. If you use the new view and just do a find and not a find/replace then a newline gets inserted after each match (for readability). Once you switch to doing a find/replace then it is up to you to put the newline character in, or anything else you want as the replace kind of becomes a formatting tool. If that is confusing try putting "Hi \0\n" for your replace statement. This is saying show group zero (group 0 is the full match) and put a newline at the end (plus a friendly greeting at the beginning).

## Screenshots ##

#### Used With Plugins As IDE ####

<img src='http://wiki-images.codeslayer.googlecode.com/git/codeslayer.png' />

#### Used As Text Editor ####

<img src='http://wiki-images.codeslayer.googlecode.com/git/codeslayer-text-editor.png' />