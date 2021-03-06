SmartFusion FreeRTOS Drivers
============================

Low-level interrupt-driven FreeRTOS(TM) drivers for common peripherals on Microsemi(R) SmartFusion(R) MSS.

This library provides C modules that allow high-performance blocking access to MSS peripherals and fabric based peripherals in SmartFusion (1). The drivers are designed with a focus on data buffering and interrupt semaphore signalling, and thus do not provide all the interfaces needed to control a peripheral (e.g. initialization, setting slave-select lines, etc.). Please use the hw_platform drivers for this purpose.

This library depends on FreeRTOS v8.0.0, and firmware drivers generated by the MSS configurator using Microsemi Libero SoC design software.

Look under each folder for driver specific instructions. In the case that a particular type of peripheral has multiple instances, e.g. UART, SPI, I2C, etc, you may need to copy the driver files multiple times, name the functions with seperate prefixes, and configure each module accordingly.

## Notes
These drivers were written by [Yichen Zhao](http://www.zhaoyichen.net) for the Drum Master project in EECS 373 at University of Michigan, Winter 2014. See this [video](https://www.youtube.com/watch?v=Fmh1cL_w1zM&feature=youtu.be) for the project. Please consult your instructors if you wish to use these drivers in your project, and discuss the level of complexity graded for your project if they are used.
