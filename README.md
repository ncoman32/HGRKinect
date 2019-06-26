# HGRKinect

The application is written in C++ and is  build over Chao Wang’s [application](https://github.com/chaowang15/RGBDCapture) used to capture RGB-D data fed by a OpenNi-drived depth camera (Kienct v1, in this case). The data saved by this application is represented by 200x200 24bit color images, 200x200 8bit depth images and a .txt file containing the contour of the hand saved as pixel coordinates pairs.

The code has been tested successfully in VS 2017 in Windows 10.

## Dependencies
- OpenCV (3.1.0 for Windows) -- Image Processing
- OpenNI2 (2.2 x64 for Windows) -- For data acquisition and image registration 
- Boost (1.64 for Windows) -- File System, Threads
- FFTW (3.3.5 x64 for Windows) - Discrete Fourier Transforms

The application is already injected with FFTW and OpenCV dependencies.

## Installation and Usage
1. Download [Kinect SDK 1.8](https://www.microsoft.com/en-us/download/details.aspx?id=40278) and install it using compatibility mode for Windows 8:
  - Right click on the setup file
  - Click on properties
  - Click on Compatibility tab
  - Put a check mark on Run this program in compatibility mode
  - Select Windows 8.1/8
  - Put a ceck mark on Run this program as administrator under Privilege level
2. Download and install [OpenNI 2.2.0.XX Beta (x64) ](https://structure.io/openni).
3. Download the zip file of [Boost 1.64](https://dl.bintray.com/boostorg/release/1.64.0/source/) and [build](https://www.youtube.com/watch?v=Mioo8Hnp6M8) the library.
4. Download and install [Git](https://git-scm.com/downloads) if not already installed. Clone the repository by opening a git bash command line and issuing the *git clone https://github.com/ncoman32/KinectApplication* command.
5. Before running the application in VS 2017  using .sln file and do : 
  - navigate to project -> properties -> C/C++ -> General -> Additional Include Directories -> Edit -> New Line (if dependency not already present), and use the following : **$(OPENNI2_INCLUDE64)**, **path to boost_1_64_0 folder** . Modify the paths of **OpenCV\include ** and  **FFTW** to match the ones on your machine.
  - navigate to project -> properties -> Linker -> Input -> Additional Dependencies -> Edit , and add on a new line (if not present): **OpenNI2.lib**, **opencv_world310d.lib**,
**libboost_filesystem-vc141-mt-gd-1_64.lib**, **libboost_system-vc141-mt-gd-1_64.lib**, **libboost_thread-vc141-mt-gd-1_64.lib**, **libfftw3-3.lib**.
- navigate to project -> properties -> Linker -> General -> Additional Library Directories -> Edit -> New Line (if dependency not already present), and use the following: **$(OPENNI2_LIB64)**, **path to boost_1_64_0 folder\stage\x64\lib**. Modify paths of **OpenCV\lib** and **FFTW** to match the ones on your machine, 
6. Set the application on **Debug** and **x64** and start it.
　　　 
## Note
* When starting the application the RGB frames and the depth frames converted to CV_8UC1 frames are displayed. The depth frames present a crosshair suggesting that the hand should be placed on it and a circle indicating what will be cropped out of the depth frame.
* Press ESC to quit scanning, S to take a snap and see the RGB frame over which the contour of the hand is overlaid, mean depth frame(obtained from 30 depth frames), the image containing only the hand, 
* The RGB-D data will be saved in a newly created folder named `saveX`, where `X` is an index integer starting from 0.
* Check the path of OPENNI2 driver files if the code cannot start.
