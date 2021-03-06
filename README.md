# Computer Graphic - Height Fields Using Shaders

##### taught by Dr. Jernej Barbič

## Before you start the code, please check your system environment.

For the Windows platform, we provide the Visual Studio 2017 and 2019 solution/project files in ./hw1-starterCode (same files for both 2017 and 2019).

For Mac OS X, before you do any coding, you must install command-line utilities (make, gcc, etc.). Install XCode from the Mac app store, then go to XCode, and use "Preferences/Download" to install the command line tools. Important: If you are using Mac OS X Mojave, you need to update the OS to the latest version of Mojave. Otherwise, OpenGL does not work. Or, you can use Catalina, Big Sur or Monterey.

On Linux, you need the libjpeg library, which can be obtained by "sudo apt-get install libjpeg62-dev libglew-dev". For Windows and Mac OS X, the starter code contains a precompiled jpeg library. On Intel-based Apple chips, the jpeg library should work as is. On Apple M1 chips (https://en.wikipedia.org/wiki/Apple_M1), you need to take the following steps to get jpeg library to compile and link:

// Do this for Apple M1 chips ONLY. If you don't do it, you will
// get linker errors related to the libjpeg library.
// The below steps are not necessary for Windows, Linux or Intel-based Apple computers.
```
cd external/jpeg-9a-mac

chmod +x configure
./configure --prefix=$(pwd)

make clean

make
chmod +x install-sh
make install

cd ../../hw1-starterCode
make
./hw1 heightmap/spiral.jpg

```
```
./hw1 heightmap/spiral.jpg
the .jpg can be any pic inside the heightmap folder
```


## My Environment:
```
OpenGL Version: 4.1 Metal - 76.1
OpenGL Renderer: Apple M1
Shading Language Version: 4.10
API: OpenGL (Core Profile)
```
## Animation
Demo video: https://youtu.be/LSj9oB2tLNk

## Function:

After running the program (by "make" + "./hw1 heightmap/GrandTeton-128.jpg"), a height field is generated by rendering using points.
Use ./hw1 heightmap/color.jpg to have a color input(ImageIO::getBytesPerPixel == 3).
I also has color1 and color2 input image, above-mentioned image can be found in heightmap folder.

Use keyboard to change the display type.

Press '1' to render the height field as points.

Press '2' to render the height field as lines(wireframe).

Press '3' to render the height field as triangles(solid triangles).

Press '4' to render the height field as smoothened triangles(solid triangles). (I use vertex shader to calculate the smoothened height and output color.

Press '5' make it scale up and down to look funny, BOUNCE!!(Extra)

Press '6' stop scaling.(Extra)

Press '7' rotate it for 2 rounds.(Extra)

Translate: /*(CTRL is not working on my MacBook m1 clip, too I change it to "ALT".)*/
Pressing 'ALT' and dragging the mouse with the left mouse clicked can translate along x and y axis. 
Pressing 'ALT' and dragging the mouse with the middle mouse clicked can translate along z axis. 

Scale: 
Pressing 'SHIFT' and dragging mouse with the left mouse clicked can scale on x and y axis.
Pressing 'SHIFT' and dragging mouse with the middle mouse clicked can scale on z axis.

Rotate: 
Dragging mouse with the left mouse clicked can rotate at x and y axis.
Dragging mouse with the middle mouse clicked can rotate at z axis.

Press 'x' to take a single screenshot of the current state.

Press 'a' to animate the height map, rotating OR scaling OR translating it, and also showing the different modes applied. (Extra)

Press 't' to take 300 screenshots while animating in different render type. (Extra)

Press 'c' for some screenshots while animating in different render type. (Extra)

Press 'r' to initialize all the land transforms. (Extra)

Press 'ESC' to exit.


## Extra Features:

1, I make the scale changeable with the size of input image.
To be specific, I make the camera position changeable with the scale of ImageWidth.

2. I improve fovy in the matrix.Perspective, so it is more fit to the window size.

3. The code can also support color in input images((ImageIO::getBytesPerPixel == 3)). 
It can detect if the image is color or grayscale, and render accordingly.
(Use "./hw1 heightmap/color.jpg" to have a color input)

4. I customize the vertex shader at its output color, so it becomes smoother.

5. I center the image, so it can rotate and scale easily.
((it is better to view the color.jpg when landRotate[0] = 30.0f;))

6. I animate my render image with interesting effect, bounce, bounce, bounce.

## EXAMPLE

![000](https://user-images.githubusercontent.com/51981236/169444165-7bedbe90-1bbf-4ed2-a07a-42cb11a1d45f.jpg)
![009](https://user-images.githubusercontent.com/51981236/169444191-ddaa3e09-6a96-40e5-abe1-8739fea0e2a9.jpg)
![020](https://user-images.githubusercontent.com/51981236/169444212-118549f0-9a56-4e8f-aebc-f9cdeb23fa53.jpg)
![057](https://user-images.githubusercontent.com/51981236/169444229-12326408-505a-423e-9b83-2d4ac171dee9.jpg)
![066](https://user-images.githubusercontent.com/51981236/169444260-a3818d38-e928-4ead-a1ef-52cbbf1103d9.jpg)

![076](https://user-images.githubusercontent.com/51981236/169444278-ba2de6cb-ea0c-4aef-b51d-80dff9e74e69.jpg)
![084](https://user-images.githubusercontent.com/51981236/169444296-99a189ed-01b4-4ae8-91ac-be582e310b0f.jpg)
![127](https://user-images.githubusercontent.com/51981236/169444325-3d27d4c3-3c97-487d-b885-0e0057da16e8.jpg)

![136](https://user-images.githubusercontent.com/51981236/169444345-0649ee5a-6c40-43f0-893d-dcb787f4321a.jpg)
![145](https://user-images.githubusercontent.com/51981236/169444364-6406af04-dffa-4f44-84f7-f22e729ffd4c.jpg)

![248](https://user-images.githubusercontent.com/51981236/169444400-9b73ea6d-cd36-4d1e-9f7b-ad07020d54df.jpg)
![259](https://user-images.githubusercontent.com/51981236/169444423-822c1882-0564-4a82-8100-74bddcf7b00f.jpg)
![275](https://user-images.githubusercontent.com/51981236/169444449-ef28b1de-881d-4a38-9e24-fd51f4cc60f5.jpg)

