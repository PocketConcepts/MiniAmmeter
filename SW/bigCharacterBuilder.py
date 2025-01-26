import tkinter as tk

rowCount = 32
columnCount = 24

# Initialize the 6x8 grid, each cell is initially set to 0 (off)
grid = [[0 for _ in range(columnCount)] for _ in range(rowCount)]

def update_grid(row, col):
    """Toggle the square at (row, col) in the grid."""
    grid[row][col] = 1 - grid[row][col]  # Flip between 0 and 1
    update_display()

def update_display():
    """Update the grid display and print the hex output."""
    for row in range(rowCount):
        for col in range(columnCount):
            color = "black" if grid[row][col] == 1 else "white"
            canvas.itemconfig(squares[row][col], fill=color)

    # Flip the grid vertically (reverse the rows) before processing
    flipped_grid = grid[::-1]

    # Convert flipped grid to hex values in correct order (row-major)
    hex_values = [[] for _ in range(4)]  # 4 pages of 8 rows each
    for col in range(columnCount):  # Process each column
        for page in range(4):  # Each page corresponds to 8 rows
            byte = 0
            for bit in range(8):  # Process 8 rows per byte
                row = page * 8 + bit
                if row < rowCount:  # Ensure we're within bounds
                    byte |= (flipped_grid[row][col] << (7 - bit))
            hex_values[page].append(f"0x{byte:02X}")

    # Print the hex values in row-major order
    for page in range(3, -1, -1):  # Loop through 4 pages
        print(", ".join(hex_values[page]))  # Print 24 bytes per row
    print("")  # Blank line after the full page

# Set up the main window
root = tk.Tk()
root.title("Font Grid")

square_size = 30

# Create the canvas to draw the grid
canvas = tk.Canvas(root, width=columnCount * square_size, height=rowCount * square_size)
canvas.pack()

# Create the grid
squares = [[None for _ in range(columnCount)] for _ in range(rowCount)]
for row in range(rowCount):
    for col in range(columnCount):
        squares[row][col] = canvas.create_rectangle(
            col * square_size, row * square_size,
            (col + 1) * square_size, (row + 1) * square_size,
            fill="white", outline="gray"
        )
        canvas.tag_bind(squares[row][col], "<Button-1>", lambda event, r=row, c=col: update_grid(r, c))

# Run the Tkinter event loop
root.mainloop()
