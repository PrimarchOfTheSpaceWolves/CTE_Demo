# CTE Demo
### Dr. Michael J. Reale, SUNY Polytechnic Institute

## Setup

### Python Dependencies

- Install either Anaconda or Miniconda
  - It is possible this might also work without conda.  
  - You may also have to be administrator for the following steps.
- With the conda tools on the path, run the following to create the environment:
```
conda create -n DEMO python=3.11
```
- This should create the DEMO environment.
- To activate it:
```
conda activate DEMO
```
- Then, install these packages via pip:
```
pip install torch==2.2.2 torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118
pip install facenet_pytorch opencv-python
```
### C++ Dependencies
You will need:
- CMake
- GLFW3
- Glew
- Assimp
- glm

On Windows, this involves compilation and installation of all dependencies (except CMake, which can be installed via an installer program).

On Linux, it should be possible to install these via the command line:
```
sudo apt-get install cmake cmake-gui
sudo apt-get install libglfw3-dev libglew-dev libglm-dev libassimp-dev
```

On Mac, it is THEORETICALLY possible to install via Homebrew:
```
brew install glfw glew assimp glm
```

## Running the Python Script
Activate the DEMO environment and run:
```
conda activate DEMO
python facesaver.py
```

This will do the following:
- Read video from a webcam
- Detect all faces in the image using a neural network approach
- Display the original image and a collage of the detected faces
- Save the face images to the folder ```face_images```

This script can be run while the C++ code is running.

## Compiling and Running the C++ Code

