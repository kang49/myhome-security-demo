import cv2
import numpy as np
import threading
from SpoutGL import SpoutReceiver

frames = {"cam1": None, "cam2": None}
running = True

def capture_spout(sender_name, cam_id):
    global running
    receiver = SpoutReceiver()
    receiver.setReceiverName(sender_name)
    GL_RGBA = 0x1908
    
    current_width, current_height = 0, 0
    img = None
    
    while running:
        if receiver.receiveTexture(): 
            width = receiver.getSenderWidth()
            height = receiver.getSenderHeight()
            
            # Re-allocate buffer if resolution changes
            if current_width != width or current_height != height:
                current_width, current_height = width, height
                img = np.zeros((height, width, 4), dtype=np.uint8) 
            
            success = receiver.receiveImage(img, GL_RGBA, False, 0)
            
            if success and img is not None:
                # Convert RGBA to BGR for opencv
                frames[cam_id] = cv2.cvtColor(img, cv2.COLOR_RGBA2BGR)

print("Waiting for Unreal Engine...")

# Start capture threads
t1 = threading.Thread(target=capture_spout, args=("Unreal_CCTV_1", "cam1"))
t2 = threading.Thread(target=capture_spout, args=("Unreal_CCTV_2", "cam2"))
t1.start()
t2.start()

while True:
    if frames["cam1"] is not None:
        cv2.imshow("Camera 1", frames["cam1"])
        
    if frames["cam2"] is not None:
        cv2.imshow("Camera 2", frames["cam2"])
        
    if cv2.waitKey(1) & 0xFF == ord('q'):
        running = False
        break

t1.join()
t2.join()
cv2.destroyAllWindows()