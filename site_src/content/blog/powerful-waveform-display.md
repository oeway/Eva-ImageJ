Title: Powerful intensity profile tool!
Date: 2013-12-13 11:27
Tags: plugin
Slug: powerful-waveform-display
Lang: en
Author: Will Ouyang
Summary:

## Intensity profile is important
Since the first intensity profile tool for Icy is developed, improvement is never stoped, the reason is that this kind of tool is useful to 1D signal based image data. Sample point of 1D signal is displayed as image pixel, but we have get used to the waveform mode.
## Features
Recently, some large scale improvement is made to IntensityInRectangle plugin. Here is some useful feature of the plugin.

* Show any number of profiles simutaneously.
* Distinguish roi pairs with different color.
* Profile canvas can be resized and operated just like a rectangle ROI
* Use a cursor line to indicate mouse position, update in realtime
* Support line, point, rectangle and more

## Modes
### Line Mode
In Line Mode, profile of the current image is calculated along the line roi.

### Point Mode
Since there is only one point, so the profile is calcualted along Z axis.

### Area Mode
All othe type of ROI except line and point is manipulated under this mode, in Area Mode, intensity profile is calculated along Z axis but the mean intensity of each Z layer is used.

See more detail about the IntensityInRectangle plugins, see [Download][../pages/downloads].
