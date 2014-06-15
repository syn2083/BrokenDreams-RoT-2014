BrokenDreams-RoT-2014
=====================

Work on RoT 2.0b5 base, so far a super basic lua implementation has been added, and changes to memory, and various other systems.

Main changes:

(to be updated)
-A lot of the alloc_perm stuff has been ripped out for (some) malloc or calloc - will change over to all calloc
-UWID framework: add unqiue identifiers to all objects/mobiles and players currently works, but ties into nothing
-MySQL integration, currently logs are sent to MySQL DB, along with some project information (to create projects in game, for whatever work you want to do, and accepts logs) and the helpfiles
-Helpfile system re-write, needs some work but entirely functional
-Added a queue to write_to_descriptor to allow for EAGAIN conditions
-Added I3 client from AckFUSS!
-Added Room/Obj progs
-Fixed various OLC bugs
-Changed VNUM limit from some sh_int or int to unsigned long
-Added some more functionality to do_look while in an OLC editor
-Fixed up bugs/issues for compilation on gcc 4.8.2 & 64bit os (ubuntu)
-Startup script changed from bin/sh to bin/bash to allow for extra commands
-Added a few more if checks to OLC mobprogs
