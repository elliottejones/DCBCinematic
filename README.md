# DCBCinematic

A DLL for Universal low-level mouse smoothing.

# What is this?

Originally indented to create a smooth camera option for a game called Dream Car Builder, DCBCinematic essentially hijacks the mouse input at the lowest level,
and smooths it using an interial based system.

This is far better than an approach like autohotkey, as most games (especially ones on Unity engine) take raw data from the windows input pipeline.

DCBCinematic uses MiniHook (link below) to tap into the raw windows input pipeline and catch every mouse input delta.
This delta is used to apply a force to a virtual "particle".
The velocity of this particle is then read at a fixed update rate (60hz by default), and that velocity is converted into smooth relative movements.

It ended up becoming a universal tool, as i realised that it works for any application that you inject the DLL into.

# IMPORTANT:

This app does directly inject input commands, which is typical of cheat software. Please be weary of anti-cheat software flagging you (While this isn't cheat software, it sure does behave like it)

# Quickstart:

- Download the DLL file from the releases tab
- Download and open DLL injector software, i would reccomend Process Hacker (link below) - the instructions will be for this software too.
- Open Dream Car Builder (Or the other game of your choice)

The following instructions are for Process Hacker, similar steps will be followed if you are using a different program.

- Find your game in Process Hacker
- Right click on the executable for the game
- Go to Miscellaneous > Inject DLL
- Select the DLL you downloaded
- Go into your game and press F6 (for some reason this may take multiple presses)
- Observe as your camera is now smooth.

# Build it yourself:

For more advanced/skeptical users, you can download the source, open the .sln project and go to dllmain.cpp, there are a few parameters that you can tweak to your liking, then rebuild the DLL yourself and follow the above instructions.

#
I hope this works well for you, message me on discord at elzi_ if you have any issues or contributions.
