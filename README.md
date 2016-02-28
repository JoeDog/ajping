# ajping README

is a utility that allows you to test connectivity to an application
server running Apache JServ Protocol (AJP). 

# INSTALLATION
--------------

The build process for ajping was implemented with GNU autotools. If
you're familiar with that process then compiling this program is pretty
straight forward:

 $ ./configure [options]
 $ make
 $ make install

If you pulled this code from Github, then you'll need to build the 
configure script. From the top level directory run this command:

 $ utils/bootstrap

You should see output that looks like this:
  + aclocal
  + autoheader
  + automake --foreign --copy
  + autoconf
  + utils/manifier doc/ajping.pod doc/ajping.1.in AJP 1.3 ping utility 1

Now you can run the configure script. 



