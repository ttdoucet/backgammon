The bearoff program generates the bearoff database in the form of
a C++ structure that is in turn included and compiled by the main
code.

This directory has snapshot copies of several source files from the
main program.  Doing it this way eliminates the need to repeatedly
build the database as the original files are changed.  Indeed, it
is perfectly okay to use old and stable files for bearoff generation
because all that matters is the output, and that should not change.

From time to time, as the files diverge from the originals it might
be good to simply copy the new files overtop of the old ones, and then
fix any issues that might result from how the originals might have
evolved.

But during the interim, the builds will remain fast and correct.
