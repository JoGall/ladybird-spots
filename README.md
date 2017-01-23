# Count and measure size of ladybird spots

An image analysis script to quantify spot melanisation in ladybirds by counting the number of spots and measuring the area of each spot. A work in progress.

Example input and output image:

<img src = "https://cloud.githubusercontent.com/assets/17113779/22203896/1977eb16-e167-11e6-9660-40cef85f39e5.jpg" alt = "raw1" width = "400" />
<img src = "https://cloud.githubusercontent.com/assets/17113779/22203897/199937bc-e167-11e6-83ed-0f99bdeeb087.jpg" alt = "mask1" width = "400" />


###Features

In short, the script:
* finds the contours of each elytron in the image (the number of elytra must be defined by user)
* uses adaptive thresholding to count the total number of pixels and number number of black pixels composing each elytron
* uses blob detection to find number of spots on each elytron
* calculates length and width of each elytron by fitting a minimum enclosing rectangle to contour
* returns features for each elytron to the standard output and outputs a .jpg of the overall image mask (for all elytra)

Allowing the user to define several biological features:
* number of spots
* overall spot area
* size of each individual spot
* proportion of elytra melanised (i.e. black pixels / total pixels)

### Usage
#### a) Compilation
The script requires an installation of the Open Source Computer Vision library, [OpenCV](http://opencv.org/quickstart.html.). The script then needs to be compiled; for example, using the GNU C++ compiler on Linux:
```
g++ -ggdb -o ./spots ./spots.cpp `pkg-config --cflags opencv` `pkg-config --libs opencv` -Wno-write-strings -std=c++0x
```

#### b) Command line use

To find two elytra in all .jpg 
in installation directory:
```
./spots -e N FILE_1 FILE_2...
```
where `N` is the number of elytra present in each image and `FILE_1 FILE_2...` are image files.

#### c) Parameter tuning

Blob detection parameters can be tuned for a single image using the `testParameters.cpp` script. For example, after compilation of the script as `./blobs`:
```
./blobs succinea_JAN16-123.jpg
```

###Current limitations
* light reflection can cause problems with blob detection (e.g. counts single blobs as two blobs, overestimating spot number and undestimating spot size)
* spot counting currently only works with red morphs (e.g. Harmonia axyridis f. succinea)
* user must define number of elytra present in each image before analysis


###TODOs:
* automatically detect number of (non-overlapping) elytra per image
* optional inpainting to interpolate light reflection (ideally, this problem avoided at image acquisition stage)

<img src = "https://cloud.githubusercontent.com/assets/17113779/22203949/543fea14-e167-11e6-9f92-37fe4952d7ac.jpg" alt = "inpaint_raw" width = "400" />
<img src = "https://cloud.githubusercontent.com/assets/17113779/22203948/543c6920-e167-11e6-8021-1a37faf01537.jpg" alt = "inpaint_mask" width = "400" />

* how to define number of spots for black morphs (e.g. Harmonia axyridis f. conspicua, f. spectabilis)? (e.g. here, one black spot or two orange spots?)

<img src = "https://cloud.githubusercontent.com/assets/17113779/22203953/5e8326d0-e167-11e6-9017-453a1526dc15.jpg" alt = "inpaint_mask" width = "400" />

* automatic tuning of blob detection parameters, perhaps by asking user to define number of spots on each elytron (or fining  n largest countours instead of blob detection?)
* return location of each spot as relative X,Y-coords
