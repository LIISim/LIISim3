# External Libraries used by LIISim3

You need to copy the content of the '*LIISim3-externalLibraries*'-respository to this folder in order to be able to compile LIISim3:
 https://github.com/LIISim/LIISim-externalLibraries
 
For further information see the **developer guide** and the **READMEs in the library directories**.

**These libraries are needed:**

* Boost library ([boost.org](http://www.boost.org/))
                                         
* MATLAB MAT file I/O library ([github.com/tbeu/matio](https://github.com/tbeu/matio))

* Qwt Plot library ([qwt.sourceforge.net](http://qwt.sourceforge.net))
Requires to be precompiled for a certain Qt-Kit (combination of Qt-Version and C++-Compiler).
	Also see: [qwt.sourceforge.net/qwtinstall.html](http://qwt.sourceforge.net/qwtinstall.html)

**For FULL version only:**

* PicoScope SDK binaries ([picotech.com](https://www.picotech.com/))
*Note:* You need to have the PicoScope driver installed in order to use the PicoScope integration.
                      
* National Instruments DAQmx (SDK) binaries ([ni.com](http://www.ni.com))
*Note:* You need to have the NI DAQmx runtime or SDK installed in order to use the DAQmx integration.