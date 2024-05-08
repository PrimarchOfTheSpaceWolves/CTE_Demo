# MODIFIED FROM: https://www.kaggle.com/code/timesler/guide-to-mtcnn-in-facenet-pytorch

from facenet_pytorch import MTCNN
import cv2
import numpy as np
import math as m
import os

def convert_faces(faces):
    if faces is None:
        return None
    
    new_face_list = []
    for face in faces:
        # Get the image itself
        face = face.numpy()
        face = face.astype(np.uint8)
        face = np.transpose(face, (1,2,0))
        new_face_list.append(face)
    return np.array(new_face_list)

def save_current_face_images(faces):
    # Make output directory
    output_dir = "./face_images"
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    
    # Delete old files
    for file in os.listdir(output_dir):
        full_path = os.path.join(output_dir, file)
        os.remove(full_path)
        
    # Save image files
    if faces is not None:
        for i in range(len(faces)):
            filename = "face_%03d.png" % i
            out_image = cv2.resize(faces[i], (256,256))
            cv2.imwrite(os.path.join(output_dir, filename), out_image)
            
        print(len(faces), "face image(s) saved.")
    else:
        print("No faces saved.")

def create_face_image(faces, face_image, faces_found):
    if faces is None:
        face_image = np.zeros((1,1,3), dtype=np.uint8)
        faces_found = 0        
    else:
        MAX_FACE_COLS = 2
        
        face_width = faces.shape[2]
        face_height = faces.shape[1]   
        
        current_face_cnt = len(faces)
        face_rows = int(m.ceil(current_face_cnt / MAX_FACE_COLS))
        
        if current_face_cnt != faces_found:
            faces_found = current_face_cnt            
            face_image = np.zeros((face_height*face_rows, face_width*MAX_FACE_COLS,3), dtype=np.uint8)
        
        for i in range(len(faces)):
            # Get the coordinates
            fr = i // MAX_FACE_COLS
            fc = i % MAX_FACE_COLS
            
            sr = fr*face_height
            er = sr + face_height
            sc = fc*face_width
            ec = sc + face_width
                        
            # Write to total face image
            face_image[sr:er, sc:ec,:] = faces[i]         
                    
    return face_image, faces_found
    

def main():

    # Create face detector
    mtcnn = MTCNN(margin=20, keep_all=True, post_process=False, device='cuda:0')

    # Load a single image and display
    camera = cv2.VideoCapture(0, cv2.CAP_DSHOW) # CAP_DSHOW recommended on Windows 

    # Did we get it?
    if not camera.isOpened():
        print("ERROR: Cannot open camera!")
        exit(1)

    # Create window ahead of time
    windowName = "Webcam"
    cv2.namedWindow(windowName)
    
    face_image = np.zeros((1,1,3), dtype=np.uint8)
    faces_found = 0    
    
    SAVE_FACE_FREQ = 30
    current_frame = 0

    # While not closed...
    key = -1
    while key == -1:
        # Get next frame from camera
        _, frame = camera.read()
        
        # Show the image
        cv2.imshow(windowName, frame)
        
        # Detect face
        faces = mtcnn(frame)
        
        # Convert the faces to a different format
        faces = convert_faces(faces)
        
        # Save faces
        if (current_frame % SAVE_FACE_FREQ) == 0:
            save_current_face_images(faces)
        
        # Display faces
        face_image, faces_found = create_face_image(faces, face_image, faces_found)
        print("FACES FOUND:", faces_found)
                    
        cv2.imshow("FACES", face_image)

        # Wait 30 milliseconds, and grab any key presses
        key = cv2.waitKey(30)
        
        # Increment frame 
        current_frame += 1

    # Release the camera and destroy the window
    camera.release()
    cv2.destroyAllWindows()

    # Close down...
    print("Closing application...")

if __name__ == "__main__":
    main()

