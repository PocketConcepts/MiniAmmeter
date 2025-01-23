import tkinter as tk

# Initialize the 6x8 grid, each cell is initially set to 0 (off)
grid = [[0 for _ in range(6)] for _ in range(8)]

def update_grid(row, col):
    """Toggle the square at (row, col) in the grid."""
    grid[row][col] = 1 - grid[row][col]  # Flip between 0 and 1
    update_display()

def update_display():
    """Update the grid display and print the hex output."""
    for row in range(8):
        for col in range(6):
            color = "black" if grid[row][col] == 1 else "white"
            canvas.itemconfig(squares[row][col], fill=color)

    # Flip the grid vertically (reverse the rows) before processing
    flipped_grid = grid[::-1]

    # Convert flipped grid to hex values (rotate it to 8x6)
    hex_values = []
    for col in range(6):  # Loop over columns for the rotated grid
        byte = 0
        for row in range(8):  # Loop over rows (now columns after rotation)
            byte |= (flipped_grid[row][col] << (7 - row))  # Shift and set bit
        hex_values.append(f"0x{byte:02X}")

    # Print the final hex values with the flipped grid
    print(", ".join(hex_values))

# Set up the main window
root = tk.Tk()
root.title("6x8 Grid")

# Create the canvas to draw the grid
canvas = tk.Canvas(root, width=180, height=240)
canvas.pack()

# Create the 6x8 grid of squares (6 columns, 8 rows)
square_size = 30  # Size of each square in pixels
squares = [[None for _ in range(6)] for _ in range(8)]
for row in range(8):
    for col in range(6):
        squares[row][col] = canvas.create_rectangle(
            col * square_size, row * square_size,
            (col + 1) * square_size, (row + 1) * square_size,
            fill="white", outline="gray"
        )
        canvas.tag_bind(squares[row][col], "<Button-1>", lambda event, r=row, c=col: update_grid(r, c))

# Run the Tkinter event loop
root.mainloop()
