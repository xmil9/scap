scap (screen capture) library
=============================

A library for capturing the screen contents and saving them as a PNG image.
There are different implementations using different technologies. Each
implementation follows the same interface defined in scap.h.

scapgdi - Windows implementation using GDI.<br/>
scapdx - Windows implementation using DirextX (Duplication API).<br/>
scapx - Linux implementation using X11.<br/>
