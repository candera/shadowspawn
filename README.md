# Status

NO LONGER SUPPORTED. It might still work - I don't know. I don't run Windows any more. Sorry! I supported it for years but hopefully there's a fork somewhere out there. If you make one and maintain it I will be happy to post a link here. 

# What Is ShadowSpawn? 

ShadowSpawn is a tool for working with shadow copies. Shadow copies
are read-only snapshots of your disk. Working with shadow copies
instead of the actual files allows you to do things like work with
in-use (locked) files. 

ShadowSpawn works by making a shadow copy of your disk, making it
available at a drive letter, then launching (spawning) another
program that you specify.

Probably the most common way to use ShadowSpawn is to use
[Robocopy](http://en.wikipedia.org/wiki/Robocopy) make a copy of files
that are currently in use.
   
# Installing ShadowSpawn

Most users can simply unzip the appropriate zip file from
[the download page](https://github.com/candera/shadowspawn/downloads).
ShadowSpawn.exe can then just be run - there is no installer.
However, ShadowSpawn uses the Visual C++  runtime, which may not be
present on some machines. If ShadowSpawn does not work for you, run the
vcredist executable available from the same download page.

# Running ShadowSpawn

ShadowSpawn is a command-line tool: there is no GUI.

ShadowSpawn take three arguments: 

1. The directory that contains the files you want to snapshot. 
1. An available drive letter where the snapshot will become visible. 
1. A command to run. 

Let's say that you wanted to use robocopy to copy files from the
`C:\foo` directory to the `C:\bar` directory. You could do that with
the following command: 

    shadowspawn C:\foo Q: robocopy Q:\ C:\bar /s
    
That would cause shadowspawn to 

1. Make a shadow copy of the C: drive. 
1. Mount the shadowed version of the C:\foo directory at Q:.
1. Launch `robocopy Q:\ C:\bar /s`
1. Wait for Robocopy to finish. 
1. Clean up the shadow copy and remove it from Q:

You can use any drive letter you want (it doesn't have to be `Q:`),
but it does have to be a drive letter that's not currently being used
for anything else.

You can run any command you want. So if you just wanted to use notepad
to look at a shadow copy of `C:\foo\blah.txt`, you'd run

    shadowspawn C:\foo Q: notepad Q:\blah.txt
    
Just remember that shadowspawn will remove the Q: drive as soon as the
command you specify exits. 

# Relationship to HoboCopy

ShadowSpawn is derived from the same source code as
[HoboCopy](https://github.com/candera/hobocopy) and is intended to
replace it. The evolution was driven by the fact that although the
shadow copy part of HoboCopy works well enough, the copying part was
nowhere near as robust as tools like RoboCopy. By providing a tool
that just takes care of the shadow copy, ShadowSpawn allows users to
work with locked and in-use files using any other tool, not just the
limited copy features provided by HoboCopy.

# Status

ShadowSpawn is currently at version 0.1.0, which is meant to indicate
that it is an initial version. While it largely consists of fairly
mature code taken from HoboCopy, there are bound to be a few issues as
early adopters identify opportunities for improvement. 

# Reporting Bugs and Requesting Features

Please report bugs and request features using either
[the project issue tracking system](https://github.com/candera/shadowspawn/issues)
or the project mailing list at shadowspawn-tool@googlegroups.com
([website](http://groups.google.com/group/shadowspawn-tool)). 
   
# USAGE: 

<pre>
Usage:

shadowspawn [ /verbosity=LEVEL ] &lt;src> &lt;drive:> &lt;command> [ &lt;arg> ... ]

Creates a shadow copy of &lt;src>, mounts it at &lt;drive:> and runs &lt;command>.

/verbosity   - Specifies how much information ShadowSpawn will emit
               during execution. Legal values are: 0 - almost no
               information will be emitted. 1 - Only error information
               will be emitted. 2 - Errors and warnings will be
               emitted. 3 - Errors, warnings, and some status
               information will be emitted. 4 - Lots of diagnostic
               information will be emitted. The default level is 2.

&lt;src>        - The directory to shadow copy (the source directory).
&lt;drive:>     - Where to mount the shadow copy. Must be a single letter
               followed by a colon. E.g. 'X:'. The drive letter must be
               available (i.e. nothing else mounted there).
&lt;command>    - A command to run. ShadowSpawn will ensure that &lt;src> is
               mounted at &lt;drive:> before starting &lt;command>, and will
               wait for &lt;command> to finish before unmounting &lt;drive:>


Exit Status:

If there is an error while processing (e.g. ShadowSpawn fails to
create the shadow copy), ShadowSpawn exits with status 1.

If there is an error in usage (i.e. the user specifies an unknown
option), ShadowSpawn exits with status 2.

If everything else executes as expected and &lt;command> exits with
status zero, ShadowSpawn also exits with status 0.

If everything else executes as expected and &lt;command> exits with a
nonzero status code n, ShadowSpawn exits with status n logically OR'ed
with 32768 (0x8000). For example, robocopy exits with status 1 when
one or more files are Scopied. So, when executing

  shadowspawn C:\foo X: robocopy X:\ C:\path\to\backup /mir

the exit code of ShadowSpawn would be 32769 (0x8000 | 0x1).
</pre>
