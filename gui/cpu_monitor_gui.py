'''
Gui for our CPU monitor.

Authors:
    Andrew Keesler
    Alden Hart

5/22/2015
'''

import Tkinter as tk

class Monitor(tk.Frame):

    def __init__(self, master=None):
        tk.Frame.__init__(self, master)
        self.grid()
        self.create_widgets()

    def create_widgets(self):
        self.quit_button = tk.Button(self, text='Quit', command=self.quit)
        self.quit_button.grid()

app = Monitor()
app.master.title("CPU Monitor")
app.mainloop()
