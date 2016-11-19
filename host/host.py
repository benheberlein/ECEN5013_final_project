import serial
import glob
import sys
import threading
import _thread
import queue
import os
import subprocess
import re
import colorama
import atexit
import time
import signal

# Serial port Baud rate
BAUD_RATE = 115200

# Serial port
ser = serial.Serial()

# Serial Rx queue
q_serial = queue.Queue()

# User Input
q_input = queue.Queue()

# st-util process
p = None 

# gdb process
d = None

# Processing threads
tr = None   # User input thread
tp = None   # st-util process thread
ts = None   # Serial thread
td = None   # gdb debug thread

def print_error(string):
    print(colorama.Fore.RED + colorama.Style.BRIGHT + string + colorama.Style.RESET_ALL)

def print_warning(string):
    print(colorama.Fore.MAGENTA + colorama.Style.BRIGHT + string + colorama.Style.RESET_ALL)

def print_info(string):
    print(colorama.Fore.BLUE + colorama.Style.BRIGHT + string + colorama.Style.RESET_ALL)

def print_prompt(string):
    print(colorama.Fore.GREEN + colorama.Style.BRIGHT + string + colorama.Style.RESET_ALL)

def print_welcome():
    print(colorama.Fore.YELLOW + colorama.Style.BRIGHT)
    print("Welcome to the Solens Debug Interface")
    print("-------------------------------------")
    print("        ........ ...                   ")
    print("       ...DI....DD...                  ")
    print("        ..?DDDDDDDDD.                  ")
    print("       .....DD8O...8.                  ")
    print("     .,D... .....=D8D.D$  ..DD=.       ")
    print("     DDN...  .D8DDD~DDD8....Z8D.       ")
    print("    .D8D...  .DDDDDDDDDD....D8DD       ")
    print("     DDDD..  .DZDDD8,D+.D...DDDD       ")
    print("     DDD8D.  .ODDO.~DDDD8..D~DDD       ")
    print("     .D$8DD,=.IDDDDD8DDDDZ.D8DD:       ")
    print("     .8DDDDD?.8DDDDDD,D~D8.DDDD=       ")
    print("     ..DDDDDDD,D$DDDDDDDD..~DD+.       ")
    print("      ..$7DDDD.DDDDD88DDDDD.8D..       ")
    print("       ....,8D$DDD8DD8NDD888:...       ")
    print("          ...DIDDDDDD.. .8DD=          ")
    print("          .....DDD8.+.  .8,D8. .       ")
    print("             .DDDDDDD,..O8DDDO..       ")
    print("          ...8DDDDDD8.DDD:DDDD..       ")
    print("          . .D8...$DDDDDD8D+....       ")
    print("          ..DDDDDDDDDDDDDDDDDI..       ")
    print("          .?DDDDDO:DDDDDDD?O.7..       ")
    print("          ...DDDDDD8..DDDDD$..         ")
    print("          . .DDDDD....DDDDD...         ")
    print("          ..8DD8...  ..DDD.  .         ")
    print("          .... .                       ")
    print("            .. .                       ")
    print("-------------------------------------")
    print(colorama.Style.RESET_ALL)

def init():
    # Create UART serial interface thread
    #global ts
    #ts = threading.Thread(target=proc_read_serial)
    #ts.daemon = True
    #ts.start()

    # Create user input thread
    global tr
    tr = threading.Thread(target=proc_read_input)
    tr.daemon = True
    tr.start()

    print_info("Ready for commands. Type 'help' to view a list of commands.")


def open_com():

    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        print_error("Unsupported platform")

    com_list = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            com_list.append(port)
        except(OSError, serial.SerialException):
            pass

    if com_list == []:
        print_error("Could not find any serial ports.\nPlease check your permissions and try again.")
        return None
    else:
        print_prompt("Please select a COM port.")

    i =  0
    for port in com_list:
        print_prompt(str(i) + ": " + port)
        i += 1

    valid_port = False

    while not valid_port:
        try:
            usr_input = int(input())
            valid_port = True
            if usr_input >= i or usr_input < 0:
                print_warning("Not a valid input. Try again")
                valid_port = False
        except(NameError, SyntaxError, ValueError):
            print_warning("Not a valid input. Try again.")
            valid_port = False
            pass

    ser = serial.Serial(com_list[usr_input], BAUD_RATE)
    print_info("Serial port " + ser.name + " opened at " + str(BAUD_RATE) + " Baud.")

    print_info("Creating STLINK-V2 connection.")
    global tp
    tp = threading.Thread(target=proc_stlink)
    tp.daemon = True
    tp.start()

    # Wait for STLINK output
    time.sleep(2)

    print_info("Created STLINK-V2 thread.")

    return ser

def proc_debug():
    global d
    binaries = os.listdir("bin")
    found_file = False
    for f in binaries:
        if f.lower().endswith(".elf"):
            found_file = True
            d = subprocess.Popen(['gnome-terminal', '-x', 'arm-none-eabi-gdb', "bin/" + f, "-x", "config/.gdbinit"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            print_info("Starting arm-none-eabi-gdb process using " + f + " file.")
            break
    
    if found_file == False:
        print_warning("Could not find a valid .elf file.\nTry rebuilding the binaries.")
        print_warning("Cancelling debug request.")

def proc_stlink():
    global p
    p = subprocess.Popen(['st-util'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    print_info("Starting st-util process with pid " + str(p.pid) + ".") 
    atexit.register(lambda: [p.kill(), p.wait()])

    while True:
        if p.stdout.readable():
            line = p.stdout.readline()
            if not line: continue
            line = "STLINK-V2: " + line.decode("utf-8").strip("\n")
            if re.search("ERROR", line):
                print_error(line)
            elif re.search("WARN", line):
                print_warning(line)
            else:
                print_info(line)
            if re.search("Address already in use", line):
                print_error("Could not open STLINK-V2 connection. There seems to be an existing instance of st-util. Try killing the process.")
                _thread.interrupt_main()
            elif re.search("Couldn't find any ST-Link", line):
                print_error("Could not open STLINK-V2 connection. Is the board plugged in?")
                _thread.interrupt_main()

#def proc_read_serial():
    #q_serial.put(ser.read())

def proc_read_input():
    while True:
        i = None
        try:
            i = input()
        except(NameError, SyntaxError, ValueError):
            print_warning("Not a valid input. Try again.")
            i = None
            pass
        if not i == None:
            q_input.put(i)

#######################################
# Input command section
def cmd_parse_input(cmd):
    cmd = cmd.lower() 
    if cmd == "help":
        cmd_help()
    elif cmd == "stlink-restart":
        cmd_stlink_restart()
    elif cmd == "debug-start":
        cmd_debug_start()
    else:
        print_warning("Invalid command. Type 'help' to view a list of commands")

def cmd_help():
    print_info("The following commands are available:\n" + 
          "\thelp:\t\tdisplay the help text\n" + 
          "\tstlink-restart:\trestart the STLINK connection\n" + 
          "\tdebug-start:\tstart a debug session")

def cmd_stlink_restart():
    global p
    global tp
    p.kill()
    p.wait()
    tp = threading.Thread(target=proc_stlink)
    tp.daemon = True
    tp.start()

def cmd_debug_start():
    global td
    td = threading.Thread(target=proc_debug)
    td.daemon = True
    td.start()


#######################################

#######################################
# Serial logger section
def serial_byte_handler(data):
    print(data)

#######################################

def main():
    print_welcome()
    ser = open_com()
    if ser == None:
        return
    init()
    
    # Main processing loop
    while True:
    #    if not q_serial.empty():
    #        print("serial byte received")
        if not q_input.empty():
            cmd_parse_input(q_input.get())

if __name__ == "__main__":
    try:
        main()
    except(KeyboardInterrupt):
        try:
            sys.exit(0)
        except(SystemExit):
            os._exit(0)
