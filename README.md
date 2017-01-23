# Count and measure size of ladybird spots

Image analysis. A work in progress.


###Features

In short, the script:
* finds the contours of each elytron in the image (the number of elytra must be defined by user)
* uses adaptive thresholding to count the total number of pixels and number number of black pixels composing each elytron
* uses blob detection to find number of spots on each elytron
* calculates length and width of each elytron by fitting a minimum enclosing rectangle to contour
* returns features for each elytron to the standard output and outputs a .jpg of the overall image mask (for all elytra)


Allowing the user to define several biological features:
* number of spots
* overall spot area (in mm<sup>2</sup>)
* size of each individual spot
* percentage of elytra melanised (i.e. black pixels / total pixels)


###Current limitations
Spot counting currently only works with red morphs (e.g. Harmonia axyridis f. succinea).


###TODOs:
* automatically detect number of (non-overlapping) elytra per image
* optional inpainting to interpolate light reflection (ideally, this problem avoided at image acquisition stage)
* how to define number of spots for black morphs (e.g. Harmonia axyridis f. conspicua, f. spectabilis)? (i.e. two red spots?)
