Usage
----

There are many environnement variable to control Box86 behavour. 

##### BOX86_LOG
Controls the Verbose level of the log
 * 0
 * NONE : No message (exept some fatal error)
 * 1
 * INFO : Current default. Show some log
 * 2
 * DEBUG : Verbose a lot of stuffs (like relocations or function called)
 * 3
 * DUMP : All DEBUG plus DUMP of all ELF Info

#### BOX86_LD_LIBRARY_PATH
Controls the folder scanned to look for x86 libs. Default is current folder and `lib` in current folder.
Also `/usr/lib/i386-linux-gnu` and `/lib/i386-linux-gnu` are added if they exists

#### BOX86_PATH
Controls the folder scanned to look for x86 executable. Default is current folder and `bin` in current folder.

#### BOX86_DLSYM_ERROR
* 0 : default. Don't log `dlsym` error
* 1 : Log dlsym error

#### BOX86_TRACE_FILE
Send all log and trace to a file instead of `stdout`

#### BOX86_TRACE
Only on build with trace enabled. Trace allow the logging of all instruction execute, along with register dump
* 0 : No trace
* 1 : Trace enabled. The trace start after the init off all depending libs is done
* symbolname : Trace only `symbolname` (trace is disable if the symbol is not found)
* 0xXXXXXXX-0xYYYYYYY : trace only between the 2 addresses

#### BOX86_TRACE_START
Only on build with trace enabled.
* NNNNNNN : Start trace anly after NNNNNNNN opcode execute (number is an `uint64_t`)

#### BOX86_TRACE_XMM
Only on build with trace enabled.
* 0 : Default, the XMM (i.e. SSE/SSE2) register will not be logged with the general and x87 registers
* 1 : Dump the XMM registers

