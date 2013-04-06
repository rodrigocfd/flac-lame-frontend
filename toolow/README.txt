TOOLOW - Thin Object Oriented Layer Over Win32
----------------------------------------------

These C++ classes are the result of about a decade of Win32 programming, where I
condensed most of my Win32 techniques and solutions. At some point, I decided to
put everything together as my personal library because I disliked all the
existing ones I found, and I wanted something to fit my personal needs. I also
don't use STL. Indeed, this is C++ written in a very C way.

If you're interested in using this library, or part of it, just go ahead.
However, I strongly recommend you to take a look at the way the things were
implemented, so that you can use them the way they were intended to be used, and
don't abuse some functionalities. Also take a look at the examples for quick a
learning, and get the overall feel of it.


Window event model
------------------

There are two types of windows to be created: those called "frames", which are
created through CreateWindowEx calls, and the "dialogs", which rely upon dialog
resources. To create a window, extend one of the final classes shown below.

Class hierarchy for the Frame family:

          +-- WindowCtrl <---+
          |                  +---- FrameCtrl
          |         <--------+
          +-- Frame
Window <--+         <--------+                 +-- FrameApp
          |                  +-- FramePopup <--+
          +-- WindowPopup <--+                 +-- FrameModal


Class hierarchy for the Dialog family:

          +-- WindowCtrl <---+
          |                  +---- DialogCtrl
          |          <-------+
          +-- Dialog
Window <--+          <-------+                  +-- DialogApp
          |                  +-- DialogPopup <--+
          +-- WindowPopup <--+                  +-- DialogModal


There must be only one App class. It's the main window of the program, which
carries all the initialization on. Modal windows are regular modal popups. Ctrl
windows are widgets to be placed upon App or Modal windows.

After extending a window class, you must override the msgHandler method, which
reflects a window procedure. You must return a call to the parent msgHandler
method; for example: on the msgHandler of a derived DialogApp window, you must
return a call to DialogApp::msgHandler(), instead of TRUE/FALSE. One exception
to this is when you intentionally want to halt the processing, like after a
successful handling of a WM_COMMAND message.