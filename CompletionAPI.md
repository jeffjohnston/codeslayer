CodeSlayer has its own completion API. I think it improves on the one included in GtkSourceView quite a bit, and gives CodeSlayer a consistent feel.

The following image shows you what the completion window looks like. This is taken from the [word completion](WordCompletionPlugin.md) plugin.

<img src='http://wiki-images.codeslayer.googlecode.com/git/completion.png' />

### How It Works ###

At a high level the way it works is you implement the completion provider interface and pass in the iter that represents the place that the completion window will appear, and then return the list of proposal objects.

```
GList*
codeslayer_completion_provider_get_proposals (CodeSlayerCompletionProvider *provider, 
                                              GtkTextIter                   iter);
```

The implementation will look roughly like this. For each proposal you pass in the label that will appear in the window, and the text that will be used when something is selected. The mark gives the place that the completion starts.

```

GList *proposals = NULL;
CodeSlayerCompletionProposal *proposal;
proposal = codeslayer_completion_proposal_new (label, text, mark);
proposals = g_list_prepend (proposals, proposal);
return proposals;
```

Once you have the interface implemented then just add your proposals provider to the editor.

```
codeslayer_editor_add_completion_provider (editor, 
                                           CODESLAYER_COMPLETION_PROVIDER (provider));

```

### Example ###

I would highly recommed that you get the source code for the [word completion](WordCompletionPlugin.md) plugin and use it as a starting point for your project. It is a very simple example of how the completion API works.