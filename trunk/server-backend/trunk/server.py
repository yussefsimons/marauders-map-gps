#!/bin/python
import socket, MySQLdb, shelve, threading, socket, shutil, time, datetime
import config

RecvBuffer = 80

## Validate config file
config_response = config.configure()
if config_response != "Error":
    print "*** Config read OK ***"
    conf = shelve.open(".config")
    for i,v in config_response.iteritems():
        conf[i] = v
    conf.close()
else:
    sys.exit()

## Make the config dictionary
config = shelve.open(".config")

dbTable = config["MySQL_table"]
ListenerHost = config["ServerIP"]
ListenerPort = int(config["ServerPort"])

###############################################################################################
###############################################################################################
## Classes

## Listener Thread
class Listener(threading.Thread):
    def __init__(self):
        self.running = True
	self.connection = False
        threading.Thread.__init__(self)
 
    def stop(self):
        self.running = False
	## The following is a HACK to make the thread stop when it is told to
	client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	client_socket.connect((ListenerHost, ListenerPort))
	client_socket.send("QUITTING")
	client_socket.close()

    def setConnection(self, con):
	self.connection = con
 
    def run(self):
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    	server_socket.bind((ListenerHost, ListenerPort))
    	server_socket.listen(5)
    	print "-> Listening for client connection... "

    	while self.running:
	    client_socket, address = server_socket.accept()
            data = client_socket.recv(RecvBuffer)
	    print "-> Recieved Client Data: " , data
	    client_socket.close()
	    try:
	    	#self.connection.send_raw(data)
	    	tid,lat,lon,alt,speed,ttime = data.split("|")
		print "\nGot Tracker (ID: " + str(tid) + ") Data:"
		print " - Latitude: " + lat
		print " - Longitude: " + lon
		print " - Altitude: " + alt
		print " - Speed: " + speed	
		print " - Time: " + ttime
		t = datetime.date.today()
		now = datetime.datetime(t.year, t.month, t.day, int(ttime[0:2]), int(ttime[2:4]), int(ttime[4:6]), 0)
		#lat = float(lat)
		#lon = float(lon)
		#alt = float(alt)
		#speed = float(speed)
		SQL = mysql()
		SQL.execute("INSERT INTO " + dbTable + " (id, lat, lon, alt, speed, time) VALUES (" + tid + "," + lat + "," + lon + "," + alt + "," + speed + ",'" + now.strftime('%Y-%m-%d %H:%M:%S') + "')")
		SQL.close();
	    except socket.error, e:
		print "*** Socket Error: %d: %s ***" % (e.args[0], e.args[1])

	server_socket.close()

## MySQL Class
class mysql:
    def __init__(self):
	self.running = True;
	try:
    	    self.sql = MySQLdb.connect(host = config["MySQL_host"], user = config["MySQL_user"], passwd = config["MySQL_passwd"], db = config["MySQL_db"], reconnect = 1)
	except MySQLdb.Error, e:
	    self.running = True;
    	    print "*** MySQL Error %d: %s ***" % (e.args[0], e.args[1])
    	    print "*** FATAL: quitting ***"
    	    sys.exit(1)
    
    def execute(self, query):
	self.cursor = self.sql.cursor()
	self.cursor.execute(query)
	self.open = True
	
    def rowcount(self):
	if self.open:
	    return self.cursor.rowcount
	else:
	    print "*** rowcount: No DB cursor is open! ***"
	    return 0

    def fetchone(self):
	if self.open:
	    return self.cursor.fetchone()
	else:
	    print "*** fetchone: No DB cursor is open! ***"
	    return 0

    def fetchall(self):
	if self.open:
	    return self.cursor.fetchall()
	else:
	    print "*** fetchall: No DB cursor is open! ***"
	    return 0

    def closecursor(self):
	if self.open:
	    self.sql.commit()
	    return self.cursor.close()
	else:
	    print "*** close: No DB cursor is open! ***"
	    return 0

    def close(self):
	if self.open:
	    self.sql.commit()
	    return self.cursor.close()
	else:
	    print "*** close: No DB cursor is open! ***"
	    return 0

###############################################################################################
###############################################################################################
## Functions

def dbVal(s): # Protect against MySQL injection attacks
    s = ''.join([ c for c in s if c not in ('\'','\x1a','\r','\n','\"','\x00', '\\')])
    return s

threadListener = Listener() # Define the listener thread
conf = shelve.open(".config")
try:
    threadListener.start()
    while True: time.sleep(100)
except (KeyboardInterrupt, SystemExit): # Wait for a keyboard interupt
    print "\n*** Received keyboard interrupt, quitting threads ***"
    threadListener.stop() # Stop the thread
    sys.exit(0)


