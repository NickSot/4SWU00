import serial
import matplotlib.pyplot as plt
from matplotlib import patheffects
from matplotlib.animation import FuncAnimation

# initialize serial port
# ser = serial.Serial('COM', 115200)

# initialize room occupancy data
occupancy_data = {'Room1': 10, 'Room2': 60, 'Room3': 60, 'Room4': 80}
red = '#EB5E55'
yellow = '#FFBF00'
green = '#9BE564'
dark_green = '#0E6D43'
dark_red = '#5A150C'
dark_yellow = '#523D00'

# # Function to update occupancy data from serial port
# def update_occupancy():
#     global occupancy_data
#     if ser.in_waiting > 0:
#         line = ser.readline().decode('utf-8').rstrip()
#         # Parse the incoming data and update occupancy_data
#         data = line.split(',')
#         for room_data in data:
#             room, occupancy = room_data.split(':')
#             occupancy_data[room] = int(occupancy)

# function to draw the museum map
def draw_map(ax):
    # define the room colors based on occupancy rate
    colors = {
        'Room1': green if occupancy_data['Room1'] <= 50 else yellow if occupancy_data['Room1'] in range(50,75) else red,
        'Room2': green if occupancy_data['Room2'] <= 50 else yellow if occupancy_data['Room2'] in range(50,75) else red,
        'Room3': green if occupancy_data['Room3'] <= 50 else yellow if occupancy_data['Room3'] in range(50,75) else red,
        'Room4': green if occupancy_data['Room4'] <= 50 else yellow if occupancy_data['Room4'] in range(50,75) else red,
    }
    
    # draw rooms 
    ax.clear()
    ax.add_patch(plt.Rectangle((0.1, 0.1), 0.3, 0.3, edgecolor='black', linewidth=1, facecolor=colors['Room1'], label='Room1'))
    ax.add_patch(plt.Rectangle((0.4, 0.1), 0.2, 0.3, edgecolor='black',linewidth=1, facecolor=colors['Room2'], label='Room2'))
    ax.add_patch(plt.Rectangle((0.6, 0.1), 0.2, 0.3, edgecolor='black',linewidth=1, facecolor=colors['Room3'], label='Room3'))
    ax.add_patch(plt.Rectangle((0.3, 0.4), 0.4, 0.3,edgecolor='black',linewidth=1, facecolor=colors['Room4'], label='Room4'))    

    # add room label
    for patch in ax.patches:
        x, y = patch.get_xy()  
        width, height = patch.get_width(), patch.get_height()  
        label_text = patch.get_label()  
        ax.text(x + width / 2, y + height / 2, label_text, ha='center', va='center', color='white', path_effects=[patheffects.withStroke(linewidth=2, foreground="black")])



    # draw doors
    ax.add_patch(plt.Rectangle((0.45, 0.39), 0.1, 0.02,  color='black', linewidth=0))
    ax.add_patch(plt.Rectangle((0.395, 0.2), 0.015, 0.1, color='black', linewidth=0))
    ax.add_patch(plt.Rectangle((0.595, 0.2), 0.015, 0.1, color='black', linewidth=0))
    
    # add room occupancy 
    for room, occupancy in occupancy_data.items():
        x, y = {'Room1': 0.1, 'Room2': 0.4, 'Room3': 0.6, 'Room4': 0.3}[room], {'Room1': 0.1, 'Room2': 0.1, 'Room3': 0.1, 'Room4': 0.4}[room]
        ax.text(x, y, f'{occupancy}%', ha='left', va='bottom', color='black')

    ax.set_xlim(0, 1)
    ax.set_ylim(0, 1)
    ax.axis('off')


def main():
    fig, ax = plt.subplots()
    plt.gcf().canvas.manager.set_window_title('Museum Map')
    ani = FuncAnimation(fig, lambda x: draw_map(ax), interval=1000)
    plt.show()

if __name__ == '__main__':
    main()
