
# Run this on the Pbridge server to listen for lighting commands over UDP

import socket
import sys
import lumiversepython as L
import time

rig = L.Rig("/home/teacher/Lumiverse/PBridge.rig.json")
rig.init()
rig.run()
deviceList = []

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the port
server_address = ('REPLACE WITH SERVER IP ADDRESS', 65000)
print >>sys.stderr, 'starting up on %s port %s' % server_address
sock.bind(server_address)

def init():

	# cache all of the device sets
	for x in xrange(188):
		top = rig.select("[$sequence=%d][$side=top]"%x)
		bottom = rig.select("[$sequence=%d][$side=bot]"%x)
		sides = [bottom,top]
		deviceList.append(sides);


def allOff():
	# using cached device sets
	for device in deviceList:
		device[0].setColorRGBRaw("color",0,0,0);
		device[1].setColorRGBRaw("color",0,0,0);


def run():
	init()
	allOff()
	while True:

		# how many bytes to listen for?
		data, address = sock.recvfrom(4500)

		if data:
			# break up arbitrary chunks of commands into individual ones
			# format will be sequence(int 1-188) | side(int 0 for bottom, 1 for top) | r(float) | g(float) | b(float) , next... 
			commands = [x.split('|') for x in data.split(',')]
			if(len(commands) > 0):
				for command in commands:
					if len(command) == 5:
						try:
							sequence = int(command[0]);
							side = int(command[1]);
							r = float(command[2]);
							g = float(command[3]);
							b = float(command[4]);
							# ok, make it so
							deviceList[sequence][side].setColorRGBRaw("color",r,g,b);
						except Exception as inst:
							print "\nskipping command ", type(inst)


run()

