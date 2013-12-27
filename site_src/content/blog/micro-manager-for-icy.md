Title: Hardware connection with Micro-Manager
Date: 2013-9-27 10:30
Tags: plugin
Slug: micro-manager-for-icy
Lang: en
Author: Will Ouyang
Summary:

## Hardware solutions for EVA
Micro-manager is a powerful framework for hardware integration, is a ideal solution for EVA project to support hardwares. Micro-manager is used to support all kinds of microscopies and accessories by providing a flexiable driver adapter machanisim.

For EVA project itself, we trying to include some open source hardware in order to build less-pricy research platform. We decide to use Micro-manager as a midware to bridge the hardware and the Icy software. 

And another reason is, Micro-Manager is fully supported in Icy, we got "Micro-Manager for Icy" and many other plugins can work with Micro-Manager.

With Micro-Manager, all kinds of data can be represent as image. Signals in 1D can also treated as a one-row image, and the value range can be 8 bit to 64 bit, so it can represent almost every kind of signal.

We will demostrate this framework with a ultrasonic imaging device.


