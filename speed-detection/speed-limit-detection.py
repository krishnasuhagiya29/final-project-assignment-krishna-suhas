import pytesseract
from PIL import Image

# Read the image using PIL
img = Image.open('60.png')

# Convert the image to grayscale
gray = img.convert('L')

# Use Tesseract OCR to extract text from the image
original = pytesseract.image_to_string(gray, config='-l eng --oem 3 --psm 12')

# Filter out non-numeric characters
required = '0123456789'
final_numbers = ''.join(filter(lambda x: x in required, original))

print(final_numbers)