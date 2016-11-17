import serial
import glob
import sys
import threading
import queue
import os
import subprocess
import re
import colorama

# Serial port Baud rate
BAUD_RATE = 115200

# Serial port
ser = serial.Serial()

# Serial Rx queue
q_serial = queue.Queue()

# User Input
q_input = queue.Queue()

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

    valid_port = False;

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
    p = subprocess.Popen(['st-util', '&'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print_info("Starting st-util process with pid " + str(p.pid) + ".")
    stdout, stderr = p.communicate()

    stdout = stdout.decode("utf-8").split("\n")
    stderr = stderr.decode("utf-8").split("\n")

    for line in stdout:
        if not line: break
        print_info(line)
    for line in stderr:
        if not line: break
        print_warning(line)
        if re.search("Address already in use", line):
            print_error("Could not open STLINK-V2 connection. There seems to be an existing instance of st-util. Try killing the process.")
            return None

    return ser

#def read_serial():
    #q_serial.put(ser.read())

def read_input():
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
def parse_input_cmd(cmd):
    cmd = cmd.lower() 
    if cmd == "help":
        help_cmd()
    else:
        print_warning("Invalid command. Type 'help' to view a list of commands")

def help_cmd():
    print_info("The following commands are available:\n" + 
          "\thelp: display the help text\n" + 
          "\t")

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
    
    #ts = threading.Thread(target=read_serial)
    #ts.start()

    tr = threading.Thread(target=read_input)
    tr.start()

    print_info("Ready for commands. Type 'help' to view a list of commands.")

    # Main processing loop
    while True:
    #    if not q_serial.empty():
    #        print("serial byte received")
        if not q_input.empty():
            parse_input_cmd(q_input.get())

if __name__ == "__main__":
    main()
