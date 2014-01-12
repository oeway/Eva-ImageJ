Title: Surface tracking plugin for Icy
Date: 2013-10-12 7:40
Tags: plugin, ultrasonic
Slug: pixel-row-align
Author: Will Ouyang
Summary:

## Motivation

Immersion testing is one of the most common method in Ultrasonic testing. A technology named "surface tracking" always used in immersion testing. By finding the first echo of the whole sequence of wave, we can get the echo of the liquid-specimen interface. With the offset of the interface, we can inspect the status of the surface, or we can align all the echoes to the same time position.

## pixelRowAlign plugin
The simplest method to achieve this goal is using a simple threshold to find the interface echo position. We have implement a plugin named "pixelRowAlign". With this plugin, you will able to find the echo by defining a threshold, you can choose differenet stratage to find the echoes.

![pixelRowAlignPlugin][]

See more detail about the plugins, see [Download](../pages/downloads).

[pixelRowAlignPlugin]:http://i.imgur.com/YGQABo6.jpg