import pytesseract
from PIL import Image
import subprocess
import time

# Function to extract data from an image using Tesseract OCR
def extract_data_from_image(image_path):
    img = Image.open(image_path)
    gray = img.convert('L')
    original = pytesseract.image_to_string(gray, config='-l eng --oem 3 --psm 12')
    required = '0123456789'
    final_numbers = ''.join(filter(lambda x: x in required, original))
    return final_numbers

# List of images
image_paths = ['board_10.png', 'board_55.png', 'board_60.png', 'board_80.png']

# Extract data from each image and send it to the server one by one
for path in image_paths:
    # Extract data from the current image
    extracted_data = extract_data_from_image(path)
    
    # Pass the extracted data as an argument to the server application
    server_process = subprocess.Popen(['./server', extracted_data])
    
    # Wait for 10 seconds before processing the next image
    time.sleep(10)
    
    # Wait for the server process to finish
    server_process.wait()
