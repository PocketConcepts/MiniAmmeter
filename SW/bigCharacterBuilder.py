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

    # Convert flipped grid to hex values
    hex_values = []
    for col in range(columnCount):  # Loop over columns for the rotated grid
        for byte_start in range(0, rowCount, 8):  # Process 8 rows (1 byte) at a time
            byte = 0
            for bit in range(8):  # Only 8 rows per byte
                row = byte_start + bit
                if row < rowCount:  # Ensure within bounds
                    byte |= (flipped_grid[row][col] << (7 - bit))  # Shift and set bit
            hex_values.append(f"0x{byte:02X}")

    # Print the final hex values with the flipped grid
    print(", ".join(hex_values))

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
