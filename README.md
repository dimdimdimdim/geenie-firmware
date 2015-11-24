# geenie-firmware
Firmware for the Geenie RPL calculator

Geenie is an open-source electronic calculator specifically designed for engineers.
The goals are summarized below:
- Cheap and easily fabricable by the DIYers
- Responsive and powerful enough
- Low power consumption
- Small and nice looking
The software shows a HP48-like interface to the user, and uses Reverse Polish Notation.

This specific part of the project is dedicated to the firmware of the calculator.

The firmware includes an OS developed from scratch, specifically for this purpose. It is developed in C++, and features multithreading, garbage collection, and a lot of advanced features usually not found in embedded developments, but that make development of the upper layers a lot more consistent and efficient.
The calculator application, built on the top of this OS, is very strongly inspired by the HP-48 (and its successors). It provides a RPN stack, supports almost the same set of objects (real, integers, strings, list, programs, ...), and uses almost the same syntax.
