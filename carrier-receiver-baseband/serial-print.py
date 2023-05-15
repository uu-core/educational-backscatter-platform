import serial
from os import name
from datetime import datetime

log = True
time = datetime.now()
logfile = f'./received_{time.year:04}-{time.month:02}-{time.day:02}_{time.hour:02}-{time.minute:02}-{time.second:02}.txt'

# print available ports
if name == 'nt':  # sys.platform == 'win32':
    from serial.tools.list_ports_windows import comports
elif name == 'posix':
    from serial.tools.list_ports_posix import comports
else:
    print('Sorry, your platform is not supported.')

# ask which to use
if len(comports()) == 0:
    print('Sorry, no serial ports are available.')
else:
    print('The available serial ports are:')
    for p in comports():
        print(f'- {p}')
    port = input('\nWhich port would you like to use? ')
    if (port in [str(p).split(' ')[0] for p in comports()]):
        print(f'Starting to read from {port}...')
        with serial.Serial(port, 115200)  as ser:
            if log:
                with open(logfile,'a',newline='\n') as openfile:
                    while True:
                        rec = ser.read().decode("utf-8")
                        print(rec,end='')
                        openfile.write(rec)
            else:
                while True:
                    print(ser.read().decode("utf-8"),end='')
    else:
        print('Sorry, the provided ports was not part of the list.')
