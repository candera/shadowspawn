# WHAT IS HOBOCOPY? 

HoboCopy is a backup/copy tool. It is inspired by robocopy in both name and in
functionality. It differs greatly from robocopy, however, in two respects: 

1. It is not as full-featured as robocopy. 
2. It uses the Volume Shadow Service (VSS) to "snapshot" the disk
before copying. It then copies from the snapshot rather than the "live" disk.
   
# INSTALLING HOBOCOPY

Most users can simply unzip the file containing hobocopy.exe into the directory 
of your choice. However, HoboCopy uses the Visual C++ 8.0 runtime, which may
not be present on some machines. If HoboCopy does not work for you, run the 
vcredist executable available from the same location you downloaded HoboCopy. 
   
# WHY DOES HOBCOPY USE THE VOLUME SHADOW SERVICE?    
   
Because HoboCopy copies from a VSS snapshot, it is able copy even files that 
are in locked by some other program. Further, certain programs (such as SQL 
Server 2005) are VSS-aware, and will write their state to disk in a consistent
state before the snapshot is taken, allowing a sort of "live backup". Files 
locked by VSS-unaware programs will still be copied in a "crash consistent"
state (i.e. whatever happens to be on the disk). This is generally a lot 
better than not being able to copy the file at all. 

# IS HOBOCOPY A BACKUP TOOL? 

Well, not exactly. It can be used that way, but it doesn't do a few things
that "real" backup tools to. For example, there's currently no support for
differential copies. Also, it does not currently make use of the OS support
for doing backups that would allow it to do things like copy even files
it does not nominally have permission to copy. 

The other caveat is that HoboCopy is a hobby project. Therefore, it is not 
recommended that anyone use it as a backup strategy for valuable information 
- no warranty is provided in the event that something goes wrong. 

That said, the author of the tool uses it to back up his own systems. 

# USAGE: 

<pre>
hobocopy [/statefile=FILE] [/verbosity=LEVEL] [ /full | /incremental ]
         [ /clear ] [ /skipdenied ] [ /y ] [ /simulate ] [/recursive]
         <src> <dest> [file [file [ ... ] ]

Recursively copies a directory tree from <src> to <dest>.

/statefile   - Specifies a file where information about the copy will
               be written. This argument is required when /incremental
               is specified, as the date and time of the last copy is
               read from this file to determine which files should be
               copied.

/verbosity   - Specifies how much information HoboCopy will emit
               during copy. Legal values are: 0 - almost no
               information will be emitted. 1 - Only error information
               will be emitted. 2 - Errors and warnings will be
               emitted. 3 - Errors, warnings, and some status
               information will be emitted. 4 - Lots of diagnostic
               information will be emitted. The default level is 2.

/full        - Perform a full copy. All files will be copied
              regardless of modification date.

/incremental - Perform an incremental copy. Only files that have
               changed since the last full copy will be copied.
               Specifying this switch requires the /statefile switch
               to be specified, as that's where the date of the last
               full copy is read from.

/clear       - Recursively delete the destination directory before
              copying. HoboCopy will ask for confirmation before
              deleting unless the /y switch is also specified.

/skipdenied  - By default, if HoboCopy does not have sufficient
               privilege to copy a file, the copy will fail with an
               error. When the /skipdenied switch is specified,
               permission errors trying to copy a source file result
               in the file being skipped and the copy continuing.

/y           - Instructs HoboCopy to proceed as if user answered yes
               to any confirmation prompts. Use with caution - in
               combination with the /clear switch, this switch will
               cause the destination directory to be deleted without
               confirmation.

/simulate    - Simulates copy only - no snapshot is taken and no copy
               is performed.

/recursive   - Copies subdirectories (including empty ones). Shortcut: /r

<src>        - The directory to copy (the source directory).
<dest>       - The directory to copy to (the destination directory).
<file>       - A file (e.g. foo.txt) or filespec (e.g. *.txt) to copy.
               Defaults to *.*.
</pre>
