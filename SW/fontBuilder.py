from PIL import Image, ImageDraw, ImageFont
import tkinter as tk
from tkinter import filedialog, simpledialog

def load_font_and_character():
    """Load a TTF font file and character from the user."""
    # Open a file dialog to select a TTF file
    font_path = filedialog.askopenfilename(
        title="Select a TTF font file",
        filetypes=[("TrueType Font Files", "*.ttf")]
    )
    if not font_path:
        print("No font file selected. Exiting...")
        return None, None
    
    # Ask for a character to display
    character = simpledialog.askstring("Input Character", "Enter a character to display:")
    if not character:
        print("No character entered. Exiting...")
        return None, None
    
    return font_path, character

def get_font_for_height(font_path, character, target_height=24):
    """Adjust font size to ensure character is exactly target_height tall."""
    font_size = 1  # Start with a very small font size
    while True:
        font = ImageFont.truetype(font_path, size=font_size)
        # Create an image to measure the text size
        img = Image.new("L", (target_height, target_height), "white")
        draw = ImageDraw.Draw(img)
        bbox = draw.textbbox((0, 0), character, font=font)
        text_height = bbox[3] - bbox[1]
        
        # If text height is close enough to the target height, stop
        if text_height >= target_height:
            break
        font_size += 1
    
    return font

def generate_grid(font_path, character, target_height=24, grid_size=(24, 32)):
    """Generate a grid showing the character rendered from the font."""
    # Get the correct font size
    font = get_font_for_height(font_path, character, target_height)

    # Create an image with a white background to render the character
    img_size = (grid_size[0], grid_size[1])  # 24x32 grid size
    img = Image.new("L", img_size, "white")  # "L" for grayscale
    draw = ImageDraw.Draw(img)

    # Calculate the size of the text using textbbox
    bbox = draw.textbbox((0, 0), character, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]

    # Calculate the position to center the text both horizontally and vertically
    horizontal_offset = (grid_size[0] - text_width) // 2
    vertical_offset = (grid_size[1] - text_height) // 2

    # Center the character in the 24x32 grid (both horizontally and vertically)
    position = (horizontal_offset, vertical_offset) # Magic centering number
    draw.text(position, character, fill="black", font=font)

    # Convert image to binary (black and white pixels)
    img = img.point(lambda p: p > 128 and 255)  # Thresholding to black and white

    # Return the grid as a 2D list of pixel values
    return [[1 if img.getpixel((x, y)) == 0 else 0 for x in range(grid_size[0])] for y in range(grid_size[1])]

def display_grid(grid):
    """Display the grid using Tkinter."""
    row_count = len(grid)
    col_count = len(grid[0])

    root = tk.Tk()
    root.title("Character Grid Display")
    
    canvas = tk.Canvas(root, width=col_count * 20, height=row_count * 20)
    canvas.pack()

    for row in range(row_count):
        for col in range(col_count):
            color = "black" if grid[row][col] == 1 else "white"
            canvas.create_rectangle(
                col * 20, row * 20,
                (col + 1) * 20, (row + 1) * 20,
                fill=color, outline="gray"
            )

    root.mainloop()

if __name__ == "__main__":
    font_path, character = load_font_and_character()
    if font_path and character:
        grid = generate_grid(font_path, character, target_height=12, grid_size=(24, 32))
        display_grid(grid)
