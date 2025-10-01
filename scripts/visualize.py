import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import os
import sys

# tkinter gui backend
matplotlib.use('TkAgg')

# Change to project root directory
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.dirname(script_dir)
os.chdir(project_root)

data = pd.read_csv('data/trajectory.csv')

# Create figure and axis
fig, ax = plt.subplots(figsize=(10, 10))
ax.set_xlim(-10, 70)
ax.set_ylim(-10, 70)
ax.set_xlabel('X Position (m)')
ax.set_ylabel('Y Position (m)')
ax.set_title('Radar Simulation - Target Trajectory')
ax.grid(True, alpha=0.3)
ax.set_aspect('equal')

# Plot elements
target_dot, = ax.plot([], [], 'ro', markersize=10, label='Target')
trail_line, = ax.plot([], [], 'r-', alpha=0.3, linewidth=1)
time_text = ax.text(0.02, 0.95, '', transform=ax.transAxes, fontsize=12)

# Initialize trail data
trail_x, trail_y = [], []

def init():
    target_dot.set_data([], [])
    trail_line.set_data([], [])
    time_text.set_text('')
    return target_dot, trail_line, time_text

def animate(frame):
    x = data['x'].iloc[frame]
    y = data['y'].iloc[frame]
    t = data['time'].iloc[frame]
    
    target_dot.set_data([x], [y])
    
    trail_x.append(x)
    trail_y.append(y)
    trail_line.set_data(trail_x, trail_y)
    
    time_text.set_text(f'Time: {t:.1f}s\nPosition: ({x:.1f}, {y:.1f})')
    
    # --- Dynamic axis update ---
    
    # Set a buffer for the axis limits
    x_buffer = (max(trail_x) - min(trail_x)) * 0.1 + 10
    y_buffer = (max(trail_y) - min(trail_y)) * 0.1 + 10

    ax.set_xlim(min(trail_x) - x_buffer, max(trail_x) + x_buffer)
    ax.set_ylim(min(trail_y) - y_buffer, max(trail_y) + y_buffer)
    
    return target_dot, trail_line, time_text



anim = animation.FuncAnimation(fig, animate, init_func=init,
                               frames=len(data), interval=10,
                               blit=False, repeat=True)

ax.legend()

# Check if we can display interactively
if '--save' in sys.argv or os.environ.get('DISPLAY') is None:
    # Save as GIF or MP4
    print("Saving animation to data/simulation.gif...")
    anim.save('data/simulation.gif', writer='pillow', fps=10)
    print("✓ Animation saved! Open data/simulation.gif")
else:
    print(f"Animating {len(data)} frames...")
    plt.show()